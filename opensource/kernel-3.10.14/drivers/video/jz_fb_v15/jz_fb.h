/* drivers/video/jz_fb_v14/jz_fb.h
 *
 * Copyright (C) 2016 Ingenic Semiconductor Inc.
 *
 * Author:clwang<chunlei.wang@ingenic.com>
 *
 * This program is free software, you can redistribute it and/or modify it
 *
 * under the terms of the GNU General Public License version 2 as published by
 *
 * the Free Software Foundation.
 */
#ifndef __JZ_FB_H__
#define	__JZ_FB_H__
#include <linux/fb.h>
#include <mach/jzfb.h>

#ifdef CONFIG_ONE_FRAME_BUFFERS
#define MAX_DESC_NUM 1
#endif

#ifdef CONFIG_TWO_FRAME_BUFFERS
#define MAX_DESC_NUM 2
#endif

#ifdef CONFIG_THREE_FRAME_BUFFERS
#define MAX_DESC_NUM 3
#endif

#define PIXEL_ALIGN 4
#define DESC_ALIGN 8
#define MAX_DESC_NUM 3
#define MAX_LAYER_NUM 2

#define DPU_SUPPORT_LAYERS	4
#define MAX_BITS_PER_PIX (32)
#define DPU_MAX_SIZE (2047)
#define DPU_MIN_SIZE (4)
#define DPU_STRIDE_SIZE (4096)
#define DPU_SCALE_MIN_SIZE (20)


#define MAX_STRIDE_VALUE (4096)

#define FRAME_CTRL_DEFAULT_SET  (0x04)

#define FRAME_CFG_ALL_UPDATE (0xFF)
enum {
	DC_WB_FORMAT_8888 = 0,
	DC_WB_FORMAT_565 = 1,
	DC_WB_FORMAT_555 = 2,
	DC_WB_FORMAT_YUV422 = 3,
	DC_WB_FORMAT_MONO = 4,
	DC_WB_FORMAT_888 = 6,
};
enum {
	LAYER_CFG_FORMAT_RGB555 = 0,
	LAYER_CFG_FORMAT_ARGB1555 = 1,
	LAYER_CFG_FORMAT_RGB565	= 2,
	LAYER_CFG_FORMAT_RGB888	= 4,
	LAYER_CFG_FORMAT_ARGB8888 = 5,
	LAYER_CFG_FORMAT_MONO8 = 6,
	LAYER_CFG_FORMAT_MONO16 = 7,
	LAYER_CFG_FORMAT_NV12 = 8,
	LAYER_CFG_FORMAT_NV21 = 9,
	LAYER_CFG_FORMAT_YUV422 = 10,
	LAYER_CFG_FORMAT_TILE_H264 = 12,
};

enum {
	LAYER_CFG_COLOR_RGB = 0,
	LAYER_CFG_COLOR_RBG = 1,
	LAYER_CFG_COLOR_GRB = 2,
	LAYER_CFG_COLOR_GBR = 3,
	LAYER_CFG_COLOR_BRG = 4,
	LAYER_CFG_COLOR_BGR = 5,
};

enum {
	LAYER_Z_ORDER_0 = 0, /* bottom */
	LAYER_Z_ORDER_1 = 1,
	LAYER_Z_ORDER_2 = 2,
	LAYER_Z_ORDER_3 = 3, /* top */
};

enum {
	RDMA_CHAIN_CFG_COLOR_RGB = 0,
	RDMA_CHAIN_CFG_COLOR_RBG = 1,
	RDMA_CHAIN_CFG_COLOR_GRB = 2,
	RDMA_CHAIN_CFG_COLOR_GBR = 3,
	RDMA_CHAIN_CFG_COLOR_BRG = 4,
	RDMA_CHAIN_CFG_COLOR_BGR = 5,
};
enum {
	RDMA_CHAIN_CFG_FORMA_555 = 0,
	RDMA_CHAIN_CFG_FORMA_565 = 2,
	RDMA_CHAIN_CFG_FORMA_888 = 4,
};

typedef union frm_size {
	uint32_t d32;
	struct {
		uint32_t width:12;
	        uint32_t reserve12_15:4;
	        uint32_t height:12;
	        uint32_t reserve28_31:4;
	}b;
} frm_size_t;

