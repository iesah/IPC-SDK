
#ifndef __XB_SND_SPDIF_H__
#define __XB_SND_SPDIF_H__

#include <asm/io.h>
#include <linux/delay.h>
#include "../interface/xb_snd_dsp_v12.h"
#include "../xb_snd_detect_v12.h"

extern unsigned int DEFAULT_REPLAY_ROUTE;
extern unsigned int DEFAULT_RECORD_ROUTE;

/**
 * global variable
 **/
extern void volatile __iomem *volatile spdif_iomem;


#define NEED_RECONF_DMA         0x00000001
#define NEED_RECONF_TRIGGER     0x00000002
#define NEED_RECONF_FILTER      0x00000004

/**
 * registers
 **/

#define SPENA 0x0
#define SPCTRL 0X4
#define SPSTATE 0x8
#define SPCFG1 0Xc
#define SPCFG2 0X10
#define SPFIFO 0X14

/**
 * spdif register control
 **/
static unsigned long read_val;
static unsigned long tmp_val;
#define spdif_write_reg(addr,val)        \
	writel(val,spdif_iomem+addr)

#define spdif_read_reg(addr)             \
	readl(spdif_iomem+addr)

#define spdif_set_reg(addr,val,mask,offset)\
	do {										\
		tmp_val = val;							\
		read_val = spdif_read_reg(addr);         \
		read_val &= (~mask);                    \
		tmp_val = ((tmp_val << offset) & mask); \
		tmp_val |= read_val;                    \
		spdif_write_reg(addr,tmp_val);           \
	} while(0)

#define spdif_get_reg(addr,mask,offset)  \
	((spdif_read_reg(addr) & mask) >> offset)

#define spdif_clear_reg(addr,mask)       \
	spdif_write_reg(addr,~mask)

/* SPENA */
#define SPDIF_ENB_OFFSET         (0)
#define SPDIF_ENB_MASK           (0x1 << SPDIF_ENB_OFFSET)

#define __spdif_enable()                 \
	spdif_set_reg(SPENA,1,SPDIF_ENB_MASK,SPDIF_ENB_OFFSET)
#define __spdif_disable()                \
	spdif_set_reg(SPENA,0,SPDIF_ENB_MASK,SPDIF_ENB_OFFSET)


/* SPCTRL */
#define SPDIF_ETUR_OFFSET        (0)
#define SPDIF_ETUR_MASK          (0x1 << SPDIF_ETUR_OFFSET)
#define SPDIF_MTRIG_OFFSET		 (1)
#define SPDIF_MTRIG_MASK		 (0x1 << SPDIF_MTRIG_OFFSET)
#define SPDIF_SELECT_OFFSET      (10)
#define SPDIF_SELECT_MASK        (0x1 << SPDIF_SELECT_OFFSET)
#define SPDIF_RST_OFFSET         (11)
#define SPDIF_RST_MASK           (0X1 << SPDIF_RST_OFFSET)
#define SPDIF_INVALID_OFFSET	 (12)
#define SPDIF_INVALID_MASK		 (0x1 << SPDIF_INVALID_OFFSET)
#define SPDIF_SIGNN_OFFSET		 (13)
#define SPDIF_SIGNN_MASK		 (0x1 << SPDIF_SIGNN_OFFSET)
#define SPDIF_DTYPE_OFFSET		 (14)
#define SPDIF_DTYPE_MASK		 (0x1 << SPDIF_DTYPE_OFFSET)
#define SPDIF_DMAEN_OFFSET       (15)
#define SPDIF_DMAEN_MASK         (0x1 << SPDIF_DMAEN_OFFSET)

#define __spdif_enable_underrun_intr()    \
	spdif_set_reg(SPCTRL,0,SPDIF_ETUR_MASK,SPDIF_ETUR_OFFSET)
#define __spdif_disable_underrun_intr()  \
	spdif_set_reg(SPCTRL,1,SPDIF_ETUR_MASK,SPDIF_ETUR_OFFSET)

#define __spdif_mask_trig()				\
	spdif_set_reg(SPCTRL,1,SPDIF_MTRIG_MASK,SPDIF_MTRIG_OFFSET)
#define __spdif_enable_trig()			\
	spdif_set_reg(SPCTRL,0,SPDIF_MTRIG_MASK,SPDIF_MTRIG_OFFSET)

