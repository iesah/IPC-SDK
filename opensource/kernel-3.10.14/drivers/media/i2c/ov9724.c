#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
//#include <media/ov9724.h>
#include <mach/ovisp-v4l2.h>
#include "ov9724.h"


#define OV9724_CHIP_ID_H	(0x97)
#define OV9724_CHIP_ID_L	(0x24)

#define MAX_WIDTH		1280
#define MAX_HEIGHT		720

#define OV9724_REG_END		0xffff
#define OV9724_REG_DELAY	0xfffe

//#define OV9724_YUV

struct ov9724_format_struct;
struct ov9724_info {
	struct v4l2_subdev sd;
	struct ov9724_format_struct *fmt;
	struct ov9724_win_setting *win;
};

struct regval_list {
	unsigned short reg_num;
	unsigned char value;
};
static struct regval_list ov9724_stream_on[] = {
	{0x0100, 0x01},

	{OV9724_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ov9724_stream_off[] = {
	/* Sensor enter LP11*/
	{0x0100, 0x00},

	{OV9724_REG_END, 0x00},	/* END MARKER */
};
static struct regval_list ov9724_init_720p_raw10_regs[] = {
	{0x0103,0x01},
	{0x3210,0x43},
	{0x5780,0x3e},
	{0x3606,0x75},
	{0x3705,0x41},
	{0x3601,0x34},
	{0x3607,0x94},
	{0x3608,0x38},
	{0x3712,0xb4},
	{0x370d,0xcc},
	{0x4010,0x08},
	{0x4000,0x01},
	// VTS
	{0x0340,0x03},
	{0x0341,0x20},
	{0x0342,0x06},
	{0x0343,0x28},
/*
	// VTL HTL
	{0x0340,0x02},
	{0x0341,0xe8},
	{0x0342,0x06},
	{0x0343,0x18},
*/
	{0x0202,0x04},
	{0x0203,0xf8},
	{0x4801,0x0f},
	{0x4801,0x8f},
	{0x4814,0x2b},
	// flip
	{0x0101,0x02},
/*
	{0x0347,0x01},
	{0x034b,0xce},
*/
	{0x5110,0x09},
	{0x4307,0x3a},
	{0x5000,0x06},
	{0x5001,0x73},//m awb

	// awb
	/*{0x0202,0x1f},
	{0x0203,0xff},*/
	{0x0205,0x10},

	// color bar
	/*{0x0601,0x02},*/

	// stream on
	/*{0x0100,0x01},*/
	{OV9724_REG_END, 0x00},	/* END MARKER */
};
static struct ov9724_format_struct {
	enum v4l2_mbus_pixelcode mbus_code;
	enum v4l2_colorspace colorspace;
} ov9724_formats[] = {
	{
		/*RAW10 FORMAT, 10 bit per pixel*/
		.mbus_code	= V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace	= V4L2_COLORSPACE_SRGB,
	},
	/*add other format supported*/
};
#define N_OV9724_FMTS ARRAY_SIZE(ov9724_formats)

static struct ov9724_win_setting {
	int	width;
	int	height;
	enum v4l2_mbus_pixelcode mbus_code;
	enum v4l2_colorspace colorspace;
	struct regval_list *regs; /* Regs to tweak */
} ov9724_win_sizes[] = {
	/* 1280*800 */
	{
		.width		= 1280,
		.height		= 720,
		.mbus_code	= V4L2_MBUS_FMT_SGRBG10_1X10,
		.colorspace	= V4L2_COLORSPACE_SRGB,
		.regs 		= ov9724_init_720p_raw10_regs,
	}
};
#define N_WIN_SIZES (ARRAY_SIZE(ov9724_win_sizes))

int ov9724_read(struct v4l2_subdev *sd, unsigned short reg,
		unsigned char *value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[2] = {reg >> 8, reg & 0xff};
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 2,
			.buf	= buf,
		},
		[1] = {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= value,
		}
	};
	int ret;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}


static int ov9724_write(struct v4l2_subdev *sd, unsigned short reg,
		unsigned char value)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char buf[3] = {reg >> 8, reg & 0xff, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= buf,
	};
	int ret;

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}


