/*
 *  Copyright (C) 2013 Fighter Sun <wanmyqawdr@126.com>
 *  HW ECC-BCH support functions
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under  the terms of the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/completion.h>

#include <soc/irq.h>
#include <soc/bch.h>

/*
 * TODO: add to kbuild
 */
#define CONFIG_JZ4780_BCH_USE_PIO
#define CONFIG_JZ4780_BCH_USE_IRQ

#define DRVNAME "jz4780-bch"

#define TIMEOUT_IN_MS 1000

#define BCH_INT_DECODE_FINISH	(1 << 3)
#define BCH_INT_ENCODE_FINISH	(1 << 2)
#define BCH_INT_UNCORRECT	(1 << 1)

#define BCH_ENABLED_INT	\
	(	\
	BCH_INT_DECODE_FINISH |	\
	BCH_INT_ENCODE_FINISH |	\
	BCH_INT_UNCORRECT	\
	)

#define BCH_CLK_RATE (200 * 1000 * 1000)

#define BCH_REGS_FILE_BASE	0x134d0000

#define CNT_PARITY  28
#define CNT_ERRROPT 64

typedef struct {
	volatile u32 bhcr;
	volatile u32 bhcsr;
	volatile u32 bhccr;
	volatile u32 bhcnt;
	volatile u32 bhdr;
	volatile u32 bhpar[CNT_PARITY];
	volatile u32 bherr[CNT_ERRROPT];
	volatile u32 bhint;
	volatile u32 bhintes;
	volatile u32 bhintec;
	volatile u32 bhinte;
	volatile u32 bhto;
} regs_file_t;

/* instance a singleton bchc */
struct {
	struct clk *clk_bch;
	struct clk *clk_bch_gate;

	regs_file_t __iomem * const regs_file;
	const struct resource regs_file_mem;

	struct list_head req_list;
	spinlock_t lock;

#ifdef CONFIG_JZ4780_BCH_USE_IRQ

	struct completion req_done;

#endif

	u32 saved_reg_bhint;

	struct task_struct *kbchd_task;
	wait_queue_head_t kbchd_wait;

	struct platform_device *pdev;
} instance = {
	.regs_file = (regs_file_t *)
					CKSEG1ADDR(BCH_REGS_FILE_BASE),
	.regs_file_mem = {
		.start = BCH_REGS_FILE_BASE,
		.end = BCH_REGS_FILE_BASE + sizeof(regs_file_t) - 1,
	},

}, *bchc = &instance;

inline static void bch_select_encode(bch_request_t *req)
{
	bchc->regs_file->bhcsr = 1 << 2;
}

inline static void bch_select_ecc_level(bch_request_t *req)
{
	bchc->regs_file->bhccr = 0x7f << 4;
	bchc->regs_file->bhcsr = req->ecc_level << 4;
}

inline static void bch_select_calc_size(bch_request_t *req)
{
	bchc->regs_file->bhcnt = 0;
	bchc->regs_file->bhcnt = (req->parity_size << 16) | req->blksz;
}

inline static void bch_select_decode(bch_request_t *req)
{
	bchc->regs_file->bhccr = 1 << 2;
}

inline static void bch_clear_pending_interrupts(void)
{
	/* clear enabled interrupts */
	bchc->regs_file->bhint = BCH_ENABLED_INT;
}

inline static void bch_wait_for_encode_done(bch_request_t *req)
{
#ifdef CONFIG_JZ4780_BCH_USE_IRQ

	int ret;
	ret = wait_for_completion_timeout(&bchc->req_done,
			msecs_to_jiffies(TIMEOUT_IN_MS));

	WARN(!ret, "%s: Timeout when do request"
			" type: %d from: %s\n",
			dev_name(&bchc->pdev->dev),
			req->type, dev_name(req->dev));
	BUG_ON(!ret);

#else

	unsigned long timeo = jiffies +
			msecs_to_jiffies(TIMEOUT_IN_MS);

	do {
		if (bchc->regs_file->bhint &
				BCH_INT_ENCODE_FINISH)
			goto done;
	} while (time_before(jiffies, timeo));

	WARN(1, "%s: Timeout when do request"
			" type: %d from: %s\n",
			dev_name(&bchc->pdev->dev),
			req->type, dev_name(req->dev));
	BUG();

done:
	bchc->saved_reg_bhint = bchc->regs_file->bhint;
	bch_clear_pending_interrupts();


#endif
}

