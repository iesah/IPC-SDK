/* *  ak4951_codec.c - Linux kernel modules for audio codec
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
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/input-polldev.h>
#include <linux/input.h>
#include <linux/gpio.h>

#include "../xb47xx_i2s_v12.h"
#include "ak4951_codec.h"

#define CODEC_DUMP_IOC_CMD 0
/*#define AK4951_DEBUG*/

#ifdef AK4951_DEBUG
#define	ak4951_print(format,...) printk(format, ##__VA_ARGS__)
#else
#define	ak4951_print(format,...)
#endif

struct ak4951en_data {
	struct i2c_client *client;
	struct snd_codec_data *pdata;
	struct mutex lock_rw;
};

struct ak4951en_data *ak4951en_save;
extern int i2s_register_codec_2(struct codec_info * codec_dev);

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

int ak4951en_i2c_read(struct ak4951en_data *ak, unsigned char reg)
{
	unsigned char value;
	struct i2c_client *client = ak->client;
	struct i2c_msg msg[2] = {
		[0] = {
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= &reg,
		},
		[1] = {
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= 1,
			.buf	= &value,
		}
	};
	int err;
	err = i2c_transfer(client->adapter, msg, 2);
	if (err < 0) {
		printk("   Read msg error %d\n",err);
		return err;
	}

	return value;
}

int ak4951en_i2c_write(struct ak4951en_data *ak, unsigned char reg, unsigned char value)
{
	struct i2c_client *client = ak->client;
	unsigned char buf[2] = {reg, value};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};
	int err;
	err = i2c_transfer(client->adapter, &msg, 1);
	if (err < 0) {
		printk("   Write msg error\n");
		return err;
	}

	return 0;
}

int ak4951en_reg_set(unsigned char reg, int start, int end, int val)
{
	int ret;
	int i = 0, mask = 0;
	unsigned char oldv = 0, new = 0;
	for (i = 0;  i < (end - start + 1); i++) {
		mask += (1 << (start + i));
	}

	oldv = ak4951en_i2c_read(ak4951en_save, reg);
	new = oldv & (~mask);
	new |= val << start;
	ret = ak4951en_i2c_write(ak4951en_save, reg, new);

	if (new != ak4951en_i2c_read(ak4951en_save, reg))
		printk("%s(%d):ak4951 write error!!\n",__func__, __LINE__);

	return ret;
}

#ifdef AK4951_DEBUG
static int dump_codec_regs(void)
{
	unsigned int i;
	int ret = 0;
	unsigned char value;

	ak4951_print("codec register list:\n");
	if (!ak4951en_save)
		return 0;
	for (i = 0; i <= 0x4f; i++) {
		value = ak4951en_i2c_read(ak4951en_save, i);
		printk("addr - val: 0x%02x - 0x%02x\n", i, value);
	}

	return ret;
}
#endif

int ak4951_init(void)
{
	/*Codec Power Up : PMVCM bit*/
	ak4951en_reg_set(POWER_MANAGEMENT1, 6, 6, 1);

	/*Dummy Command*/
	ak4951en_i2c_write(ak4951en_save, 0x01, 0x08);
	ak4951en_i2c_write(ak4951en_save, 0x05, 0x6b);
	ak4951en_i2c_write(ak4951en_save, 0x06, 0x0b);
	mdelay(2);

	/*Set CODEC PLL Mode : PMPLL bit*/
	ak4951en_reg_set(POWER_MANAGEMENT2, 2, 2, 1);

	/*Set MCKI 24M: PLL3 = 0, PLL2 = 1, PLL1 = 1, PLL0 = 1*/
	ak4951en_reg_set(MODE_CONTROL1, 4, 7, PLL_REF_CLOCK_12M);
	mdelay(10);

	/*Set CODEC Master Mode : M/S bit*/
	ak4951en_reg_set(POWER_MANAGEMENT2, 3, 3, 1);
	mdelay(20);

	/*Set Bit Clock*/
	ak4951en_reg_set(MODE_CONTROL1, 3, 3, BICK_OUTPUT_64FS);

	/*Set Audio Data Fmt*/
	ak4951en_reg_set(MODE_CONTROL1, 0, 1, AUDIO_DATA_FMT_ADC_24MSB_DAC_24MSB);
	/*ak4951en_reg_set(MODE_CONTROL1, 0, 1, AUDIO_DATA_FMT_ADC_I2S_DAC_I2S);*/

	return 0;

}

