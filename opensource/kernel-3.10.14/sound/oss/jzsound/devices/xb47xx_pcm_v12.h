
#ifndef __XB_SND_PCM_H__
#define __XB_SND_PCM_H__

#include <asm/io.h>
#include "../interface/xb_snd_dsp.h"

/**
 * global variable
 **/
extern void volatile __iomem *volatile pcm_iomem;


#define NEED_RECONF_DMA         0x00000001
#define NEED_RECONF_TRIGGER     0x00000002
#define NEED_RECONF_FILTER      0x00000004

/**
 * registers
 **/

#define PCMCTL0		0x00
#define PCMCFG0		0x04
#define PCMDP0		0x08
#define PCMINTC0	0x0C
#define PCMINTS0	0x10
#define PCMDIV0		0x14

/**
 * i2s register control
 **/
static unsigned long read_val;
static unsigned long tmp_val;
#define pcm_write_reg(addr,val)        \
	writel(val,pcm_iomem+addr)

#define pcm_read_reg(addr)             \
	readl(pcm_iomem+addr)

#define pcm_set_reg(addr,val,mask,offset)\
	do {										\
		tmp_val = val;							\
		read_val = pcm_read_reg(addr);         \
		read_val &= (~mask);                    \
		tmp_val = ((tmp_val << offset) & mask); \
		tmp_val |= read_val;                    \
		pcm_write_reg(addr,tmp_val);           \
	}while(0)

#define pcm_get_reg(addr,mask,offset)  \
	((pcm_read_reg(addr) & mask) >> offset)

#define pcm_clear_reg(addr,mask)       \
	pcm_write_reg(addr,~mask)

/*PCMCTL0*/
#define PCM_ERDMA_OFFSET       (9)
#define PCM_ERDMA_MASK         (0x1 << PCM_ERDMA_OFFSET)
#define PCM_ETDMA_OFFSET       (8)
#define PCM_ETDMA_MASK         (0x1 << PCM_ETDMA_OFFSET)
#define PCM_LSMP_OFFSET		(7)
#define PCM_LSMP_MASK			(0x1 << PCM_LSMP_OFFSET)
#define PCM_ERPL_OFFSET		(6)
#define PCM_ERPL_MASK			(0x1 << PCM_ERPL_OFFSET)
#define PCM_EREC_OFFSET        (5)
#define PCM_EREC_MASK          (0x1 << PCM_EREC_OFFSET)
#define PCM_FLUSH_OFFSET       (4)
#define PCM_FLUSH_MASK         (0x1 << PCM_FLUSH_OFFSET)
#define PCM_RST_OFFSET			(3)
#define PCM_RST_MASK			(0x1 << PCM_RST_OFFSET)
#define PCM_CLKEN_OFFSET       (1)
#define PCM_CLKEN_MASK         (0x1 << PCM_CLKEN_OFFSET)
#define PCM_PCMEN_OFFSET       (0)
#define PCM_PCMEN_MASK         (0x1 << PCM_PCMEN_OFFSET)

#define __pcm_enable_transmit_dma()    \
	pcm_set_reg(PCMCTL0,1,PCM_ETDMA_MASK,PCM_ETDMA_OFFSET)
#define __pcm_disable_transmit_dma()   \
	pcm_set_reg(PCMCTL0,0,PCM_ETDMA_MASK,PCM_ETDMA_OFFSET)
#define __pcm_enable_receive_dma()     \
	pcm_set_reg(PCMCTL0,1,PCM_ERDMA_MASK,PCM_ERDMA_OFFSET)
#define __pcm_disable_receive_dma()    \
	pcm_set_reg(PCMCTL0,0,PCM_ERDMA_MASK,PCM_ERDMA_OFFSET)

#define __pcm_play_zero()              \
	pcm_set_reg(PCMCTL0,0,PCM_LSMP_MASK,PCM_LSMP_OFFSET)
