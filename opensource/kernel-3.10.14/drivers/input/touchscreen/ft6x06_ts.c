/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft6x06 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/regulator/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/input/ft6x06_ts.h>
#include <soc/gpio.h>
#include <jz_notifier.h>


/* After report early 10 points, skip report point(1/3). */
#define DEBUG_SKIP_REPORT_POINT
#define DEBUG_SKIP_POINT_DIVIDE_RATIO (3) /* only report 1/3 */


//#define FTS_CTL_IIC
//#define SYSFS_DEBUG
//#define FTS_APK_DEBUG
#define DEBUG_LCD_VCC_ALWAYS_ON
#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif
#ifdef SYSFS_DEBUG
#include "ft6x06_ex_fun.h"
#endif
struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u8 au8_touch_wight[CFG_MAX_TOUCH_POINTS];	/*touch Wight */
	u16 pressure;
	u8 touch_point;
};

struct ft6x06_ts_data {
	unsigned int irq;
	unsigned int irq_pin;
	unsigned int x_max;
	unsigned int y_max;
	unsigned int va_x_max;
	unsigned int va_y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct ft6x06_platform_data *pdata;
	struct work_struct  work;
	struct workqueue_struct *workqueue;
	struct regulator *vcc_reg;

#ifdef DEBUG_SKIP_REPORT_POINT
	unsigned int report_count;
#endif
};

#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02

unsigned char key_home_status = 0;
unsigned char key_back_status = 0;
unsigned char key_menu_status = 0;

/*
*ft6x06_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*/
int ft6x06_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}

int ft6x06_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}

/*release the point*/
static void ft6x06_ts_release(struct ft6x06_ts_data *data)
{
#ifdef DEBUG_SKIP_REPORT_POINT
	data->report_count = 0;
#endif

	input_report_key(data->input_dev, BTN_TOUCH, 0);
	if(1 == key_menu_status){
		input_event(data->input_dev,EV_KEY,KEY_MENU,0);
		key_menu_status = 0;
	}
	if(1 == key_home_status){
		input_event(data->input_dev,EV_KEY,KEY_HOMEPAGE,0);
		key_home_status = 0;
	}
	if(1 == key_back_status){
		input_event(data->input_dev,EV_KEY,KEY_BACK,0);
		key_back_status = 0;
	}
	input_mt_sync(data->input_dev);
	input_sync(data->input_dev);
}

static int ft6x06_read_Touchdata(struct ft6x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft6x06_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);

	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = buf[2] & 0x0F;
	if (event->touch_point == 0) {
		ft6x06_ts_release(data);
		return 1;
	}

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		event->au8_touch_wight[i] =
			(buf[FT_TOUCH_WEIGHT_POS + FT_TOUCH_STEP * i]);
	}

	event->pressure = FT_PRESS;

	return 0;
}

static void ft6x06_report_value(struct ft6x06_ts_data *data)
{
	struct ts_event *event = &data->event;

	int i = 0;
	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */
		if (event->au16_x[i] < data->va_x_max
		    && event->au16_y[i] < data->va_y_max) {
#ifdef DEBUG_SKIP_REPORT_POINT
			int report_count;
			report_count = data->report_count;
			data->report_count++;
			/* After report early 10 points, skip report point(1/3). */
			if ( (report_count < 10) || ( report_count % DEBUG_SKIP_POINT_DIVIDE_RATIO == 0) ) {
#endif	/* DEBUG_SKIP_REPORT_POINT */

#ifdef CONFIG_FT6X06_MULTITOUCH
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					event->au16_y[i]);
			input_report_abs(data->input_dev,ABS_MT_TOUCH_MAJOR,
					event->au8_touch_wight[i]);
			input_report_abs(data->input_dev,ABS_MT_WIDTH_MAJOR,
					event->au8_touch_wight[i]);
			input_mt_sync(data->input_dev);

#else
			if(0 == i){
				s16 convert_x = 0;
				s16 convert_y = 0;
				convert_x = event->au16_x[0];
				convert_y = event->au16_y[0];
#ifndef  CONFIG_ANDROID
				convert_x = (event->au16_x[0] * 8 / 5);
				convert_y = (event->au16_y[0] * 5 / 3);
#endif
				if (event->touch_point == 1) {
					input_report_abs(data->input_dev, ABS_X, (u16)convert_x);
					input_report_abs(data->input_dev, ABS_Y, (u16)convert_y);
					input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);
				}
				input_report_key(data->input_dev, BTN_TOUCH, 1);
			}
#endif
#ifdef DEBUG_SKIP_REPORT_POINT
			}
