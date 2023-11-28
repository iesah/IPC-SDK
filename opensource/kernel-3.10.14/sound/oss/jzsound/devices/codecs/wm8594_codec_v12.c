/* *  wm8594.c - Linux kernel modules for audio codec
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/hwmon.h>
#include <linux/miscdevice.h>
#include <linux/input-polldev.h>
#include <linux/input.h>
#include <linux/earlysuspend.h>

#include "../xb47xx_i2s_v1_2.h"
#include "wm8594_codec_v1_2.h"

//#define WM8594_INMSTR_MODE
#define CODEC_DUMP_IOC_CMD 0
#define WM8594_USE_DAC1
#define WM8594_USE_DAC2
//#define WM8594_DEBUG

struct wm8594_data {
	struct i2c_client *client;
	struct snd_codec_data *pdata;
	struct mutex lock_rw;
};

struct wm8594_data *wm8594_save;
extern int i2s_register_codec(char *name, void *codec_ctl,unsigned long codec_clk,enum codec_mode mode);

/*-------------------*/
#if CODEC_DUMP_IOC_CMD
#define DUMP_IOC_CMD(value)	\
	do {	\
		printk("[codec IOCTL]++++++++++++++++++++++++++++\n");			\
		printk("%s  cmd = %d, arg = %lu-----%s\n", __func__, cmd, arg, value);	\
		codec_print_ioc_cmd(cmd);						\
		printk("[codec IOCTL]----------------------------\n");			\
	} while (0)
#else //CODEC_DUMP_IOC_CMD
#define DUMP_IOC_CMD(value)
#endif //CODEC_DUMP_IOC_CMD

static int wm8594_i2c_read(struct wm8594_data *wm, u8 *reg, u8 *buf, int len)
{
	int err;
	struct i2c_msg msgs[] = {
		{
			.addr = wm->client->addr,
			.flags = 0,
			.len = 1,
			.buf = reg,
		},
		{
			.addr = wm->client->addr,
			.flags = I2C_M_RD,
			.len = len,
			.buf = buf,
		},
	};

	err = i2c_transfer(wm->client->adapter, msgs, 2);
	if (err < 0) {
		printk("   Read msg error %d\n",err);
		return err;
	}

	return 0;
}

static int wm8594_i2c_write(struct wm8594_data *wm, u8 *buf, int len)
{
	int err;
	struct i2c_msg msgs[] = {
		{
			.addr = wm->client->addr,
			.flags = 0,
			.len = len,
			.buf = buf,
		},
	};

	err = i2c_transfer(wm->client->adapter, msgs, 1);
	if (err < 0) {
		printk("   Write msg error\n");
		return err;
	}

	return 0;
}

static int wm8594_i2c_read_data(struct wm8594_data *wm, u8 reg, u16 *rxdata, int length)
{
	int ret, i;
	u8 *buf;

	buf = (u8 *)kzalloc(sizeof(u8) * length * 2, GFP_KERNEL);

//	mutex_lock(wm->lock_rw);
	ret = wm8594_i2c_read(wm, &reg, buf, length * 2);
//	mutex_unlock(wm->lock_rw);
	if (ret < 0) {
		kfree(buf);
		return ret;
	} else {
		for (i = 0; i < length * 2; i += 2) {
			rxdata[i/2] = (buf[i] << 8) + buf[i + 1];
		}
	}

	kfree(buf);
	return 0;
}

static int wm8594_i2c_write_data(struct wm8594_data *wm, u8 reg, u16 *txdata, int length)
{
	int ret, i;
	u8 *buf;

	buf = (u8 *)kzalloc(sizeof(u8) * (length+1) * 2, GFP_KERNEL);

	buf[0] = reg;
	for (i = 1; i < length * 2; i += 2) {
		buf[i] = txdata[i/2] >> 8;
		buf[i+1] = txdata[i/2];
	}

//	mutex_lock(&wm->lock_rw);
	ret = wm8594_i2c_write(wm, buf, length * 2 + 1);
//	mutex_unlock(&wm->lock_rw);

	kfree(buf);
	if (ret < 0)
		return ret;
	else
		return 0;
}


