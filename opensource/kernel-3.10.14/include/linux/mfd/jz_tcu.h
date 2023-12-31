#ifndef __JZ_TCU_H__
#define __JZ_TCU_H__

#include <soc/tcu.h>
#include <irq.h>

#define timeout	500

#define NR_TCU_CHNS TCU_NR_IRQS

#define TCU_CHN_TDFR	0x0
#define TCU_CHN_TDHR	0x4
#define TCU_CHN_TCNT	0x8
#define TCU_CHN_TCSR	0xc

#define tcu_readl(tcu,reg)			\
	__raw_readl((tcu)->iomem + TCU_##reg)
#define tcu_writel(tcu,reg,value)			\
	__raw_writel((value), (tcu)->iomem + TCU_##reg)

#define tcu_chn_readl(tcu_chn,reg)					\
	__raw_readl((tcu_chn)->tcu->iomem + (tcu_chn)->reg_base + TCU_##reg)
#define tcu_chn_writel(tcu_chn,reg,value)				\
	__raw_writel((value), (tcu_chn)->tcu->iomem + (tcu_chn)->reg_base + TCU_##reg)

enum tcu_prescale {
	TCU_PRESCALE_1,
	TCU_PRESCALE_4,
	TCU_PRESCALE_16,
	TCU_PRESCALE_64,
	TCU_PRESCALE_256,
	TCU_PRESCALE_1024
};

enum tcu_irq_type {
	NULL_IRQ_MODE,
	FULL_IRQ_MODE,
	HALF_IRQ_MODE,
	FULL_HALF_IRQ_MODE,
};

enum tcu_clksrc {
	TCU_CLKSRC_PCK = 1,
	TCU_CLKSRC_RTC = 2,
	TCU_CLKSRC_EXT = 4
};

enum tcu_mode {
	TCU_MODE_1 = 1,
	TCU_MODE_2 = 2,
};

struct jz_tcu {
	void __iomem *iomem;
	int irq;
	int irq_base;
	spinlock_t lock;
};

struct jz_tcu_chn {
	int index;			/* Channel number */
	u32 reg_base;
	enum tcu_irq_type irq_type;
	enum tcu_clksrc clk_src;
	enum tcu_mode tcu_mode;
	enum tcu_prescale prescale;

	unsigned int divi_ratio;  /*  0/1/2/3/4/5/something else------>1/4/16/64/256/1024/mask  */
	int half_num;
	int full_num;
	int init_level;

	struct jz_tcu *tcu;
};

static inline void jz_tcu_enable_counter(struct jz_tcu_chn *tcu_chn)
{
	tcu_writel(tcu_chn->tcu, TESR, 1 << tcu_chn->index);
}

static inline int jz_tcu_disable_counter(struct jz_tcu_chn *tcu_chn)
{
	int i=0;

	tcu_writel(tcu_chn->tcu, TECR, 1 << tcu_chn->index);

	if(tcu_chn->tcu_mode == TCU_MODE_2) {
		if(tcu_chn->index == 1){
			while(tcu_readl(tcu_chn->tcu, TSTR) & TSTR_BUSY1){
				i++;
				if(i>timeout) {
					printk("the reset of counter is not finished now");
					return -1;
				}
			}
		} else {
			while(tcu_readl(tcu_chn->tcu, TSTR) & TSTR_BUSY2){
				i++;
				if(i>timeout) {
					printk("the reset of counter is not finished now");
					return -1;
				}
			}
		}
	}
	return 1;
}

static inline void jz_tcu_start_counter(struct jz_tcu_chn *tcu_chn)
{
	tcu_writel(tcu_chn->tcu, TSCR, 1 << tcu_chn->index);
}

static inline void jz_tcu_stop_counter(struct jz_tcu_chn *tcu_chn)
{
	tcu_writel(tcu_chn->tcu, TSSR, 1 << tcu_chn->index);
}

static inline void jz_tcu_set_count(struct jz_tcu_chn *tcu_chn, u16 cnt)
{
	tcu_chn_writel(tcu_chn, CHN_TCNT, cnt);
}

static inline int jz_tcu_get_count(struct jz_tcu_chn *tcu_chn)
{
	int tmp;
	int i = 0;

	if (tcu_chn->tcu_mode == TCU_MODE_2){
		while(tmp == 0 && i < timeout) {
			if(tcu_chn->index == 1){
				tmp = tcu_readl(tcu_chn->tcu, TSSR) & TSTR_REAL1;
			} else {
				tmp = tcu_readl(tcu_chn->tcu, TSSR) & TSTR_REAL2;
			}
			i++;
		}
		if(tmp == 0)    return -EINVAL; /*TCU MODE 2 may not read success*/
	}
	return tcu_chn_readl(tcu_chn, CHN_TCNT);
}

static inline void jz_tcu_set_period(struct jz_tcu_chn *tcu_chn, uint16_t period)
{
	tcu_chn_writel(tcu_chn, CHN_TDFR, period);
}

static inline void jz_tcu_set_duty(struct jz_tcu_chn *tcu_chn, uint16_t duty)
{
	tcu_chn_writel(tcu_chn, CHN_TDHR, duty);
}

void jz_tcu_config_chn(struct jz_tcu_chn *tcu_chn);

#endif /* __JZ_TCU_H__ */