inline static void bch_wait_for_decode_done(bch_request_t *req)
{
#ifdef CONFIG_JZ4780_BCH_USE_IRQ

	int ret;
	ret = wait_for_completion_timeout(&bchc->req_done,
			msecs_to_jiffies(TIMEOUT_IN_MS));

	WARN(!ret, "%s: Timeout when do request"
			" type: %d from: %s\n",
			dev_name(&bchc->pdev->dev),
			req->type, dev_name(req->dev));
	BUG_ON(!ret);

#else

	unsigned long timeo = jiffies +
			msecs_to_jiffies(TIMEOUT_IN_MS);

	do {
		if (bchc->regs_file->bhint &
				(BCH_INT_DECODE_FINISH |
						BCH_INT_UNCORRECT))
			goto done;
	} while (time_before(jiffies, timeo));

	WARN(1, "%s: Timeout when do request"
			" type: %d from: %s\n",
			dev_name(&bchc->pdev->dev),
			req->type, dev_name(req->dev));
	BUG();

done:
	bchc->saved_reg_bhint = bchc->regs_file->bhint;
	bch_clear_pending_interrupts();

#endif
}

inline static void bch_start_new_operation(void)
{
	/* start operation */
	bchc->regs_file->bhcsr = 1 << 1;
}

#ifdef CONFIG_JZ4780_BCH_USE_PIO

inline static void write_data_by_cpu(const void *data, u32 size)
{
	int i = size / sizeof(u32);
	int j = size & 0x3;

	volatile void *dst = &bchc->regs_file->bhdr;
	const u32 *src32;
	const u8 *src8;

	src32 = (u32 *)data;
	while (i--)
		*(u32 *)dst = *src32++;

	src8 = (u8 *)src32;
	while (j--)
		*(u8 *)dst = *src8++;
}

inline static void read_err_report_by_cpu(bch_request_t *req)
{
	if (unlikely(bchc->saved_reg_bhint & BCH_INT_UNCORRECT)) {
		dev_err(&bchc->pdev->dev, "uncorrectable errors"
				" when do request from %s\n", dev_name(req->dev));
		req->errrept_word_cnt = 0;
		req->cnt_ecc_errors = 0;
		req->ret_val = BCH_RET_UNCORRECTABLE;

	} else if (bchc->saved_reg_bhint & BCH_INT_DECODE_FINISH) {
		int i;

		req->errrept_word_cnt = (bchc->saved_reg_bhint
				& (0x7f << 24)) >> 24;
		req->cnt_ecc_errors = (bchc->saved_reg_bhint
				& (0x7f << 16)) >> 16;

		for (i = 0; i < req->errrept_word_cnt; i++)
			req->errrept_data[i] = bchc->regs_file->bherr[i];

		req->ret_val = BCH_RET_OK;

	} else {
		req->errrept_word_cnt = 0;
		req->cnt_ecc_errors = 0;
		req->ret_val = BCH_RET_UNEXPECTED;
	}
}

inline static void read_parity_by_cpu(bch_request_t *req)
{
	if (likely(bchc->saved_reg_bhint & BCH_INT_ENCODE_FINISH)) {
		int i = req->parity_size / sizeof(u32);
		int j = req->parity_size & 0x3;
		u32 *ecc_data32;
		u8 *ecc_data8;
		volatile u32 *parity32;

		ecc_data32 = (u32 *)req->ecc_data;
		parity32 = bchc->regs_file->bhpar;
		while (i--)
			*ecc_data32++ = *parity32++;

		ecc_data8 = (u8 *)ecc_data32;
		switch (j) {
		case 3:
			j = *parity32;
			*ecc_data8++ = j & 0xff;
			*ecc_data8++ = (j >> 8) & 0xff;
			*ecc_data8++ = (j >> 16) & 0xff;
			break;

		case 2:
			j = *parity32;
			*ecc_data8++ = j & 0xff;
			*ecc_data8++ = (j >> 8) & 0xff;
			break;

		case 1:
			j = *parity32;
			*ecc_data8++ = j & 0xff;
			break;

		default:
			break;
		}

		req->ret_val = BCH_RET_OK;
	} else {
		req->ret_val = BCH_RET_UNEXPECTED;
	}
}

#else

/* TODO: fill them */

inline static void bch_dma_config(void)
{

}