static int ak4951_set_buildin_mic(void)
{
	/*Set Sample rate*/
	ak4951en_reg_set(MODE_CONTROL2, 0, 3, SAMPLE_RATE_8K);

	/*Set Microphone Gain Amplifier: +27DB*/
	/*ak4951en_reg_set(SIGNAL_SELECT1, 6, 6, 1);*/
	ak4951en_reg_set(SIGNAL_SELECT1, 0, 2, MIC_GAIN_3DB);

	/*Set MIC Power Up*/
	ak4951en_reg_set(SIGNAL_SELECT1, 3, 4, SELECT_EXTERNAL_MIC);

	/*Select MIC Input*/
	ak4951en_reg_set(SIGNAL_SELECT2, 0, 3, SELECT_LIN2_RIN2);

	/*****************************/
	ak4951en_reg_set(0x09, 0, 7, 0);
	ak4951en_reg_set(0x0a, 0, 7, 0x6c);
	ak4951en_reg_set(0x0b, 0, 7, 0x2e);
	ak4951en_reg_set(0x0c, 0, 7, 0xe1);
	ak4951en_reg_set(0x0d, 0, 7, 0xe1);
	ak4951en_reg_set(0x1a, 0, 7, 0x2c);//wind noise
	ak4951en_reg_set(0x1b, 0, 7, 0x01);//wind noise
	/*****************************/

	/*Set ADC Mono/Stereo Mode*/
	ak4951en_reg_set(POWER_MANAGEMENT1, 0, 1, ADCLD_R_ADCRD_R);
	ak4951en_reg_set(POWER_MANAGEMENT1, 7, 7, 1);

	return 0;
}

static int ak4951_set_speaker(void)
{
	/*Set Sample rate*/
	ak4951en_reg_set(MODE_CONTROL2, 0, 3, SAMPLE_RATE_8K);

	//DACS
	ak4951en_reg_set(0x02, 5, 5, 1);

	/*ak4951en_reg_set(0x1d, 2, 3, 2);*/
	ak4951en_reg_set(0x04, 0, 1, 3);

	/*Set gain and output level*/
	ak4951en_reg_set(SIGNAL_SELECT2, 6, 7, SET_GAIN_LEVEL_11_1DB);

	ak4951en_reg_set(MODE_CONTROL1, 0, 1, 3);

	/**********/
	ak4951en_reg_set(0x0a, 0, 7, 0x6c);
	ak4951en_reg_set(0x0b, 0, 7, 0x2e);
	/**********/

	/*Set DAC Power Up*/
	ak4951en_reg_set(POWER_MANAGEMENT1, 2, 2, 1);

	/*Set Speaker Output*/
	ak4951en_reg_set(POWER_MANAGEMENT2, 0, 0, 0);


	/*Set speaker amplifier is powered-up/down*/
	ak4951en_reg_set(POWER_MANAGEMENT2, 1, 1, 1);

	mdelay(2);//reduce pop noise
	ak4951en_reg_set(SIGNAL_SELECT1, 7, 7, 1);

	/*ak4951en_reg_set(0x13, 0, 7, 0x0c);//6db*/
	/*ak4951en_reg_set(0x13, 0, 7, 0x06);//9db*/
	ak4951en_reg_set(0x13, 0, 7, 0);//0db
	ak4951en_reg_set(0x14, 0, 7, 0);//0db

#if 0
	int val = -1;
	if (gpio_request(GPIO_PA(10),"codec apm en") < 0) {
		printk("----%s %d: gpio pa10 = error\n", __func__, __LINE__);
	}
	val = gpio_get_value(GPIO_PA(10));
	printk("----%s %d: gpio pa10 = %d\n", __func__, __LINE__, val);
	gpio_direction_output(GPIO_PA(10) , 1);
	val = gpio_get_value(GPIO_PA(10));
	printk("----%s %d: gpio pa10 = %d\n", __func__, __LINE__, val);
#endif

	return 0;
}

unsigned int cur_out_device = -1;
static int ak4951_set_device(struct ak4951en_data *ak, enum snd_device_t device)
{
	int ret = 0;
	int iserror = 0;

	ak4951_print("ak4951_set_device %d \n",device);
	switch (device) {
	case SND_DEVICE_HEADSET:
	case SND_DEVICE_HEADPHONE:
		ak4951_print("%s: set device: headphone...\n", __func__);
		break;
	case SND_DEVICE_SPEAKER:
		ret = ak4951_set_speaker();
		ak4951_print("%s: set device: speaker...\n", __func__);
		break;

	case SND_DEVICE_BUILDIN_MIC:
		ak4951_print("%s: set device: MIC...\n", __func__);
		ret = ak4951_set_buildin_mic();
		break;
	default:
		iserror = 1;
		printk("JZ CODEC: Unkown ioctl argument %d in SND_SET_DEVICE\n",device);
	};

	if (!iserror)
		cur_out_device = device;

	return ret;
}

