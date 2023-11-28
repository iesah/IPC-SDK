
#ifndef __XB_SND_I2S_H__
#define __XB_SND_I2S_H__

#include <asm/io.h>
#include <linux/delay.h>
#include "../interface/xb_snd_dsp.h"
#include "../xb_snd_detect.h"

extern unsigned int DEFAULT_REPLAY_ROUTE;
extern unsigned int DEFAULT_RECORD_ROUTE;

/**
 * global variable
 **/


/**
 *	codec mode
 **/

enum codec_mode {
	CODEC_MASTER,
	CODEC_SLAVE,
};
extern void volatile __iomem *volatile i2s_iomem;

struct i2s_device {
	int i2s_irq;
	spinlock_t i2s_irq_lock;
	spinlock_t i2s_lock;
	char name[20];

	struct resource *res;
	struct clk * i2s_clk;
	struct clk * aic_clk;
	void __iomem * i2s_iomem;
	bool i2s_is_incall_state;

#ifdef CONFIG_JZ_INTERNAL_CODEC_V12
	struct workqueue_struct *i2s_work_queue;
	struct work_struct  i2s_codec_work;
#endif

	struct workqueue_struct *i2s_work_queue_1;
	struct work_struct i2s_work;
	unsigned int ioctl_cmd;
	unsigned long ioctl_arg;

	struct dsp_endpoints *i2s_endpoints;
	struct codec_info *cur_codec;
};


struct codec_info {
	struct list_head list;
	struct i2s_device * codec_parent;
	/*char *name;*/
	char name[20];
	unsigned long record_rate;
	unsigned long replay_rate;
	int record_codec_channel;
	int replay_codec_channel;
	int record_format;
	int replay_format;
	enum codec_mode codec_mode;
	unsigned long codec_clk;
	int (*codec_ctl)(unsigned int cmd, unsigned long arg);
	int (*codec_ctl_2)(struct codec_info *codec_dev, unsigned int cmd, unsigned long arg);
	struct dsp_endpoints *dsp_endpoints;
};

#define NEED_RECONF_DMA         0x00000001
#define NEED_RECONF_TRIGGER     0x00000002
#define NEED_RECONF_FILTER      0x00000004

/**
 * registers
 **/
#define AICFR 0x0
#define AICCR 0x4
#define AICCR1 0x8
#define AICCR2 0xc
#define I2SCR 0x10
#define AICSR 0x14
#define A0CSR  0x18
#define I2SSR 0x1c
#define A0CCAR 0x20
#define A0CCDR 0x24
#define A0CSAR 0x28
#define A0CSDR 0x2c
#define I2SDIV 0x30
#define AICDR 0x34
#define CKCFG  0xa0
#define RGADW  0xa4
#define RGDATA 0xa8

static unsigned long read_val;
static unsigned long tmp_val;

/**
 * i2s register control
 **/
#define i2s_write_reg(i2s_dev, addr,val)        \
	writel(val,i2s_dev->i2s_iomem+addr)

#define i2s_read_reg(i2s_dev, addr)             \
	readl(i2s_dev->i2s_iomem+addr)

#define i2s_set_reg(i2s_dev, addr,val,mask,offset)\
	do {										\
		unsigned long flags;					\
		spin_lock_irqsave(&i2s_dev->i2s_lock, flags);		\
		tmp_val = val;							\
		read_val = i2s_read_reg(i2s_dev, addr);         \
		read_val &= (~mask);                    \
		tmp_val = ((tmp_val << offset) & mask); \
		tmp_val |= read_val;                    \
		i2s_write_reg(i2s_dev, addr,tmp_val);           \
		spin_unlock_irqrestore(&i2s_dev->i2s_lock, flags);			\
	} while(0)

#define i2s_get_reg(i2s_dev, addr,mask,offset)  \
	((i2s_read_reg(i2s_dev, addr) & mask) >> offset)

#define i2s_clear_reg(i2s_dev, addr,mask)       \
	i2s_write_reg(i2s_dev, addr,~mask)