static int wm8594_write_reg16(u8 reg, u16 value)
{
	int ret = 0;
	u16 buf[] = {0, 0, 0};

	buf[0] = value;
	ret = wm8594_i2c_write_data(wm8594_save, reg, buf, 1);
	if (ret)
		dev_err(&wm8594_save->client->dev, "set adc1l vol failed %d \n", ret);

	return ret;
}

#ifdef WM8594_DEBUG
static int dump_codec_regs(void)
{
	unsigned int i;
	int ret = 0;
	u16 buf[] = {0, 0, 0};

	printk("codec register list:\n");
	if (!wm8594_save)
		return 0;
	for (i = 0; i <= 0x24; i++) {
		ret = wm8594_i2c_read_data(wm8594_save, i, buf, 1);
		if (ret)
			break;
		printk("address = 0x%02x, data = 0x%02x\n", i, buf[0]);
	}

	return ret;
}
#endif

static void set_extern_codec_write(void)
{
#if 1
	/* power on init */
	wm8594_write_reg16(0x23, 0x001d);
	wm8594_write_reg16(0x22, 0x1fc0);

	wm8594_write_reg16(0x23, 0x009d);
	udelay(1500000);
	wm8594_write_reg16(0x23, 0x00bc);

	wm8594_write_reg16(0x02, 0x0102);
	wm8594_write_reg16(0x03, 0x001b);
#if 0
	/*slave mode */
	wm8594_write_reg16(0x04, 0x0000);
#else
	/* master mode */
	wm8594_write_reg16(0x04, 0x0001);
#endif
	/* DAC volume */
	wm8594_write_reg16(0x05, 0x01c8);
	wm8594_write_reg16(0x06, 0x01c8);

	/*07h - 0Bh is DAC2 controller */
	wm8594_write_reg16(0x0c, 0x0001);
	//0dh - 11h is ADC controller */
	/*PGA Volume */
	wm8594_write_reg16(0x17, 0x010c);
	wm8594_write_reg16(0x18, 0x010c);

	/* 1, 2 is disable ,so not set them */
	wm8594_write_reg16(0x19, 0x0000);

	//wm8594_write_reg16(0x1a, 0x001e);
	wm8594_write_reg16(0x1a, 0x0000);
	wm8594_write_reg16(0x1b, 0x0018);

	wm8594_write_reg16(0x1d, 0x0a90);
	wm8594_write_reg16(0x1f, 0x00ff);

	wm8594_write_reg16(0x23, 0x007c);
	//wm8594_write_reg16(0x1f, 0x0030);
	wm8594_write_reg16(0x22, 0x1fc0);
#if 0
	//slave mode
	wm8594_write_reg16(0x24, 0x040a);
#else
	//master mode
	wm8594_write_reg16(0x24, 0x0402);
#endif
#endif

}

