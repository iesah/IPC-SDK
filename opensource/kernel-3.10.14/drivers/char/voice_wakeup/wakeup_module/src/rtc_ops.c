#include <common.h>
#include "interface.h"
#include "rtc_ops.h"
#include "rtc-jz.h"
#include "voice_wakeup.h"
#include "dmic_ops.h"
#include "trigger_value_adjust.h"

#define TRIGGER_CHANGE_TIME		60*5		/*60s * 5*/


static unsigned int jzrtc_readl(int offset)
{
	unsigned int data, timeout = 0x100000;

	do {
		data = REG32(RTC_IOBASE + offset);
	} while (REG32(RTC_IOBASE + offset) != data && timeout--);

	if (timeout <= 0)
		printk("RTC : rtc_read_reg timeout!\n");
	return data;
}

static inline void wait_write_ready()
{
	int timeout = 0x100000;

	while (!(jzrtc_readl(RTC_RTCCR) & RTCCR_WRDY) && timeout--);
	if (timeout <= 0)
		printk("RTC : %s timeout!\n",__func__);
}

static void jzrtc_writel(int offset, unsigned int value)
{
	int timeout = 0x100000;

	REG32(RTC_IOBASE + RTC_WENR) = WENR_WENPAT_WRITABLE;
	wait_write_ready();

	while (!(jzrtc_readl(RTC_WENR) & WENR_WEN) && timeout--);
	if (timeout <= 0)
		printk("RTC :  wait_writable timeout!\n");

	wait_write_ready();
	REG32(RTC_IOBASE + offset) = value;
	wait_write_ready();
}

static inline void jzrtc_clrl(int offset, unsigned int value)
{
	jzrtc_writel(offset, jzrtc_readl(offset) & ~(value));
}

static inline void jzrtc_setl(int offset, unsigned int value)
{
	jzrtc_writel(offset,jzrtc_readl(offset) | (value));
}

static void dump_rtc_regs(void)
{
	 printk("*******************************************************************\n");
	 printk("******************************jz_rtc_dump**********************\n\n");
	 printk("jz_rtc_dump-----RTC_RTCCR is --0x%08x--\n",jzrtc_readl( RTC_RTCCR));
	 printk("jz_rtc_dump-----RTC_RTCSR is --0x%08x--\n",jzrtc_readl( RTC_RTCSR));
	 printk("jz_rtc_dump-----RTC_RTCSAR is --0x%08x--\n",jzrtc_readl(RTC_RTCSAR));
	 printk("jz_rtc_dump-----RTC_RTCGR is --0x%08x--\n",jzrtc_readl( RTC_RTCGR));
	 printk("jz_rtc_dump-----RTC_HCR is --0x%08x--\n",jzrtc_readl( RTC_HCR));
	 printk("jz_rtc_dump-----RTC_HWFCR is --0x%08x--\n",jzrtc_readl( RTC_HWFCR));
	 printk("jz_rtc_dump-----RTC_HRCR is --0x%08x--\n",jzrtc_readl( RTC_HRCR));
	 printk("jz_rtc_dump-----RTC_HWCR is --0x%08x--\n",jzrtc_readl( RTC_HWCR));
	 printk("jz_rtc_dump-----RTC_HWRSR is --0x%08x--\n",jzrtc_readl(RTC_HWRSR));
	 printk("jz_rtc_dump-----RTC_HSPR is --0x%08x--\n",jzrtc_readl( RTC_HSPR));
	 printk("jz_rtc_dump-----RTC_WENR is --0x%08x--\n",jzrtc_readl( RTC_WENR));
	 printk("jz_rtc_dump-----RTC_CKPCR is --0x%08x--\n",jzrtc_readl(RTC_CKPCR));
	 printk("jz_rtc_dump-----RTC_PWRONCR is -0x%08x-\n",jzrtc_readl(RTC_PWRONCR));
	 printk("***************************jz_rtc_dump***************************\n");
	 printk("*******************************************************************\n\n");

}



struct rtc_config {
	unsigned long alarm_val;
	unsigned int alarm_enabled;
	unsigned int alarm_pending;
	unsigned int alarm_int_en;
	unsigned int systimer_configed;
};

static struct rtc_config old_config;
static struct rtc_config rtc_config;


#define ALARM_VALUE		(30) /*10s*/



static int rtc_save(void)
{
	unsigned int rtc_rcr;
	if(jzrtc_readl(RTC_RTCSAR) < jzrtc_readl(RTC_RTCSR)) {
		/* alarm value < current second, then systimer not set.*/
		//old_config.alarm_val = 0;
		old_config.systimer_configed = 0;
	} else {
		//old_config.alarm_val = jzrtc_readl(RTC_RTCSAR);
		old_config.systimer_configed = 1;
	}
	old_config.alarm_val = jzrtc_readl(RTC_RTCSAR);
	rtc_rcr = jzrtc_readl(RTC_RTCCR);
	old_config.alarm_enabled  = (rtc_rcr & RTCCR_AIE) ? 1 : 0;
	old_config.alarm_pending = (rtc_rcr & RTCCR_AF) ? 1 : 0;
	old_config.alarm_int_en = (rtc_rcr & RTCCR_AIE) ? 1 : 0;

	rtc_config.alarm_enabled = 1;
	return 0;
}
static int rtc_restore(void)
{
	jzrtc_writel(RTC_RTCSAR, old_config.alarm_val);
	return 0;
}
int rtc_set_alarm(unsigned long alarm_seconds)
{
	unsigned int temp;
	jzrtc_writel(RTC_RTCSAR, jzrtc_readl(RTC_RTCSR) + alarm_seconds);

	temp = jzrtc_readl(RTC_RTCCR);
	temp &= ~RTCCR_AF;
	temp |= RTCCR_AIE | RTCCR_AE;
	jzrtc_writel(RTC_RTCCR, temp);
	return 0;
}
int rtc_init(void)
{

	old_config.alarm_val = 0;
	rtc_save();

	rtc_set_alarm(ALARM_VALUE);

	jzrtc_setl(RTC_HWCR, 1);

	dump_rtc_regs();
	return 0;
}

int rtc_exit(void)
{
	rtc_restore();

	return 0;
}


void process_dmic_timer()
{
	reconfig_thr_value();
}


int rtc_int_handler(void)
{
	//if(((jzrtc_readl(RTC_RTCSR)+ALARM_VALUE) >= old_config.alarm_val) && (old_config.alarm_val != 0)) {
	if(((jzrtc_readl(RTC_RTCSR)+ALARM_VALUE) >= old_config.alarm_val) && (old_config.systimer_configed == 1)) {
		/* kernel alarm arrived, priority higher. wakeup OS, imediatly */
		TCSM_PCHAR('S');
		TCSM_PCHAR('Y');
		TCSM_PCHAR('S');
		TCSM_PCHAR('T');
		TCSM_PCHAR('I');
		TCSM_PCHAR('M');
		TCSM_PCHAR('E');
		TCSM_PCHAR('R');
		return SYS_TIMER;
	} else {
		/* our timer arrived. do stuffs  here*/
		TCSM_PCHAR('D');
		TCSM_PCHAR('M');
		TCSM_PCHAR('I');
		TCSM_PCHAR('C');
		TCSM_PCHAR('T');
		TCSM_PCHAR('I');
		TCSM_PCHAR('M');
		TCSM_PCHAR('E');
		TCSM_PCHAR('R');
		process_dmic_timer();
		rtc_set_alarm(ALARM_VALUE);
		return DMIC_TIMER;
	}
	return DMIC_TIMER;
}