/* AICFR */
#define I2S_ENB_OFFSET         (0)
#define I2S_ENB_MASK           (0x1 << I2S_ENB_OFFSET)
#define I2S_SYNCD_OFFSET       (1)
#define I2S_SYNCD_MASK         (0x1 << I2S_SYNCD_OFFSET)
#define I2S_BCKD_OFFSET        (2)
#define I2S_BCKD_MASK          (0x1 << I2S_BCKD_OFFSET)
#define I2S_RST_OFFSET         (3)
#define I2S_RST_MASK           (0X1 << I2S_RST_OFFSET)
#define I2S_AUSEL_OFFSET       (4)
#define I2S_AUSEL_MASK         (0x1 << I2S_AUSEL_OFFSET)
#define I2S_ICDC_OFFSET        (5)
#define I2S_ICDC_MASK          (0x1 << I2S_ICDC_OFFSET)
#define I2S_LSMP_OFFSET        (6)
#define I2S_LSMP_MASK          (0x1 << I2S_LSMP_OFFSET)
#define I2S_ICS_OFFSET         (7)
#define I2S_ICS_MASK           (0x1 << I2S_ICS_OFFSET)
#define I2S_DMODE_OFFSET       (8)
#define I2S_DMODE_MASK         (0x1 << I2S_DMODE_OFFSET)
#define I2S_ISYNCD_OFFSET      (9)
#define I2S_ISYNCD_MASK        (0x1 << I2S_ISYNCD_OFFSET)
#define I2S_IBCKD_OFFSET       (10)
#define I2S_IBCKD_MASK         (0x1 << I2S_IBCKD_OFFSET)
#define I2S_MSB_OFFSET         (12)
#define I2S_MSB_MASK           (0x1 << I2S_MSB_OFFSET)
#define I2S_TFTH_OFFSET        (16)
#define I2S_TFTH_MASK          (0x1f << I2S_TFTH_OFFSET)
#define I2S_RFTH_OFFSET        (24)
#define I2S_RFTH_MASK          (0xf << I2S_RFTH_OFFSET)

#define __aic_select_i2s(i2s_dev)             \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_AUSEL_MASK,I2S_AUSEL_OFFSET)
#define __aic_select_aclink(i2s_dev)          \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_AUSEL_MASK,I2S_AUSEL_OFFSET)
#define __i2s_set_transmit_trigger(i2s_dev, n)  \
	i2s_set_reg(i2s_dev, AICFR,n,I2S_TFTH_MASK,I2S_TFTH_OFFSET)
#define __i2s_set_receive_trigger(i2s_dev, n)   \
	i2s_set_reg(i2s_dev, AICFR,n,I2S_RFTH_MASK,I2S_RFTH_OFFSET)
#define __i2s_internal_codec_master(i2s_dev)              \
do {	\
	i2s_set_reg(i2s_dev, AICFR,1,I2S_ICS_MASK,I2S_ICS_OFFSET);	\
	i2s_set_reg(i2s_dev, AICFR,1,I2S_ICDC_MASK,I2S_ICDC_OFFSET);	\
} while (0)
#define __i2s_internal_codec_slave(i2s_dev)              \
do {	\
	i2s_set_reg(i2s_dev, AICFR,1,I2S_ICS_MASK,I2S_ICS_OFFSET);	\
	i2s_set_reg(i2s_dev, AICFR,0,I2S_ICDC_MASK,I2S_ICDC_OFFSET);	\
} while (0)
#define __i2s_external_codec(i2s_dev)               \
do {	\
	i2s_set_reg(i2s_dev, AICFR,0,I2S_ICS_MASK,I2S_ICS_OFFSET);	\
	i2s_set_reg(i2s_dev, AICFR,0,I2S_ICDC_MASK,I2S_ICDC_OFFSET);	\
} while(0)
#define __i2s_select_share_clk(i2s_dev)		\
do {	\
	i2s_set_reg(i2s_dev, AICFR,0,I2S_DMODE_MASK,I2S_DMODE_OFFSET);\
} while (0)
#define __i2s_select_spilt_clk(i2s_dev)		\
do {	\
	i2s_set_reg(i2s_dev, AICFR,1,I2S_DMODE_MASK,I2S_DMODE_OFFSET);\
} while (0)

