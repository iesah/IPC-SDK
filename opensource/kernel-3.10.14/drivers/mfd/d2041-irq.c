/*
 * d2041-irq.c: IRQ support for Dialog D2041
 *
 * Copyright(c) 2011 Dialog Semiconductor Ltd.
 *
 * Author: Dialog Semiconductor Ltd. D. Chen, D. Patel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bug.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/irq.h>
#include <linux/kthread.h>

//#include <asm/mach/irq.h>
#include <asm/gpio.h>

#include <linux/d2041/d2041_reg.h>
#include <linux/d2041/d2041_core.h>
#include <linux/d2041/d2041_pmic.h>
#include <linux/d2041/d2041_rtc.h>


#define D2041_NUM_IRQ_EVT_REGS		4

#define D2041_INT_OFFSET_1		0
#define D2041_INT_OFFSET_2		1
#define D2041_INT_OFFSET_3		2
#define D2041_INT_OFFSET_4		3

struct d2041_irq_data {
	int reg;
	int mask;
};

static struct d2041_irq_data d2041_irqs[] = {
	/* EVENT Register A start */
	[D2041_IRQ_EVDDMON] = {
		.reg = D2041_INT_OFFSET_1,
		.mask = D2041_IRQMASKA_MVDDMON,
	},
	[D2041_IRQ_EALRAM] = {
		.reg = D2041_INT_OFFSET_1,
		.mask = D2041_IRQMASKA_MALRAM,
	},
	[D2041_IRQ_ESEQRDY] = {
		.reg = D2041_INT_OFFSET_1,
		.mask = D2041_IRQMASKA_MSEQRDY,
	},
	[D2041_IRQ_ETICK] = {
		.reg = D2041_INT_OFFSET_1,
		.mask = D2041_IRQMASKA_MTICK,
	},
	/* EVENT Register B start */
	[D2041_IRQ_ENONKEY_LO] = {
		.reg = D2041_INT_OFFSET_2,
		.mask = D2041_IRQMASKB_MNONKEY_LO,
	},
	[D2041_IRQ_ENONKEY_HI] = {
		.reg = D2041_INT_OFFSET_2,
		.mask = D2041_IRQMASKB_MNONKEY_HI,
	},
	[D2041_IRQ_ENONKEY_HOLDON] = {
		.reg = D2041_INT_OFFSET_2,
		.mask = D2041_IRQMASKB_MNONKEY_HOLDON,
	},
	[D2041_IRQ_ENONKEY_HOLDOFF] = {
		.reg = D2041_INT_OFFSET_2,
		.mask = D2041_IRQMASKB_MNONKEY_HOLDOFF,
	},
	/* EVENT Register C start */
	[D2041_IRQ_ETA] = {
		.reg = D2041_INT_OFFSET_3,
		.mask = D2041_IRQMASKC_MTA,
	},
	[D2041_IRQ_ENJIGON] = {
		.reg = D2041_INT_OFFSET_3,
		.mask = D2041_IRQMASKC_MNJIGON,
	},
	/* EVENT Register D start */
	[D2041_IRQ_EGPI0] = {
		.reg = D2041_INT_OFFSET_4,
		.mask = D2041_IRQMASKD_MGPI0,
	},
};



static void d2041_irq_call_handler(struct d2041 *d2041, int irq)
{
	mutex_lock(&d2041->irq_mutex);

	if (d2041->irq[irq].handler) {
		d2041->irq[irq].handler(irq, d2041->irq[irq].data);

	} else {
		d2041_mask_irq(d2041, irq);
	}
	mutex_unlock(&d2041->irq_mutex);
}

/*
 * This is a threaded IRQ handler so can access I2C.  Since all
 * interrupts are clear on read the IRQ line will be reasserted and
 * the physical IRQ will be handled again if another interrupt is
 * asserted while we run - in the normal course of events this is a
 * rare occurrence so we save I2C/SPI reads.
 */
