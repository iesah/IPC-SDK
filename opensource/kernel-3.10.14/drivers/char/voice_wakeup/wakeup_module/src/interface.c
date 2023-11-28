/*
 * main.c
 */
#include <common.h>
#include <jz_cpm.h>

#include "ivDefine.h"
#include "ivIvwDefine.h"
#include "ivIvwErrorCode.h"
#include "ivIVW.h"
#include "ivPlatform.h"

#include "interface.h"
#include "voice_wakeup.h"
#include "dma_ops.h"
#include "dmic_ops.h"
#include "jz_dmic.h"
#include "rtc_ops.h"
#include "dmic_config.h"
#include "jz_dma.h"
#include "tcu_timer.h"

#define TAG	"[voice_wakeup]"

char __bss_start[0] __attribute__((section(".__bss_start")));
char __bss_end[0] __attribute__((section(".__bss_end")));

int (*h_handler)(const char *fmt, ...);

enum wakeup_source {
	WAKEUP_BY_OTHERS = 1,
	WAKEUP_BY_DMIC
};

/* global config: default value */
unsigned int _dma_channel = 5;
static unsigned int tcu_channel = 5;

unsigned int cpu_wakeup_by = 0;
unsigned int open_cnt = 0;
unsigned int current_mode = 0;
struct sleep_buffer *g_sleep_buffer;
static int voice_wakeup_enabled = 0;
static int dmic_record_enabled = 0;




void dump_voice_wakeup(void)
{
	printk("###########dump voice wakeup status#######\n");
	printk("cpu_wakeup_by:			%d\n", cpu_wakeup_by);
	printk("open_cnt:				%d\n", open_cnt);
	printk("voice_wakeup_enabled:	%d\n", voice_wakeup_enabled);
	printk("dmic_record_enabled:	%d\n", dmic_record_enabled);

	printk("wakeup_failed_times:	%d\n", wakeup_failed_times);
	printk("dmic_current_state:	%d\n", dmic_current_state);

	printk("###########dump voice wakeup status#######\n");

}
int module_init(void)
{
	int i;
	/*clear bss*/
	unsigned char *p = __bss_start;
	for(i=0; i<((char*)&__bss_end - __bss_start); i++) {
		*p++ = 0;
	}

	/*global init*/
	_dma_channel = 5;
	tcu_channel = 5;
	cpu_wakeup_by = 0;
	open_cnt = 0;
	current_mode = 0;
	g_sleep_buffer = NULL;
	voice_wakeup_enabled = 0;
	dmic_record_enabled = 0;
}

int module_exit(void)
{

}
int open(int mode)
{
	switch (mode) {
		case EARLY_SLEEP:
			break;
		case DEEP_SLEEP:
			printk("deep sleep open\n");
			if(!voice_wakeup_enabled) {
				return 0;
			}
			rtc_init();
			dmic_init_mode(DEEP_SLEEP);
			wakeup_open();
#ifdef CONFIG_CPU_IDLE_SLEEP
			tcu_timer_request(tcu_channel);
#endif
			/* UNMASK INTC we used */
			REG32(0xB000100C) = 1<<0; /*dmic int en*/
			REG32(0xB000100C) = 1<<26; /*tcu1 int en*/
			REG32(0xB000102C) = 1<<0; /*rtc int en*/
			dump_voice_wakeup();
			break;
		case NORMAL_RECORD:
			dmic_init_mode(NORMAL_RECORD);
			dmic_record_enabled = 1;
			break;
		case NORMAL_WAKEUP:
			wakeup_open();
			dmic_init_mode(NORMAL_RECORD);
			break;
		default:
			printk("%s:bad open mode\n", TAG);
			break;
	}

	dma_config_normal();
	dma_start(_dma_channel);
	dmic_enable();

	open_cnt++;
	return 0;
}

static inline void flush_dcache_all()
{
	u32 addr;

	for (addr = 0x80000000; addr < 0x80000000 + CONFIG_SYS_DCACHE_SIZE; addr += CONFIG_SYS_CACHELINE_SIZE) {
		cache_op(0x01, addr); /*Index Invalid. */
	}
}