#define __i2s_bclk_input(i2s_dev)             \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_BCKD_MASK,I2S_BCKD_OFFSET)
#define __i2s_bclk_output(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_BCKD_MASK,I2S_BCKD_OFFSET)
#define __i2s_sync_input(i2s_dev)             \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_SYNCD_MASK,I2S_SYNCD_OFFSET)
#define __i2s_sync_output(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_SYNCD_MASK,I2S_SYNCD_OFFSET)
#define __i2s_ibclk_input(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_IBCKD_MASK,I2S_IBCKD_OFFSET)
#define __i2s_ibclk_output(i2s_dev)           \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_IBCKD_MASK,I2S_IBCKD_OFFSET)
#define __i2s_isync_input(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_ISYNCD_MASK,I2S_ISYNCD_OFFSET)
#define __i2s_enable_24bitmsb(i2s_dev)			\
	i2s_set_reg(i2s_dev, AICFR,1,I2S_MSB_MASK,I2S_MSB_OFFSET)
#define __i2s_disable_24bitmsb(i2s_dev)			\
	i2s_set_reg(i2s_dev, AICFR,0,I2S_MSB_MASK,I2S_MSB_OFFSET)
#define __i2s_isync_output(i2s_dev)           \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_ISYNCD_MASK,I2S_ISYNCD_OFFSET)

#define __i2s_slave_clkset(i2s_dev)        \
	do {                                            \
		__i2s_bclk_input(i2s_dev);                    \
		__i2s_sync_input(i2s_dev);                    \
		__i2s_isync_input(i2s_dev);                   \
		__i2s_ibclk_input(i2s_dev);                   \
	}while(0)

#define __i2s_master_clkset(i2s_dev)        \
	do {                                            \
		__i2s_bclk_output(i2s_dev);                    \
		__i2s_sync_output(i2s_dev);                    \
	}while(0)
		/*__i2s_isync_output(i2s_dev);                   \
		__i2s_ibclk_output(i2s_dev);                   \*/


#define __i2s_play_zero(i2s_dev)              \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_LSMP_MASK,I2S_LSMP_OFFSET)
#define __i2s_play_lastsample(i2s_dev)        \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_LSMP_MASK,I2S_LSMP_OFFSET)

#define __i2s_reset(i2s_dev)                  \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_RST_MASK,I2S_RST_OFFSET)

#define __i2s_enable(i2s_dev)                 \
	i2s_set_reg(i2s_dev, AICFR,1,I2S_ENB_MASK,I2S_ENB_OFFSET)
#define __i2s_disable(i2s_dev)                \
	i2s_set_reg(i2s_dev, AICFR,0,I2S_ENB_MASK,I2S_ENB_OFFSET)