static int ov9724_read_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != OV9724_REG_END) {
		if (vals->reg_num == OV9724_REG_DELAY) {
			if (vals->value >= (1000 / HZ))
				msleep(vals->value);
			else
				mdelay(vals->value);
		} else {
			ret = ov9724_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
		printk("vals->reg_num:%x, vals->value:%x\n",vals->reg_num, val);
		//mdelay(200);
		vals++;
	}
	//printk("vals->reg_num:%x, vals->value:%x\n", vals->reg_num, vals->value);
	ov9724_write(sd, vals->reg_num, vals->value);
	return 0;
}
static int ov9724_write_array(struct v4l2_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != OV9724_REG_END) {
		if (vals->reg_num == OV9724_REG_DELAY) {
			if (vals->value >= (1000 / HZ))
				msleep(vals->value);
			else
				mdelay(vals->value);
		} else {
			ret = ov9724_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		//printk("vals->reg_num:%x, vals->value:%x\n",vals->reg_num, vals->value);
		//mdelay(200);
		vals++;
	}
	//printk("vals->reg_num:%x, vals->value:%x\n", vals->reg_num, vals->value);
	//ov9724_write(sd, vals->reg_num, vals->value);
	return 0;
}

/* R/G and B/G of typical camera module is defined here. */
static unsigned int rg_ratio_typical = 0x58;
static unsigned int bg_ratio_typical = 0x5a;

static int ov9724_update_awb_gain(struct v4l2_subdev *sd,
				unsigned int R_gain, unsigned int G_gain, unsigned int B_gain)
{
	return 0;
}

static int ov9724_reset(struct v4l2_subdev *sd, u32 val)
{
	return 0;
}

static int ov9724_detect(struct v4l2_subdev *sd);
static int ov9724_init(struct v4l2_subdev *sd, u32 val)
{
	struct ov9724_info *info = container_of(sd, struct ov9724_info, sd);
	int ret = 0;

	info->fmt = NULL;
	info->win = NULL;

	if (ret < 0)
		return ret;

	return 0;
}
static int ov9724_get_sensor_vts(struct v4l2_subdev *sd, unsigned short *value)
{
	unsigned char h,l;
	int ret = 0;
	ret = ov9724_read(sd, 0x0340, &h);
	if (ret < 0)
		return ret;
	ret = ov9724_read(sd, 0x0341, &l);
	if (ret < 0)
		return ret;
	*value = h;
	*value = (*value << 8) | (l);
	return ret;
}

static int ov9724_get_sensor_lans(struct v4l2_subdev *sd, unsigned char *value)
{
	*value = 1;
	return 0;
}
static int ov9724_detect(struct v4l2_subdev *sd)
{
	unsigned char v;
	int ret;

	ret = ov9724_read(sd, 0x0000, &v);
	if (ret < 0)
		return ret;

	if (v != OV9724_CHIP_ID_H)
		return -ENODEV;
	ret = ov9724_read(sd, 0x0001, &v);
	if (ret < 0)
		return ret;
	if (v != OV9724_CHIP_ID_L)
		return -ENODEV;
	return 0;
}

static int ov9724_enum_mbus_fmt(struct v4l2_subdev *sd, unsigned index,
					enum v4l2_mbus_pixelcode *code)
{
	if (index >= N_OV9724_FMTS)
		return -EINVAL;

	*code = ov9724_formats[index].mbus_code;
	return 0;
}

static int ov9724_try_fmt_internal(struct v4l2_subdev *sd,
		struct v4l2_mbus_framefmt *fmt,
		struct ov9724_win_setting **ret_wsize)
{
	struct ov9724_win_setting *wsize;