#define __interface_select_spdif()      \
	spdif_set_reg(SPCTRL,1,SPDIF_SELECT_MASK,SPDIF_SELECT_OFFSET)

#define __spdif_reset()                 \
	spdif_set_reg(SPCTRL,1,SPDIF_RST_MASK,SPDIF_RST_OFFSET)
#define __spdif_get_reset()                 \
	spdif_get_reg(SPCTRL,SPDIF_RST_MASK,SPDIF_RST_OFFSET)

#define __spdif_set_invalid()			\
	spdif_set_reg(SPCTRL,1,SPDIF_INVALID_MASK,SPDIF_INVALID_OFFSET)
#define __spdif_set_valid()				\
	spdif_set_reg(SPCTRL,0,SPDIF_INVALID_MASK,SPDIF_INVALID_OFFSET)

#define __spdif_set_signn()				\
	spdif_set_reg(SPCTRL,1,SPDIF_SIGNN_MASK,SPDIF_SIGNN_OFFSET)
#define __spdif_clear_signn()			\
	spdif_set_reg(SPCTRL,0,SPDIF_SIGNN_MASK,SPDIF_SIGNN_OFFSET)

#define __spdif_set_dtype(n)			\
	spdif_set_reg(SPCTRL,n,SPDIF_DTYPE_MASK,SPDIF_DTYPE_OFFSET)

#define __spdif_enable_transmit_dma()    \
	spdif_set_reg(SPCTRL,1,SPDIF_DMAEN_MASK,SPDIF_DMAEN_OFFSET)
#define __spdif_disable_transmit_dma()   \
	spdif_set_reg(SPCTRL,0,SPDIF_DMAEN_MASK,SPDIF_DMAEN_OFFSET)


/* SPSTATE */
#define SPDIF_TUR_OFFSET         (0)
#define SPDIF_TUR_MASK           (0x1 << SPDIF_TUR_OFFSET)
#define SPDIF_FTRIG_OFFSET		 (1)
#define SPDIF_FTRIG_MASK		 (0x1 << SPDIF_F_TRIG)
#define SPDIF_BUSY				 (7)
#define SPDIF_BUSY_MASK			 (0x1 << SPDIF_BUSY)
#define SPDIF_FIFO_LVL			 (8)
#define SPDIF_FIFO_LVL_MASK		 (0x3f << SPDIF_FIFO_LVL_MASK)

#define __spdif_clear_tur()	\
	spdif_set_reg(SPSTATE,0,SPDIF_TUR_MASK,SPDIF_TUR_OFFSET)
#define __spdif_test_tur()               \
	spdif_get_reg(SPSTATE,SPDIF_TUR_MASK,SPDIF_TUR_OFFSET)

#define __spdif_test_trig()               \
	spdif_get_reg(SPSTATE,SPDIF_FTRIG_MASK,SPDIF_F_TRIG)

#define __spdif_test_busy()               \
	spdif_get_reg(SPSTATE,SPDIF_BUSY_MASK,SPDIF_BUSY_MASK)

#define __spdif_get_fifo_lvl() \
	spdif_get_reg(SPSTATE,SPDIF_FIFO_LVL_MASK,SPDIF_FIFO_LVL)

/* SPFIFO */
#define SPDIF_DATA_OFFSET        (0)
#define SPDIF_DATA_MASK          (0xffffff << SPDIF_DATA_OFFSET)

#define __spdif_write_tfifo(v)           \
	spdif_set_reg(SPFIFO,v,SPDIF_DATA_MASK,SPDIF_DATA_OFFSET)

/* SPCFG1 */
#define SPDIF_INITLVL_OFFSET	 (17)
#define SPDIF_INITLVL_MASK		 (0x1 << SPDIF_INITLVL_OFFSET)
#define SPDIF_LSMP_OFFSET        (16)
#define SPDIF_LSMP_MASK          (0x1 << SPDIF_LSMP_OFFSET)
#define SPDIF_TRIG_OFFSET        (12)
#define SPDIF_TRIG_MASK          (0x3 << SPDIF_TRIG_OFFSET)
#define SPDIF_SRCNUM_OFFSET		 (8)
#define SPDIF_SRCNUM_MASK		 (0xf << SPDIF_SRCNUM_OFFSET)
#define SPDIF_CH1NUM_OFFSET		 (4)
#define SPDIF_CH1NUM_MASK		 (0xf << SPDIF_CH1NUM_OFFSET)
#define SPDIF_CH2NUM_OFFSET		 (0)
#define	SPDIF_CH2NUM_MASK		 (0xf << SPDIF_CH2NUM_OFFSET)