static int ak4951_set_sample_rate(unsigned int rate)
{
	int i = 0;
	/*8K 12K 16K 11.025K 22.05K 24K 32K 48K 44.1K */
	unsigned int mrate[9] = {8000, 12000, 16000, 11025, 22050, 24000, 32000, 48000, 44100};
	unsigned int rate_fs[9] = {SAMPLE_RATE_8K, SAMPLE_RATE_12K, SAMPLE_RATE_16K, SAMPLE_RATE_11_025K,
							   SAMPLE_RATE_22_05K, SAMPLE_RATE_24K, SAMPLE_RATE_32K, SAMPLE_RATE_48K,
							   SAMPLE_RATE_44_1K};

	for (i = 0; i < 9; i++) {
		if (rate <= mrate[i])
			break;
	}
	if (i == 9)
		i = 0;

	ak4951en_reg_set(MODE_CONTROL2, 0, 3, rate_fs[i]);

	return 0;
}

static int codec_turn_off(int mode)
{
	int ret = 0;

	if (mode & CODEC_RMODE) {
		/*close record*/
		ak4951en_reg_set(0x00, 0, 7, 0x40);
		ak4951en_reg_set(0x0b, 0, 7, 0x0e);
	}
	if (mode & CODEC_WMODE) {
		/*close replay*/
		/*gpio_free(GPIO_PA(10));*/

		ak4951en_reg_set(SIGNAL_SELECT1, 5, 5, 0);
		/*ak4951en_reg_set(0x02, 0, 7, 0x20);*/
		/*ak4951en_reg_set(0x02, 0, 7, 0x00);*/
		/*ak4951en_reg_set(0x01, 0, 7, 0x0c);*/
		/*ak4951en_reg_set(0x00, 0, 7, 0x40);*/
	}

	return ret;
}

void ak4951_set_play_volume(int * vol)
{
	int volume = 0;

	if (*vol < 0)
		*vol = 0;

	if (*vol > 0xcb)
		*vol = 0xcb;

	volume = 0xcb - (*vol);
	ak4951_print("%s %d: ak4951 set play volume = 0x%02x\n",__func__ , __LINE__,  volume);
	ak4951en_reg_set(0x13, 0, 7, volume);
}

void ak4951_set_record_volume(int * vol)
{
	/***************************************************\
	| vol :0  1  2  3  4   5   6   7   8   9   a        |
	| gain:0  3  6  9  12  15  18  21  24  27  30 (+DB) |
	\***************************************************/
	int volume = 0;

	if (*vol < 0)
		*vol = 0;

	if (*vol > 10)
		*vol = 10;

	volume = *vol;
	ak4951_print("%s %d: ak4951 set record volume = 0x%02x\n", __func__, __LINE__, volume);
	if (volume <= 7)
		ak4951en_reg_set(0x2, 0, 2, volume);
	if (volume > 7) {
		ak4951en_reg_set(0x2, 6, 6, 1);
		ak4951en_reg_set(0x2, 0, 2, volume-8);
	}
}