#endif /* DEBUG_SKIP_REPORT_POINT */
		}
		/*Virtual key*/
		else{
			if ((event->au8_touch_event[i] == FTS_POINT_DOWN)
					|| (event->au8_touch_event[i] == FTS_POINT_CONTACT)) {
				if(event->au16_y[i] >= data->va_y_max
						&& event->au16_y[i] <= data->y_max) {
					/*menu key*/
					if (event->au16_x[i] >= 0 && event->au16_x[i] < 100){
						if(0 == key_menu_status){
							input_event(data->input_dev,EV_KEY,KEY_MENU,1);
							key_menu_status = 1;
						}
					}
					/*home key*/
					if (event->au16_x[i] >= 100 && event->au16_x[i] < 200){
						if(0 == key_home_status){
							input_event(data->input_dev,EV_KEY,KEY_HOMEPAGE,1);
							key_home_status = 1;
						}
					}
					/*back key*/
					if (event->au16_x[i] >= 200 && event->au16_x[i] < 300){
						if(0 == key_back_status){
							input_event(data->input_dev,EV_KEY,KEY_BACK,1);
							key_back_status = 1;
						}
					}
				}
			}else{
				if(1 == key_menu_status){
					input_event(data->input_dev,EV_KEY,KEY_MENU,0);
					key_menu_status = 0;
				}
				if(1 == key_home_status){
					input_event(data->input_dev,EV_KEY,KEY_HOMEPAGE,0);
					key_home_status = 0;
				}
				if(1 == key_back_status){
					input_event(data->input_dev,EV_KEY,KEY_BACK,0);
					key_back_status = 0;
				}
			}
		}
	}

	dev_dbg(&data->client->dev, "$ly-test----%s: x1:%d y1:%d |<*_*>| \
			x2:%d y2:%d \n", __func__,
			event->au16_x[0], event->au16_y[0],
			event->au16_x[1], event->au16_y[1]);
	input_sync(data->input_dev);
}

static void ft6x06_work_handler(struct work_struct *work)
{
	struct ft6x06_ts_data *ft6x06_ts = container_of(work, struct ft6x06_ts_data, work);
	int ret = 0;

	ret = ft6x06_read_Touchdata(ft6x06_ts);
	if (ret == 0)
		ft6x06_report_value(ft6x06_ts);

	enable_irq(ft6x06_ts->irq);
}

