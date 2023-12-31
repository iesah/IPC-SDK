#ifndef __I2D__H__
#define __I2D__H__



#define I2D_BASE	0xB30B0000

#define I2D_RTL_VERSION	        0x00
#define I2D_SHD_CTRL        0x08
#define I2D_CTRL	        0x10
#define I2D_IMG_SIZE	    0x14
#define I2D_IMG_MODE	    0x18
#define I2D_SRC_ADDR_Y	    0x20
#define I2D_SRC_ADDR_UV	    0x24
#define I2D_SRC_Y_STRID	    0x28
#define I2D_SRC_UV_STRID	0x2C
#define I2D_DST_ADDR_Y	    0x30
#define I2D_DST_ADDR_UV	    0x34
#define I2D_DST_Y_STRID	    0x38
#define I2D_DST_UV_STRID	0x3C
#define I2D_IRQ_STATE	    0x80
#define I2D_IRQ_CLEAR	    0x84
#define I2D_IRQ_MASK	    0x88
#define I2D_TIMEOUT_VALUE	0x90
#define I2D_TIMEOUT_MODE	0x94
#define I2D_CLK_GATE	    0x98
#define I2D_DBG_0	        0xA0

#define u32 unsigned int

struct i2d_reg_struct {
	char *name;
	unsigned int addr;
};

struct i2d_buf_info {
	unsigned int vaddr_alloc;
	unsigned int paddr;
	unsigned int paddr_align;
	unsigned int size;
};

struct jz_i2d {

	int irq;
	char name[16];

	struct clk *clk;
	struct clk *ahb0_gate; /* T31 IPU mount at AHB1*/
	void __iomem *iomem;
	struct device *dev;
	struct resource *res;
	struct miscdevice misc_dev;

	struct mutex mutex;
	struct mutex irq_mutex;
	struct completion done_i2d;
	struct completion done_buf;
	struct i2d_buf_info pbuf;
};

struct i2d_flush_cache_para
{
	void *addr;
	unsigned int size;
};

struct i2d_param
{
	unsigned int 		src_w;
	unsigned int 		src_h;
	unsigned int 		data_type;
	unsigned int		rotate_enable;
	unsigned int		rotate_angle;
	unsigned int		flip_enable;
	unsigned int		mirr_enable;

    unsigned int		src_addr_y;
	unsigned int		src_addr_uv;
	unsigned int		dst_addr_y;
	unsigned int		dst_addr_uv;

	unsigned int		src_y_strid;
	unsigned int		src_uv_strid;
	unsigned int		dst_y_strid;
	unsigned int		dst_uv_strid;

};

enum
{
    FLIP_MODE_0_DEGREE   = 1,
    FLIP_MODE_90_DEGREE  = 2,
    FLIP_MODE_180_DEGREE = 3,
    FLIP_MODE_270_DEGREE = 4,
    FLIP_MODE_MIRR       = 5,
    FLIP_MODE_FLIP       = 6,
};

/* match HAL_PIXEL_FORMAT_ in system/core/include/system/graphics.h */
enum {
	HAL_PIXEL_FORMAT_RGBA_8888    = 1,
	HAL_PIXEL_FORMAT_RGBX_8888    = 2,
	HAL_PIXEL_FORMAT_RGB_888      = 3,
	HAL_PIXEL_FORMAT_RGB_565      = 4,
	HAL_PIXEL_FORMAT_BGRA_8888    = 5,
	//HAL_PIXEL_FORMAT_BGRX_8888    = 0x8000, /* Add BGRX_8888, Wolfgang, 2010-07-24 */
	HAL_PIXEL_FORMAT_BGRX_8888  	= 0x1ff, /* 2012-10-23 */
	HAL_PIXEL_FORMAT_RGBA_5551    = 6,
	HAL_PIXEL_FORMAT_RGBA_4444    = 7,
	HAL_PIXEL_FORMAT_ABGR_8888    = 8,
	HAL_PIXEL_FORMAT_ARGB_8888    = 9,
	HAL_PIXEL_FORMAT_YCbCr_422_SP = 0x10,
	HAL_PIXEL_FORMAT_YCbCr_420_SP = 0x11,
	HAL_PIXEL_FORMAT_YCbCr_422_P  = 0x12,
	HAL_PIXEL_FORMAT_YCbCr_420_P  = 0x13,
	HAL_PIXEL_FORMAT_YCbCr_420_B  = 0x24,
	HAL_PIXEL_FORMAT_YCbCr_422_I  = 0x14,
	HAL_PIXEL_FORMAT_YCbCr_420_I  = 0x15,
	HAL_PIXEL_FORMAT_CbYCrY_422_I = 0x16,
	HAL_PIXEL_FORMAT_CbYCrY_420_I = 0x17,
	HAL_PIXEL_FORMAT_NV12		  = 0x18,
	HAL_PIXEL_FORMAT_NV21 		  = 0x19,
	HAL_PIXEL_FORMAT_BGRA_5551    = 0x1a,
	HAL_PIXEL_FORMAT_RAW8         = 0x1b,

};

