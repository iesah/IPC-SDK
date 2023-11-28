/*
 * ps5258.c
 *
 * Copyright (C) 2012 Ingenic Semiconductor Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/proc_fs.h>
#include <soc/gpio.h>

#include <tx-isp-common.h>
#include <sensor-common.h>

#define PS5258_CHIP_ID_H	(0x52)
#define PS5258_CHIP_ID_L	(0x58)
#define PS5258_REG_END		0xffff
#define PS5258_REG_DELAY	0xfffe
#define PS5258_SUPPORT_SCLK_FPS_25 (81000000)
#define SENSOR_OUTPUT_MAX_FPS 30
#define SENSOR_OUTPUT_MIN_FPS 5
#define DRIVE_CAPABILITY_1
#define SENSOR_VERSION	"H20201221a"

static int reset_gpio = GPIO_PC(28);
module_param(reset_gpio, int, S_IRUGO);
MODULE_PARM_DESC(reset_gpio, "Reset GPIO NUM");

static int pwdn_gpio = -1;
module_param(pwdn_gpio, int, S_IRUGO);
MODULE_PARM_DESC(pwdn_gpio, "Power down GPIO NUM");


struct regval_list {
	uint16_t reg_num;
	unsigned char value;
};

/*
 * the part of driver maybe modify about different sensor and different board.
 */
struct again_lut {
	unsigned int value;
	unsigned int gain;
};

struct again_lut ps5258_again_lut[] = {
	{0, 0},
	{1, 5731},
	{2, 11136},
	{3, 16247},
	/* start frome 1.25x */
	{4, 21097},
	{5, 25710},
	{6, 30108},
	{7, 34311},
	{8, 38335},
	{9, 42195},
	{10, 45903},
	{11, 49471},
	{12, 52910},
	{13, 56227},
	{14, 59433},
	{15, 62533},
	{16, 65535},
	{17, 71266},
	{18, 76671},
	{19, 81782},
	{20, 86632},
	{21, 91245},
	{22, 95643},
	{23, 99846},
	{24, 103870},
	{25, 107730},
	{26, 111438},
	{27, 115006},
	{28, 118445},
	{29, 121762},
	{30, 124968},
	{31, 128068},
	{32, 131070},
	{33, 136801},
	{34, 142206},
	{35, 147317},
	{36, 152167},
	{37, 156780},
	{38, 161178},
	{39, 165381},
	{40, 169405},
	{41, 173265},
	{42, 176973},
	{43, 180541},
	{44, 183980},
	{45, 187297},
	{46, 190503},
	{47, 193603},
	{48, 196605},
	{49, 202336},
	{50, 207741},
	{51, 212852},
	{52, 217702},
	{53, 222315},
	{54, 226713},
	{55, 230916},
	{56, 234940},
	{57, 238800},
	{58, 242508},
	{59, 246076},
	{60, 249515},
	{61, 252832},
	{62, 256038},
	{63, 259138},
	{64, 262140},
	{65, 267871},
	{66, 273276},
	{67, 278387},
	{68, 283237},
	{69, 287850},
	{70, 292248},
	{71, 296451},
	{72, 300475},
	{73, 304335},
	{74, 308043},
	{75, 311611},
	{76, 315050},
	{77, 318367},
	{78, 321573},
	{79, 324673},
	{80, 327675},
};

struct tx_isp_sensor_attribute ps5258_attr;

unsigned int ps5258_alloc_again(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_again)
{
	struct again_lut *lut = ps5258_again_lut;
	while(lut->gain <= ps5258_attr.max_again) {
		if(isp_gain == 0) {
			*sensor_again = lut[0].value;
			return lut[0].gain;
		}
		else if(isp_gain < lut->gain) {
			*sensor_again = (lut - 1)->value;
			return (lut - 1)->gain;
		}
		else{
			if((lut->gain == ps5258_attr.max_again) && (isp_gain >= lut->gain)) {
				*sensor_again = lut->value;
				return lut->gain;
			}
		}

		lut++;
	}

	return isp_gain;
}