#define __spdif_init_set_low()			\
	spdif_set_reg(SPCFG1,0,SPDIF_INITLVL_MASK,SPDIF_INITLVL_OFFSET)
#define __spdif_init_set_high()			\
	spdif_set_reg(SPCFG1,1,SPDIF_INITLVL_MASK,SPDIF_INITLVL_OFFSET)

#define __spdif_play_zero()              \
	spdif_set_reg(SPCFG1,0,SPDIF_LSMP_MASK,SPDIF_LSMP_OFFSET)
#define __spdif_play_lastsample()        \
	spdif_set_reg(SPCFG1,1,SPDIF_LSMP_MASK,SPDIF_LSMP_OFFSET)

#define __spdif_set_srcnum(n)		\
	spdif_set_reg(SPCFG1,n,SPDIF_SRCNUM_MASK,SPDIF_SRCNUM_OFFSET)
#define __spdif_get_srcnum()			\
	spdif_get_reg(SPCFG1,SPDIF_SRCNUM_MASK,SPDIF_SRCNUM_OFFSET)

#define __spdif_set_ch1num(n)		\
	spdif_set_reg(SPCFG1,n,SPDIF_CH1NUM_MASK,SPDIF_CH1NUM_OFFSET)
#define __spdif_get_ch1num()		\
	spdif_set_reg(SPCFG1,SPDIF_CH1NUM_MASK,SPDIF_CH1NUM_OFFSET)

#define __spdif_set_ch2num(n)		\
	spdif_set_reg(SPCFG1,n,SPDIF_CH2NUM_MASK,SPDIF_CH2NUM_OFFSET)
#define __spdif_get_ch2num()		\
	spdif_set_reg(SPCFG1,SPDIF_CH2NUM_MASK,SPDIF_CH2NUM_OFFSET)

static inline unsigned long  __spdif_set_transmit_trigger(int n)
{
/* modified */

	int div, div1;
	div = n / 4;
	div -= 1;
	if (div > 1 && div < 7)
		div = 2;
	else if(div >= 7)
		div = 3;
	else if (div < 0)
		div = 0;

	spdif_set_reg(SPCFG1,div,SPDIF_TRIG_MASK,SPDIF_TRIG_OFFSET);
	div1 = spdif_get_reg(SPCFG1,SPDIF_TRIG_MASK,SPDIF_TRIG_OFFSET);

	if (div1 != div)
		printk("%d  write trigger failed!\n",__LINE__);
	return 0;
}

/* SPCFG2 */
#define SPDIF_FS_OFFSET          (26)
#define SPDIF_FS_MASK            (0xf << SPDIF_FS_OFFSET)
#define SPDIF_ORGFRQ_OFFSET		 (22)
#define SPDIF_ORGFRQ_MASK		 (0xf << SPDIF_ORGFRQ_OFFSET)
#define SPDIF_OSS_OFFSET         (19)
#define SPDIF_OSS_MASK           (0x7 << SPDIF_OSS_OFFSET)
#define SPDIF_OSS_MAX_OFFSET     (18)
#define SPDIF_OSS_MAX_MASK       (0x7 << SPDIF_OSS_MAX_OFFSET)
#define SPDIF_CLKACU_OFFSET		 (16)
#define SPDIF_CLKACU_MASK		 (0x3 << SPDIF_CLKACU_OFFSET)
#define SPDIF_CATC_OFFSET		 (8)
#define SPDIF_CATC_MASK			 (0xff << SPDIF_CATC_OFFSET)
#define SPDIF_CHMD_OFFSET		 (6)
#define SPDIF_CHMD_MASK			 (0x3 << SPDIF_CHMD_OFFSET)
#define SPDIF_PRE_OFFSET		 (3)
#define SPDIF_PRE_MASK			 (0x1 << SPDIF_PRE_OFFSET)
#define SPDIF_COPYN_OFFSET		 (2)
#define SPDIF_COPYN_MASK		 (0x1 << SPDIF_COPYN_OFFSET)
#define SPDIF_AUDION_OFFSET		 (1)
#define SPDIF_AUDION_MASK		 (0x1 << SPDIF_AUDION_OFFSET)
#define SPDIF_CPRO_OFFSET		 (0)
#define SPDIF_CPRO_MASK			 (0x1 << SPDIF_CPRO_OFFSET)

