#include <linux/fb.h>
//#include <linux/earlysuspend.h>

#ifdef CONFIG_TWO_FRAME_BUFFERS
#define NUM_FRAME_BUFFERS 2
#endif

#ifdef CONFIG_THREE_FRAME_BUFFERS
#define NUM_FRAME_BUFFERS 3
#endif

#define PIXEL_ALIGN 4
#define MAX_DESC_NUM 4

/**
 * @next: physical address of next frame descriptor
 * @databuf: physical address of buffer
 * @id: frame ID
 * @cmd: DMA command and buffer length(in word)
 * @offsize: DMA off size, in word
 * @page_width: DMA page width, in word
 * @cpos: smart LCD mode is commands' number, other is bpp,
 * premulti and position of foreground 0, 1
 * @desc_size: alpha and size of foreground 0, 1
 */
struct jzfb_framedesc {
	uint32_t next;
	uint32_t databuf;
	uint32_t id;
	uint32_t cmd;
	uint32_t offsize;
	uint32_t page_width;
	uint32_t cpos;
	uint32_t desc_size;
};

struct jzfb_display_size {
	u32 fg0_line_size;
	u32 fg0_frm_size;
	u32 panel_line_size;
	u32 height_width;
};

enum jzfb_format_order {
	FORMAT_X8R8G8B8 = 1,
	FORMAT_X8B8G8R8,
};

/**
 * @fg: foreground 0 or foreground 1
 * @bpp: foreground bpp
 * @x: foreground start position x
 * @y: foreground start position y
 * @w: foreground width
 * @h: foreground height
 */
struct jzfb_fg_t {
	u32 fg;
	u32 bpp;
	u32 x;
	u32 y;
	u32 w;
	u32 h;
};

/**
 *@decompress: enable decompress function, used by FG0
 *@block: enable 16x16 block function
 *@fg0: fg0 info
 *@fg1: fg1 info
 */
struct jzfb_osd_t {
	int block;
	struct jzfb_fg_t fg0;
	struct jzfb_fg_t fg1;
};

struct jzfb {
	int is_lcd_en;		/* 0, disable  1, enable */
	int is_clk_en;		/* 0, disable  1, enable */
	int irq;		/* lcdc interrupt num */
	int open_cnt;
	int irq_cnt;
	int desc_num;
	char clk_name[16];
	char pclk_name[16];
	char pwcl_name[16];
	char irq_name[16];

	struct fb_info *fb;
	struct device *dev;
	struct jzfb_platform_data *pdata;
	void __iomem *base;
	struct resource *mem;
#ifdef CONFIG_JZ_MIPI_DSI
	struct dsi_device *dsi;
	struct platform_driver *jz_dsi_driver;
	struct platform_device *jz_dsi_device;
#endif


	size_t vidmem_size;
	void *vidmem;
	dma_addr_t vidmem_phys;
	void *desc_cmd_vidmem;
	dma_addr_t desc_cmd_phys;

	int frm_size;
	int current_buffer;
	/* dma 0 descriptor base address */
	struct jzfb_framedesc *framedesc[MAX_DESC_NUM];
	struct jzfb_framedesc *fg1_framedesc;	/* FG 1 dma descriptor */
	dma_addr_t framedesc_phys;

	struct completion vsync_wq;
	struct task_struct *vsync_thread;
	unsigned int vsync_skip_map;	/* 10 bits width */
	int vsync_skip_ratio;

	struct mutex lock;
	struct mutex suspend_lock;

	enum jzfb_format_order fmt_order;	/* frame buffer pixel format order */
	struct jzfb_osd_t osd;	/* osd's config information */

	struct clk *clk;
	struct clk *pclk;
	struct clk *pwcl;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	int is_suspend;
	unsigned int pan_display_count;
	int blank;

        char eventbuf[64];
        int is_vsync;

        ktime_t timestamp_array[16];
        int timestamp_irq_pos;
        int timestamp_thread_pos;
        spinlock_t vsync_lock;
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

/* structures for frame buffer ioctl */
struct jzfb_fg_pos {
	__u32 fg;		/* 0:fg0, 1:fg1 */
	__u32 x;
	__u32 y;
};

struct jzfb_fg_size {
	__u32 fg;
	__u32 w;
	__u32 h;
};

struct jzfb_fg_alpha {
	__u32 fg;		/* 0:fg0, 1:fg1 */
	__u32 enable;
	__u32 mode;		/* 0:global alpha, 1:pixel alpha */
	__u32 value;		/* 0x00-0xFF */
};

struct jzfb_bg {
	__u32 fg;		/* 0:fg0, 1:fg1 */
	__u32 red;
	__u32 green;
	__u32 blue;
};

struct jzfb_color_key {
	__u32 fg;		/* 0:fg0, 1:fg1 */
	__u32 enable;
	__u32 mode;		/* 0:color key, 1:mask color key */
	__u32 red;
	__u32 green;
	__u32 blue;
};

struct jzfb_mode_res {
	__u32 index;		/* 1-64 */
	__u32 w;
	__u32 h;
};

/* ioctl commands base fb.h FBIO_XXX */
/* image_enh.h: 0x142 -- 0x162 */
#define JZFB_GET_MODENUM		_IOR('F', 0x100, int)
#define JZFB_GET_MODELIST		_IOR('F', 0x101, int)
#define JZFB_SET_VIDMEM			_IOW('F', 0x102, unsigned int *)
#define JZFB_SET_MODE			_IOW('F', 0x103, int)
#define JZFB_ENABLE			_IOW('F', 0x104, int)
#define JZFB_GET_RESOLUTION		_IOWR('F', 0x105, struct jzfb_mode_res)

/* Reserved for future extend */
#define JZFB_SET_FG_SIZE		_IOW('F', 0x116, struct jzfb_fg_size)
#define JZFB_GET_FG_SIZE		_IOWR('F', 0x117, struct jzfb_fg_size)
#define JZFB_SET_FG_POS			_IOW('F', 0x118, struct jzfb_fg_pos)
#define JZFB_GET_FG_POS			_IOWR('F', 0x119, struct jzfb_fg_pos)
#define JZFB_GET_BUFFER			_IOR('F', 0x120, int)
/* Reserved for future extend */
#define JZFB_SET_ALPHA			_IOW('F', 0x123, struct jzfb_fg_alpha)
#define JZFB_SET_BACKGROUND		_IOW('F', 0x124, struct jzfb_bg)
#define JZFB_SET_COLORKEY		_IOW('F', 0x125, struct jzfb_color_key)
#define JZFB_16X16_BLOCK_EN		_IOW('F', 0x127, int)
#define JZFB_ENABLE_LCDC_CLK		_IOW('F', 0x130, int)
/* Reserved for future extend */
#define JZFB_ENABLE_FG0			_IOW('F', 0x139, int)
#define JZFB_ENABLE_FG1			_IOW('F', 0x140, int)
#define JZFB_GET_LCDTYPE		_IOR('F', 0x122, int)

/* Reserved for future extend */
#define JZFB_SET_VSYNCINT		_IOW('F', 0x210, int)

/* define in image_enh.c */
extern int jzfb_config_image_enh(struct fb_info *info);
extern int jzfb_image_enh_ioctl(struct fb_info *info, unsigned int cmd,
				unsigned long arg);
extern int update_slcd_frame_buffer(void);
extern int lcd_display_inited_by_uboot(void);