inline static void write_data_by_dma(const void *data, u32 size)
{

}

inline static void read_err_report_by_dma(bch_request_t *req)
{

}

inline static void read_parity_by_dma(bch_request_t *req)
{

}

#endif

inline static void bch_encode(bch_request_t *req)
{
	/*
	 * step1. basic config
	 */
	bch_select_encode(req);
	bch_select_ecc_level(req);
	bch_select_calc_size(req);

	/*
	 * step2.
	 */
	bch_start_new_operation();


#ifdef CONFIG_JZ4780_BCH_USE_PIO
	/*
	 * step3. transfer raw data which to be encoded
	 */
	write_data_by_cpu(req->raw_data, req->blksz);

	/*
	 * step4.
	 */
	bch_wait_for_encode_done(req);

	/*
	 * step5. read out parity data
	 */
	read_parity_by_cpu(req);

#else

	/*
	 * TODO
	 */
	#error "TODO: implement DMA transfer"


#endif
}

inline static void bch_decode(bch_request_t *req)
{
	/*
	 * step1. basic config
	 */
	bch_select_ecc_level(req);
	bch_select_decode(req);
	bch_select_calc_size(req);

	/*
	 * step2.
	 */
	bch_start_new_operation();

#ifdef CONFIG_JZ4780_BCH_USE_PIO
	/*
	 * step3. transfer raw data which to be decoded
	 */
	write_data_by_cpu(req->raw_data, req->blksz);

	/*
	 * step4. following transfer ECC code
	 */
	write_data_by_cpu(req->ecc_data, req->parity_size);

	/*
	 * step5.
	 */
	bch_wait_for_decode_done(req);

	/*
	 * step6. read out error report data
	 */
	read_err_report_by_cpu(req);

#else

	/*
	 * TODO
	 */
	#error "TODO: implement DMA transfer"

#endif
}

inline static void bch_correct(bch_request_t *req)
{
	int i;
	int mask;
	int index;
	u16 *raw_data;

	raw_data = (u16 *)req->raw_data;
	for (i = 0; i < req->errrept_word_cnt; i++) {
		index = req->errrept_data[i] & 0xffff;
		mask = (req->errrept_data[i] & (0xffff << 16)) >> 16;
		raw_data[index] ^= mask;
	}

	req->ret_val = BCH_RET_OK;
}

inline static void bch_decode_correct(bch_request_t *req)
{
	/* start decode process */
	bch_decode(req);

	/* return if req is not correctable */
	if (req->ret_val)
		return;

	/* start correct process */
	bch_correct(req);
}

inline static void bch_pio_config(void)
{
	bchc->regs_file->bhccr = 1 << 11;
}

inline static void bch_enable(void)
{
	/* enable bchc */
	bchc->regs_file->bhcsr = 1;

	/* do not bypass decoder */
	bchc->regs_file->bhccr = 1 << 12;
}

inline static int bch_request_enqueue(bch_request_t *req)
{
	INIT_LIST_HEAD(&req->node);

	spin_lock(&bchc->lock);
	list_add_tail(&req->node, &bchc->req_list);
	wake_up(&bchc->kbchd_wait);
	spin_unlock(&bchc->lock);

	return 0;
}

inline static int bch_request_dequeue(void)
{
	bch_request_t *req;

	while (1) {
		spin_lock(&bchc->lock);
		if (list_empty(&bchc->req_list)) {
			spin_unlock(&bchc->lock);
			break;
		}

		req = list_first_entry(&bchc->req_list,
				bch_request_t, node);
		list_del(&req->node);
		spin_unlock(&bchc->lock);

		switch (req->type) {
		case BCH_REQ_ENCODE:
			bch_encode(req);
			break;

		case BCH_REQ_DECODE:
			bch_decode(req);
			break;

		case BCH_REQ_DECODE_CORRECT:
			bch_decode_correct(req);
			break;

		case BCH_REQ_CORRECT:
			bch_correct(req);
			break;

		default:
			WARN(1, "unknown request type: %d from %s\n",
					req->type, dev_name(req->dev));
			req->ret_val = BCH_RET_UNSUPPORTED;
			break;
		}

		if (req->complete)
			req->complete(req);
	}

	return 0;
}