	if(fmt->width > MAX_WIDTH || fmt->height > MAX_HEIGHT)
		return -EINVAL;
	for (wsize = ov9724_win_sizes; wsize < ov9724_win_sizes + N_WIN_SIZES;
	     wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;
	if (wsize >= ov9724_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;
	fmt->width = wsize->width;
	fmt->height = wsize->height;
	fmt->code = wsize->mbus_code;
	fmt->field = V4L2_FIELD_NONE;
	fmt->colorspace = wsize->colorspace;
	return 0;
}

static int ov9724_try_mbus_fmt(struct v4l2_subdev *sd,
			    struct v4l2_mbus_framefmt *fmt)
{
	return ov9724_try_fmt_internal(sd, fmt, NULL);
}

static int ov9724_s_mbus_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *fmt)
{
	struct ov9724_info *info = container_of(sd, struct ov9724_info, sd);
	struct v4l2_fmt_data *data = v4l2_get_fmt_data(fmt);
	struct ov9724_win_setting *wsize;
	int ret;

	ret = ov9724_try_fmt_internal(sd, fmt, &wsize);
	if (ret)
		return ret;
	if ((info->win != wsize) && wsize->regs) {
		ret = ov9724_write_array(sd, wsize->regs);
		if (ret)
			return ret;
	}
	data->i2cflags = 0;
	data->mipi_clk = 282;
	ret = ov9724_get_sensor_vts(sd, &(data->vts));
	if(ret < 0)
		return ret;

	ret = ov9724_get_sensor_lans(sd, &(data->lans));
	if(ret < 0)
		return ret;

	info->win = wsize;

	return 0;
}

static int ov9724_s_stream(struct v4l2_subdev *sd, int enable)
{
	int ret = 0;

	if (enable) {
		ret = ov9724_write_array(sd, ov9724_stream_on);
		printk("ov9724 stream on\n");
	}
	else {
		ret = ov9724_write_array(sd, ov9724_stream_off);
		printk("ov9724 stream off\n");
	}
	return ret;
}

static int ov9724_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	a->c.left	= 0;
	a->c.top	= 0;
	a->c.width	= MAX_WIDTH;
	a->c.height	= MAX_HEIGHT;
	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

static int ov9724_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= MAX_WIDTH;
	a->bounds.height		= MAX_HEIGHT;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	return 0;
}

static int ov9724_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int ov9724_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	return 0;
}

static int ov9724_frame_rates[] = { 30, 15, 10, 5, 1 };

static int ov9724_enum_frameintervals(struct v4l2_subdev *sd,
		struct v4l2_frmivalenum *interval)
{
	if (interval->index >= ARRAY_SIZE(ov9724_frame_rates))
		return -EINVAL;
	interval->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	interval->discrete.numerator = 1;
	interval->discrete.denominator = ov9724_frame_rates[interval->index];
	return 0;
}

static int ov9724_enum_framesizes(struct v4l2_subdev *sd,
		struct v4l2_frmsizeenum *fsize)
{
	int i;
	int num_valid = -1;
	__u32 index = fsize->index;

	/*
	 * If a minimum width/height was requested, filter out the capture
	 * windows that fall outside that.
	 */
	for (i = 0; i < N_WIN_SIZES; i++) {
		struct ov9724_win_setting *win = &ov9724_win_sizes[index];
		if (index == ++num_valid) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width = win->width;
			fsize->discrete.height = win->height;
			return 0;
		}
	}

	return -EINVAL;
}

static int ov9724_queryctrl(struct v4l2_subdev *sd,
		struct v4l2_queryctrl *qc)
{
	return 0;
}

static int ov9724_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	return 0;
}

static int ov9724_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	return 0;
}

static int ov9724_g_chip_ident(struct v4l2_subdev *sd,
		struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

//	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_OV9724, 0);
	return v4l2_chip_ident_i2c_client(client, chip, 123, 0);
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov9724_g_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	unsigned char val = 0;
	int ret;

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = ov9724_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;
	return ret;
}

static int ov9724_s_register(struct v4l2_subdev *sd, struct v4l2_dbg_register *reg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	if (!v4l2_chip_match_i2c_client(client, &reg->match))
		return -EINVAL;
	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;
	ov9724_write(sd, reg->reg & 0xffff, reg->val & 0xff);
	return 0;
}
#endif

static int ov9724_s_power(struct v4l2_subdev *sd, int on)
{
	return 0;
}

static const struct v4l2_subdev_core_ops ov9724_core_ops = {
	.g_chip_ident = ov9724_g_chip_ident,
	.g_ctrl = ov9724_g_ctrl,
	.s_ctrl = ov9724_s_ctrl,
	.queryctrl = ov9724_queryctrl,
	.reset = ov9724_reset,
	.init = ov9724_init,
	.s_power = ov9724_s_power,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.g_register = ov9724_g_register,
	.s_register = ov9724_s_register,
#endif
};