typedef union frm_ctrl {
	uint32_t d32;
	struct {
		uint32_t stop:1;
		uint32_t wb_en:1;
		uint32_t direct_en:1;
		uint32_t change_2_rdma:1;
		uint32_t wb_dither_en:1;
		uint32_t wb_dither_auto:1;
		uint32_t reserve6_15:10;
		uint32_t wb_format:3;
		uint32_t reserve19:1;
		uint32_t wb_dither_b_dw:2;
		uint32_t wb_dither_g_dw:2;
		uint32_t wb_dither_r_dw:2;
		uint32_t reserve26_31:6;
	}b;
} frm_ctrl_t;

typedef union lay_cfg_en {
	uint32_t d32;
	struct {
		uint32_t lay0_scl_en:1;
		uint32_t lay1_scl_en:1;
		uint32_t lay2_scl_en:1;
		uint32_t lay3_scl_en:1;
		uint32_t lay0_en:1;
		uint32_t lay1_en:1;
		uint32_t lay2_en:1;
		uint32_t lay3_en:1;
		uint32_t lay0_z_order:2;
		uint32_t lay1_z_order:2;
		uint32_t lay2_z_order:2;
		uint32_t lay3_z_order:2;
		uint32_t leserve16_31:16;
	}b;
} lay_cfg_en_t;

typedef union cmp_irq_ctrl {
	uint32_t d32;
	struct {
		uint32_t reserve0_8:9;
		uint32_t eoc_msk:1;
		uint32_t reserve10_13:4;
		uint32_t soc_msk:1;
		uint32_t eow_msk:1;
		uint32_t reserve_16:1;
		uint32_t eod_msk:1;
		uint32_t reserve18_31:14;
	}b;
} cmp_irq_ctrl_t;

typedef union lay_size {
	uint32_t d32;
	struct {
		uint32_t width:12;
	        uint32_t reserve12_15:4;
	        uint32_t height:12;
	        uint32_t reserve28_31:4;
	}b;
} lay_size_t;

typedef union lay_cfg {
	uint32_t d32;
	struct {
		uint32_t g_alpha:8;
		uint32_t reserve8_9:2;
		uint32_t color:3;
		uint32_t g_alpha_en:1;
		uint32_t domain_multi:1;
		uint32_t reserve15:1;
		uint32_t format:4;
		uint32_t sharpl:2;
		uint32_t reserve22_31:10;
	}b;
} lay_cfg_t;

typedef union lay_scale {
	uint32_t d32;
	struct {
		uint32_t target_width:12;
	        uint32_t reserve12_15:4;
	        uint32_t target_height:12;
	        uint32_t reserve28_31:4;
	}b;
} lay_scale_t;

typedef union lay_rotation {
	uint32_t d32;
	struct {
		uint32_t rotation:3;
		uint32_t reserve3_31:29;
	}b;
} lay_rotation_t;

typedef union lay_pos {
	uint32_t d32;
	struct {
		uint32_t x_pos:12;
	        uint32_t reserve12_15:4;
	        uint32_t y_pos:12;
	        uint32_t reserve28_31:4;
	}b;
} lay_pos_t;

typedef union chain_cfg {
	uint32_t d32;
	struct {
		uint32_t chain_end:1;
		uint32_t change_2_cmp:1;
		uint32_t reserve2_15:14;
		uint32_t color:3;
		uint32_t format:4;
		uint32_t reserve23_31:9;
	}b;
} chain_cfg_t;

typedef union rdma_irq_ctrl {
	uint32_t d32;
	struct {
		uint32_t reserve_0:1;
		uint32_t eos_msk:1;
		uint32_t sos_msk:1;
		uint32_t reserve3_16:14;
		uint32_t eod_msk:1;
		uint32_t reserve18_31:4;
	}b;
} rdma_irq_ctrl_t;

struct jzfb_framedesc {
	uint32_t	   FrameNextCfgAddr;
	frm_size_t	   FrameSize;
	frm_ctrl_t	   FrameCtrl;
	uint32_t	   WritebackAddr;
	uint32_t	   WritebackStride;
	uint32_t	   Layer0CfgAddr;
	uint32_t	   Layer1CfgAddr;
	uint32_t 	   Layer2CfgAddr;
	uint32_t	   Layer3CfgAddr;
	lay_cfg_en_t       LayCfgEn;
	cmp_irq_ctrl_t	   InterruptControl;
};

struct jzfb_layerdesc {
	lay_size_t	LayerSize;
	lay_cfg_t	LayerCfg;
	uint32_t	LayerBufferAddr;
	lay_scale_t	LayerScale;
	lay_rotation_t	LayerRotation;
	uint32_t	LayerScratch;
	lay_pos_t	LayerPos;
	uint32_t	layerresizecoef_x;
	uint32_t	layerresizecoef_y;
	uint32_t	LayerStride;
	uint32_t        UVBufferAddr;
	uint32_t        UVStride;

};