#define __pcm_play_lastsample()        \
	pcm_set_reg(PCMCTL0,1,PCM_LSMP_MASK,PCM_LSMP_OFFSET)

#define __pcm_enable_replay()          \
	pcm_set_reg(PCMCTL0,1,PCM_ERPL_MASK,PCM_ERPL_OFFSET)
#define __pcm_disable_replay()         \
	pcm_set_reg(PCMCTL0,0,PCM_ERPL_MASK,PCM_ERPL_OFFSET)
#define __pcm_enable_record()          \
	pcm_set_reg(PCMCTL0,1,PCM_EREC_MASK,PCM_EREC_OFFSET)
#define __pcm_disable_record()         \
	pcm_set_reg(PCMCTL0,0,PCM_EREC_MASK,PCM_EREC_OFFSET)

#define __pcm_flush_fifo()            \
	pcm_set_reg(PCMCTL0,1,PCM_FLUSH_MASK,PCM_FLUSH_OFFSET)

#define __pcm_reset()                  \
	pcm_set_reg(PCMCTL0,1,PCM_RST_MASK,PCM_RST_OFFSET)

#define __pcm_clock_enable()			\
	pcm_set_reg(PCMCTL0,1,PCM_CLKEN_MASK,PCM_CLKEN_OFFSET)
#define __pcm_clock_disable()			\
	pcm_set_reg(PCMCTL0,0,PCM_CLKEN_MASK,PCM_CLKEN_OFFSET)

#define __pcm_enable()                 \
	pcm_set_reg(PCMCTL0,1,PCM_PCMEN_MASK,PCM_PCMEN_OFFSET)
#define __pcm_disable()                \
	pcm_set_reg(PCMCTL0,0,PCM_PCMEN_MASK,PCM_PCMEN_OFFSET)

/*PCMCFG0*/
#define PCM_SLOT_OFFSET        (13)
#define PCM_SLOT_MASK          (0x3 << PCM_SLOT_OFFSET)
#define PCM_ISS_OFFSET			(12)
#define PCM_ISS_MASK			(0x1 << PCM_ISS_OFFSET)
#define PCM_OSS_OFFSET			(11)
#define PCM_OSS_MASK			(0x1 << PCM_OSS_OFFSET)
#define PCM_IMSBPOS_OFFSET     (10)
#define PCM_IMSBPOS_MASK       (0x1 << PCM_IMSBPOS_OFFSET)
#define PCM_OMSBPOS_OFFSET     (9)
#define PCM_OMSBPOS_MASK       (0x1 << PCM_OMSBPOS_OFFSET)
#define PCM_RFTH_OFFSET        (5)
#define PCM_RFTH_MASK          (0xf << PCM_RFTH_OFFSET)
#define PCM_TFTH_OFFSET        (1)
#define PCM_TFTH_MASK          (0xf << PCM_TFTH_OFFSET)
#define PCM_PCMMOD_OFFSET      (0)
#define PCM_PCMMOD_MASK        (0x1 << PCM_PCMMOD_OFFSET)

#define __pcm_set_slot(n)	\
	pcm_set_reg(PCMCFG0,n,PCM_SLOT_MASK,PCM_SLOT_OFFSET)

#define __pcm_set_oss_sample_size(n)   \
	pcm_set_reg(PCMCFG0,n,PCM_OSS_MASK,PCM_OSS_OFFSET)
#define __pcm_set_iss_sample_size(n)   \
	pcm_set_reg(PCMCFG0,n,PCM_ISS_MASK,PCM_ISS_OFFSET)

#define __pcm_set_msb_normal_in()   \
	pcm_set_reg(PCMCFG0,0,PCM_IMSBPOS_MASK,PCM_IMSBPOS_OFFSET)
#define __pcm_set_msb_one_shift_in()   \
	pcm_set_reg(PCMCFG0,1,PCM_IMSBPOS_MASK,PCM_IMSBPOS_OFFSET)

