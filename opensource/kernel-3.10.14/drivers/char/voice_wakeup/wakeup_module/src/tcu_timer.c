#include "interface.h"
#include "tcu_timer.h"
#include <jz_tcu.h>
#include <jz_cpm.h>
#include <common.h>

static int tcu_channel = 2; /*default 2*/
unsigned int clk_enabled;
#ifdef CONFIG_SLEEP_DEBUG
unsigned long time = 0;
#endif

#define TIME_1S		(1000)

#define CLK_DIV         64
#define CSRDIV(x)      ({int n = 0;int d = x; while(d){ d >>= 2;n++;};(n-1) << 3;})


#define TIME_PER_COUNT	(1953)	/* 1.953ms * 1000 */


unsigned int save_tcsr;

static inline void tcu_save(void)
{
	save_tcsr = tcu_readl(CH_TCSR(tcu_channel));
}
static inline void tcu_restore(void)
{
	tcu_writel(CH_TCSR(tcu_channel), save_tcsr);
}

static void tcu_dump_reg(void)
{

	printk("TCU_TCSR:%08x\n", tcu_readl(CH_TCSR(tcu_channel)));
	printk("TCU_TCNT:%08x\n", tcu_readl(CH_TCNT(tcu_channel)));
	printk("TCU_TER:%08x\n", tcu_readl(TCU_TER));
	printk("TCU_TFR:%08x\n", tcu_readl(TCU_TFR));
	printk("TCU_TMR:%08x\n", tcu_readl(TCU_TMR));
	printk("TCU_TSR:%08x\n", tcu_readl(TCU_TSR));
	printk("TCU_TSTR:%08x\n", tcu_readl(TCU_TSTR));

}

static void tcu_dump_reg_hex(void)
{
	TCSM_PCHAR('G');
	serial_put_hex(tcu_readl(CH_TCSR(tcu_channel)));
	serial_put_hex(tcu_readl(CH_TCNT(tcu_channel)));
	serial_put_hex(tcu_readl(TCU_TER));
	serial_put_hex(tcu_readl(TCU_TFR));
	serial_put_hex(tcu_readl(TCU_TMR));
	serial_put_hex(tcu_readl(TCU_TSR));
	serial_put_hex(tcu_readl(TCU_TSTR));
	TCSM_PCHAR('H');
}

static inline void stop_timer()
{
	/* disable tcu n */
	tcu_writel(TCU_TECR,(1 << tcu_channel));
	tcu_writel(TCU_TFCR , (1 << tcu_channel));
}
static inline void start_timer()
{
	tcu_writel(TCU_TESR, (1 << tcu_channel));
}

static void reset_timer(int count)
{
	unsigned int tcsr = tcu_readl(CH_TCSR(tcu_channel));

	/* set count */
	tcu_writel(CH_TDFR(tcu_channel),count);
	tcu_writel(CH_TDHR(tcu_channel),count/2);

	tcu_writel(TCU_TMCR , (1 << tcu_channel));
	start_timer();
}


void tcu_timer_del(void)
{
	tcu_writel(TCU_TMSR , (1 << tcu_channel));
	stop_timer();
#ifdef CONFIG_SLEEP_DEBUG
	time = 0;
#endif
}


/* @fn: convert ms to count
 * @ms: input ms
 * @return: count.
 * */
unsigned long ms_to_count(unsigned long ms)
{
	unsigned long count;
	 /* one count is about 1.953 ms */
	count = (ms * 1000 + TIME_PER_COUNT - 1) / TIME_PER_COUNT;
	return count;
}

/* @fn: mod timer.
 * @timer_cnt: cnt to be written to register.
 * */
unsigned int tcu_timer_mod(unsigned long timer_cnt)
{
	int count;
	int current_count;
	count = timer_cnt;
	current_count = tcu_readl(CH_TCNT(tcu_channel));

	tcu_writel(CH_TCNT(tcu_channel), 0);

	tcu_writel(TCU_TSCR , (1 << tcu_channel));
	tcu_writel(TCU_TECR,(1 << tcu_channel));

	reset_timer(count);

#ifdef CONFIG_SLEEP_DEBUG
	if(time >= TIME_1S) {
		time = 0;
		TCSM_PCHAR('.');
	}
	time += TCU_TIMER_MS;
#endif
	return current_count;
}


/*
 * @fn: request a tcu channel.
 * @tcu_chan: channel to request.
 * */
void tcu_timer_request(int tcu_chan)
{
	tcu_channel = tcu_chan;
	REG32(CPM_IOBASE + CPM_CLKGR0) &= ~(1<<30);
	tcu_dump_reg();
	tcu_save();
	/* stop clear */
	tcu_writel(TCU_TSCR,(1 << tcu_channel));

	tcu_writel(TCU_TECR,(1 << tcu_channel));

	tcu_writel(TCU_TMSR,(1 << tcu_channel)|(1 << (tcu_channel + 16)));

	tcu_writel(TCU_TFCR, (1 << tcu_channel) | (1 << (tcu_channel + 16)));

	tcu_writel(CH_TDHR(tcu_channel), 1);
	/* Mask interrupt */

	/* RTC CLK, 32768
	 * DIV:     64.
	 * TCOUNT:  1: 1.953125ms
	 * */
	tcu_writel(CH_TCSR(tcu_channel),CSRDIV(CLK_DIV) | CSR_RTC_EN);
	tcu_dump_reg();
}

/*
 * @fn: release a tcu timer. this should close tcu channel.
 * @tcu_chan: channel to release.
 * */
void tcu_timer_release(int tcu_chan)
{
	tcu_writel(TCU_TMSR , (1 << tcu_channel));
	stop_timer();
	tcu_restore();

	REG32(CPM_IOBASE + CPM_CLKGR0) |= 1<<30;
}

/*
 * @fn: tcu timer handler, called when tcu int happens. only
 *		in cpu idle mode.
 * */

void tcu_timer_handler(void)
{
	int ctrlbit = 1 << (tcu_channel);

	if(tcu_readl(TCU_TFR) & ctrlbit) {
		/* CLEAR INT */
		tcu_writel(TCU_TFCR,ctrlbit);
		tcu_timer_mod(ms_to_count(TCU_TIMER_MS));
	}
}