/* AICCR */
#define I2S_EREC_OFFSET        (0)
#define I2S_EREC_MASK          (0x1 << I2S_EREC_OFFSET)
#define I2S_ERPL_OFFSET        (1)
#define I2S_ERPL_MASK          (0x1 << I2S_ERPL_OFFSET)
#define I2S_ENLBF_OFFSET       (2)
#define I2S_ENLBF_MASK         (0x1 << I2S_ENLBF_OFFSET)
#define I2S_ETFS_OFFSET        (3)
#define I2S_ETFS_MASK          (0X1 << I2S_ETFS_OFFSET)
#define I2S_ERFS_OFFSET        (4)
#define I2S_ERFS_MASK          (0x1 << I2S_ERFS_OFFSET)
#define I2S_ETUR_OFFSET        (5)
#define I2S_ETUR_MASK          (0x1 << I2S_ETUR_OFFSET)
#define I2S_EROR_OFFSET        (6)
#define I2S_EROR_MASK          (0x1 << I2S_EROR_OFFSET)
#define I2S_RFLUSH_OFFSET      (7)
#define I2S_RFLUSH_MASK        (0x1 << I2S_RFLUSH_OFFSET)
#define I2S_TFLUSH_OFFSET      (8)
#define I2S_TFLUSH_MASK        (0x1 << I2S_TFLUSH_OFFSET)
#define I2S_ASVTSU_OFFSET      (9)
#define I2S_ASVTSU_MASK        (0x1 << I2S_ASVTSU_OFFSET)
#define I2S_ENDSW_OFFSET       (10)
#define I2S_ENDSW_MASK         (0x1 << I2S_ENDSW_OFFSET)
#define I2S_M2S_OFFSET         (11)
#define I2S_M2S_MASK           (0x1 << I2S_M2S_OFFSET)
#define I2S_TDMS_OFFSET        (14)
#define I2S_TDMS_MASK          (0x1 << I2S_TDMS_OFFSET)
#define I2S_RDMS_OFFSET        (15)
#define I2S_RDMS_MASK          (0x1 << I2S_RDMS_OFFSET)
#define I2S_ISS_OFFSET         (16)
#define I2S_ISS_MASK           (0x7 << I2S_ISS_OFFSET)
#define I2S_OSS_OFFSET         (19)
#define I2S_OSS_MASK           (0x7 << I2S_OSS_OFFSET)
#define I2S_CHANNEL_OFFSET     (24)
#define I2S_CHANNEL_MASK       (0x7 << I2S_CHANNEL_OFFSET)
#define I2S_PACK16_OFFSET      (28)
#define I2S_PACK16_MASK        (0x1 << I2S_PACK16_OFFSET)


#define __i2s_enable_pack16(i2s_dev)          \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_PACK16_MASK,I2S_PACK16_OFFSET)
#define __i2s_disable_pack16(i2s_dev)         \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_PACK16_MASK,I2S_PACK16_OFFSET)
#define __i2s_out_channel_select(i2s_dev, n)    \
	i2s_set_reg(i2s_dev, AICCR,n,I2S_CHANNEL_MASK,I2S_CHANNEL_OFFSET)
#define __i2s_set_oss_sample_size(i2s_dev, n)   \
	i2s_set_reg(i2s_dev, AICCR,n,I2S_OSS_MASK,I2S_OSS_OFFSET)
#define __i2s_set_iss_sample_size(i2s_dev, n)   \
	i2s_set_reg(i2s_dev, AICCR,n,I2S_ISS_MASK,I2S_ISS_OFFSET)

#define __i2s_enable_transmit_dma(i2s_dev)    \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_TDMS_MASK,I2S_TDMS_OFFSET)
#define __i2s_disable_transmit_dma(i2s_dev)   \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_TDMS_MASK,I2S_TDMS_OFFSET)
#define __i2s_enable_receive_dma(i2s_dev)     \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_RDMS_MASK,I2S_RDMS_OFFSET)
#define __i2s_disable_receive_dma(i2s_dev)    \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_RDMS_MASK,I2S_RDMS_OFFSET)

#define __i2s_enable_mono2stereo(i2s_dev)     \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_M2S_MASK,I2S_M2S_OFFSET)
#define __i2s_disable_mono2stereo(i2s_dev)    \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_M2S_MASK,I2S_M2S_OFFSET)

#define __i2s_enable_byteswap(i2s_dev)        \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ENDSW_MASK,I2S_ENDSW_OFFSET)
#define __i2s_disable_byteswap(i2s_dev)       \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ENDSW_MASK,I2S_ENDSW_OFFSET)

#define __i2s_enable_signadj(i2s_dev)       \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ASVTSU_MASK,I2S_ASVTSU_OFFSET)
#define __i2s_disable_signadj(i2s_dev)      \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ASVTSU_MASK,I2S_ASVTSU_OFFSET)

#define __i2s_flush_tfifo(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_TFLUSH_MASK,I2S_TFLUSH_OFFSET)
#define __i2s_flush_rfifo(i2s_dev)            \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_RFLUSH_MASK,I2S_RFLUSH_OFFSET)
#define __i2s_test_flush_tfifo(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICCR,I2S_TFLUSH_MASK,I2S_TFLUSH_OFFSET)