unsigned int ps5258_alloc_dgain(unsigned int isp_gain, unsigned char shift, unsigned int *sensor_dgain)
{
	return 0;
}

struct tx_isp_sensor_attribute ps5258_attr={
	.name = "ps5258",
	.chip_id = 0x5258,
	.cbus_type = TX_SENSOR_CONTROL_INTERFACE_I2C,
	.cbus_mask = TISP_SBUS_MASK_SAMPLE_8BITS | TISP_SBUS_MASK_ADDR_16BITS,
	.cbus_device = 0x48,
	.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI,
	.mipi = {
		.mode = SENSOR_MIPI_OTHER_MODE,
		.clk = 600,
		.lans = 2,
		.settle_time_apative_en = 0,
		.mipi_sc.sensor_csi_fmt = TX_SENSOR_RAW10,//RAW10
		.mipi_sc.hcrop_diff_en = 0,
		.mipi_sc.mipi_vcomp_en = 0,
		.mipi_sc.mipi_hcomp_en = 0,
		.mipi_sc.line_sync_mode = 0,
		.mipi_sc.work_start_flag = 0,
		.image_twidth = 1920,
		.image_theight = 1080,
		.mipi_sc.mipi_crop_start0x = 0,
		.mipi_sc.mipi_crop_start0y = 0,
		.mipi_sc.mipi_crop_start1x = 0,
		.mipi_sc.mipi_crop_start1y = 0,
		.mipi_sc.mipi_crop_start2x = 0,
		.mipi_sc.mipi_crop_start2y = 0,
		.mipi_sc.mipi_crop_start3x = 0,
		.mipi_sc.mipi_crop_start3y = 0,
		.mipi_sc.data_type_en = 0,
		.mipi_sc.data_type_value = RAW10,
		.mipi_sc.del_start = 0,
		.mipi_sc.sensor_frame_mode = TX_SENSOR_DEFAULT_FRAME_MODE,
		.mipi_sc.sensor_fid_mode = 0,
		.mipi_sc.sensor_mode = TX_SENSOR_DEFAULT_MODE,
	},
	.max_again = 327675,
	.max_dgain = 0,
	.min_integration_time = 2,
	.min_integration_time_native = 2,
	.max_integration_time_native = 1125 - 1,
	.integration_time_limit = 1125 - 1,
	.total_width = 2400,
	.total_height = 1125,
	.max_integration_time = 1125 - 1,
	.integration_time_apply_delay = 2,
	.again_apply_delay = 2,
	.dgain_apply_delay = 0,
	.sensor_ctrl.alloc_again = ps5258_alloc_again,
	.sensor_ctrl.alloc_dgain = ps5258_alloc_dgain,
};