/*The ft6x06 device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft6x06_ts_interrupt(int irq, void *dev_id)
{
	struct ft6x06_ts_data *ft6x06_ts = dev_id;

	jz_notifier_call(NOTEFY_PROI_NORMAL, JZ_CLK_CHANGING, NULL);
	disable_irq_nosync(ft6x06_ts->irq);

#if 0
	if (ft6x060_ts->is_suspend)
		return IRQ_HANDLED;
#endif

	if (!work_pending(&ft6x06_ts->work)) {
		queue_work(ft6x06_ts->workqueue, &ft6x06_ts->work);
	} else {
		enable_irq(ft6x06_ts->irq);
	}

	return IRQ_HANDLED;
}

static void ft6x06_ts_reset(struct ft6x06_ts_data *ts)
{
	//printk("[FTS] ft6x06_ts_reset()\n");
	gpio_set_value(ts->pdata->reset, 1);
	msleep(5);
	gpio_set_value(ts->pdata->reset, 0);
	msleep(10);
	gpio_set_value(ts->pdata->reset, 1);
	msleep(15);
}

static void ft6x06_close(struct input_dev *dev)
{
	struct ft6x06_ts_data *ts = input_get_drvdata(dev);

	dev_dbg(&ts->client->dev, "[FTS]ft6x06 suspend\n");
	disable_irq(ts->pdata->irq);
}

static int ft6x06_open(struct input_dev *dev)
{
	struct ft6x06_ts_data *ts = input_get_drvdata(dev);

	dev_dbg(&ts->client->dev, "[FTS]ft6x06 resume.\n");
	ft6x06_ts_reset(ts);
	enable_irq(ts->pdata->irq);
	return 0;
}

static int ft6x06_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct ft6x06_platform_data *pdata =
	    (struct ft6x06_platform_data *)client->dev.platform_data;
	struct ft6x06_ts_data *ft6x06_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft6x06_ts = kzalloc(sizeof(struct ft6x06_ts_data), GFP_KERNEL);

	if (!ft6x06_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, ft6x06_ts);
	ft6x06_ts->irq_pin = pdata->irq;
	ft6x06_ts->irq = gpio_to_irq(pdata->irq);
	client->irq = ft6x06_ts->irq;
	ft6x06_ts->client = client;
	ft6x06_ts->pdata = pdata;
	ft6x06_ts->x_max = pdata->x_max - 1;
	ft6x06_ts->y_max = pdata->y_max - 1;
	ft6x06_ts->va_x_max = pdata->va_x_max - 1;
	ft6x06_ts->va_y_max = pdata->va_y_max - 1;

#ifdef CONFIG_PM
	err = gpio_request(pdata->reset, "ft6x06 reset");
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
			__func__);
		goto exit_request_fail;
	}
	gpio_direction_output(pdata->reset, 1);
#endif

	err = gpio_request(pdata->irq,"ft6x06 irq");
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to set gpio irq.\n",
			__func__);
		goto exit_request_fail;
	}
	gpio_direction_input(pdata->irq);
#ifndef DEBUG_LCD_VCC_ALWAYS_ON
	ft6x06_ts->vcc_reg = regulator_get(NULL, "vlcd");
	if (IS_ERR(ft6x06_ts->vcc_reg)) {
		dev_err(&client->dev, "failed to get VCC regulator.");
		err = PTR_ERR(ft6x06_ts->vcc_reg);
		goto exit_request_reset;
	}
	regulator_enable(ft6x06_ts->vcc_reg);
#endif
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft6x06_ts->input_dev = input_dev;

#ifdef CONFIG_FT6X06_MULTITOUCH
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);

	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ft6x06_ts->va_x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ft6x06_ts->va_y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
#else
	set_bit(ABS_X, input_dev->absbit);
	set_bit(ABS_Y, input_dev->absbit);
	set_bit(ABS_PRESSURE, input_dev->absbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(BTN_TOUCH, input_dev->keybit);

	input_set_abs_params(input_dev, ABS_X, 0, ft6x06_ts->va_x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_Y, 0, ft6x06_ts->va_y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_PRESSURE, 0, PRESS_MAX, 0 , 0);
#endif
	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);

	set_bit(KEY_HOMEPAGE, input_dev->keybit);
	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(INPUT_PROP_DIRECT, input_dev->propbit);

	input_dev->name = FT6X06_NAME;
	input_dev->id.bustype = BUS_I2C;
	input_dev->id.vendor = 0xDEAD;
	input_dev->id.product = 0xBEEF;
	input_dev->id.version = 10427;

	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft6x06_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}

	input_dev->open = ft6x06_open;
	input_dev->close = ft6x06_close;
	input_set_drvdata(input_dev, ft6x06_ts);
	/*make sure CTP already finish startup process */
	ft6x06_ts_reset(ft6x06_ts);
	msleep(150);

	/*get some register information */
	uc_reg_addr = FT6x06_REG_FW_VER;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT6x06_REG_POINT_RATE;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
		uc_reg_value * 10);

	/* Try to slow down point rate, Failed. */
	if (0) {
		char write_buf[2];
		write_buf[0] = FT6x06_REG_POINT_RATE;
		write_buf[1] = 1;
		ft6x06_i2c_Write(client, &write_buf[0], 2);

		uc_reg_addr = FT6x06_REG_POINT_RATE;
		ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
		dev_err(&client->dev, "[FTS] report rate is %dHz.\n",
			uc_reg_value * 10);


		uc_reg_addr = 0x89;
		ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
		dev_err(&client->dev, "[FTS] report rate 89is %dHz.\n",
			uc_reg_value * 10);

		write_buf[0] = 0x89;
		write_buf[1] = 1;
		ft6x06_i2c_Write(client, &write_buf[0], 2);

		uc_reg_addr = 0x89;
		ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
		dev_err(&client->dev, "[FTS] report rate 89is %dHz.\n",
			uc_reg_value * 10);
	}


	uc_reg_addr = FT6x06_REG_THGROUP;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
		uc_reg_value * 4);

	uc_reg_addr = FT6x06_REG_D_MODE;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] DEVICE_MODE = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT6x06_REG_G_MODE;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] G_MODE = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT6x06_REG_P_MODE;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] POWER_MODE = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT6x06_REG_STATE;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] STATE = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT6x06_REG_CTRL;
	ft6x06_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	dev_dbg(&client->dev, "[FTS] CTRL = 0x%x\n", uc_reg_value);