static const struct v4l2_subdev_video_ops ov9724_video_ops = {
	.enum_mbus_fmt = ov9724_enum_mbus_fmt,
	.try_mbus_fmt = ov9724_try_mbus_fmt,
	.s_mbus_fmt = ov9724_s_mbus_fmt,
	.s_stream = ov9724_s_stream,
	.cropcap = ov9724_cropcap,
	.g_crop	= ov9724_g_crop,
	.s_parm = ov9724_s_parm,
	.g_parm = ov9724_g_parm,
	.enum_frameintervals = ov9724_enum_frameintervals,
	.enum_framesizes = ov9724_enum_framesizes,
};

static const struct v4l2_subdev_ops ov9724_ops = {
	.core = &ov9724_core_ops,
	.video = &ov9724_video_ops,
};

static ssize_t ov9724_rg_ratio_typical_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", rg_ratio_typical);
}

static ssize_t ov9724_rg_ratio_typical_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	char *endp;
	int value;

	value = simple_strtoul(buf, &endp, 0);
	if (buf == endp)
		return -EINVAL;

	rg_ratio_typical = (unsigned int)value;

	return size;
}

static ssize_t ov9724_bg_ratio_typical_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d", bg_ratio_typical);
}

static ssize_t ov9724_bg_ratio_typical_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	char *endp;
	int value;

	value = simple_strtoul(buf, &endp, 0);
	if (buf == endp)
		return -EINVAL;

	bg_ratio_typical = (unsigned int)value;

	return size;
}

static DEVICE_ATTR(ov9724_rg_ratio_typical, 0664, ov9724_rg_ratio_typical_show, ov9724_rg_ratio_typical_store);
static DEVICE_ATTR(ov9724_bg_ratio_typical, 0664, ov9724_bg_ratio_typical_show, ov9724_bg_ratio_typical_store);

static int ov9724_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct ov9724_info *info;
	int ret;

	printk("---- in the ov9724 init ----\n");
	info = kzalloc(sizeof(struct ov9724_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	v4l2_i2c_subdev_init(sd, client, &ov9724_ops);

	/* Make sure it's an ov9724 */
//aaa:
	ret = ov9724_detect(sd);
	if (ret) {
		v4l_err(client,
			"chip found @ 0x%x (%s) is not an ov9724 chip.\n",
			client->addr, client->adapter->name);
//		goto aaa;
		kfree(info);
		return ret;
	}
	v4l_info(client, "ov9724 chip found @ 0x%02x (%s)\n",
			client->addr, client->adapter->name);

	ret = device_create_file(&client->dev, &dev_attr_ov9724_rg_ratio_typical);
	if(ret){
		v4l_err(client, "create dev_attr_ov9724_rg_ratio_typical failed!\n");
		goto err_create_dev_attr_ov9724_rg_ratio_typical;
	}

	ret = device_create_file(&client->dev, &dev_attr_ov9724_bg_ratio_typical);
	if(ret){
		v4l_err(client, "create dev_attr_ov9724_bg_ratio_typical failed!\n");
		goto err_create_dev_attr_ov9724_bg_ratio_typical;
	}

	printk("probe ok ------->ov9724\n");
	return 0;

err_create_dev_attr_ov9724_bg_ratio_typical:
	device_remove_file(&client->dev, &dev_attr_ov9724_rg_ratio_typical);
err_create_dev_attr_ov9724_rg_ratio_typical:
	kfree(info);
	return ret;
}

static int ov9724_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct ov9724_info *info = container_of(sd, struct ov9724_info, sd);

	device_remove_file(&client->dev, &dev_attr_ov9724_rg_ratio_typical);
	device_remove_file(&client->dev, &dev_attr_ov9724_bg_ratio_typical);

	v4l2_device_unregister_subdev(sd);
	kfree(info);
	return 0;
}

static const struct i2c_device_id ov9724_id[] = {
	{ "ov9724", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ov9724_id);

static struct i2c_driver ov9724_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ov9724",
	},
	.probe		= ov9724_probe,
	.remove		= ov9724_remove,
	.id_table	= ov9724_id,
};

static __init int init_ov9724(void)
{
	return i2c_add_driver(&ov9724_driver);
}

static __exit void exit_ov9724(void)
{
	i2c_del_driver(&ov9724_driver);
}

module_init(init_ov9724);
module_exit(exit_ov9724);

MODULE_DESCRIPTION("A low-level driver for OmniVision ov9724 sensors");
MODULE_LICENSE("GPL");