static struct regval_list ps5258_init_regs_2560_1920_15fps_mipi[] = {
        {0x010B, 0x07},//Cmd_Sw_TriState[0]=1
        {0x0114, 0x09},//Cmd_LineTime[12:0]=2400
        {0x0115, 0x60},//Cmd_LineTime[12:0]=2400
        {0x0178, 0xB0},//B06A: Version
        {0x0179, 0x6A},//B06A: Version
        {0x020A, 0x33},//T_ODACMODE=1, B04A - improve streaking
        {0x020D, 0x01},//T_GDACMODE=1, B04A - improve streaking
        {0x022E, 0x0E},//T_spll_predivider[5:0]=14
        {0x022F, 0x19},//T_spll_postdivider[5:0]=25
        {0x022D, 0x01},//T_spll_enh[0]=1
        {0x021C, 0x00},//T_FAE_CLK_SEL[0]=0
        {0x023C, 0x37},//T_clamp_offset_lvl[2:0]=3, B05A - improve streaking
        {0x0240, 0x15},//T_compf_fast[2:0]=5, B03A - improve RTS noise & LowVol preformance
        {0x0252, 0x16},//T_pll_predivider[5:0]=22
        {0x0254, 0x61},//T_pll_enh[0]=1
        {0x0659, 0x5E},//R_comp_rst_r3[7:0]=94 - improve left/right display uniformity
        {0x0684, 0x00},//R_cout_reset_enl_f1=2, B02A - improve straight line
        {0x0685, 0x02},//R_cout_reset_enl_f1=2, B02A - improve straight line
        {0x069A, 0x00},//Cmd_INTREFHD_enH=0, B03A
        {0x06AC, 0x04},//Cmd_vbt_isel_R_G3[6:0]=4, B04A - improve streaking
        {0x0B02, 0x02},//Cmd_RClkDly_Sel[3:0]=2, B05A
        {0x0B0C, 0x00},//Cmd_MIPI_Clk_Gated[0]=0
        {0x0E0C, 0x04},//Cmd_WOI_VOffset=4
        {0x0E0E, 0x38},//Cmd_WOI_VSize=1080
        {0x0E10, 0x07},//Cmd_WOI_HOffset=4
        {0x0E12, 0x80},//Cmd_WOI_HSize=1920
        {0x145B, 0x10},//R_MIPI_line_num_en=0, R_MIPI_frm_num_en=0, R_MIPI_skip_line_sp=1,  B04A - work-around MIPI display
        {0x14B0, 0x01},//R_MIPI_line_num_en=0, R_MIPI_frm_num_en=0, R_MIPI_skip_line_sp=1,  B04A - work-around MIPI display
        {0x140F, 0x01},//R_CSI2_enable=1
        {0x0116, 0x05},
        {0x0117, 0x46},
        {0x0111, 0x01},//UpdateFlag
        {0x010F, 0x01},//Sensor_EnH=1
	{PS5258_REG_END, 0x00},/* END MARKER */
};


static struct tx_isp_sensor_win_setting ps5258_win_sizes[] = {
	/* 2560*1920 */
	{
		.width		= 1920,
		.height		= 1080,
		.fps		= 25 << 16 | 1,
		.mbus_code	= TISP_VI_FMT_SGBRG10_1X10,
		.colorspace	= TISP_COLORSPACE_SRGB,
		.regs 		= ps5258_init_regs_2560_1920_15fps_mipi,
	}
};

struct tx_isp_sensor_win_setting *wsize = &ps5258_win_sizes[0];

/*
 * the part of driver was fixed.
 */

static struct regval_list ps5258_stream_on_mipi[] = {
	//{0x0100, 0x01},
	{PS5258_REG_END, 0x00},	/* END MARKER */
};

static struct regval_list ps5258_stream_off_mipi[] = {
//	{0x0100, 0x00},
	{PS5258_REG_END, 0x00},	/* END MARKER */
};

int ps5258_read(struct tx_isp_subdev *sd, uint16_t reg,
		unsigned char *value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[2] = {(reg >> 8) & 0xff, reg & 0xff};
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
	ret = private_i2c_transfer(client->adapter, msg, 2);
	if (ret > 0)
		ret = 0;

	return ret;
}

int ps5258_write(struct tx_isp_subdev *sd, uint16_t reg,
		 unsigned char value)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	uint8_t buf[3] = {(reg >> 8) & 0xff, reg & 0xff, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 3,
		.buf	= buf,
	};
	int ret;
	ret = private_i2c_transfer(client->adapter, &msg, 1);
	if (ret > 0)
		ret = 0;

	return ret;
}

#if 0
static int ps5258_read_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	unsigned char val;
	while (vals->reg_num != PS5258_REG_END) {
		if (vals->reg_num == PS5258_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = ps5258_read(sd, vals->reg_num, &val);
			if (ret < 0)
				return ret;
		}
	         pr_debug("vals->reg_num:0x%02x, vals->value:0x%02x\n",vals->reg_num, val);
                vals++;
	}

	return 0;
}
#endif