#if 0
static void set_extern_codec_read(void)
{
#if 1
	/* power on init */
	wm8594_write_reg16(0x23, 0x001d);
	wm8594_write_reg16(0x22, 0x1fc0);

	wm8594_write_reg16(0x23, 0x009d);
	udelay(1500000);
	wm8594_write_reg16(0x23, 0x00bc);

	/*07h - 0Bh is DAC2 controller */
	wm8594_write_reg16(0x0c, 0x0001);
	//0dh - 11h is ADC controller */
	wm8594_write_reg16(0x0D, 0x0042);
	wm8594_write_reg16(0x0e, 0x001b);
	//wm8594_write_reg16(0x0f, 0x0000);
	wm8594_write_reg16(0x0f, 0x0001);

	wm8594_write_reg16(0x10, 0x01cf);
	wm8594_write_reg16(0x11, 0x01cf);

	wm8594_write_reg16(0x1a, 0x0001);
	wm8594_write_reg16(0x1b, 0x0018);

//	wm8594_write_reg16(0x1d, 0x0a90);
	wm8594_write_reg16(0x1e, 0x0780);

	wm8594_write_reg16(0x1f, 0x00f0);

	wm8594_write_reg16(0x23, 0x007c);
	wm8594_write_reg16(0x22, 0x1fc0);
	//master mode
	wm8594_write_reg16(0x24, 0x0400);
#endif

}
#endif
static int wm8594_set_rw(int mode);
static int wm8594_codec_ctl(unsigned int cmd, unsigned long arg)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	DUMP_IOC_CMD("enter");
	{
		switch (cmd) {

		case CODEC_INIT:
			printk("---- i2s call codec init cmd\n");
			break;

		case CODEC_TURN_OFF:
			break;

		case CODEC_SHUTDOWN:
			break;

		case CODEC_RESET:
			break;

		case CODEC_SUSPEND:
			break;

		case CODEC_RESUME:
			break;

		case CODEC_ANTI_POP:
			wm8594_set_rw((int) arg);
			break;

		case CODEC_SET_DEFROUTE:
			break;

		case CODEC_SET_DEVICE:
			break;

		case CODEC_SET_STANDBY:
			break;

		case CODEC_SET_RECORD_RATE:
			break;

		case CODEC_SET_RECORD_DATA_WIDTH:
			break;

		case CODEC_SET_MIC_VOLUME:
			break;

		case CODEC_SET_RECORD_VOLUME:
			break;

		case CODEC_SET_RECORD_CHANNEL:
			break;

		case CODEC_SET_REPLAY_RATE:
			break;

		case CODEC_SET_REPLAY_DATA_WIDTH:
			if (wm8594_save) {
				ret = wm8594_i2c_read_data(wm8594_save, CODEC_REG_DAC1_CRTL1, buf, 1);
				buf[0] &= ~(0x3<<DAC1_WL);
				if ((int)arg == 16)
					buf[0] |= 0x0<<DAC1_WL;
				else if ((int)arg == 24)
					buf[0] |= 0x2<<DAC1_WL;
				ret = wm8594_i2c_write_data(wm8594_save, CODEC_REG_DAC1_CRTL1, buf, 1);
			}
			break;

		case CODEC_SET_REPLAY_VOLUME:
			break;

		case CODEC_SET_REPLAY_CHANNEL:
			break;

		case CODEC_GET_REPLAY_FMT_CAP:
		case CODEC_GET_RECORD_FMT_CAP:
			break;

		case CODEC_DAC_MUTE:
			break;
		case CODEC_ADC_MUTE:
			break;
		case CODEC_DEBUG_ROUTINE:
			break;

		case CODEC_IRQ_HANDLE:
			break;

		case CODEC_GET_HP_STATE:
			break;

		case CODEC_DUMP_REG:
			//dump_codec_regs();
			break;
		case CODEC_DUMP_GPIO:
			break;
		case CODEC_CLR_ROUTE:
			break;
		case CODEC_DEBUG:
			break;
		default:
			break;
		}
	}

	DUMP_IOC_CMD("leave");

	return ret;
}

static int wm8594_set_dac1_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/***********************\
	|* set dac1lr volume   *|
	\***********************/
	buf[0] = 0xc8<<DAC1L_VOL;
	buf[0] |= 0x1<<DAC1L_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC1L_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set adc1l vol failed %d \n", ret);

	buf[0] = 0xc8<<DAC1R_VOL;
	buf[0] |= 0x1<<DAC1R_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC1R_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set adc1r vol failed %d \n", ret);

	return ret;
}

static int wm8594_set_dac2_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/***********************\
	|* set dac1lr volume   *|
	\***********************/
	buf[0] = 0xc8<<DAC2L_VOL;
	buf[0] |= 0x1<<DAC2L_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC2L_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set adc2l vol failed %d \n", ret);

	buf[0] = 0xc8<<DAC2R_VOL;
	buf[0] |= 0x1<<DAC2R_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC2R_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set adc2r vol failed %d \n", ret);

	return ret;
}

static int wm8594_set_adc_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/***********************\
	|* set adc1lr volume   *|
	\***********************/
	buf[0] = 0xc3<<ADCL_VOL;
	buf[0] |= 0x1<<ADCL_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADCL_VOL, buf, 1);
	if (ret)
		return ret;

	buf[0] = 0xc3<<ADCR_VOL;
	buf[0] |= 0x1<<ADCR_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADCR_VOL, buf, 1);
	if (ret)
		return ret;

	return ret;
}