void d2041_irq_worker(struct work_struct *work)
{
	struct d2041 *d2041 = container_of(work, struct d2041, irq_work);
	u8 reg_val;
	u8 sub_reg[D2041_NUM_IRQ_EVT_REGS] = {0,};
	int read_done[D2041_NUM_IRQ_EVT_REGS];
	struct d2041_irq_data *data;
	int i;

	memset(&read_done, 0, sizeof(read_done));

	for (i = 0; i < ARRAY_SIZE(d2041_irqs); i++) {
		data = &d2041_irqs[i];

		if (!read_done[data->reg]) {
			d2041_reg_read(d2041, D2041_EVENTA_REG + data->reg, &reg_val);
			sub_reg[data->reg] = reg_val;
			//      d2041_reg_read(d2041, D2041_EVENTA_REG + data->reg);
			d2041_reg_read(d2041, D2041_IRQMASKA_REG + data->reg, &reg_val);
			sub_reg[data->reg] &= ~reg_val;
			//	    ~d2041_reg_read(d2041, D2041_IRQMASKA_REG + data->reg);
			read_done[data->reg] = 1;
		}

	if (sub_reg[data->reg] & data->mask) {
		d2041_irq_call_handler(d2041, i);
			/* Now clear EVENT registers */
			d2041_set_bits(d2041, D2041_EVENTA_REG + data->reg, d2041_irqs[i].mask);
			//dev_info(d2041->dev, "\nIRQ Register [%d] MASK [%d]\n",D2041_EVENTA_REG + data->reg, d2041_irqs[i].mask);
		}
	}
	enable_irq(d2041->chip_irq);
	/* DLG Test Print */
	dev_info(d2041->dev, "IRQ Generated [d2041_irq_worker EXIT]\n");
}


#ifdef D2041_INT_USE_THREAD
/*
 * This is a threaded IRQ handler so can access I2C/SPI.  Since all
 * interrupts are clear on read the IRQ line will be reasserted and
 * the physical IRQ will be handled again if another interrupt is
 * asserted while we run - in the normal course of events this is a
 * rare occurrence so we save I2C/SPI reads.
 */
void d2041_irq_thread(void *d2041_irq_tcb)
{
	struct d2041 *d2041 = (struct d2041 *)d2041_irq_tcb;
	u8 sub_reg[D2041_NUM_IRQ_EVT_REGS];
	int read_done[D2041_NUM_IRQ_EVT_REGS];
	struct d2041_irq_data *data;
	int i;


    //set_freezable();
    printk(KERN_ERR "d2041_irq_thread start  !!!!!! \n");
    while(!kthread_should_stop())
    {
        set_current_state(TASK_INTERRUPTIBLE);
	memset(&read_done, 0, sizeof(read_done));

	for (i = 0; i < ARRAY_SIZE(d2041_irqs); i++) {
		data = &d2041_irqs[i];

		if (!read_done[data->reg]) {
			sub_reg[data->reg] =
				d2041_reg_read(d2041, D2041_EVENTA_REG + data->reg);
			sub_reg[data->reg] &=
				~d2041_reg_read(d2041, D2041_IRQMASKA_REG + data->reg);
			read_done[data->reg] = 1;
		}

		if (sub_reg[data->reg] & data->mask) {
			d2041_irq_call_handler(d2041, i);
				/* Now clear EVENT registers */
				d2041_set_bits(d2041, D2041_EVENTA_REG + data->reg, d2041_irqs[i].mask);
				//dev_info(d2041->dev, "\nIRQ Register [%d] MASK [%d]\n",D2041_EVENTA_REG + data->reg, d2041_irqs[i].mask);
			}
		}
		enable_irq(d2041->chip_irq);
	}
	/* DLG Test Print */
	//dev_info(d2041->dev, "IRQ Generated [d2041_irq_worker EXIT]\n");
}
#endif /* D2041_INT_USE_THREAD */

/* Kernel version 2.6.28 start */
static irqreturn_t d2041_irq(int irq, void *data)
{
#if 1   //DLG eric. 05/Dec/2011. Update IRQ.
	struct d2041 *d2041 = data;
    u8 reg_val;
    u8 sub_reg[D2041_NUM_IRQ_EVT_REGS] = {0,};
    int read_done[D2041_NUM_IRQ_EVT_REGS];
    struct d2041_irq_data *pIrq;
    int i;

	printk("in the d2041_irq xyfu------------------------------\n");
    memset(&read_done, 0, sizeof(read_done));

    for (i = 0; i < ARRAY_SIZE(d2041_irqs); i++) {
        pIrq = &d2041_irqs[i];

        if (!read_done[pIrq->reg]) {
            d2041_reg_read(d2041, D2041_EVENTA_REG + pIrq->reg, &reg_val);
            sub_reg[pIrq->reg] = reg_val;
            //      d2041_reg_read(d2041, D2041_EVENTA_REG + data->reg);
            d2041_reg_read(d2041, D2041_IRQMASKA_REG + pIrq->reg, &reg_val);
            sub_reg[pIrq->reg] &= ~reg_val;
            //      ~d2041_reg_read(d2041, D2041_IRQMASKA_REG + data->reg);
            read_done[pIrq->reg] = 1;
        }

        if (sub_reg[pIrq->reg] & pIrq->mask) {
            d2041_irq_call_handler(d2041, i);
            /* Now clear EVENT registers */
            d2041_set_bits(d2041, D2041_EVENTA_REG + pIrq->reg, d2041_irqs[i].mask);
            //dev_info(d2041->dev, "\nIRQ Register [%d] MASK [%d]\n",D2041_EVENTA_REG + data->reg, d2041_irqs[i].mask);
        }
    }
    //enable_irq(d2041->chip_irq);
    /* DLG Test Print */
    dev_info(d2041->dev, "IRQ Generated [d2041_irq_worker EXIT]\n");
	return IRQ_HANDLED;
#else
	struct d2041 *d2041 = data;

	schedule_work(&d2041->irq_work);
	disable_irq_nosync(irq);
	/* DLG Test Print */
	return IRQ_HANDLED;
#endif
}
/* Kernel version 2.6.28 end */