static int ps5258_write_array(struct tx_isp_subdev *sd, struct regval_list *vals)
{
	int ret;
	while (vals->reg_num != PS5258_REG_END) {
		if (vals->reg_num == PS5258_REG_DELAY) {
			msleep(vals->value);
		} else {
			ret = ps5258_write(sd, vals->reg_num, vals->value);
			if (ret < 0)
				return ret;
		}
		vals++;
	}

	return 0;
}

static int ps5258_reset(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	return 0;
}

static int ps5258_detect(struct tx_isp_subdev *sd, unsigned int *ident)
{
	int ret;
	unsigned char v;

	ret = ps5258_read(sd, 0x0100, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != PS5258_CHIP_ID_H)
		return -ENODEV;
	*ident = v;

	ret = ps5258_read(sd, 0x0101, &v);
	ISP_WARNING("-----%s: %d ret = %d, v = 0x%02x\n", __func__, __LINE__, ret,v);
	if (ret < 0)
		return ret;
	if (v != PS5258_CHIP_ID_L)
		return -ENODEV;
	*ident = (*ident << 8) | v;

	return 0;
}

static int ps5258_set_integration_time(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
        int Cmd_Lpf = ps5258_attr.total_height;
        int Cmd_OffNy;

        Cmd_OffNy = Cmd_Lpf + 1 - value;

        ret = ps5258_write(sd, 0x0118, (unsigned char)(Cmd_OffNy >> 8));
	ret += ps5258_write(sd, 0x0119, (unsigned char)(Cmd_OffNy & 0xff));
	ret += ps5258_write(sd, 0x0111, 0x01);
	if (ret < 0)
		return ret;

	return 0;
}

static int ps5258_set_analog_gain(struct tx_isp_subdev *sd, int value)
{
	int ret = 0;
        unsigned int gain = value;
        //return 0;

	ret = ps5258_write(sd, 0x012b, (unsigned char)(gain & 0xff));
	ret += ps5258_write(sd, 0x0111, 0x01);
	if (ret < 0)
		return ret;
	return 0;
}

static int ps5258_set_digital_gain(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int ps5258_get_black_pedestal(struct tx_isp_subdev *sd, int value)
{
	return 0;
}

static int ps5258_init(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = 0;

	if(!init->enable)
		return ISP_SUCCESS;

	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	sensor->video.state = TX_ISP_MODULE_DEINIT;

	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	sensor->priv = wsize;

	return 0;
}

static int ps5258_s_stream(struct tx_isp_subdev *sd, struct tx_isp_initarg *init)
{
	int ret = 0;
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);

	if (init->enable) {
	    if(sensor->video.state == TX_ISP_MODULE_DEINIT){
            ret = ps5258_write_array(sd, wsize->regs);
            if (ret)
                return ret;
            sensor->video.state = TX_ISP_MODULE_INIT;
	    }
	    if(sensor->video.state == TX_ISP_MODULE_INIT){
            ret = ps5258_write_array(sd, ps5258_stream_on_mipi);
            ISP_WARNING("ps5258 stream on\n");
            sensor->video.state = TX_ISP_MODULE_RUNNING;
	    }
	}
	else {
		ret = ps5258_write_array(sd, ps5258_stream_off_mipi);
		ISP_WARNING("ps5258 stream off\n");
		sensor->video.state = TX_ISP_MODULE_DEINIT;
	}

	return ret;
}

