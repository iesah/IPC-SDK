#ifndef __JZ4780_EFUSE_H__
#define __JZ4780_EFUSE_H__

#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/timer.h>

#define JZ_EFUCTRL		(0x0)
#define JZ_EFUCFG		(0x4)
#define JZ_EFUSTATE		(0x8)
#define JZ_EFUDATA(n)	(0xC + (n)*4)

/* EFUCTRL:	EFUSE Control Register */
/* EFUCFG:	EFUSE Configure Register */
/* EFUSTATE:	EFUSE Status Register */
#define RD_DONE			(1 << 0)
#define WR_DONE			(1 << 1)

struct jz4780_efuse_platform_data {
	unsigned int gpio_vddq_en_n;	/* supply 2.5V to VDDQ */
};

struct jz_efuse_strict {
	unsigned int min_rd_adj;
	unsigned int min_rd_adj_strobe;
	unsigned int min_wr_adj;
	unsigned int min_wr_adj_strobe;
	unsigned int max_wr_adj_strobe;
};

struct jz_efucfg_info {
	unsigned int		rd_adj;
	unsigned int		rd_strobe;
	unsigned int		wr_adj;
	unsigned int		wr_strobe;
	struct jz_efuse_strict	strict;
};

struct jz_efuse {
	struct device		*dev;
	spinlock_t		lock;
	void __iomem		*iomem;
	struct miscdevice	mdev;
	struct clk		*clk;
	int			max_program_length;
	int			use_count;
	struct timer_list	vddq_protect_timer;
	unsigned int		is_timer_on;
	unsigned int		gpio_vddq_en_n;
	struct jz_efucfg_info	efucfg_info;
};

void jz_efuse_id_read(int is_chip_id, uint32_t *buf);

#endif