static int wm8594_select_dac1(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/*********************************************\
	|* DAC1 select i2s mode, 16-bit word length, *|
	|* BCLK & LRCLK not inverted, normal mode    *|
	|* no deemphasis, use zero-cross, stereo     *|
	\*********************************************/
	buf[0] = 0x2<<DAC1_FMT;
	buf[0] |= 0x0<<DAC1_WL; /* word length 0-16bit, if you test 24bit or other data length ,you must set this bit ,or change this driver*/
	buf[0] |= 0x0<<DAC1_BCP;
	buf[0] |= 0x0<<DAC1_LRP;
	buf[0] |= 0x0<<DAC1_DEEMPH;
	buf[0] |= 0x0<<DAC1_ZCEN;
	buf[0] |= 0x1<<DAC1_EN;
	buf[0] |= 0x0<<DAC1_MUTE;
	buf[0] |= 0x0<<DAC1_OP_MUX;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC1_CRTL1, buf, 1);
	if (ret)
		return ret;

#ifdef WM8594_INMSTR_MODE
	buf[0] = 1<<DAC1_MSTR;
#else
	buf[0] = 0<<DAC1_MSTR;
#endif
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC1_CRTL3, buf, 1);  /* select master mode */
	if (ret)
		return ret;

	/*******************************************\
	|* set MCLK, DAC1_SR: 0x3 - 256fs          *|
	|* set BCLK rate, DAC1_BCLKDIV: 0x3 - 64fs *|
	\*******************************************/
	buf[0] = 0x3<<DAC1_SR;
	buf[0] |= 0x3<<DAC1_BCLKDIV;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC1_CRTL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set adc1 sr failed %d \n", ret);

	ret = wm8594_set_dac1_vol(wm);

	return ret;
}

static int wm8594_select_dac2(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/*********************************************\
	|* DAC2 select i2s mode, 16-bit word length, *|
	|* BCLK & LRCLK not inverted, normal mode    *|
	|* no deemphasis, use zero-cross, stereo     *|
	\*********************************************/
	buf[0] = 0x2<<DAC2_FMT;
	buf[0] |= 0x0<<DAC2_WL;
	buf[0] |= 0x0<<DAC2_BCP;
	buf[0] |= 0x0<<DAC2_LRP;
	buf[0] |= 0x0<<DAC2_DEEMPH;
	buf[0] |= 0x0<<DAC2_ZCEN;
	buf[0] |= 0x1<<DAC2_EN;
	buf[0] |= 0x0<<DAC2_MUTE;
	buf[0] |= 0x0<<DAC2_OP_MUX;

	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC2_CRTL1, buf, 1);
	if (ret)
		return ret;
#ifdef WM8594_INMSTR_MODE
	buf[0] = 1<<DAC2_MSTR;
#else
	buf[0] = 0<<DAC2_MSTR;
#endif
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC2_CRTL3, buf, 1);
	if (ret)
		return ret;

	/*******************************************\
	|* set MCLK, DAC2_SR: 0x3 - 256fs          *|
	|* set BCLK rate, DAC2_BCLKDIV: 0x3 - 64fs *|
	\*******************************************/
	buf[0] = 0x3<<DAC2_SR;
//	buf[0] = 0x0<<DAC2_SR;
	buf[0] |= 0x3<<DAC2_BCLKDIV;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_DAC2_CRTL2, buf, 1);

	ret = wm8594_set_dac2_vol(wm);

	return ret;
}

static int wm8594_select_adc(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/*********************************************\
	|* ADC select i2s mode, 16-bit word length,  *|
	|* BCLK & LRCLK not inverted, normal mode    *|
	|* no deemphasis, use zero-cross, stereo     *|
	\*********************************************/
	buf[0] = 0x2<<ADC_FMT;
	buf[0] |= 0x0<<ADC_WL;
	buf[0] |= 0x0<<ADC_BCP;
	buf[0] |= 0x0<<ADC_LRP;
	buf[0] |= 0x1<<ADC_EN;
	buf[0] |= 0x0<<ADC_LRSWAP;
	buf[0] |= 0x0<<ADCR_INV;
	buf[0] |= 0x0<<ADCL_INV;
	buf[0] |= 0x0<<ADC_DATA_SEL;
	buf[0] |= 0x0<<ADC_HPD;
	buf[0] |= 0x0<<ADC_ZCEN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADC_CTRL1, buf, 1);
	if (ret)
		return ret;
	/* select master mode */
