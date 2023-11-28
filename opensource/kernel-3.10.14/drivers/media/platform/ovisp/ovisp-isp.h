#ifndef __OVISP_ISP_H__
#define __OVISP_ISP_H__

#include <linux/errno.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include <mach/mfp.h>
//#include <mach/i2c.h>
//#include <mach/gpio.h>
#include <mach/camera.h>
#include <mach/ovisp-v4l2.h>
#include <mach/regs-isp.h>

/* ISP notify types. */
#define ISP_NOTIFY_DATA_START			(0x00010000)
#define ISP_NOTIFY_DATA_START0			(0x00000001)
#define ISP_NOTIFY_DATA_START1			(0x00000002)
#define ISP_NOTIFY_DATA_DONE			(0x00020000)
#define ISP_NOTIFY_DATA_DONE0			(0x00000010)
#define ISP_NOTIFY_DATA_DONE1			(0x00020020)
#define ISP_NOTIFY_DROP_FRAME			(0x00040000)
#define ISP_NOTIFY_DROP_FRAME0			(0x00000100)
#define ISP_NOTIFY_DROP_FRAME1			(0x00000200)
#define ISP_NOTIFY_OVERFLOW			(0x00080000)
#define ISP_NOTIFY_UPDATE_BUF			(0x80000000)

/*ISP group write config*/
#define I2C_CMD_READ		0x0001
#define I2C_CMD_ADDR_16BIT	0x0002
#define I2C_CMD_DATA_16BIT	0x0004


/* ISP clock number. */
#define ISP_CLK_NUM				(5)
#define ISP_MAX_OUTPUT_VIDEOS			(2)
#define ISP_OUTPUT_INFO_LENS			(30)
struct isp_device;

struct isp_reg_t {
	unsigned int reg;
	unsigned char value;
};
struct isp_buffer {
	unsigned long addr;
};

struct ovisp_video_format {
	unsigned int width;
	unsigned int height;
	unsigned int fourcc;
	unsigned int dev_width;
	unsigned int dev_height;
	unsigned int dev_fourcc;
	enum v4l2_field field;
	enum v4l2_colorspace colorspace;
	unsigned int depth;
};
struct isp_format {
	struct ovisp_video_format vfmt;
	struct v4l2_fmt_data *fmt_data;
};

struct isp_capture {
	int snapshot;
	struct isp_buffer buf;
	struct ovisp_camera_client *client;
};

struct isp_prop {
	int index;
	int bypass;
};
struct isp_input_parm {
	unsigned int width;
	unsigned int height;
	unsigned int idi_in_width;
	unsigned int idi_in_height;
	unsigned int idi_out_width;
	unsigned int idi_out_height;
	unsigned int format;
	unsigned int sequence;
	unsigned int addrnums;
	unsigned int addroff[3];
};
struct isp_output_parm{
	unsigned int		width;
	unsigned int		height;
	unsigned int 		format;
	unsigned int 		fourcc;
	enum v4l2_field		field;
	unsigned int 		depth;
	unsigned int		bytesperline;	/* for padding, zero if unused */
	unsigned int		sizeimage;
	enum v4l2_colorspace	colorspace;
	unsigned int		addrnums;
	unsigned int		addroff[3];
};
struct isp_parm {
	int contrast;
	int effects;
	int flicker;
	int brightness;
	int flash_mode;
	int focus_mode;
	int iso;
	int exposure;
	int auto_exposure;
	int gain;
	int saturation;
	int scene_mode;
	int sharpness;
	int white_balance;
	int auto_white_balance; // xhshen
	int zoom;
	int hflip;
	int vflip;
	int frame_rate;
	int crop_x;
	int crop_y;
	int crop_width;
	int crop_height;
	int ratio_d;
	int ratio_dcw;
	int ratio_up;
	int dowscaleFlag;
	int dcwFlag;
	int vts;
	struct isp_input_parm input;
	int out_videos;
	int c_video;
	struct isp_output_parm output[2];
};