static int ak4951en_codec_ctl(struct codec_info *codec_dev, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	DUMP_IOC_CMD("enter");
	{
		switch (cmd) {
		case CODEC_INIT:
			ak4951_print("AK CODEC: %s, %d\n", __func__, __LINE__);
			ret = ak4951_init();
			break;

		case CODEC_TURN_OFF:
			ak4951_print("%s: set CODEC_TURN_OFF...\n", __func__);
			ret = codec_turn_off(arg);
			break;

		case CODEC_SHUTDOWN:
			ak4951_print("%s: set CODEC_SHUTDOWN...\n", __func__);
			break;

		case CODEC_RESET:
			break;

		case CODEC_SUSPEND:
			break;

		case CODEC_RESUME:
			break;

		case CODEC_ANTI_POP:
			break;

		case CODEC_SET_DEFROUTE:
			break;

		case CODEC_SET_DEVICE:
			ak4951_print("%s: set device...\n", __func__);
			ret = ak4951_set_device(ak4951en_save, *(enum snd_device_t *)arg);
			break;

		case CODEC_SET_STANDBY:
			break;

		case CODEC_SET_REPLAY_RATE:
		case CODEC_SET_RECORD_RATE:
			ak4951_print("%s: set sample rate...\n", __func__);
			ret = ak4951_set_sample_rate(*(enum snd_device_t *)arg);
			break;

		case CODEC_SET_RECORD_DATA_WIDTH:
			break;

		case CODEC_SET_MIC_VOLUME:
			break;

		case CODEC_SET_RECORD_VOLUME:
			ak4951_set_record_volume((int *)arg);
			break;

		case CODEC_SET_RECORD_CHANNEL:
			break;


		case CODEC_SET_REPLAY_DATA_WIDTH:
			if (ak4951en_save) {
			}
			break;

		case CODEC_SET_REPLAY_VOLUME:
			ak4951_set_play_volume((int *)arg);
			break;

		case CODEC_SET_REPLAY_CHANNEL:
			ak4951_print("%s: set repaly channel...\n", __func__);
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
#ifdef AK4951_DEBUG
			dump_codec_regs();
#endif
			break;
		case CODEC_DUMP_GPIO:
			break;
		case CODEC_CLR_ROUTE:
			ak4951_print("%s: set CODEC_CLR_ROUTE...\n", __func__);
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

/******************************************\
|* this the poweron sequense, we must     *|
|* follow this sequense to avoid pop      *|
|* and click noise                        *|
\******************************************/
static int ak4951en_set_poweron(struct ak4951en_data *ak)
{
	int ret = 0;

	return ret;
}

#define AK_PROC_DEBUG
#ifdef AK_PROC_DEBUG
unsigned char * sbuff = NULL;
#define SBUFF_SIZE		128
static ssize_t ak4951_proc_read(struct file *filp, char __user * buff, size_t len, loff_t * offset)
{
	ak4951_print(KERN_INFO "[+ak4951_proc] call ak4951_proc_read()\n");

	len = strlen(sbuff);
	if (*offset >= len) {
		return 0;
	}
	len -= *offset;
	if (copy_to_user(buff, sbuff + *offset, len)) {
		return -EFAULT;
	}
	*offset += len;
	ak4951_print("%s: sbuff = %s\n", __func__, sbuff);

	return len;
}
static ssize_t ak4951_proc_write(struct file *filp, char __user * buff, size_t len, loff_t * offset)
{
	int i = 0;
	int ret = 0;
	int control[4] = {0};
	unsigned char *p = NULL;
	char *after = NULL;

	ak4951_print("%s: set ak codec proc\n", __func__);
	memset(sbuff, 0, SBUFF_SIZE);
	len = len < SBUFF_SIZE ? len : SBUFF_SIZE;
	if (copy_from_user(sbuff, buff, len)) {
		printk(KERN_INFO "[+ak4951_proc]: copy_from_user() error!\n");
		return -EFAULT;
	}
	p = sbuff;
	control[0] = simple_strtoul(p, &after, 0);
	/*ak4951_print("control[0] = 0x%08x, after = %s\n", control[0], after);*/
	for (i = 1; i < 4; i++) {
		if (after[0] == ' ')
			after++;
		p = after;
		control[i] = simple_strtoul(p, &after, 0);
		/*ak4951_print("control[%d] = 0x%08x, after = %s\n", i, control[i], after);*/
	}

	if (control[0] == 99) {
		ak4951_print("read reg:0x%02x = 0x%02x\n",control[1],
				ak4951en_i2c_read(ak4951en_save, control[1]));
	} else {
		ak4951_print("reg   :0x%02x\n", control[0]);
		ak4951_print("start :0x%02x\n", control[1]);
		ak4951_print("end   :0x%02x\n", control[2]);
		ak4951_print("val   :0x%02x\n", control[3]);
		ak4951en_reg_set(control[0], control[1], control[2], control[3]);
	}
#ifdef AK4951_DEBUG
	if (control[0] == 100) {
		ret = dump_codec_regs();
	}
#endif

	return len;
}

static const struct file_operations ak4951en_devices_fileops = {
	.owner		= THIS_MODULE,
	.read		= ak4951_proc_read,
	.write		= ak4951_proc_write,
};
#endif

int ak4951en_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct ak4951en_data *ak;
	int ret = 0;


	printk("%s, probe start !\n", __func__);
	if (!i2c_check_functionality(client->adapter,
				I2C_FUNC_SMBUS_BYTE | I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&client->dev, "client not i2c capable\n");
		ret = -ENODEV;
		goto err_dev;
	}
	ak = kzalloc(sizeof(struct ak4951en_data), GFP_KERNEL);
	if (NULL == ak) {
		dev_err(&client->dev, "failed to allocate memery for module data\n");
		ret = -ENOMEM;
		goto err_klloc;
	}

	ak->pdata = kmalloc(sizeof(*ak->pdata), GFP_KERNEL);
	if (NULL == ak->pdata) {
		dev_err(&client->dev, "failed to allocate memery for pdata!\n");
		ret = -ENOMEM;
		goto err_pdata;
	}
	memcpy(ak->pdata, client->dev.platform_data, sizeof(*ak->pdata));

	ak->client = client;
	client->dev.init_name = client->name;

	i2c_set_clientdata(client, ak);
	ret = ak4951en_set_poweron(ak);
	if (ret) {
		printk("poweron this codec error!\n");
		goto err_addr;
	}

	ak4951en_save = ak;
#ifdef AK4951_DEBUG
	ret = dump_codec_regs();
	if (ret)
		printk("dump ak4951en register failed!\n");
#endif

#ifdef AK_PROC_DEBUG
	static struct proc_dir_entry *proc_ak4951_dir;
	struct proc_dir_entry *entry;

	sbuff = kmalloc(SBUFF_SIZE, GFP_KERNEL);
	proc_ak4951_dir = proc_mkdir("ak4951en", NULL);
	if (!proc_ak4951_dir)
		return -ENOMEM;

	entry = proc_create("akcodec", 0, proc_ak4951_dir,
			    &ak4951en_devices_fileops);
	if (!entry) {
		printk("%s: create proc_create error!\n", __func__);
		return -1;
	}
#endif

	printk("%s : ak4951en probe success!\n", __func__);

	return 0;

err_addr:
err_pdata:
	kfree(ak->pdata);
err_klloc:
	kfree(ak);
err_dev:
	return ret;
}

static int ak4951en_remove(struct i2c_client *client)
{
	int result = 0;

	return result;
}

static const struct i2c_device_id ak4951en_id[] = {
	{ "ak4951en", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, ak4951en_id);

static struct i2c_driver ak4951en_driver = {
	.driver = {
		.name = "ak4951en",
		.owner = THIS_MODULE,
	},

	.probe = ak4951en_probe,
	.remove = ak4951en_remove,
	.id_table = ak4951en_id,
};

#define T15_EXTERNAL_CODEC_CLOCK 12000000
static int __init ak4951en_init(void)
{
	int ret;

	struct codec_info * codec_dev;

	printk("------------ init ak4951en i2c driver start!\n");
	codec_dev = kzalloc(sizeof(struct codec_info), GFP_KERNEL);
	if(!codec_dev) {
		printk("alloc codec device error\n");
		return -1;
	}
	sprintf(codec_dev->name, "i2s_external_codec");
	codec_dev->codec_ctl_2 = ak4951en_codec_ctl;
	codec_dev->codec_clk = T15_EXTERNAL_CODEC_CLOCK;
	codec_dev->codec_mode = CODEC_MASTER;

	i2s_register_codec_2(codec_dev);

	if (gpio_request(GPIO_PA(27),"codec vcc en") < 0) {
		printk("----%s %d: gpio pa27 = error\n", __func__, __LINE__);
	}
	gpio_direction_output(GPIO_PA(27) , 0);

	mdelay(1);

	if (gpio_request(GPIO_PA(26),"codec reset") < 0) {
		printk("----%s %d: gpio pa26 = error\n", __func__, __LINE__);
	}
	gpio_direction_output(GPIO_PA(26) , 1);

	ret = i2c_add_driver(&ak4951en_driver);
	if (ret) {
		printk("------------ add ak4951en i2c driver failed\n");
		return -ENODEV;
	}
	printk("------------ init ak4951en i2c driver end!\n");

	return ret;
}

static void __init ak4951en_exit(void)
{
	gpio_free(GPIO_PA(27));
	gpio_free(GPIO_PA(26));
	i2c_del_driver(&ak4951en_driver);
}

module_init(ak4951en_init);
module_exit(ak4951en_exit);

MODULE_AUTHOR("Elvis <huan.wang@ingenic.com>");
MODULE_DESCRIPTION("AK4951 codec driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