#define __i2s_enable_overrun_intr(i2s_dev)    \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_EROR_MASK,I2S_EROR_OFFSET)
#define __i2s_disable_overrun_intr(i2s_dev)   \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_EROR_MASK,I2S_EROR_OFFSET)

#define __i2s_enable_underrun_intr(i2s_dev)   \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ETUR_MASK,I2S_ETUR_OFFSET)
#define __i2s_disable_underrun_intr(i2s_dev)  \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ETUR_MASK,I2S_ETUR_OFFSET)

#define __i2s_enable_transmit_intr(i2s_dev)   \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ETFS_MASK,I2S_ETFS_OFFSET)
#define __i2s_disable_transmit_intr(i2s_dev)  \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ETFS_MASK,I2S_ETFS_OFFSET)

#define __i2s_enable_receive_intr(i2s_dev)    \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ERFS_MASK,I2S_ERFS_OFFSET)
#define __i2s_disable_receive_intr(i2s_dev)   \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ERFS_MASK,I2S_ERFS_OFFSET)

#define __i2s_enable_loopback(i2s_dev)        \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ENLBF_MASK,I2S_ENLBF_OFFSET)
#define __i2s_disable_loopback(i2s_dev)       \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ENLBF_MASK,I2S_ENLBF_OFFSET)

#define __i2s_enable_replay(i2s_dev)          \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_ERPL_MASK,I2S_ERPL_OFFSET)
#define __i2s_disable_replay(i2s_dev)         \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_ERPL_MASK,I2S_ERPL_OFFSET)

#define __i2s_enable_record(i2s_dev)          \
	i2s_set_reg(i2s_dev, AICCR,1,I2S_EREC_MASK,I2S_EREC_OFFSET)
#define __i2s_disable_record(i2s_dev)         \
	i2s_set_reg(i2s_dev, AICCR,0,I2S_EREC_MASK,I2S_EREC_OFFSET)

/* I2SCR */
#define I2S_AMSL_OFFSET        (0)
#define I2S_AMSL_MASK          (0x1 << I2S_AMSL_OFFSET)
#define I2S_ESCLK_OFFSET       (4)
#define I2S_ESCLK_MASK         (0x1 << I2S_ESCLK_OFFSET)
#define I2S_STPBK_OFFSET       (12)
#define I2S_STPBK_MASK         (0x1 << I2S_STPBK_OFFSET)
#define I2S_ISTPBK_OFFSET      (13)
#define I2S_ISTPBK_MASK        (0X1 << I2S_ISTPBK_OFFSET)
#define I2S_SWLH_OFFSET        (16)
#define I2S_SWLH_MASK          (0x1 << I2S_SWLH_OFFSET)
#define I2S_RFIRST_OFFSET      (17)
#define I2S_RFIRST_MASK        (0x1 << I2S_RFIRST_OFFSET)

#define __i2s_send_rfirst(i2s_dev)            \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_RFIRST_MASK,I2S_RFIRST_OFFSET)
#define __i2s_send_lfirst(i2s_dev)            \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_RFIRST_MASK,I2S_RFIRST_OFFSET)

#define __i2s_switch_lr(i2s_dev)              \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_SWLH_MASK,I2S_SWLH_OFFSET)
#define __i2s_unswitch_lr(i2s_dev)            \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_SWLH_MASK,I2S_SWLH_OFFSET)

#define __i2s_stop_bitclk(i2s_dev)            \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_STPBK_MASK,I2S_STPBK_OFFSET)
#define __i2s_start_bitclk(i2s_dev)           \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_STPBK_MASK,I2S_STPBK_OFFSET)

#define __i2s_stop_ibitclk(i2s_dev)           \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_ISTPBK_MASK,I2S_ISTPBK_OFFSET)
#define __i2s_start_ibitclk(i2s_dev)          \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_ISTPBK_MASK,I2S_ISTPBK_OFFSET)