static int ps5258_set_fps(struct tx_isp_subdev *sd, int fps)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	unsigned int sclk = PS5258_SUPPORT_SCLK_FPS_25;
	unsigned int hts = 0;
	unsigned int vts = 0;
	unsigned char val = 0;
	unsigned int newformat = 0; //the format is 24.8
	int ret = 0;
	newformat = (((fps >> 16) / (fps & 0xffff)) << 8) + ((((fps >> 16) % (fps & 0xffff)) << 8) / (fps & 0xffff));
	if(newformat > (SENSOR_OUTPUT_MAX_FPS << 8) || newformat < (SENSOR_OUTPUT_MIN_FPS << 8)) {
		ISP_ERROR("warn: fps(%d) no in range\n", fps);
		return -1;
	}

	ret += ps5258_read(sd, 0x0114, &val);
	hts = val << 8;
	ret += ps5258_read(sd, 0x0115, &val);
	hts = (hts | val) - 1;
	if (0 != ret) {
		ISP_ERROR("err: ps5258 read err\n");
		return -1;
	}

	vts = sclk * (fps & 0xffff) / hts / ((fps & 0xffff0000) >> 16);
	ret = ps5258_write(sd, 0x0117, (unsigned char)(vts & 0xff));
	ret += ps5258_write(sd, 0x0116, (unsigned char)(vts >> 8));
	ret += ps5258_write(sd, 0x0111, 0x01);
	if (0 != ret) {
		ISP_ERROR("err: ps5258_write err\n");
		return ret;
	}
	sensor->video.fps = fps;
	sensor->video.attr->max_integration_time_native = vts - 1;
	sensor->video.attr->integration_time_limit = vts - 1;
	sensor->video.attr->total_height = vts;
	sensor->video.attr->max_integration_time = vts - 1;
	ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);

	return ret;
}

static int ps5258_set_mode(struct tx_isp_subdev *sd, int value)
{
	struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
	int ret = ISP_SUCCESS;

	if(wsize){
		sensor->video.mbus.width = wsize->width;
		sensor->video.mbus.height = wsize->height;
		sensor->video.mbus.code = wsize->mbus_code;
		sensor->video.mbus.field = TISP_FIELD_NONE;
		sensor->video.mbus.colorspace = wsize->colorspace;
		sensor->video.fps = wsize->fps;
		ret = tx_isp_call_subdev_notify(sd, TX_ISP_EVENT_SYNC_SENSOR_ATTR, &sensor->video);
	}

	return ret;
}

static int sensor_attr_check(struct tx_isp_subdev *sd){
    struct tx_isp_sensor *sensor = sd_to_sensor_device(sd);
    struct tx_isp_sensor_register_info *info = &sensor->info;

    switch(info->default_boot){
        case 0:

            break;
        default:
            ISP_ERROR("Have no this setting!!!\n");
    }

    switch(info->video_interface){
        case TISP_SENSOR_VI_MIPI_CSI0:
            ps5258_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            ps5258_attr.mipi.index = 0;
            break;
        case TISP_SENSOR_VI_MIPI_CSI1:
            ps5258_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_MIPI;
            ps5258_attr.mipi.index = 1;
            break;
        case TISP_SENSOR_VI_DVP:
            ps5258_attr.dbus_type = TX_SENSOR_DATA_INTERFACE_DVP;
            break;
        default:
            ISP_ERROR("Have no this interface!!!\n");
    }

    switch(info->mclk){
        case TISP_SENSOR_MCLK0:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim0");
            set_sensor_mclk_function(0);
            break;
        case TISP_SENSOR_MCLK1:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim1");
            set_sensor_mclk_function(1);
            break;
        case TISP_SENSOR_MCLK2:
            sensor->mclk = private_devm_clk_get(sensor->dev, "div_cim2");
            set_sensor_mclk_function(2);
            break;
        default:
            ISP_ERROR("Have no this MCLK Source!!!\n");
    }

    if (IS_ERR(sensor->mclk)) {
        ISP_ERROR("Cannot get sensor input clock cgu_cim\n");
        goto err_get_mclk;
    }
    private_clk_set_rate(sensor->mclk, 24000000);
    private_clk_prepare_enable(sensor->mclk);

    reset_gpio = info->rst_gpio;
    pwdn_gpio = info->pwdn_gpio;

    return 0;

    err_get_mclk:
    return -1;
}