#define __spdif_set_oss_sample_size(n)		\
	spdif_set_reg(SPCFG2,n,SPDIF_OSS_MASK,SPDIF_OSS_OFFSET)
#define __spdif_set_max_wl(n)				\
	spdif_set_reg(SPCFG2,n,SPDIF_OSS_MAX_MASK,SPDIF_OSS_MAX_OFFSET)

#define __spdif_set_category_code_normal()	\
	spdif_set_reg(SPCFG2,0,SPDIF_CATC_MASK,SPDIF_CATC_OFFSET)

#define __spdif_set_clkacu(n)				\
	spdif_set_reg(SPCFG2,n,SPDIF_CLKACU_MASK,SPDIF_CLKACU_OFFSET)

#define __spdif_choose_chmd()				\
	spdif_set_reg(SPCFG2,0,SPDIF_CHMD_MASK,SPDIF_CHMD_OFFSET)

#define __spdif_set_pre()					\
	spdif_set_reg(SPCFG2,1,SPDIF_PRE_MASK,SPDIF_PRE_OFFSET)
#define __spdif_clear_pre()					\
	spdif_set_reg(SPCFG2,0,SPDIF_PRE_MASK,SPDIF_PRE_OFFSET)

#define __spdif_set_copyn()					\
	spdif_set_reg(SPCFG2,1,SPDIF_COPYN_MASK,SPDIF_COPYN_OFFSET)
#define __spdif_clear_copyn()				\
	spdif_set_reg(SPCFG2,0,SPDIF_COPYN_MASK,SPDIF_COPYN_OFFSET)

#define __spdif_set_audion()				\
	spdif_set_reg(SPCFG2,1,SPDIF_AUDION_MASK,SPDIF_AUDION_MASK)
#define __spdif_clear_audion()				\
	spdif_set_reg(SPCFG2,0,SPDIF_AUDION_MASK,SPDIF_AUDION_MASK)

#define __spdif_choose_consumer()					\
	spdif_set_reg(SPCFG2,0,SPDIF_CPRO_MASK,SPDIF_CPRO_OFFSET)
#define __spdif_choose_professional()					\
	spdif_set_reg(SPCFG2,1,SPDIF_CPRO_MASK,SPDIF_CPRO_OFFSET)

static inline unsigned long  __spdif_set_sample_freq(unsigned long sync)
{
	int div = 0;;
	switch(sync) {
		case 44100: div = 0x0;
			break;
		case 48000: div = 0x2;
			break;
		case 32000: div = 0x3;
			break;
		case 96000: div = 0xa;
			break;
		case 192000: div = 0xd;
			break;
		default : div = 0;
			break;
	}
	spdif_set_reg(SPCFG2,div,SPDIF_FS_MASK,SPDIF_FS_OFFSET);

	return div;
}

static inline unsigned long  __spdif_set_ori_sample_freq(unsigned long sync)
{
	int div = 0;;
	switch(sync) {
		case 44100: div = 0xf;
			break;
		case 48000: div = 0xd;
			break;
		case 32000: div = 0xc;
			break;
		case 96000: div = 0x5;
			break;
		case 192000: div = 0x1;
			break;
		default : div = 0;
			break;
	}
	spdif_set_reg(SPCFG2,div,SPDIF_ORGFRQ_MASK,SPDIF_ORGFRQ_OFFSET);

	return div;
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
 * spdif codec control cmd
 **/
/*enum codec_ioctl_cmd_t {
	CODEC_INIT,
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
};*/
/**
 *	spdif switch state
 **/
void *jz_set_hp_detect_type(int type,struct snd_board_gpio *hp_det,
		struct snd_board_gpio *mic_det,
		struct snd_board_gpio *mic_detect_en,
		struct snd_board_gpio *mic_select,
		int  hook_active_level);

/**
 *	codec mode
 **/

/*enum codec_mode {
	CODEC_MASTER,
	CODEC_SLAVE,
};*/

void spdif_replay_zero_for_flush_codec(void);


#if defined(CONFIG_JZ_INTERNAL_CODEC)
extern void codec_irq_set_mask(void);
#endif

#endif /* _XB_SND_SPDIF_H_ */