#define __pcm_set_msb_normal_out()   \
	pcm_set_reg(PCMCFG0,0,PCM_OMSBPOS_MASK,PCM_OMSBPOS_OFFSET)
#define __pcm_set_msb_one_shift_out()   \
	pcm_set_reg(PCMCFG0,1,PCM_OMSBPOS_MASK,PCM_OMSBPOS_OFFSET)

#define __pcm_set_transmit_trigger(n)  \
	pcm_set_reg(PCMCFG0,n,PCM_TFTH_MASK,PCM_TFTH_OFFSET)
#define __pcm_set_receive_trigger(n)   \
	pcm_set_reg(PCMCFG0,n,PCM_RFTH_MASK,PCM_RFTH_OFFSET)

#define __pcm_as_master()             \
	pcm_set_reg(PCMCFG0,0,PCM_PCMMOD_MASK,PCM_PCMMOD_OFFSET)
#define __pcm_as_slaver()             \
	pcm_set_reg(PCMCFG0,1,PCM_PCMMOD_MASK,PCM_PCMMOD_OFFSET)

/*PCMDP0*/
#define PCM_PCMDP_OFFSET		(0)
#define PCM_PCMDP_MASK			(~0)

#define __pcm_read_fifo()		\
	pcm_get_reg(PCMDP0,PCM_PCMDP_MASK,PCM_PCMDP_OFFSET);

/*PCMINTC0*/
#define PCM_ETFS_OFFSET        (3)
#define PCM_ETFS_MASK          (0X1 << PCM_ETFS_OFFSET)
#define PCM_ETUR_OFFSET        (2)
#define PCM_ETUR_MASK          (0x1 << PCM_ETUR_OFFSET)
#define PCM_ERFS_OFFSET        (1)
#define PCM_ERFS_MASK          (0x1 << PCM_ERFS_OFFSET)
#define PCM_EROR_OFFSET        (0)
#define PCM_EROR_MASK          (0x1 << PCM_EROR_OFFSET)

#define __pcm_enable_receive_intr()    \
	pcm_set_reg(PCMINTC0,1,PCM_ERFS_MASK,PCM_ERFS_OFFSET)
#define __pcm_disable_receive_intr()   \
	pcm_set_reg(PCMINTC0,0,PCM_ERFS_MASK,PCM_ERFS_OFFSET)

#define __pcm_enable_underrun_intr()   \
	pcm_set_reg(PCMINTC0,1,PCM_ETUR_MASK,PCM_ETUR_OFFSET)
#define __pcm_disable_underrun_intr()  \
	pcm_set_reg(PCMINTC0,0,PCM_ETUR_MASK,PCM_ETUR_OFFSET)

#define __pcm_enable_transmit_intr()   \
	pcm_set_reg(PCMINTC0,1,PCM_ETFS_MASK,PCM_ETFS_OFFSET)
#define __pcm_disable_transmit_intr()  \
	pcm_set_reg(PCMINTC0,0,PCM_ETFS_MASK,PCM_ETFS_OFFSET)

#define __pcm_enable_overrun_intr()    \
	pcm_set_reg(PCMINTC0,1,PCM_EROR_MASK,PCM_EROR_OFFSET)
#define __pcm_disable_overrun_intr()   \
	pcm_set_reg(PCMINTC0,0,PCM_EROR_MASK,PCM_EROR_OFFSET)

/*PCMINTS0*/
#define PCM_RSTS_OFFSET        (14)
#define PCM_RSTS_MASK          (0x1 << PCM_RSTS_OFFSET)
#define PCM_TFL_OFFSET         (9)
#define PCM_TFL_MASK           (0x1f << PCM_TFL_OFFSET)
#define PCM_TFS_OFFSET         (8)
#define PCM_TFS_MASK           (0x1 << PCM_TFS_OFFSET)
#define PCM_TUR_OFFSET         (7)
#define PCM_TUR_MASK           (0x1 << PCM_TUR_OFFSET)
#define PCM_RFL_OFFSET         (2)
#define PCM_RFL_MASK           (0x1f << PCM_RFL_OFFSET)
#define PCM_RFS_OFFSET         (1)
#define PCM_RFS_MASK           (0x1 << PCM_RFS_OFFSET)
#define PCM_ROR_OFFSET         (0)
#define PCM_ROR_MASK           (0X1 << PCM_ROR_OFFSET)