#ifdef WM8594_INMSTR_MODE
	buf[0] = 1<<ADC_MSTR;
#else
	buf[0] = 0<<ADC_MSTR;
#endif
	buf[0] = 1<<ADC_MSTR;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADC_CTRL3, buf, 1);
	if (ret)
		return ret;

	/*******************************************\
	|* set MCLK, ADC_SR: 0x3 - 256fs           *|
	|* set BCLK rate, ADC_BCLKDIV: 0x3 - 64fs  *|
	\*******************************************/
	buf[0] = 0x3<<ADC_SR;
	buf[0] |= 0x3<<ADC_BCLKDIV;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADC_CTRL2, buf, 1);
	if (ret)
		return ret;

	ret = wm8594_set_adc_vol(wm);

	return ret;
}

/************** global_enable ***************/
static int wm8594_global_enable(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/**************************************************\
	|* ADC, DAC and PGA ramp control circuity enable, *|
	|* DAC2 settings independent of DAC1              *|
	\**************************************************/
	buf[0] = 0x1<<GLOBAL_ENABLE;
	buf[0] |= 0x0<<DAC2_COPY_DAC1;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ENABLE, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "global enable failed %d \n", ret);

	return ret;
}

/* set pga input selector */
static int wm8594_output_selector(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/*********************************************************\
	|* any analogue PGA can be routed to any analogue outpu  *|
	|* and each output is routed to only one input.          *|
	|* I routed PGA1LR to VOUT1LR, PGA2LR to VOUT2LR...      *|
	\*********************************************************/
	buf[0] = 0x0<<VOUT1L_SEL;
	buf[0] |= 0x1<<VOUT1R_SEL;
	buf[0] |= 0x2<<VOUT2L_SEL;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_OUTPUT_CTRL1, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "output select failed %d \n", ret);

	buf[0] = 0x3<<VOUT2R_SEL;
	buf[0] |= 0x4<<VOUT3L_SEL;
	buf[0] |= 0x5<<VOUT3R_SEL;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_OUTPUT_CTRL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "output select failed %d \n", ret);

	/*********************************************\
	|* VOUTxLR_TRI: 0 represent normal mode      *|
	|* APE_B: 1 - clamp not active               *|
	|* VOUTXLR_EN: 1 represent amplifier enabled *|
	\*********************************************/
	buf[0] = 0x0<<VOUT1L_TRI;
	buf[0] |= 0x1<<APE_B;
	buf[0] |= 0x3f<<VOUT1L_EN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_OUTPUT_CTRL3, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "output select failed %d \n", ret);

	return ret;
}

/*** mute all ***/
static int wm8594_set_pga_muteall(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	buf[0] = 0x1<<MUTE_ALL;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA_CTRL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	return ret;
}

/*** mute all exit ***/
static int wm8594_pga_allmute_exit(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	buf[0] = 0x0;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA_CTRL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	return ret;
}

/* set pga input selector */
static int wm8594_pga_input_selector(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/**************************\
	|* set pga input selector *|
	|* pga1lr select dac1lr   *|
	|* pga2lr select dac2lr   *|
	|* pga3lr select vin1lr   *|
	\**************************/
	buf[0] = 0x2<<PGA1L_IN_SEL;
	buf[0] |= 0x2<<PGA1R_IN_SEL;
	buf[0] |= 0xb<<PGA2L_IN_SEL;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_INPUT_CTRL1, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "global enable failed %d \n", ret);

	buf[0] = 0xb<<PGA2R_IN_SEL;
	buf[0] |= 0x9<<PGA3L_IN_SEL;
	buf[0] |= 0xa<<PGA3R_IN_SEL;

	ret = wm8594_i2c_write_data(wm, CODEC_REG_INPUT_CTRL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "global enable failed %d \n", ret);

	/*********************\
	|* enable pgaxlr     *|
	\*********************/
	buf[0] = 0x30;
	buf[0] |= 0x3<<ADCL_AMP_EN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_INPUT_CTRL4, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "global enable failed %d \n", ret);

	return ret;
}

