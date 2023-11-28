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
#define MAX_LAYER_NUM 4
#define MAX_BITS_PER_PIX (32)

#define MAX_STRIDE_VALUE (4096)

#define FRAME_CTRL_DEFAULT_SET  (0x04)

#define FRAME_CFG_ALL_UPDATE (0xFF)

enum {
	LAYER_CFG_FORMAT_RGB555 = 0,
	LAYER_CFG_FORMAT_ARGB1555 = 1,
	LAYER_CFG_FORMAT_RGB565	= 2,
	LAYER_CFG_FORMAT_RGB888	= 4,
	LAYER_CFG_FORMAT_ARGB8888 = 5,
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
		uint32_t bit1_keep0:1;
		uint32_t bit2_keep1:1;
		uint32_t bit3_4_keep0:2;
		uint32_t reserve5_31:27;
	}b;
} frm_ctrl_t;

typedef union lay_cfg_en {
	uint32_t d32;
	struct {
		uint32_t bit0_3_keep0:4;
		uint32_t lay0_en:1;
		uint32_t lay1_en:1;
		uint32_t bit6_7_keep0:2;
		uint32_t lay0_z_order:2;
		uint32_t lay1_z_order:2;
		uint32_t bit12_15_keep0:4;
		uint32_t reserve16_31:16;
	}b;
} lay_cfg_en_t;

typedef union cmp_irq_ctrl {
	uint32_t d32;
	struct {
		uint32_t reserve_0:1;
		uint32_t eof_msk:1;
		uint32_t sof_msk:1;
		uint32_t reserve3_14:12;
		uint32_t bit15_keep0:1;
		uint32_t reserve_16:1;
		uint32_t eod_msk:1;
		uint32_t reserve16_31:14;
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
		uint32_t reserve14_15:1;
		uint32_t format:4;
		uint32_t reserve22_31:12;
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

typedef union lay_pos {
	uint32_t d32;
	struct {
		uint32_t x_pos:12;
	        uint32_t reserve12_15:4;
	        uint32_t y_pos:12;
	        uint32_t reserve28_31:4;
	}b;
} lay_pos_t;

struct jzfb_framedesc {
	uint32_t	   FrameNextCfgAddr;
	uint32_t	   FrameSize;
	uint32_t	   FrameCtrl;
	uint32_t	   WritebackAddr;
	uint32_t	   WritebackStride;
	uint32_t	   Layer0CfgAddr;
	uint32_t	   Layer1CfgAddr;
	uint32_t 	   Layer2CfgAddr;
	uint32_t	   Layer3CfgAddr;
	uint32_t	   LayCfgEn;
	uint32_t	   InterruptControl;
};

struct jzfb_layerdesc {
	uint32_t	LayerSize;
	uint32_t	LayerCfg;
	uint32_t	LayerBufferAddr;
	uint32_t	LayerScale;
	uint32_t	layer_reserve0;
	uint32_t	LayerScratch;	//reserve
	uint32_t	LayerPos;
	uint32_t	LayerResizCoefX;
	uint32_t	LayerResizCoefY;
	uint32_t	LayerStride;
	uint32_t	UVBufferAddr;
	uint32_t	UVStride;

};

struct jzfb_rdmadesc {
	uint32_t	RdmaNextCfgAddr;
	uint32_t	FrameBufferAddr;
	uint32_t	Stride;
	uint32_t	RdmaCfg;
	uint32_t	RdmaInterruptControl;
};

struct jzfb_lay_cfg {
	unsigned int lay_en;
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
};

struct jzfb_frm_cfg {
	struct jzfb_lay_cfg lay_cfg[MAX_LAYER_NUM];
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

struct jzfb_frmdesc_msg {
	struct jzfb_framedesc *addr_virt;
	dma_addr_t addr_phy;
	struct list_head list;
	int state;
	int index;
};

struct jzfb {
	int is_lcd_en;		/* 0, disable  1, enable */
	int is_clk_en;		/* 0, disable  1, enable */
	int irq;		/* lcdc interrupt num */
	int open_cnt;
	int irq_cnt;
	int tft_undr_cnt;
	int frm_start;

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

	size_t vidmem_size;
	void *vidmem[MAX_DESC_NUM][MAX_LAYER_NUM];
	dma_addr_t vidmem_phys[MAX_DESC_NUM][MAX_LAYER_NUM];

	int current_frm_desc;
	struct jzfb_frm_mode current_frm_mode;

	size_t frm_size;
	struct jzfb_framedesc *framedesc[MAX_DESC_NUM];
	dma_addr_t framedesc_phys[MAX_DESC_NUM];
	struct jzfb_layerdesc *layerdesc[MAX_DESC_NUM][MAX_LAYER_NUM];
	dma_addr_t layerdesc_phys[MAX_DESC_NUM][MAX_LAYER_NUM];
	struct jzfb_rdmadesc *rdmadesc[MAX_DESC_NUM];
	dma_addr_t rdmadesc_phys[MAX_DESC_NUM];

	wait_queue_head_t gen_stop_wq;
	wait_queue_head_t vsync_wq;
	unsigned int vsync_skip_map;	/* 10 bits width */
	int vsync_skip_ratio;

	struct jzfb_frmdesc_msg frmdesc_msg[MAX_DESC_NUM];
	struct list_head desc_run_list;

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


	struct completion       display_end;
	struct completion       wb_end;
	struct completion	rdma_end;
	struct completion	composer_end;
	struct completion	cmp_stopped;
	struct completion	srd_stopped;
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
#define JZFB_CMP_START			_IOW('F', 0X141, int)
#define JZFB_TFT_START			_IOW('F', 0X143, int)
#define JZFB_SLCD_START			_IOW('F', 0X144, int)
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