#ifdef SYSFS_DEBUG
	ft6x06_create_sysfs(client);
#endif

#ifdef FTS_CTL_IIC
	if (ft_rw_iic_drv_init(client) < 0)
		dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
				__func__);
#endif

	INIT_WORK(&ft6x06_ts->work, ft6x06_work_handler);
	ft6x06_ts->workqueue = create_singlethread_workqueue("ft6x06_tsc");

	err = request_irq(ft6x06_ts->irq, ft6x06_ts_interrupt,
			pdata->irqflags, client->dev.driver->name,
			ft6x06_ts);
	if (err < 0) {
		dev_err(&client->dev, "ft6x06_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

	disable_irq(ft6x06_ts->irq);
	jzgpio_ctrl_pull(pdata->irq / 32, 0, BIT(pdata->irq % 32));
	enable_irq(ft6x06_ts->irq);

	return 0;

exit_irq_request_failed:

exit_input_register_device_failed:
	input_free_device(input_dev);

#ifndef DEBUG_LCD_VCC_ALWAYS_ON
exit_request_reset:
#endif
#ifdef CONFIG_PM
	gpio_free(ft6x06_ts->pdata->reset);
#endif
exit_request_fail:
exit_input_dev_alloc_failed:
#ifndef DEBUG_LCD_VCC_ALWAYS_ON
	if (!IS_ERR(ft6x06_ts->vcc_reg)) {
		regulator_disable(ft6x06_ts->vcc_reg);
		regulator_put(ft6x06_ts->vcc_reg);
	}
#endif
	i2c_set_clientdata(client, NULL);
	kfree(ft6x06_ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

static int ft6x06_ts_remove(struct i2c_client *client)
{
	struct ft6x06_ts_data *ft6x06_ts;
	ft6x06_ts = i2c_get_clientdata(client);
	input_unregister_device(ft6x06_ts->input_dev);
#ifdef CONFIG_PM
	gpio_free(ft6x06_ts->pdata->reset);
#endif

#ifdef SYSFS_DEBUG
	ft6x06_release_sysfs(client);
#endif
#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
#endif
	free_irq(client->irq, ft6x06_ts);
#ifndef DEBUG_LCD_VCC_ALWAYS_ON
	if (!IS_ERR(ft6x06_ts->vcc_reg)) {
		regulator_disable(ft6x06_ts->vcc_reg);
		regulator_put(ft6x06_ts->vcc_reg);
	}
#endif
	kfree(ft6x06_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft6x06_ts_id[] = {
	{FT6X06_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ft6x06_ts_id);

static struct i2c_driver ft6x06_ts_driver = {
	.probe = ft6x06_ts_probe,
	.remove = ft6x06_ts_remove,
	.id_table = ft6x06_ts_id,
	.driver = {
		.name = FT6X06_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init ft6x06_ts_init(void)
{
	int ret;
	ret = i2c_add_driver(&ft6x06_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding ft6x06 driver failed "
				"(errno = %d)\n", ret);
	} else {
		pr_info("Successfully added driver %s\n",
				ft6x06_ts_driver.driver.name);
	}
	return ret;
}

static void __exit ft6x06_ts_exit(void)
{
	i2c_del_driver(&ft6x06_ts_driver);
}

module_init(ft6x06_ts_init);
module_exit(ft6x06_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft6x06 TouchScreen driver");
MODULE_LICENSE("GPL");