#define __pcm_test_rst_complie()	\
	!pcm_get_reg(PCMINTS0,PCM_RSTS_MASK,PCM_RSTS_OFFSET)
#define __pcm_test_tfl()               \
	pcm_get_reg(PCMINTS0,PCM_TFL_MASK,PCM_TFL_OFFSET)
#define __pcm_test_tfs()               \
	pcm_get_reg(PCMINTS0,PCM_TFS_MASK,PCM_TFS_OFFSET)
#define __pcm_test_tur()               \
	pcm_get_reg(PCMINTS0,PCM_TUR_MASK,PCM_TUR_OFFSET)
#define __pcm_clear_tur()	\
	pcm_set_reg(PCMINTS0,0,PCM_TUR_MASK,PCM_TUR_OFFSET)
#define __pcm_test_rfl()               \
	pcm_get_reg(PCMINTS0,PCM_RFL_MASK,PCM_RFL_OFFSET)
#define __pcm_test_rfs()               \
	pcm_get_reg(PCMINTS0,PCM_RFS_MASK,PCM_RFS_OFFSET)
#define __pcm_clear_ror()	\
	pcm_set_reg(PCMINTS0,0,PCM_ROR_MASK,PCM_ROR_OFFSET)
#define __pcm_test_ror()               \
	pcm_get_reg(PCMINTS0,PCM_ROR_MASK,PCM_ROR_OFFSET)
/* PCMDIV0 */
#define PCM_SYNC_OFFSET		(11)
#define PCM_SYNC_MASK			(0x3f << PCM_SYNC_OFFSET)
#define PCM_SYNDIV_OFFSET		(6)
#define PCM_SYNDIV_MASK		(0x1f << PCM_SYNDIV_OFFSET)
#define PCM_CLKDIV_OFFSET		(0)
#define PCM_CLKDIV_MASK		(0x3f << PCM_CLKDIV_OFFSET)

#define __pcm_set_sync(n)	\
	pcm_set_reg(PCMDIV0,n,PCM_SYNC_MASK,PCM_SYNC_OFFSET)
#define __pcm_set_syndiv(n)	\
	pcm_set_reg(PCMDIV0,n,PCM_SYNDIV_MASK,PCM_SYNDIV_OFFSET)
#define __pcm_set_clkdiv(n)	\
pcm_set_reg(PCMDIV0,n,PCM_CLKDIV_MASK,PCM_CLKDIV_OFFSET)


#define CODEC_RMODE                     0x1
#define CODEC_WMODE                     0x2
#define CODEC_RWMODE                    0x3

typedef enum {
	COINCIDENT_MODE,
	ONE_SHIFT_MODE,
} snd_pcm_data_start_mode_t;

typedef enum {
	PCM_MASTER,
	PCM_SLAVE,
} snd_pcm_mode_t;

typedef unsigned long snd_pcm_sync_len_t;
typedef unsigned long snd_pcm_solt_num_t;

struct pcm_conf_info {
	unsigned long pcm_sync_clk;		//rate
	unsigned long pcm_clk;			//bit clk
	unsigned long pcm_sysclk;		//sys clk if you choose master mode it can caculate by software
	unsigned long iss;
	unsigned long oss;
	snd_pcm_mode_t pcm_device_mode;
	snd_pcm_data_start_mode_t pcm_imsb_pos;
	snd_pcm_data_start_mode_t pcm_omsb_pos;
	snd_pcm_sync_len_t pcm_sync_len;
	snd_pcm_solt_num_t pcm_slot_num;
};
#endif /* _XB_SND_I2S_H_ */