static int ps5258_g_chip_ident(struct tx_isp_subdev *sd,
			       struct tx_isp_chip_ident *chip)
{
	struct i2c_client *client = tx_isp_get_subdevdata(sd);
	unsigned int ident = 0;
	int ret = ISP_SUCCESS;

	sensor_attr_check(sd);
	if(reset_gpio != -1){
		ret = private_gpio_request(reset_gpio,"ps5258_reset");
		if(!ret){
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 0);
			private_msleep(5);
			private_gpio_direction_output(reset_gpio, 1);
			private_msleep(5);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",reset_gpio);
		}
	}
	if(pwdn_gpio != -1){
		ret = private_gpio_request(pwdn_gpio,"ps5258_pwdn");
		if(!ret){
			private_gpio_direction_output(pwdn_gpio, 0);
			private_msleep(5);
			private_gpio_direction_output(pwdn_gpio, 1);
			private_msleep(5);
		}else{
			ISP_ERROR("gpio requrest fail %d\n",pwdn_gpio);
		}
	}
	ret = ps5258_detect(sd, &ident);
	if (ret) {
		ISP_ERROR("chip found @ 0x%x (%s) is not an ps5258 chip.\n",
		       client->addr, client->adapter->name);
		return ret;
	}
	ISP_WARNING("ps5258 chip found @ 0x%02x (%s)\n", client->addr, client->adapter->name);
	ISP_WARNING("sensor driver version %s\n",SENSOR_VERSION);
	if(chip){
		memcpy(chip->name, "ps5258", sizeof("ps5258"));
		chip->ident = ident;
		chip->revision = SENSOR_VERSION;
	}

	return 0;
}