static inline void powerdown_wait(void)
{
	unsigned int opcr;
	unsigned int cpu_no;
	unsigned int lcr;

	volatile unsigned int temp;
	/* cpu enter sleep */
	lcr = REG32(CPM_IOBASE + CPM_LCR);
	lcr &= ~3;
	lcr |= 1;
	REG32(CPM_IOBASE + CPM_LCR) = lcr;

	cpu_no = get_cp0_ebase();
	opcr = REG32(CPM_IOBASE + CPM_OPCR);
	opcr &= ~(3<<25);
	opcr |= (cpu_no + 1) << 25;

	opcr |= 1 << 30;
	REG32(CPM_IOBASE + CPM_OPCR) = opcr;
	temp = REG32(CPM_IOBASE + CPM_OPCR);
	__asm__ volatile(".set mips32\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			".set mips32 \n\t");
	TCSM_PCHAR('Q');
}

static inline void sleep_wait(void)
{
	unsigned int opcr;
	unsigned int lcr;
	unsigned int temp;
	/* cpu enter sleep */
	lcr = REG32(CPM_IOBASE + CPM_LCR);
	lcr &= ~3;
	lcr |= 1;
	REG32(CPM_IOBASE + CPM_LCR) = lcr;

	opcr = REG32(CPM_IOBASE + CPM_OPCR);
	opcr &= ~(3<<25);	/* keep cpu poweron */

	opcr |= 1 << 30;	/* int mask */
	REG32(CPM_IOBASE + CPM_OPCR) = opcr;
	temp = REG32(CPM_IOBASE + CPM_OPCR);

	__asm__ volatile(".set mips32\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			".set mips32 \n\t");

	TCSM_PCHAR('s');
}

static inline void idle_wait(void)
{
	unsigned int opcr;
	unsigned int lcr;
	unsigned int temp;
	/* cpu enter idle */
	lcr = REG32(CPM_IOBASE + CPM_LCR);
	lcr &= ~3;
	REG32(CPM_IOBASE + CPM_LCR) = lcr;
	temp = REG32(CPM_IOBASE + CPM_LCR);

	__asm__ volatile(".set mips32\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			"wait\n\t"
			"nop\n\t"
			"nop\n\t"
			"nop\n\t"
			".set mips32\n\t");
}

/*	int mask that we care about.
 *	DMIC INTS: used to wakeup cpu.
 *	RTC	 INTS: used to sum wakeup failed times. and adjust thr value.
 *	TCU	 INTS: used for cpu to process data.
 * */
#define INTC0_MASK	0xfBfffffe
#define INTC1_MASK	0xfffffffe

/* desc: this function is only called when cpu is in deep sleep
 * @par: no use.
 * @return : SYS_WAKEUP_OK, SYS_WAKEUP_FAILED.
 * */
int handler(int par)
{
	volatile int ret;
	volatile unsigned int int0;
	volatile unsigned int int1;

	while(1) {

		int0 = REG32(0xb0001010);
		int1 = REG32(0xb0001030);
		if((REG32(0xb0001010) & INTC0_MASK) || (REG32(0xb0001030) & INTC1_MASK)) {
			serial_put_hex(REG32(0xb0001010));
			serial_put_hex(REG32(0xb0001030));
			cpu_wakeup_by = WAKEUP_BY_OTHERS;
			ret = SYS_WAKEUP_OK;
			break;
		}

		/* RTC interrupt pending */
		if(REG32(0xb0001030) & (1<<0)) {
			TCSM_PCHAR('R');
			TCSM_PCHAR('T');
			TCSM_PCHAR('C');
			ret = rtc_int_handler();
			if(ret == SYS_TIMER) {
				serial_put_hex(REG32(0xb0001010));
				serial_put_hex(REG32(0xb0001030));
				ret = SYS_WAKEUP_OK;
				cpu_wakeup_by = WAKEUP_BY_OTHERS;
				break;
			} else if (ret == DMIC_TIMER){

			}
		}

#ifdef CONFIG_CPU_IDLE_SLEEP
		if(REG32(0xb0001010) & (1 << 27)) {
			tcu_timer_handler();
		} else if(REG32(0xb0001010) & (1 << 26)) {
			tcu_timer_handler();
		} else if(REG32(0xb0001010) & (1 << 25)) {
			tcu_timer_handler();
		}
#endif
		ret = dmic_handler(int1);
		if(ret == SYS_WAKEUP_OK) {
			cpu_wakeup_by = WAKEUP_BY_DMIC;
			break;
		} else if(ret == SYS_NEED_DATA){
#ifdef CONFIG_CPU_IDLE_SLEEP
			idle_wait();
#else
			if(cpu_should_sleep()) {
				flush_dcache_all();
				__asm__ __volatile__("sync");
				powerdown_wait();
			} else {
				idle_wait();
			}
#endif
		} else if(ret == SYS_WAKEUP_FAILED) {
			/* deep sleep */
			flush_dcache_all();
			__asm__ __volatile__("sync");
			powerdown_wait();
		}

	}

	if(ret == SYS_WAKEUP_OK) {
		flush_dcache_all();
		rtc_exit();
#ifdef CONFIG_CPU_IDLE_SLEEP
		tcu_timer_release(tcu_channel);
#endif
	}
	return ret;
}
int close(int mode)
{
	printk("module close\n");
	/* MASK INTC*/
	if(mode == DEEP_SLEEP) {
		if(voice_wakeup_enabled) {
			REG32(0xB0001008) |= 1<< 0;
			//REG32(0xB0001008) |= 1<< 26;
			REG32(0xB0001028) |= 1<< 0;
			dmic_disable_tri();
			wakeup_close();
			dmic_ioctl(DMIC_IOCTL_SET_SAMPLERATE, 16000);
			dump_voice_wakeup();
		}
		return 0;
	} else if(mode == NORMAL_RECORD) {
		dmic_record_enabled = 0;
	}



	if((--open_cnt) == 0) {
		printk("[voice wakeup] wakeup module closed for real. \n");
		dmic_disable();
		dma_close();
	}
	return 0;
}

int set_handler(void *handler)
{
	h_handler = (int(*)(const char *fmt, ...)) handler;

	return 0;
}


/* @fn: used by recorder to get address for now.
 * @return : dma trans address.
 * */
unsigned int get_dma_address(void)
{
	return pdma_trans_addr(_dma_channel, DMA_DEV_TO_MEM);
}

/* @fn: used by recorder to change dmic config.
 *
 * */
int ioctl(int cmd, unsigned long args)
{
	return dmic_ioctl(cmd, args);
}


/* @fn: used by deep sleep procedure to load module to L2 cache.
 *
 * */
void cache_prefetch(void)
{
	int i;
	volatile unsigned int *addr = (unsigned int *)0x8ff00000;
	volatile unsigned int a;
	for(i=0; i<LOAD_SIZE/32; i++) {
		a = *(addr + i * 8);
	}
}


/* @fn: used by wakeup driver to get voice_resrouce buffer address.
 *
 * */
unsigned char * get_resource_addr(void)
{
	printk("wakeup_res addrs:%08x\n", wakeup_res);
	return (unsigned char *)wakeup_res;
}

/* used by wakeup driver.
 * @return SYS_WAKEUP_FAILED, SYS_WAKEUP_OK.
 * */
int process_data(void)
{
	return process_dma_data();
}
/* used by wakeup drvier */
int is_cpu_wakeup_by_dmic(void)
{
	printk("[voice wakeup] cpu wakeup by:%d, ----:%p\n", cpu_wakeup_by, &cpu_wakeup_by);
	return cpu_wakeup_by == WAKEUP_BY_DMIC ? 1 : 0;
}

/* used by wakeup driver when earyl sleep. */
int set_sleep_buffer(struct sleep_buffer *sleep_buffer)
{
	int i;
	g_sleep_buffer = sleep_buffer;

	dma_stop(_dma_channel);

	/* switch to early sleep */
	dma_config_early_sleep(sleep_buffer);

	dma_start(_dma_channel);

	return 0;
}

/* used by cpu eary sleep.
 * @return SYS_WAKEUP_OK, SYS_WAKEUP_FAILED.
 * */
int get_sleep_process(void)
{
	struct sleep_buffer *sleep_buffer = g_sleep_buffer;
	int i, ret = SYS_WAKEUP_FAILED;

	if(!voice_wakeup_enabled) {
		return SYS_WAKEUP_FAILED;
	}

	for(i = 0; i < sleep_buffer->nr_buffers; i++) {
		if(process_buffer_data(sleep_buffer->buffer[i], sleep_buffer->total_len/sleep_buffer->nr_buffers) == SYS_WAKEUP_OK) {
			cpu_wakeup_by = WAKEUP_BY_DMIC;
			ret = SYS_WAKEUP_OK;
			break;
		}
	}

	dma_stop(_dma_channel);
	/* switch to dmic normal config */
	dma_config_normal();

	wakeup_reset_fifo();

	dma_start(_dma_channel);
	return ret;
}

int set_dma_channel(int channel)
{
	_dma_channel = channel;
	dma_set_channel(channel);
	return 0;
}
int voice_wakeup_enable(int enable)
{
	voice_wakeup_enabled = enable;
}
int is_voice_wakeup_enabled(void)
{
	return voice_wakeup_enabled;
}