/**
 * IMP图像格式定义.
 */
typedef enum {
	PIX_FMT_YUV420P,   /**< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples) */
	PIX_FMT_YUYV422,   /**< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr */
	PIX_FMT_UYVY422,   /**< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1 */
	PIX_FMT_YUV422P,   /**< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples) */
	PIX_FMT_YUV444P,   /**< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples) */
	PIX_FMT_YUV410P,   /**< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples) */
	PIX_FMT_YUV411P,   /**< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) */
	PIX_FMT_GRAY8,     /**<	   Y	    ,  8bpp */
	PIX_FMT_MONOWHITE, /**<	   Y	    ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb */
	PIX_FMT_MONOBLACK, /**<	   Y	    ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb */

	PIX_FMT_NV12,      /**< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V) */
	PIX_FMT_NV21,      /**< as above, but U and V bytes are swapped */

	PIX_FMT_RGB24,     /**< packed RGB 8:8:8, 24bpp, RGBRGB... */
	PIX_FMT_BGR24,     /**< packed RGB 8:8:8, 24bpp, BGRBGR... */

	PIX_FMT_ARGB,      /**< packed ARGB 8:8:8:8, 32bpp, ARGBARGB... */
	PIX_FMT_RGBA,	   /**< packed RGBA 8:8:8:8, 32bpp, RGBARGBA... */
	PIX_FMT_ABGR,	   /**< packed ABGR 8:8:8:8, 32bpp, ABGRABGR... */
	PIX_FMT_BGRA,	   /**< packed BGRA 8:8:8:8, 32bpp, BGRABGRA... */

	PIX_FMT_RGB565BE,  /**< packed RGB 5:6:5, 16bpp, (msb)	  5R 6G 5B(lsb), big-endian */
	PIX_FMT_RGB565LE,  /**< packed RGB 5:6:5, 16bpp, (msb)	  5R 6G 5B(lsb), little-endian */
	PIX_FMT_RGB555BE,  /**< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0 */
	PIX_FMT_RGB555LE,  /**< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0 */

	PIX_FMT_BGR565BE,  /**< packed BGR 5:6:5, 16bpp, (msb)	 5B 6G 5R(lsb), big-endian */
	PIX_FMT_BGR565LE,  /**< packed BGR 5:6:5, 16bpp, (msb)	 5B 6G 5R(lsb), little-endian */
	PIX_FMT_BGR555BE,  /**< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1 */
	PIX_FMT_BGR555LE,  /**< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1 */

	PIX_FMT_0RGB,      /**< packed RGB 8:8:8, 32bpp, 0RGB0RGB... */
	PIX_FMT_RGB0,	   /**< packed RGB 8:8:8, 32bpp, RGB0RGB0... */
	PIX_FMT_0BGR,	   /**< packed BGR 8:8:8, 32bpp, 0BGR0BGR... */
	PIX_FMT_BGR0,	   /**< packed BGR 8:8:8, 32bpp, BGR0BGR0... */

	PIX_FMT_BAYER_BGGR8,    /**< bayer, BGBG..(odd line), GRGR..(even line), 8-bit samples */
	PIX_FMT_BAYER_RGGB8,    /**< bayer, RGRG..(odd line), GBGB..(even line), 8-bit samples */
	PIX_FMT_BAYER_GBRG8,    /**< bayer, GBGB..(odd line), RGRG..(even line), 8-bit samples */
	PIX_FMT_BAYER_GRBG8,    /**< bayer, GRGR..(odd line), BGBG..(even line), 8-bit samples */

	PIX_FMT_RAW,

	PIX_FMT_HSV,

	PIX_FMT_NB,
	PIX_FMT_YUV422,
	PIX_FMT_YVU422,
	PIX_FMT_UVY422,
	PIX_FMT_VUY422,
	PIX_FMT_RAW8,
	PIX_FMT_RAW16,
} IMPPixelFormat;