static int ps5258_sensor_ops_ioctl(struct tx_isp_subdev *sd, unsigned int cmd, void *arg)
{
	long ret = 0;
    struct tx_isp_sensor_value *sensor_val = arg;

	if(IS_ERR_OR_NULL(sd)){
		ISP_ERROR("[%d]The pointer is invalid!\n", __LINE__);
		return -EINVAL;
	}
	switch(cmd){
	case TX_ISP_EVENT_SENSOR_EXPO:
//		if(arg)
//			ret = ps5258_set_expo(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_INT_TIME:
		if(arg)
			ret = ps5258_set_integration_time(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_AGAIN:
		if(arg)
			ret = ps5258_set_analog_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_DGAIN:
		if(arg)
			ret = ps5258_set_digital_gain(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_BLACK_LEVEL:
		if(arg)
			ret = ps5258_get_black_pedestal(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_RESIZE:
		if(arg)
			ret = ps5258_set_mode(sd, sensor_val->value);
		break;
	case TX_ISP_EVENT_SENSOR_PREPARE_CHANGE:
		ret = ps5258_write_array(sd, ps5258_stream_off_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FINISH_CHANGE:
		ret = ps5258_write_array(sd, ps5258_stream_on_mipi);
		break;
	case TX_ISP_EVENT_SENSOR_FPS:
		if(arg)
			ret = ps5258_set_fps(sd, sensor_val->value);
		break;
	default:
		break;
	}

	return ret;
}

static int ps5258_g_register(struct tx_isp_subdev *sd, struct tx_isp_dbg_register *reg)
{
	unsigned char val = 0;
	int len = 0;
	int ret = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	ret = ps5258_read(sd, reg->reg & 0xffff, &val);
	reg->val = val;
	reg->size = 2;

	return ret;
}

static int ps5258_s_register(struct tx_isp_subdev *sd, const struct tx_isp_dbg_register *reg)
{
	int len = 0;

	len = strlen(sd->chip.name);
	if(len && strncmp(sd->chip.name, reg->name, len)){
		return -EINVAL;
	}
	if (!private_capable(CAP_SYS_ADMIN))
		return -EPERM;
	ps5258_write(sd, reg->reg & 0xffff, reg->val & 0xff);

	return 0;
}

static struct tx_isp_subdev_core_ops ps5258_core_ops = {
	.g_chip_ident = ps5258_g_chip_ident,
	.reset = ps5258_reset,
	.init = ps5258_init,
	.g_register = ps5258_g_register,
	.s_register = ps5258_s_register,
};

static struct tx_isp_subdev_video_ops ps5258_video_ops = {
	.s_stream = ps5258_s_stream,
};

static struct tx_isp_subdev_sensor_ops	ps5258_sensor_ops = {
	.ioctl	= ps5258_sensor_ops_ioctl,
};

static struct tx_isp_subdev_ops ps5258_ops = {
	.core = &ps5258_core_ops,
	.video = &ps5258_video_ops,
	.sensor = &ps5258_sensor_ops,
};

/* It's the sensor device */
static u64 tx_isp_module_dma_mask = ~(u64)0;
struct platform_device sensor_platform_device = {
	.name = "ps5258",
	.id = -1,
	.dev = {
		.dma_mask = &tx_isp_module_dma_mask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data = NULL,
	},
	.num_resources = 0,
};


static int ps5258_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct tx_isp_subdev *sd;
	struct tx_isp_video_in *video;
	struct tx_isp_sensor *sensor;

	sensor = (struct tx_isp_sensor *)kzalloc(sizeof(*sensor), GFP_KERNEL);
	if(!sensor){
		ISP_ERROR("Failed to allocate sensor subdev.\n");
		return -ENOMEM;
	}
	memset(sensor, 0 ,sizeof(*sensor));

	/*
	  convert sensor-gain into isp-gain,
	*/
	sd = &sensor->sd;
	video = &sensor->video;
	ps5258_attr.expo_fs = 1;
	sensor->dev = &client->dev;
	sensor->video.attr = &ps5258_attr;
	sensor->video.vi_max_width = wsize->width;
	sensor->video.vi_max_height = wsize->height;
	sensor->video.mbus.width = wsize->width;
	sensor->video.mbus.height = wsize->height;
	sensor->video.mbus.code = wsize->mbus_code;
	sensor->video.mbus.field = TISP_FIELD_NONE;
	sensor->video.mbus.colorspace = wsize->colorspace;
	sensor->video.fps = wsize->fps;
	tx_isp_subdev_init(&sensor_platform_device, sd, &ps5258_ops);
	tx_isp_set_subdevdata(sd, client);
	tx_isp_set_subdev_hostdata(sd, sensor);
	private_i2c_set_clientdata(client, sd);

	pr_debug("probe ok ------->ps5258\n");

	return 0;
}

static int ps5258_remove(struct i2c_client *client)
{
	struct tx_isp_subdev *sd = private_i2c_get_clientdata(client);
	struct tx_isp_sensor *sensor = tx_isp_get_subdev_hostdata(sd);

	if(reset_gpio != -1)
		private_gpio_free(reset_gpio);
	if(pwdn_gpio != -1)
		private_gpio_free(pwdn_gpio);

    private_clk_disable_unprepare(sensor->mclk);
    private_devm_clk_put(&client->dev, sensor->mclk);
    tx_isp_subdev_deinit(sd);
	kfree(sensor);

	return 0;
}

static const struct i2c_device_id ps5258_id[] = {
	{ "ps5258", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ps5258_id);

static struct i2c_driver ps5258_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "ps5258",
	},
	.probe		= ps5258_probe,
	.remove		= ps5258_remove,
	.id_table	= ps5258_id,
};

static __init int init_ps5258(void)
{
	return private_i2c_add_driver(&ps5258_driver);
}

static __exit void exit_ps5258(void)
{
	private_i2c_del_driver(&ps5258_driver);
}

module_init(init_ps5258);
module_exit(exit_ps5258);

MODULE_DESCRIPTION("A low-level driver for Smartsens ps5258 sensors");
MODULE_LICENSE("GPL");