#define __i2s_enable_sysclk_output(i2s_dev)   \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_ESCLK_MASK,I2S_ESCLK_OFFSET)
#define __i2s_disable_sysclk_output(i2s_dev)  \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_ESCLK_MASK,I2S_ESCLK_OFFSET)
#define __i2s_select_i2s(i2s_dev)             \
	i2s_set_reg(i2s_dev, I2SCR,0,I2S_AMSL_MASK,I2S_AMSL_OFFSET)
#define __i2s_select_msbjustified(i2s_dev)    \
	i2s_set_reg(i2s_dev, I2SCR,1,I2S_AMSL_MASK,I2S_AMSL_OFFSET)


/* AICSR*/
#define I2S_TFS_OFFSET         (3)
#define I2S_TFS_MASK           (0x1 << I2S_TFS_OFFSET)
#define I2S_RFS_OFFSET         (4)
#define I2S_RFS_MASK           (0x1 << I2S_RFS_OFFSET)
#define I2S_TUR_OFFSET         (5)
#define I2S_TUR_MASK           (0x1 << I2S_TUR_OFFSET)
#define I2S_ROR_OFFSET         (6)
#define I2S_ROR_MASK           (0X1 << I2S_ROR_OFFSET)
#define I2S_TFL_OFFSET         (8)
#define I2S_TFL_MASK           (0x3f << I2S_TFL_OFFSET)
#define I2S_RFL_OFFSET         (24)
#define I2S_RFL_MASK           (0x3f << I2S_RFL_OFFSET)

#define __i2s_clear_tur(i2s_dev)	\
	i2s_set_reg(i2s_dev, AICSR,0,I2S_TUR_MASK,I2S_TUR_OFFSET)
#define __i2s_test_tur(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_TUR_MASK,I2S_TUR_OFFSET)
#define __i2s_clear_ror(i2s_dev)	\
	i2s_set_reg(i2s_dev, AICSR,0,I2S_ROR_MASK,I2S_ROR_OFFSET)
#define __i2s_test_ror(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_ROR_MASK,I2S_ROR_OFFSET)
#define __i2s_test_tfs(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_TFS_MASK,I2S_TFS_OFFSET)
#define __i2s_test_rfs(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_RFS_MASK,I2S_RFS_OFFSET)
#define __i2s_test_tfl(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_TFL_MASK,I2S_TFL_OFFSET)
#define __i2s_test_rfl(i2s_dev)               \
	i2s_get_reg(i2s_dev, AICSR,I2S_RFL_MASK,I2S_RFL_OFFSET)
/* I2SSR */
#define I2S_BSY_OFFSET         (2)
#define I2S_BSY_MASK           (0x1 << I2S_BSY_OFFSET)
#define I2S_RBSY_OFFSET        (3)
#define I2S_RBSY_MASK          (0x1 << I2S_RBSY_OFFSET)
#define I2S_TBSY_OFFSET        (4)
#define I2S_TBSY_MASK          (0x1 << I2S_TBSY_OFFSET)
#define I2S_CHBSY_OFFSET       (5)
#define I2S_CHBSY_MASK         (0X1 << I2S_CHBSY_OFFSET)

#define __i2s_is_busy(i2s_dev)                \
	i2s_get_reg(i2s_dev, I2SSR,I2S_BSY_MASK,I2S_BSY_OFFSET)
#define __i2s_rx_is_busy(i2s_dev)             \
	i2s_get_reg(i2s_dev, I2SSR,I2S_RBSY_MASK,I2S_RBSY_OFFSET)
#define __i2s_tx_is_busy(i2s_dev)             \
	i2s_get_reg(i2s_dev, I2SSR,I2S_TBSY_MASK,I2S_TBSY_OFFSET)
#define __i2s_channel_is_busy(i2s_dev)        \
	i2s_get_reg(i2s_dev, I2SSR,I2S_CHBSY_MASK,I2S_CHBSY_OFFSET)
/* AICDR */
#define I2S_DATA_OFFSET        (0)
#define I2S_DATA_MASK          (0xffffff << I2S_DATA_OFFSET)