#define JZI2D_IOC_MAGIC  'D'
#define IOCTL_I2D_START			_IO(JZI2D_IOC_MAGIC, 106)
#define IOCTL_I2D_LISTEN		_IO(JZI2D_IOC_MAGIC, 116)
#define IOCTL_I2D_RES_PBUFF		_IO(JZI2D_IOC_MAGIC, 114)
#define IOCTL_I2D_GET_PBUFF		_IO(JZI2D_IOC_MAGIC, 115)
#define IOCTL_I2D_BUF_UNLOCK	_IO(JZI2D_IOC_MAGIC, 117)
#define IOCTL_I2D_BUF_FLUSH_CACHE	_IO(JZI2D_IOC_MAGIC, 118)

/* VERSION*/
#define I2D_VERSION (1 << 0)

/* SHD_CTRL */
#define I2D_MODE_SHADOW (1 << 1)

/* SHD_CTRL */
#define I2D_MODE_SHADOW (1 << 1)
#define I2D_ADDR_SHADOW (1 << 0)

/* CTRL */
#define I2D_RESET       (1 << 16)
#define I2D_SAFE_RESET  (1 << 4)
#define I2D_START       (1 << 0)

/* IMG_SIZE */
#define I2D_WIDTH       (16)
#define I2D_HEIGHT      (0)

/* I2D_MODE */
#define I2D_DATA_TYPE            (4)
#define I2D_DATA_TYPE_NV12       (0 << I2D_DATA_TYPE)
#define I2D_DATA_TYPE_RAW8       (1 << I2D_DATA_TYPE)
#define I2D_DATA_TYPE_RGB565     (2 << I2D_DATA_TYPE)
#define I2D_DATA_TYPE_ARGB8888   (3 << I2D_DATA_TYPE)

#define I2D_FLIP_MODE            (2)
#define I2D_FLIP_MODE_0          (0 << I2D_FLIP_MODE)
#define I2D_FLIP_MODE_90         (1 << I2D_FLIP_MODE)
#define I2D_FLIP_MODE_180        (2 << I2D_FLIP_MODE)
#define I2D_FLIP_MODE_270        (3 << I2D_FLIP_MODE)

#define I2D_MIRR        (1 << 1)
#define I2D_FLIP        (1 << 0)

/* SRC_ADDR_Y */
#define I2D_SRC_Y       (0)

/* SRC_ADDR_UV */
#define I2D_SRC_UV      (0)

/* SRC_Y_STRID */
#define I2D_SRC_STRID_Y       (0)

/* SRC_UV_STRID */
#define I2D_SRC_STRID_UV      (0)

/* DST_ADDR_Y */
#define I2D_DST_Y       (0)

/* DST_ADDR_UV */
#define I2D_DST_UV      (0)

/* DST_Y_STRID */
#define I2D_DST_STRID_Y       (0)

/* DST_UV_STRID */
#define I2D_DST_STRID_UV      (0)

/* IRQ_STATE */
#define I2D_IRQ_TIMEOUT       (1)
#define I2D_IRQ_FRAME_DONE    (0)

/* IRQ_CLEAR */
#define I2D_IRQ_TIMEOUT_CLR       (1 << 1)
#define I2D_IRQ_FRAME_DONE_CLR    (1 << 0)

/* IRQ_MASK */
#define I2D_IRQ_TIMEOUT_MASK       (0 << 1)
#define I2D_IRQ_FRAME_DONE_MASK    (0 << 0)

/* TIMEOUT_MODE */
#define I2D_TIMEOUT_RST       (0)


static inline unsigned int reg_read(struct jz_i2d *jzi2d, int offset)
{
	return readl(jzi2d->iomem + offset);
}

static inline void reg_write(struct jz_i2d *jzi2d, int offset, unsigned int val)
{
	writel(val, jzi2d->iomem + offset);
}

#define __i2d_mask_irq()		reg_bit_set(i2d, I2D_IRQ_MASK, (I2D_IRQ_TIMEOUT_MASK | I2D_IRQ_FRAME_DONE_MASK))
#define __i2d_irq_clear(value)		reg_bit_set(i2d, I2D_IRQ_CLEAR, value)

#define __start_i2d()			reg_bit_set(i2d, I2D_CTRL, I2D_START)
#define __reset_safe_i2d()		reg_bit_set(i2d, I2D_CTRL, I2D_SAFE_RESET)
#define __reset_i2d()			reg_bit_set(i2d, I2D_CTRL, I2D_RESET)

#endif