/* set adc input selector */
static int wm8594_adc_input_selector(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/**************************\
	|* set adc input selector *|
	|* adclr select vin1lr    *|
	\**************************/
	buf[0] = 0x0<<ADCL_SEL;
	buf[0] |= 0x8<<ADCR_SEL;
	buf[0] |= 0x1<<ADC_AMP_VOL;
	buf[0] |= 0x1<<ADC_SWITCH_EN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_INPUT_CTRL3, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "adc input select failed %d \n", ret);

	return ret;
}

/*************** pga1 volume ***************/
static int wm8594_set_pga1_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	buf[0] = 0xc<<PGA1L_VOL;
	buf[0] |= 0x1<<PGA1L_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA1L_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga1l vol failed %d \n", ret);

	buf[0] = 0xc<<PGA1R_VOL;
	buf[0] |= 0x1<<PGA1R_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA1R_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga1r vol failed %d \n", ret);

	return ret;
}

/*************** pga2 volume ***************/
static int wm8594_set_pga2_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	buf[0] = 0xc<<PGA2L_VOL;
	buf[0] |= 0x1<<PGA2L_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA2L_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga2l vol failed %d \n", ret);

	buf[0] = 0xc<<PGA2R_VOL;
	buf[0] |= 0x1<<PGA2R_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA2R_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga2r vol failed %d \n", ret);

	return ret;
}

/*************** pga2 volume ***************/
static int wm8594_set_pga3_vol(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	buf[0] = 0xc<<PGA3L_VOL;
	buf[0] |= 0x1<<PGA3L_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA3L_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga3l vol failed %d \n", ret);

	buf[0] = 0xc<<PGA3R_VOL;
	buf[0] |= 0x1<<PGA3R_VU;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA3R_VOL, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set pga3r vol failed %d \n", ret);

	return ret;
}

/* set pgax volume */
static int wm8594_set_pga_vol(struct wm8594_data *wm)
{
	int ret = 0;


	ret = wm8594_set_pga1_vol(wm);
	if (ret)
		goto err;

	ret = wm8594_set_pga2_vol(wm);
	if (ret)
		goto err;

	ret = wm8594_set_pga3_vol(wm);

err:
	return ret;
}

/* set global pga */
static int wm8594_set_pga(struct wm8594_data *wm)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	/******************************\
	|* set decay&attack bypass,   *|
	|* set zero cross to prevent  *|
	|* pop and click noise        *|
	\******************************/
	buf[0] = 0x1<<DECAY_BYPASS;
	buf[0] |= 0x1<<ATTACK_BYPASS;
	buf[0] |= 0x1<<PGA1L_ZC;
	buf[0] |= 0x1<<PGA1R_ZC;
	buf[0] |= 0x1<<PGA2L_ZC;
	buf[0] |= 0x1<<PGA2R_ZC;
	buf[0] |= 0x1<<PGA3L_ZC;
	buf[0] |= 0x1<<PGA3R_ZC;

	buf[0] = 0x0;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA_CTRL1, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "set decay attack & zero cross failed %d \n", ret);

	/******************************\
	|* set sample rate for pga    *|
	|* PGA_SR: 0x1 - 44.1kHz      *|
	|* AUTO_INC: auto_increment   *|
	\******************************/
	buf[0] = 0x1<<AUTO_INC;
	buf[0] |= 0x1<<PGA_SR;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_ADD_CTRL1, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, " set sample rate for pga failed %d \n", ret);

	/******************************\
	|* set safe change clock,     *|
	|* pga clock source           *|
	\******************************/
	buf[0] = 0x0<<PGA_SAFE_FW;
	buf[0] |= 0x1<<PGA_SEL;
	buf[0] |= 0x1<<PGA_UPD;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_PGA_CTRL3, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, " set sample rate for pga failed %d \n", ret);

	ret = wm8594_set_pga_vol(wm);
	if (ret)
		dev_err(&wm->client->dev, " set pga volume failed %d \n", ret);

	return ret;
}

/******** set route to replay or record ***********/
static int wm8594_set_route(struct wm8594_data *wm)
{
	int ret = 0;

	ret = wm8594_pga_input_selector(wm);
	if (ret) {
		dev_err(&wm->client->dev, "set pga input failed %d \n", ret);
		goto done;
	}

	ret = wm8594_output_selector(wm);
	if (ret) {
		dev_err(&wm->client->dev, "set output input failed %d \n", ret);
		goto done;
	}

	ret = wm8594_adc_input_selector(wm);
	if (ret)
		dev_err(&wm->client->dev, "set adc input failed %d \n", ret);

done:
	return ret;
}