struct jzfb_sreadesc {
	uint32_t	RdmaNextCfgAddr;
	uint32_t	FrameBufferAddr;
	uint32_t	Stride;
	chain_cfg_t	ChainCfg;
	rdma_irq_ctrl_t	InterruptControl;
};

struct jzfb_lay_cfg {
	unsigned int lay_en;
	unsigned int lay_scale_en;
	unsigned int lay_z_order;
	unsigned int pic_width;
	unsigned int pic_height;
	unsigned int disp_pos_x;
	unsigned int disp_pos_y;
	unsigned int g_alpha_en;
	unsigned int g_alpha_val;
	unsigned int color;
	unsigned int domain_multi;
	unsigned int format;
	unsigned int stride;
	unsigned int scale_w;
	unsigned int scale_h;
	unsigned int addr[MAX_DESC_NUM];
};

struct jzfb_frm_cfg {
	struct jzfb_lay_cfg lay_cfg[DPU_SUPPORT_LAYERS];
};

typedef enum stop_mode {
	QCK_STOP,
	GEN_STOP,
} stop_mode_t;

typedef enum csc_mode {
	CSC_MODE_0,
	CSC_MODE_1,
	CSC_MODE_2,
	CSC_MODE_3,
} csc_mode_t;


typedef enum framedesc_st {
	FRAME_DESC_AVAILABLE = 1,
	FRAME_DESC_SET_OVER = 2,
	FRAME_DESC_USING = 3,
}framedesc_st_t;

typedef enum frm_cfg_st {
	FRAME_CFG_NO_UPDATE,
	FRAME_CFG_UPDATE,
}frm_cfg_st_t;

struct jzfb_frm_mode {
	struct jzfb_frm_cfg frm_cfg;
	frm_cfg_st_t update_st[MAX_DESC_NUM];
};

typedef enum {
	DESC_ST_FREE,
	DESC_ST_AVAILABLE,
	DESC_ST_RUNING,
} frm_st_t;


#define CONFIG_DEBUG_DPU_IRQCNT
#ifdef CONFIG_DEBUG_DPU_IRQCNT

struct dbg_irqcnt {
	unsigned long long irqcnt;
	unsigned long long cmp_start;
	unsigned long long stop_disp_ack;
	unsigned long long disp_end;
	unsigned long long tft_under;
	unsigned long long wdma_over;
	unsigned long long wdma_end;
	unsigned long long layer3_end;
	unsigned long long layer2_end;
	unsigned long long layer1_end;
	unsigned long long layer0_end;
	unsigned long long clr_cmp_end;
	unsigned long long stop_wrbk_ack;
	unsigned long long srd_start;
	unsigned long long srd_end;
	unsigned long long cmp_w_slow;
};

#define dbg_irqcnt_inc(dbg,x)	dbg.x++

#else
struct dbg_irqcnt{
};
#define dbg_irqcnt_inc(dbg,x) do {} while(0)

#endif


struct jzfb {
	int is_lcd_en;		/* 0, disable  1, enable */
	int is_clk_en;		/* 0, disable  1, enable */
	int irq;		/* lcdc interrupt num */
	int open_cnt;
	int irq_cnt;
	int tft_undr_cnt;
	int frm_start;
	int wback_en;		/*writeback enable*/
	int csc_mode;		/*csc_mode*/

	struct dbg_irqcnt dbg_irqcnt;

	char clk_name[16];
	char pclk_name[16];
	char irq_name[16];
	struct clk *clk;
	struct clk *pclk;


	struct fb_info *fb;
	struct device *dev;
	struct jzfb_platform_data *pdata;
	void __iomem *base;
	struct resource *mem;

#ifdef CONFIG_JZ_MIPI_DSI
	struct dsi_device *dsi;
#endif

	size_t vidmem_size;
	void *vidmem[MAX_DESC_NUM][MAX_LAYER_NUM];
	dma_addr_t vidmem_phys[MAX_DESC_NUM][MAX_LAYER_NUM];
	size_t sread_vidmem_size;
	void * sread_vidmem[MAX_DESC_NUM];
	dma_addr_t sread_vidmem_phys[MAX_DESC_NUM];

	int current_frm_desc;
	struct jzfb_frm_mode current_frm_mode;