int bch_request_submit(bch_request_t *req)
{
	if (req->blksz & 0x3 || req->blksz > 1900)
		return -EINVAL;
	else if (req->ecc_level & 0x3 || req->ecc_level > 64)
		return -EINVAL;
	else if (req->dev == NULL)
		return -ENODEV;

	req->parity_size = req->ecc_level * 14 >> 3;

	return bch_request_enqueue(req);
}
EXPORT_SYMBOL(bch_request_submit);

inline static int bch_thread(void *__unused)
{
	set_freezable();

	do {
		bch_request_dequeue();
		wait_event_freezable(bchc->kbchd_wait,
				!list_empty(&bchc->req_list) ||
				kthread_should_stop());

		/* TODO: thread exiting control */
	} while (!kthread_should_stop() || !list_empty(&bchc->req_list));

	dev_err(&bchc->pdev->dev, "kbchd exiting.\n");

	return 0;
}

inline static void bch_clk_config(void)
{
	bchc->clk_bch = clk_get(&bchc->pdev->dev, "cgu_bch");
	bchc->clk_bch_gate = clk_get(&bchc->pdev->dev, "bch");

	clk_enable(bchc->clk_bch_gate);

	/* TODO: consider variable clk rate */
	clk_set_rate(bchc->clk_bch, BCH_CLK_RATE);
	clk_enable(bchc->clk_bch);
}

inline static void bch_irq_config(void)
{
	bchc->regs_file->bhintec = 0;
	bchc->regs_file->bhint = ~(u32)0;

	/*
	 * enable de/encodeing finish
	 * and uncorrectable interrupt
	 */

#ifdef CONFIG_JZ4780_BCH_USE_IRQ
	bchc->regs_file->bhintes = BCH_ENABLED_INT;
#endif
}

#ifdef CONFIG_JZ4780_BCH_USE_IRQ

inline static irqreturn_t bch_isr(int irq, void *__unused)
{
	bchc->saved_reg_bhint = bchc->regs_file->bhint;

	/* care only en/decode finish interrupts */
	if (bchc->regs_file->bhint & BCH_ENABLED_INT) {
		complete(&bchc->req_done);
	}

	bch_clear_pending_interrupts();

	return IRQ_HANDLED;
}

#endif

inline static int bch_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;

	spin_lock_init(&bchc->lock);
	INIT_LIST_HEAD(&bchc->req_list);
	init_waitqueue_head(&bchc->kbchd_wait);

#ifdef CONFIG_JZ4780_BCH_USE_IRQ

	init_completion(&bchc->req_done);

#endif

	bchc->pdev = pdev;

	res = request_mem_region(bchc->regs_file_mem.start,
			resource_size(&bchc->regs_file_mem), DRVNAME);
	if (!res) {
		dev_err(&bchc->pdev->dev, "failed to grab regs file.\n");
		return -EBUSY;
	}

	bch_clk_config();

	bch_enable();

#ifdef CONFIG_JZ4780_BCH_USE_PIO
	bch_pio_config();
#else
	bch_dma_config();
#endif

	bch_irq_config();

	bchc->kbchd_task = kthread_run(bch_thread, NULL, "kbchd");
	if (IS_ERR(bchc->kbchd_task)) {
		ret = PTR_ERR(bchc->kbchd_task);
		dev_err(&bchc->pdev->dev, "failed to start kbchd: %d\n", ret);
		goto err_release_mem;
	}

#ifdef CONFIG_JZ4780_BCH_USE_IRQ

	ret = request_irq(IRQ_BCH, bch_isr, 0, DRVNAME, NULL);
	if (ret) {
		dev_err(&bchc->pdev->dev, "failed to request interrupt.\n");
		goto err_release_mem;
	}

#endif

	dev_info(&bchc->pdev->dev, "SoC-jz4780 HW ECC-BCH support "
			"functions initialized.\n");

	return ret;

err_release_mem:
	release_mem_region(bchc->regs_file_mem.start,
			resource_size(&bchc->regs_file_mem));

	return ret;
}

static struct platform_driver bch_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = DRVNAME,
	},
};

static struct platform_device bch_device = {
	.name = DRVNAME,
};

static int __init bch_init(void)
{
	platform_device_register(&bch_device);
	return platform_driver_probe(&bch_driver, bch_probe);
}

postcore_initcall(bch_init);

MODULE_AUTHOR("Fighter Sun <wanmyqawdr@126.com>");
MODULE_DESCRIPTION("SoC-jz4780 HW ECC-BCH support functions");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:"DRVNAME);