#define __i2s_write_tfifo(i2s_dev, v)           \
	i2s_set_reg(i2s_dev, AICDR,v,I2S_DATA_MASK,I2S_DATA_OFFSET)
#define __i2s_read_rfifo(i2s_dev)             \
	i2s_get_reg(i2s_dev, AICDR,I2S_DATA_MASK,I2S_DATA_OFFSET)

/* I2SDIV */
#define I2S_DV_OFFSET          (0)
#define I2S_DV_MASK            (0xf << I2S_DV_OFFSET)
#define I2S_IDV_OFFSET         (8)
#define I2S_IDV_MASK           (0xf << I2S_IDV_OFFSET)

static inline unsigned long  __i2s_set_sample_rate(struct i2s_device *i2s_dev, unsigned long sys_clk, unsigned long sync)
{
	/*int div = sys_clk/(64*sync) - 1;*/
	int div = sys_clk/(64*sync);
//	div = 3;
	i2s_set_reg(i2s_dev, I2SDIV,div,I2S_DV_MASK,I2S_DV_OFFSET);

	return sys_clk/(64*(div + 1));
}

static inline unsigned long  __i2s_set_isample_rate(struct i2s_device *i2s_dev, unsigned long sys_clk, unsigned long sync)
{
	int div = sys_clk/(64*sync) - 1;
	i2s_set_reg(i2s_dev, I2SDIV,div,I2S_IDV_MASK,I2S_IDV_OFFSET);

	return sys_clk/(64*(div + 1));
}
/*
 * CKCFG
 */
/*useless*/

/*
 * RGADW
 */
#define I2S_RGDIN_OFFSET       (0)
#define I2S_RGDIN_MASK         (0xff << I2S_RGDIN_OFFSET)
#define I2S_RGADDR_OFFSET      (8)
#define I2S_RGADDR_MASK        (0x7f << I2S_RGADDR_OFFSET)
#define I2S_RGWR_OFFSET        (16)
#define I2S_RGWR_MASK          (0x1  << I2S_RGWR_OFFSET)

#define test_rw_inval(i2s_dev)         \
	i2s_get_reg(i2s_dev, RGADW,I2S_RGWR_MASK,I2S_RGWR_OFFSET)
/*
 * RGDATA
 */
#define I2S_RGDOUT_OFFSET      (0)
#define I2S_RGDOUT_MASK        (0xff << I2S_RGDOUT_OFFSET)
#define I2S_IRQ_OFFSET         (8)
#define I2S_IRQ_MASK           (0x1  << I2S_IRQ_OFFSET)
#define I2S_RINVAL_OFFSET      (31)
#define I2S_RINVAL_MASK        (0x1  << I2S_RINVAL_OFFSET)

static int inline read_inter_codec_reg(struct i2s_device *i2s_dev, int addr)
{
	int reval;
	while(test_rw_inval(i2s_dev));
	i2s_write_reg(i2s_dev, RGADW,((addr << I2S_RGADDR_OFFSET) & I2S_RGADDR_MASK ));

	reval = i2s_read_reg(i2s_dev, RGDATA);
	reval = i2s_read_reg(i2s_dev, RGDATA);
	reval = i2s_read_reg(i2s_dev, RGDATA);
	reval = i2s_read_reg(i2s_dev, RGDATA);
	reval = i2s_read_reg(i2s_dev, RGDATA);
	reval = i2s_read_reg(i2s_dev, RGDATA);

	return reval & I2S_RGDOUT_MASK;
}

static int inline write_inter_codec_reg(struct i2s_device *i2s_dev, int addr,int data)
{
	while(test_rw_inval(i2s_dev));
	i2s_write_reg(i2s_dev, RGADW,(((addr << I2S_RGADDR_OFFSET) & I2S_RGADDR_MASK) |
				(((data)<< I2S_RGDIN_OFFSET)& I2S_RGDIN_MASK)));
	i2s_write_reg(i2s_dev,  RGADW,(((addr << I2S_RGADDR_OFFSET) & I2S_RGADDR_MASK) |
			(((data)<< I2S_RGDIN_OFFSET)& I2S_RGDIN_MASK) |
			(1 << I2S_RGWR_OFFSET)));
	if (data != read_inter_codec_reg(i2s_dev, addr))
		return -1;
	return 0;
}