static int wm8594_init_client(struct wm8594_data *wm)
{
	int ret = 0;

#ifdef WM8594_USE_DAC1
	ret = wm8594_select_dac1(wm);
	if (ret)
		goto done;
#endif

#ifdef WM8594_USE_DAC2
	ret = wm8594_select_dac2(wm);
	if (ret)
		goto done;
#endif

//#ifdef WM8594_USE_ADC
	ret = wm8594_select_adc(wm);
	if (ret)
		goto done;
//#endif
//
	ret = wm8594_set_pga(wm);
	if (ret)
		goto done;

	ret = wm8594_global_enable(wm);

done:
	return ret;
}

/******************************************\
|* this the poweron sequense, we must     *|
|* follow this sequense to avoid pop      *|
|* and click noise                        *|
\******************************************/
static int wm8594_set_poweron(struct wm8594_data *wm)
{
	int ret = 0;
	u16 buf[] = {0, 0, 0};

	buf[0] = 0x1<<SOFT_ST;
	buf[0] |= 0x1<<FAST_EN;
	buf[0] |= 0x1<<POB_CTRL;
	buf[0] |= 0x1<<BUFIO_EN;
//	buf[0] = 0x7c;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	buf[0] = 0x1<<VOUT1L_EN;
	buf[0] |= 0x1<<VOUT2L_EN;
	buf[0] |= 0x1<<VOUT3L_EN;
	buf[0] |= 0x1<<VOUT1R_EN;
	buf[0] |= 0x1<<VOUT2R_EN;
	buf[0] |= 0x1<<VOUT3R_EN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_OUTPUT_CTRL2, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	ret = wm8594_i2c_read_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	buf[0] |= 0x1<<VMID_SEL;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	mdelay(1500);

	ret = wm8594_i2c_read_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	buf[0] |= 0x1<<BIAS_EN;
	ret = wm8594_i2c_write_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	ret = wm8594_i2c_read_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	buf[0] &= ~(0x1<<POB_CTRL);
	ret = wm8594_i2c_write_data(wm, CODEC_REG_BIAS, buf, 1);
	if (ret)
		dev_err(&wm->client->dev, "write register failed %d \n", ret);

	return ret;
}

static int wm8594_set_rw(int mode)
{
	u16 buf[] = {0, 0, 0};
	int ret = 0;

	switch(mode) {
		case CODEC_RMODE:
//			set_extern_codec_read();
		buf[0] |= 0x0<<DAC1_EN;
		ret = wm8594_i2c_write_data(wm8594_save, CODEC_REG_DAC1_CRTL1, buf, 1);
		if (ret)
			break;
		ret = wm8594_i2c_write_data(wm8594_save, CODEC_REG_DAC2_CRTL1, buf, 1);
		if (ret)
			break;
		ret = wm8594_set_pga_muteall(wm8594_save);
		buf[0] = 0x400;
		ret = wm8594_i2c_write_data(wm8594_save, CODEC_REG_PGA_CTRL3, buf, 1);
		printk("%s close dac \n",__func__);
		break;
		case CODEC_RWMODE:
		case CODEC_WMODE:
			set_extern_codec_write();
#ifdef WM8594_USE_DAC1
		ret = wm8594_select_dac1(wm8594_save);
		if (ret)
			break;
#endif
#ifdef WM8594_USE_DAC2
		ret = wm8594_select_dac2(wm8594_save);
#endif
		buf[0] = 0x0<<PGA_SAFE_FW;
		buf[0] |= 0x1<<PGA_SEL;
		buf[0] |= 0x1<<PGA_UPD;
		ret = wm8594_i2c_write_data(wm8594_save, CODEC_REG_PGA_CTRL3, buf, 1);
		if (ret)
			dev_err(&wm8594_save->client->dev, " set sample rate for pga failed %d \n", ret);

		if (ret) {
			printk("pga mute exit error!\n");
			break;
		}
		printk("%s open dac \n",__func__);
		break;
	}

	//dump_codec_regs();
	return ret;
}