int d2041_register_irq(struct d2041 * const d2041, int const irq,
			irq_handler_t handler, unsigned long flags,
			const char * const name, void * const data)
{
	if (irq < 0 || irq >= D2041_NUM_IRQ || !handler)
		return -EINVAL;

	if (d2041->irq[irq].handler)
		return -EBUSY;
	mutex_lock(&d2041->irq_mutex);
	d2041->irq[irq].handler = handler;
	d2041->irq[irq].data = data;
	mutex_unlock(&d2041->irq_mutex);
	/* DLG Test Print */
        dev_info(d2041->dev, "\nIRQ After MUTEX UNLOCK [%s]\n", __func__);

	d2041_unmask_irq(d2041, irq);
	return 0;
}
EXPORT_SYMBOL_GPL(d2041_register_irq);

int d2041_free_irq(struct d2041 *d2041, int irq)
{
	if (irq < 0 || irq >= D2041_NUM_IRQ)
		return -EINVAL;

	d2041_mask_irq(d2041, irq);

	mutex_lock(&d2041->irq_mutex);
	d2041->irq[irq].handler = NULL;
	mutex_unlock(&d2041->irq_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(d2041_free_irq);

int d2041_mask_irq(struct d2041 *d2041, int irq)
{
	return d2041_set_bits(d2041, D2041_IRQMASKA_REG + d2041_irqs[irq].reg,
			       d2041_irqs[irq].mask);
}
EXPORT_SYMBOL_GPL(d2041_mask_irq);

int d2041_unmask_irq(struct d2041 *d2041, int irq)
{
	dev_info(d2041->dev, "\nIRQ[%d] Register [%d] MASK [%d]\n",irq, D2041_IRQMASKA_REG + d2041_irqs[irq].reg, d2041_irqs[irq].mask);
	return d2041_clear_bits(d2041, D2041_IRQMASKA_REG + d2041_irqs[irq].reg,
				 d2041_irqs[irq].mask);
}
EXPORT_SYMBOL_GPL(d2041_unmask_irq);

int d2041_irq_init(struct d2041 *d2041, int irq,
		    struct d2041_platform_data *pdata)
{
	int ret = -EINVAL;
	int reg_data, maskbit;

	if (!irq) {
	    dev_err(d2041->dev, "No IRQ configured \n");
	    return -EINVAL;
	}
	reg_data = 0xFFFFFFFF;
	d2041_block_write(d2041, D2041_EVENTA_REG, 4, (u8 *)&reg_data);
	reg_data = 0;
	d2041_block_write(d2041, D2041_EVENTA_REG, 4, (u8 *)&reg_data);

	/* Clear Mask register starting with Mask A*/
	maskbit = 0xFFFFFFFF;
	d2041_block_write(d2041, D2041_IRQMASKA_REG, 4, (u8 *)&maskbit);

	mutex_init(&d2041->irq_mutex);
#ifdef D2041_INT_USE_THREAD
	d2041->irq_task = kthread_run(d2041_irq_thread, d2041, "d2041_irq_task");
#else
	//DLG eric. 05/Dec/2011. Update IRQ. INIT_WORK(&d2041->irq_work, d2041_irq_worker);
#endif
	if (irq) {
		/* DLG Test Print */
		dev_err(d2041->dev, "\n\n############## IRQ configured [%d] ###############\n\n", irq);

		//ret = request_irq(irq, d2041_irq, IRQF_TRIGGER_FALLING,
		//		  "d2041", d2041);
		ret = request_threaded_irq(irq, NULL, d2041_irq, IRQF_DISABLED|IRQF_TRIGGER_FALLING|IRQF_NO_SUSPEND,
				  "d2041", d2041);
		if (ret != 0) {
			dev_err(d2041->dev, "Failed to request IRQ: %d\n", irq);
			return ret;
		}
	} else {
		dev_err(d2041->dev, "No IRQ configured\n");
		return ret;
	}

	d2041->chip_irq = irq;
	return ret;
}

int d2041_irq_exit(struct d2041 *d2041)
{
	free_irq(d2041->chip_irq, d2041);
	return 0;
}