struct isp_ops {
	int (*init)(struct isp_device *, void *);
	int (*release)(struct isp_device *, void *);
	int (*open)(struct isp_device *, struct isp_prop *);
	int (*close)(struct isp_device *, struct isp_prop *);
	int (*config)(struct isp_device *, void *);
	int (*suspend)(struct isp_device *, void *);
	int (*resume)(struct isp_device *, void *);
	int (*mclk_on)(struct isp_device *, int);
	int (*mclk_off)(struct isp_device *, int);
	int (*offline_process)(struct isp_device *, void *, void*);
	int (*start_capture)(struct isp_device *, struct isp_capture *);
	int (*stop_capture)(struct isp_device *, void *);
	int (*enable_capture)(struct isp_device *, struct isp_buffer *);
	int (*disable_capture)(struct isp_device *, void *);
	int (*update_buffer)(struct isp_device *, struct isp_buffer *, int);
	int (*check_fmt)(struct isp_device *, struct isp_format *);
	int (*try_fmt)(struct isp_device *, struct isp_format *);
	int (*pre_fmt)(struct isp_device *, struct isp_format *);
	int (*s_fmt)(struct isp_device *, struct isp_format *);
	int (*g_fmt)(struct isp_device *, struct v4l2_format *f);
	int (*g_size)(struct isp_device *, unsigned long *);
	int (*g_devfmt)(struct isp_device *, struct isp_format *f);
	int (*g_outinfo)(struct isp_device *, unsigned char *);
	int (*s_ctrl)(struct isp_device *, struct v4l2_control *);
	int (*g_ctrl)(struct isp_device *, struct v4l2_control *);
	int (*s_parm)(struct isp_device *, struct v4l2_streamparm *);
	int (*g_parm)(struct isp_device *, struct v4l2_streamparm *);
	int (*save_flags)(struct isp_device *, struct isp_format *f);
	int (*restore_flags)(struct isp_device *, struct isp_format *f);
	int (*process_raw)(struct isp_device *, struct isp_format *, unsigned int, unsigned int, unsigned char *);
	int (*bypass_capture)(struct isp_device *, struct isp_format *, unsigned int);
	int (*tlb_deinit)(struct isp_device *);
	int (*tlb_map_one_vaddr)(struct isp_device *, unsigned int, unsigned int);
	int (*tlb_unmap_all_vaddr)(struct isp_device *);
};

struct isp_i2c_cmd {
	unsigned int flags;
	unsigned char addr;
	unsigned short reg;
	unsigned short data;
};

struct isp_i2c {
	struct mutex lock;
	struct i2c_adapter adap;
	struct ovisp_i2c_platform_data *pdata;
	int (*xfer_cmd)(struct isp_device *, struct isp_i2c_cmd *);
};

struct isp_debug{
	int settingfile_loaded;
	int status;
};

typedef union isp_intc_register {
	struct isp_intc_bits{
		unsigned int	c0:8;
		unsigned int	c1:8;
		unsigned int	c2:8;
		unsigned int	c3:8;
	} bits;
	unsigned int intc;
} isp_intc_regs_t;

struct isp_device {
	struct device *dev;
	struct resource *res;
	void __iomem *base;
	struct isp_ops *ops;
	struct isp_i2c i2c;
	struct isp_parm parm;
	struct isp_reg_t preview[100];
	struct isp_reg_t captureraw[100];
	struct isp_reg_t capture[100];
	struct completion completion;
	struct completion bracket_capture;
	struct completion frame_eof;
	int clk_enable[ISP_CLK_NUM];
	struct clk *clk[ISP_CLK_NUM];
	struct v4l2_fmt_data fmt_data;
	struct ovisp_camera_client *client;
	struct ovisp_camera_platform_data *pdata;
	int (*irq_notify)(unsigned int, void *);
	void *data;
	spinlock_t lock;
	isp_intc_regs_t intr;
	isp_intc_regs_t mac_intr;
	int format_active;
	int snapshot;
	int running;
	int poweron;

	/*csi power*/
	struct regulator * csi_power;
	int boot;
	int input;
	int irq;
	bool bypass;
	bool first_init;
	bool wait_eof;
	bool capture_raw_enable;
//	bool pp_buf;
	struct isp_buffer buf_start;
	struct isp_debug debug;
	unsigned char bracket_end;
	int hdr_mode;
	int MaxExp;
	int hdr[8];
	int tlb_flag; // 0 disable; 1 enble
};


int isp_device_init(struct isp_device* isp);
int isp_device_release(struct isp_device* isp);

/* add settting of isp's function by wqyan */
typedef struct __isp_setting {
	unsigned int offset;
	unsigned char val;
} OV_CALIBRATION_SETTING;

#define OV_CALIBRATION_SETTING_END 0xffff

#endif/*__OVISP_ISP_H__*/