static __devinit int wm8594_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct wm8594_data *wm;
	int ret = 0;
	u16 buf[5] = {0, 0, 0, 0, 0};

	printk("--------------- probe start !\n");
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		ret = -ENODEV;
		goto err_dev;
	}
	wm = kzalloc(sizeof(*wm), GFP_KERNEL);
	if (NULL == wm) {
		dev_err(&client->dev, "failed to allocate memery for module data\n");
		ret = -ENOMEM;
		goto err_klloc;
	}

	wm->pdata = kmalloc(sizeof(*wm->pdata), GFP_KERNEL);
	if (NULL == wm->pdata) {
		dev_err(&client->dev, "failed to allocate memery for pdata!\n");
		ret = -ENOMEM;
		goto err_pdata;
	}
	memcpy(wm->pdata, client->dev.platform_data, sizeof(*wm->pdata));




	wm->client = client;
	client->dev.init_name = client->name;

	ret = wm8594_i2c_read_data(wm, CODEC_REG_RSET_ID, buf, 1);
	if (!ret) {
		if (buf[0] == 0x8594)
			printk("probe coedc id success addr: 0x%x!", client->addr);
		else
			printk("probe error: check id failed maybe addr:0x%x is error!", client->addr);
	} else {
		client->addr = 0x1b;
		ret = wm8594_i2c_read_data(wm, CODEC_REG_RSET_ID, buf, 1);
		if (!ret) {
			printk("probe error: check id failed maybe addr:0x%x is error!", client->addr);
			goto err_addr;
		} else {
			if (buf[0] == 0x8594)
				printk("probe coedc id success!");
			else {
				printk("check id error getid: 0x%x not match 0x8594\n", buf[0]);
				goto err_addr;
			}
		}
	}

	i2c_set_clientdata(client, wm);
	ret = wm8594_set_poweron(wm);
	if (ret) {
		printk("poweron this codec error!\n");
		goto err_addr;
	}

	ret = wm8594_set_pga_muteall(wm);
	if (ret)
		printk("muteall codec error!\n");

	ret = wm8594_init_client(wm);
	if (ret) {
		printk("init this codec error!\n");
		goto err_addr;
	}
	ret = wm8594_set_route(wm);
	if (ret) {
		printk("set route error!\n");
		goto err_addr;
	}
	ret = wm8594_pga_allmute_exit(wm);
	if (ret) {
		printk("pga mute exit error!\n");
		goto err_addr;
	}

	wm8594_save = wm;
	/*set_extern_codec_read();*/
#ifdef WM8594_DEBUG
	ret = dump_codec_regs();
	if (ret)
	printk("     dump wm8594 register failed!\n");
#endif
	printk("-----------     wm8594 probe success!\n");

	return 0;

err_addr:
err_pdata:
	kfree(wm->pdata);
err_klloc:
	kfree(wm);
err_dev:
	return ret;
}

static int __devexit wm8594_remove(struct i2c_client *client)
{
	int result = 0;
	/*struct wm8594_data *wm = i2c_get_clientdata(client);*/

	return result;
}


static const struct i2c_device_id wm8594_id[] = {
	{ "wm8594", 0},
	{ }
};

static struct i2c_driver wm8594_driver = {
	.driver = {
		.name = "wm8594",
		.owner = THIS_MODULE,
	},

	.probe = wm8594_probe,
	.remove = __devexit_p(wm8594_remove),
	.id_table = wm8594_id,
};
#define JZ4780_EXTERNAL_CODEC_CLOCK 12000000
static int __init wm8594_init(void)
{
	int ret;

	printk("------------ init wm8594 i2c driver start\n");
	i2s_register_codec("i2s_external_codec", (void *)wm8594_codec_ctl,JZ4780_EXTERNAL_CODEC_CLOCK,CODEC_SLAVE);

	ret = i2c_add_driver(&wm8594_driver);
	if (ret) {
		printk("------------ add wm8594 i2c driver failed\n");
		return -ENODEV;
	}

	return ret;
}

static void __init wm8594_exit(void)
{
	i2c_del_driver(&wm8594_driver);
}

module_init(wm8594_init);
module_exit(wm8594_exit);

MODULE_AUTHOR("hyang <hyang@ingenic.cn>");
MODULE_DESCRIPTION("WM8594 codec driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