static int inline read_inter_codec_irq(struct i2s_device *i2s_dev)
{
	int val = i2s_read_reg(i2s_dev, RGDATA);
	return (val& I2S_IRQ_MASK);
}


static void inline write_inter_codec_reg_bit(struct i2s_device *i2s_dev, int addr,int bitval,int offset)
{
	int val_tmp;
	val_tmp = read_inter_codec_reg(i2s_dev, addr);
	if (bitval)
		val_tmp |= (1 << offset);
	else
		val_tmp &= ~(1 << offset);

	write_inter_codec_reg(i2s_dev, addr,val_tmp);
}

static void inline write_inter_codec_reg_mask(struct i2s_device *i2s_dev, int addr,int val, int mask,int offset)
{
	int val_tmp;
	val_tmp = read_inter_codec_reg(i2s_dev, addr);
	write_inter_codec_reg(i2s_dev, addr,((val_tmp&(~mask)) | ((val << offset) & mask)));
}

/**
 * default parameter
 **/
#define DEF_REPLAY_FMT			16
#define DEF_REPLAY_CHANNELS		2
#define DEF_REPLAY_RATE			44100

#define DEF_RECORD_FMT			16
#define DEF_RECORD_CHANNELS		2
#define DEF_RECORD_RATE			44100

#define CODEC_RMODE                     0x1
#define CODEC_WMODE                     0x2
#define CODEC_RWMODE                    0x3


/**
 * i2s codec control cmd
 **/
enum codec_ioctl_cmd_t {
	CODEC_INIT,
	CODEC_TURN_ON,
	CODEC_TURN_OFF,
	CODEC_SHUTDOWN,
	CODEC_RESET,
	CODEC_SUSPEND,
	CODEC_RESUME,
	CODEC_ANTI_POP,
	CODEC_SET_DEFROUTE,
	CODEC_SET_DEVICE,
	CODEC_SET_RECORD_RATE,
	CODEC_SET_RECORD_DATA_WIDTH,
	CODEC_SET_MIC_VOLUME,
	CODEC_SET_RECORD_VOLUME,
	CODEC_SET_RECORD_CHANNEL,
	CODEC_SET_REPLAY_RATE,
	CODEC_SET_REPLAY_DATA_WIDTH,
	CODEC_SET_REPLAY_VOLUME,
	CODEC_SET_CALL_REPLAY_VOLUME,
	CODEC_SET_REPLAY_CHANNEL,
	CODEC_DAC_MUTE,
	CODEC_ADC_MUTE,
	CODEC_DEBUG_ROUTINE,
	CODEC_SET_STANDBY,
	CODEC_GET_RECORD_FMT_CAP,
	CODEC_GET_RECORD_FMT,
	CODEC_GET_REPLAY_FMT_CAP,
	CODEC_GET_REPLAY_FMT,
	CODEC_IRQ_HANDLE,
	CODEC_GET_HP_STATE,
	CODEC_DUMP_REG,
	CODEC_DUMP_GPIO,
	CODEC_CLR_ROUTE,	//just use for phone pretest
	CODEC_DEBUG,
};
/**
 *	i2s switch state
 **/
void *jz_set_hp_detect_type(int type,struct snd_board_gpio *hp_det,
		struct snd_board_gpio *mic_det,
		struct snd_board_gpio *mic_detect_en,
		struct snd_board_gpio *mic_select,
		int  hook_active_level);


/*void i2s_replay_zero_for_flush_codec(void);*/


#if 0
#if defined(CONFIG_JZ_INTERNAL_CODEC)
extern void codec_irq_set_mask(struct codec_info *codec_dev);
#endif
#endif



#endif /* _XB_SND_I2S_H_ */