	size_t frm_size;
	struct jzfb_framedesc *framedesc[MAX_DESC_NUM];
	dma_addr_t framedesc_phys[MAX_DESC_NUM];
	struct jzfb_layerdesc *layerdesc[MAX_DESC_NUM][DPU_SUPPORT_LAYERS];
	dma_addr_t layerdesc_phys[MAX_DESC_NUM][DPU_SUPPORT_LAYERS];
	struct jzfb_sreadesc *sreadesc[MAX_DESC_NUM];
	dma_addr_t sreadesc_phys[MAX_DESC_NUM];

	wait_queue_head_t gen_stop_wq;
	wait_queue_head_t vsync_wq;
	unsigned int vsync_skip_map;	/* 10 bits width */
	int vsync_skip_ratio;

#define TIMESTAMP_CAP	16
	struct {
		volatile int wp; /* write position */
		int rp;	/* read position */
		u64 value[TIMESTAMP_CAP];
	} timestamp;

	struct mutex lock;
	struct mutex suspend_lock;
	spinlock_t	irq_lock;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int is_suspend;
	unsigned int pan_display_count;
	int blank;
	unsigned int pseudo_palette[16];
	int slcd_continua;

	unsigned int tlb_en;
	unsigned int user_addr;
	unsigned int tlba;
	unsigned int tlb_err_cnt;
};

struct dpu_dmmu_map_info {
	unsigned int addr;
	unsigned int len;
};

struct smash_mode {
    unsigned long vaddr;
    int mode;
};

void jzfb_clk_enable(struct jzfb *jzfb);
void jzfb_clk_disable(struct jzfb *jzfb);
static inline unsigned long reg_read(struct jzfb *jzfb, int offset)
{
	return readl(jzfb->base + offset);
}

static inline void reg_write(struct jzfb *jzfb, int offset, unsigned long val)
{
	writel(val, jzfb->base + offset);
}

#define JZFB_PUT_FRM_CFG		_IOWR('F', 0x101, struct jzfb_frm_cfg *)
#define JZFB_GET_FRM_CFG		_IOWR('F', 0x102, struct jzfb_frm_cfg *)
#define JZFB_SET_LAYER_POS		_IOWR('F', 0x105, struct jzfb_layer_pos *)
#define JZFB_SET_LAYER_SIZE		_IOWR('F', 0x106, struct jzfb_layer_size *)
#define JZFB_SET_LAYER_FORMAT		_IOWR('F', 0x109, struct jzfb_layer_format *)
#define JZFB_SET_LAYER_ALPHA		_IOWR('F', 0x151, struct jzfb_layer_alpha *)

#define JZFB_GET_LAYER_POS		_IOR('F', 0x115, struct jzfb_layer_pos *)
#define JZFB_GET_LAYER_SIZE		_IOR('F', 0x116, struct jzfb_layer_size *)

#define JZFB_SET_CSC_MODE		_IOW('F', 0x120, csc_mode_t)
#define JZFB_DIS_TLB			_IOW('F', 0x123, unsigned int)
#define JZFB_USE_TLB			_IOW('F', 0x124, unsigned int)
#define JZFB_DMMU_MAP			_IOWR('F', 0x130, struct dpu_dmmu_map_info)
#define JZFB_DMMU_UNMAP			_IOWR('F', 0x131, struct dpu_dmmu_map_info)
#define JZFB_DMMU_UNMAP_ALL		_IOWR('F', 0x132, struct dpu_dmmu_map_info)
#define JZFB_DMMU_MEM_SMASH		_IOWR('F', 0x133, struct smash_mode)
#define JZFB_DMMU_DUMP_MAP		_IOWR('F', 0x134, unsigned long)
#define JZFB_DMMU_FLUSH_CACHE		_IOWR('F', 0x135, struct dpu_dmmu_map_info)
#define JZFB_CMP_START			_IOW('F', 0X141, int)
#define JZFB_GEN_STP_CMP		_IOW('F', 0x145, int)
#define JZFB_QCK_STP_CMP		_IOW('F', 0x147, int)
#define JZFB_DUMP_LCDC_REG		_IOW('F', 0x150, int)

#define JZFB_SET_VSYNCINT		_IOW('F', 0x210, int)

/* define in image_enh.c */
extern int jzfb_config_image_enh(struct fb_info *info);
extern int jzfb_image_enh_ioctl(struct fb_info *info, unsigned int cmd,
				unsigned long arg);
extern int update_slcd_frame_buffer(void);
extern int lcd_display_inited_by_uboot(void);
#endif
