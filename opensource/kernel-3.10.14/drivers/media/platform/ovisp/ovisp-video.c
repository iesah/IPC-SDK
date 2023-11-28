
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/bug.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>

#include <media/media-device.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-dma-contig.h>

#include "ovisp-isp.h"
#include "ovisp-video.h"
#include "ovisp-videobuf.h"
#include "ovisp-debugtool.h"
#include "ovisp-base.h"
#include "isp-debug.h"

#include "ovisp-csi.h"

#define CAMERA_NEED_REGULATOR		(0)

static struct ovisp_camera_format isp_oformats[] = {
	{
		.name     = "YUV 4:2:2 packed, YCbYCr",
		.fourcc   = V4L2_PIX_FMT_YUYV,
		.depth    = 16,
	},
	{
		.name     = "YUV 4:2:0 packed",
		.fourcc   = V4L2_PIX_FMT_YUV420,
		.depth    = 12,
	},
	{
		.name     = "YUV 4:2:0 semi planar, Y/CbCr",
		.fourcc   = V4L2_PIX_FMT_NV12,
		.depth    = 12,
	},
	{
		.name	  ="RAW8",
		.fourcc	  = V4L2_PIX_FMT_SBGGR8,
		.depth	  = 8,
	},
	{
		.name	  ="RAW10 (BGGR)",
		.fourcc	  = V4L2_PIX_FMT_SBGGR10,
		.depth	  = 16,
	},
	{
		.name	  ="RAW10 (GRBG)",
		.fourcc	  = V4L2_PIX_FMT_SGRBG10,
		.depth	  = 16,
	}
};

static struct ovisp_camera_format sensor_oformats[] = {
	{
		.name     = "YUV 4:2:2 packed, YCbYCr",
		.code	  = V4L2_MBUS_FMT_YUYV8_2X8,
		.fourcc   = V4L2_PIX_FMT_YUYV,
		.depth    = 16,
	},
	{
		.name	  ="RAW8 (BGGR)",
		.code	  = V4L2_MBUS_FMT_SBGGR8_1X8,
		.fourcc	  = V4L2_PIX_FMT_SBGGR8,
		.depth	  = 8,
	},
	{
		.name	  ="RAW10 (BGGR)",
		.code	  = V4L2_MBUS_FMT_SBGGR10_1X10,
		.fourcc	  = V4L2_PIX_FMT_SBGGR10,
		.depth	  = 16,
	},
	{
		.name	  ="RAW10 (GRBG)",
		.code	  = V4L2_MBUS_FMT_SGRBG10_1X10,
		.fourcc	  = V4L2_PIX_FMT_SGRBG10,
		.depth	  = 16,
	},
	{
		.name	  ="YUV422",
		.code	  = V4L2_MBUS_FMT_YUYV8_1X16,
		.fourcc	  = V4L2_PIX_FMT_YUYV,
		.depth	  = 16,
	}
};

static int ovisp_subdev_mclk_on(struct ovisp_camera_dev *camdev,
			int index)
{
	int ret;

	if (camdev->csd[index].mclk) {
		clk_enable(camdev->csd[index].mclk);
	} else {
		ret = isp_dev_call(camdev->isp, mclk_on, index);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			return -EINVAL;
	}

	return 0;
}

static int ovisp_subdev_mclk_off(struct ovisp_camera_dev *camdev,
			int index)
{
	int ret;

	if (camdev->csd[index].mclk) {
		clk_disable(camdev->csd[index].mclk);
	} else {
		ret = isp_dev_call(camdev->isp, mclk_off, index);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			return -EINVAL;
	}

	return 0;
}

static int ovisp_subdev_power_on(struct ovisp_camera_dev *camdev,
					struct ovisp_camera_client *client,
					int index)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[index];
	struct v4l2_cropcap caps;
	int ret;

	ret = ovisp_subdev_mclk_on(camdev, index);
	if (ret)
		return ret;

	/* first camera work power on */
#if (CAMERA_NEED_REGULATOR)
	if(!regulator_is_enabled(camdev->camera_power))
	    regulator_enable(camdev->camera_power);
#endif

	if (client->power) {
		ret = client->power(1);
		if (ret)
			goto err;
	}

	if (client->reset) {
		ret = client->reset();
		if (ret)
			goto err;
	}

	csd->prop.bypass = csd->bypass;
	csd->prop.index = index;
	ret = isp_dev_call(camdev->isp, open, &csd->prop);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		goto err;


	ISP_PRINT(ISP_INFO,"--%s:%d index=%d\n", __func__, __LINE__, index);
	if (camdev->input >= 0) {
		ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		ret = v4l2_subdev_call(csd->sd, core, s_power, 1);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			goto err;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		ret = v4l2_subdev_call(csd->sd, core, reset, 1);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			goto err;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		ret = v4l2_subdev_call(csd->sd, core, init, 1);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			goto err;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		ret = v4l2_subdev_call(csd->sd, video, cropcap, &caps);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			goto err;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		csd->max_width = caps.bounds.width;
		csd->max_height = caps.bounds.height;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
		ret = isp_dev_call(camdev->isp, config, NULL);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			goto err;
	}
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
	return 0;

err:
	ovisp_subdev_mclk_off(camdev, index);
	return -EINVAL;
}

static int ovisp_subdev_power_off(struct ovisp_camera_dev *camdev,
					struct ovisp_camera_client *client,
					int index)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[index];
	int ret;

	if (camdev->input >= 0) {
		ret = v4l2_subdev_call(csd->sd, core, s_power, 0);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			return -EINVAL;
	}

	ret = isp_dev_call(camdev->isp, close, &csd->prop);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	/*digital power down*/
	if (client->power) {
		ret = client->power(0);
		if (ret)
			return -EINVAL;
	}
	/*cpu_xxx power should not be power on or off*/
	/*analog power off*/
#if (CAMERA_NEED_REGULATOR)
	if(regulator_is_enabled(camdev->camera_power))
		        regulator_disable(camdev->camera_power);
#endif

	ovisp_subdev_mclk_off(camdev, index);

	return 0;
}

struct ovisp_camera_format *ovisp_camera_find_format(struct v4l2_mbus_framefmt *mbus,
				struct v4l2_format *f, int index)
{
	struct ovisp_camera_format *fmt = NULL;
	unsigned int num;
	int i;

	if(mbus){
		fmt = sensor_oformats;
		num = ARRAY_SIZE(sensor_oformats);
		for (i = 0; i < num; i++) {
			if (fmt->code == mbus->code)
				break;
			fmt++;
		}
	}else{
		fmt = isp_oformats;
		num = ARRAY_SIZE(isp_oformats);
		for (i = 0; i < num; i++) {
			if (f && fmt->fourcc == f->fmt.pix.pixelformat)
				break;
			if(i == index)
				break;
			fmt++;
		}
	}

	if (i == num)
		return NULL;

	return fmt;
}

static int ovisp_camera_get_mclk(struct ovisp_camera_dev *camdev,
					struct ovisp_camera_client *client,
					int index)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[index];
	struct clk* mclk_parent;

	if (!(client->flags & CAMERA_CLIENT_CLK_EXT) || !client->mclk_name) {
		csd->mclk = NULL;
		return 0;
	}

	csd->mclk = clk_get(camdev->dev, client->mclk_name);
	if (IS_ERR(csd->mclk)) {
		ISP_PRINT(ISP_ERROR,"Cannot get sensor input clock \"%s\"\n",
			client->mclk_name);
		return PTR_ERR(csd->mclk);
	}

	if (client->mclk_parent_name) {
		mclk_parent = clk_get(camdev->dev, client->mclk_parent_name);
		if (IS_ERR(mclk_parent)) {
			ISP_PRINT(ISP_ERROR,"Cannot get sensor input parent clock \"%s\"\n",
				client->mclk_parent_name);
			clk_put(csd->mclk);
			return PTR_ERR(mclk_parent);
		}

		clk_set_parent(csd->mclk, mclk_parent);
		clk_put(mclk_parent);
	}

	clk_set_rate(csd->mclk, client->mclk_rate);

	return 0;
}

static int ovisp_camera_init_client(struct ovisp_camera_dev *camdev,
					struct ovisp_camera_client *client,
					int index)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[index];
	int i2c_adapter_id;
	int err = 0;
	int ret;

	if (!client->board_info) {
		ISP_PRINT(ISP_ERROR,"Invalid client info\n");
		return -EINVAL;
	}

	if (client->flags & CAMERA_CLIENT_INDEP_I2C) {
		i2c_adapter_id = client->i2c_adapter_id;
		ISP_PRINT(ISP_INFO,"11111 %s i2c_adapter_id = %d\n",__func__,i2c_adapter_id);
	}
	else {
		i2c_adapter_id = camdev->pdata->i2c_adapter_id;
		ISP_PRINT(ISP_INFO,"22222 %s i2c_adapter_id = %d\n",__func__,i2c_adapter_id);
	}
	if (client->flags & CAMERA_CLIENT_ISP_BYPASS)
		csd->bypass = 1;
	else
		csd->bypass = 0;

	ret = ovisp_camera_get_mclk(camdev, client, index);
	//if (ret)
	//return ret;

	ret = isp_dev_call(camdev->isp, init, NULL);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to init isp\n");
		clk_put(csd->mclk);
		return -EINVAL;
	}

	ret = ovisp_subdev_power_on(camdev, client, index);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to power on subdev(%s)\n",
			client->board_info->type);
		clk_put(csd->mclk);
		err = -ENODEV;
		goto isp_dev_release;
	}

	csd->i2c_adap = i2c_get_adapter(i2c_adapter_id);
	if (!csd->i2c_adap) {
		ISP_PRINT(ISP_ERROR,"Cannot get I2C adapter(%d)\n",
			i2c_adapter_id);
		clk_put(csd->mclk);
		err = -ENODEV;
		goto subdev_power_off;
	}
	csd->sd = v4l2_i2c_new_subdev_board(&camdev->v4l2_dev,
		csd->i2c_adap,
		client->board_info,
		NULL);
	if (!csd->sd) {
		ISP_PRINT(ISP_ERROR,"Cannot get subdev(%s)\n",
			client->board_info->type);
		clk_put(csd->mclk);
		i2c_put_adapter(csd->i2c_adap);
		err = -EIO;
		goto subdev_power_off;
	}

subdev_power_off:
	ret = ovisp_subdev_power_off(camdev, client, index);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to power off subdev(%s)\n",
			client->board_info->type);
		err = -EINVAL;
	}

isp_dev_release:
	ret = isp_dev_call(camdev->isp, release, NULL);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to init isp\n");
		err = -EINVAL;
	}
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
	return err;
}

static void ovisp_camera_free_client(struct ovisp_camera_dev *camdev,
			int index)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[index];

	v4l2_device_unregister_subdev(csd->sd);
	i2c_unregister_device(v4l2_get_subdevdata(csd->sd));
	i2c_put_adapter(csd->i2c_adap);
	clk_put(csd->mclk);
}

static int ovisp_camera_init_subdev(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_platform_data *pdata = camdev->pdata;
	unsigned int i;
	int ret;

	if (!pdata->client) {
		ISP_PRINT(ISP_ERROR,"Invalid client data\n");
		return -EINVAL;
	}

	camdev->clients = 0;
	for (i = 0; i < pdata->client_num; i++) {
		ret = ovisp_camera_init_client(camdev,
					&pdata->client[i], camdev->clients);
		if (ret)
			continue;

		camdev->csd[camdev->clients++].client = &pdata->client[i];
		if (camdev->clients > OVISP_CAMERA_CLIENT_NUM) {
			ISP_PRINT(ISP_ERROR,"Too many clients\n");
			return -EINVAL;
		}
	}

	ISP_PRINT(ISP_INFO,"Detect %d clients\n", camdev->clients);

	if (camdev->clients == 0)
		return -ENODEV;

	return 0;
}

static void ovisp_camera_free_subdev(struct ovisp_camera_dev *camdev)
{
	unsigned int i;

	for (i = 0; i < camdev->clients; i++)
		ovisp_camera_free_client(camdev, i);
}

static int ovisp_camera_active(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_capture *capture = &camdev->capture;

	return capture->running;
}

static int ovisp_camera_update_buffer(struct ovisp_camera_dev *camdev, int index)
{
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct isp_buffer buf;
	int ret;
	if(camdev->vbq.memory == V4L2_MEMORY_MMAP){
		buf.addr = ovisp_vb2_plane_paddr(&capture->active[index]->vb, 0);
	}
	else if(camdev->vbq.memory == V4L2_MEMORY_USERPTR){
		buf.addr = ovisp_vb2_plane_vaddr(&capture->active[index]->vb, 0);
	}else{
		ISP_PRINT(ISP_ERROR,"the type of memory isn't supported!\n");
		return -EINVAL;
	}
//	ISP_PRINT(ISP_INFO,"%s:buf.addr:0x%lx  index = %d\n", __func__, buf.addr, index);
	ret = isp_dev_call(camdev->isp, update_buffer, &buf, index);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	return 0;
}
#if 0
static int ovisp_camera_enable_capture(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct isp_buffer buf;
	int ret;

	if(camdev->vbq.memory == V4L2_MEMORY_MMAP){
		buf.addr = ovisp_vb2_plane_paddr(&capture->active->vb, 0);
	}else if(camdev->vbq.memory == V4L2_MEMORY_USERPTR){
		buf.addr = ovisp_vb2_plane_vaddr(&capture->active->vb, 0);
	}else{
		ISP_PRINT(ISP_ERROR,"the type of memory isn't supported!\n");
		return -EINVAL;
	}
	ISP_PRINT(ISP_INFO,"%s:%d buf.addr:0x%lx\n", __func__,__LINE__, buf.addr);
	ret = isp_dev_call(camdev->isp, enable_capture, &buf);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	return 0;
}

static int ovisp_camera_disable_capture(struct ovisp_camera_dev *camdev)
{
	int ret;

	ret = isp_dev_call(camdev->isp, disable_capture, NULL);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	return 0;
}
#endif
static int ovisp_camera_start_capture(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct isp_capture cap;
	int ret;
	if (capture->running)
		return 0;

	ISP_PRINT(ISP_INFO,"*********************start_capture begin*************************\n");
	ISP_PRINT(ISP_INFO,"Start capture(%s)\n",
			camdev->snapshot ? "snapshot" : "preview");
#if 0
	/*get physical addr*/
	if(camdev->vbq.memory == V4L2_MEMORY_MMAP){
		cap.buf.addr = ovisp_vb2_plane_paddr(&capture->active->vb, 0);
	}
	else if(camdev->vbq.memory == V4L2_MEMORY_USERPTR){
		cap.buf.addr = ovisp_vb2_plane_vaddr(&capture->active->vb, 0);
	}else{
		ISP_PRINT(ISP_ERROR,"the type of memory isn't supported!\n");
		return -EINVAL;
	}
#endif
	cap.snapshot = camdev->snapshot;
	cap.client = camdev->csd[camdev->input].client;
	capture->running = 1;
	ret = isp_dev_call(camdev->isp, start_capture, &cap);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Start capture failed %d\n", ret);
		return -EINVAL;
	}

	/*ret = v4l2_subdev_call(sd, core, g_register, &frame->vmfmt);*/
	ISP_PRINT(ISP_INFO,"*********************start_capture end*************************\n");
	return 0;
}

static int ovisp_camera_stop_capture(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_capture *capture = &camdev->capture;
	int ret;

	ISP_PRINT(ISP_INFO,"Stop capture(%s)\n",
			camdev->snapshot ? "snapshot" : "preview");

	ret = isp_dev_call(camdev->isp, stop_capture, NULL);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	capture->active[0] = NULL;
	capture->active[1] = NULL;
	capture->running = 0;

	return 0;
}

static int ovisp_camera_start_streaming(struct ovisp_camera_dev *camdev)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct ovisp_camera_frame *frame = &camdev->frame;
	struct ovisp_camera_devfmt *cfmt = &frame->cfmt;
	struct isp_format *ifmt = &frame->ifmt;
	int ret;
//	capture->running = 0;
	capture->active[0] = NULL;
	capture->active[1] = NULL;

	ISP_PRINT(ISP_WARNING,"Set format(%s).\n", camdev->snapshot ? "snapshot" : "preview");
	/*1. get camera's format */
	ret = isp_dev_call(camdev->isp, g_devfmt, ifmt);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to set isp format\n");
		return -EINVAL;
	}
	cfmt->vmfmt.width = ifmt->vfmt.dev_width;
	cfmt->vmfmt.height = ifmt->vfmt.dev_height;
	ifmt->fmt_data = &cfmt->fmt_data;
	memset(ifmt->fmt_data, 0, sizeof(*ifmt->fmt_data));

	ret = v4l2_subdev_call(csd->sd, video, s_mbus_fmt, &cfmt->vmfmt);
	if (ret && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to set device format\n");
		return -EINVAL;
	}

	/*2. csi phy start here*/
	ret = isp_dev_call(camdev->isp, pre_fmt, ifmt);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to set isp format\n");
		return -EINVAL;
	}

	if(camdev->first_init) {
		ret = v4l2_subdev_call(csd->sd, video, s_stream, 1);
		camdev->first_init = 0;
	}

	/*now csi got data!!!!!*/
	if (ret && ret != -ENOIOCTLCMD)
		if (ret && ret != -ENOIOCTLCMD) {
			ISP_PRINT(ISP_ERROR,"Failed to set device stream\n");
			return -EINVAL;
		}


	/*now start capture,
	 * in this procedure, we will use firmware to set set format
	 *
	 * */
	if (!capture->running) {
		ISP_PRINT(ISP_INFO,"%s:%d start_capture!\n",__func__,__LINE__);
		ovisp_camera_start_capture(camdev);
	}
	ISP_PRINT(ISP_INFO,"the main procedure ended ........., now start capture ......\n");
	return 0;
}

static int ovisp_camera_stop_streaming(struct ovisp_camera_dev *camdev)
{
	int ret = 0;
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	int status;

	if (!ovisp_camera_active(camdev))
		return 0;

	status = ovisp_camera_stop_capture(camdev);
	if(camdev->first_init == 0) {
		ret = v4l2_subdev_call(csd->sd, video, s_stream, 0);
		if (ret && ret != -ENOIOCTLCMD)
			return ret;
	}

	return status;
}

static int ovisp_camera_try_format(struct ovisp_camera_dev *camdev, struct v4l2_format *f)
{
	struct ovisp_camera_frame *frame = &camdev->frame;
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	struct ovisp_camera_devfmt *cfmt = &frame->cfmt;
	struct isp_format *ifmt = &frame->ifmt;
	struct ovisp_camera_format *in_fmt = NULL;
	struct ovisp_camera_format *out_fmt = NULL;
	int ret;

	ISP_PRINT(ISP_INFO,"--%s:%d  %d\n", __func__, __LINE__, camdev->input);

	cfmt->vmfmt.width = f->fmt.pix.width;
	cfmt->vmfmt.height = f->fmt.pix.height;

	/*to match the sensor supported format*/
	ret = v4l2_subdev_call(csd->sd, video, try_mbus_fmt, &cfmt->vmfmt);
	if (ret && ret != -ENOIOCTLCMD)
		return -EINVAL;

	in_fmt = ovisp_camera_find_format(&cfmt->vmfmt, NULL, -1);
	if (!in_fmt) {
		ISP_PRINT(ISP_ERROR,"Sensor output format (0x%08x) invalid\n",
				in_fmt->fourcc);
		return -EINVAL;
	}

	out_fmt = ovisp_camera_find_format(NULL, f, -1);
	if (!out_fmt) {
		ISP_PRINT(ISP_ERROR,"Fourcc format (0x%08x) invalid\n",
				f->fmt.pix.pixelformat);
		return -EINVAL;
	}
	ifmt->vfmt.dev_width = cfmt->vmfmt.width;
	ifmt->vfmt.dev_height = cfmt->vmfmt.height;
	ifmt->vfmt.dev_fourcc = in_fmt->fourcc;
	ifmt->vfmt.colorspace = cfmt->vmfmt.colorspace;
	ifmt->vfmt.field = cfmt->vmfmt.field;
	ifmt->vfmt.width = f->fmt.pix.width;
	ifmt->vfmt.height = f->fmt.pix.height;
	ifmt->vfmt.fourcc = f->fmt.pix.pixelformat;
	ifmt->vfmt.depth = out_fmt->depth;

	/*to match the format we supported*/
	ret = isp_dev_call(camdev->isp, try_fmt, ifmt);

	ISP_PRINT(ISP_INFO,"%s:%d\n", __func__, __LINE__);
	if (ret < 0 && ret != -ENOIOCTLCMD)
		return -EINVAL;

	f->fmt.pix.bytesperline =
		(f->fmt.pix.width * out_fmt->depth) >> 3;
	f->fmt.pix.sizeimage =
		f->fmt.pix.height * f->fmt.pix.bytesperline;

	ISP_PRINT(ISP_INFO,"%s:%d dev_width %d out_width %d\n", __func__, __LINE__, ifmt->vfmt.dev_width, f->fmt.pix.width);
	ISP_PRINT(ISP_INFO,"%s:%d dev_height %d out_height %d\n", __func__, __LINE__, ifmt->vfmt.dev_height, f->fmt.pix.height);
	ISP_PRINT(ISP_INFO,"%s:%d mbus %08X format %08X\n", __func__, __LINE__, ifmt->vfmt.dev_fourcc, f->fmt.pix.pixelformat);

	return 0;
}

static int ovisp_camera_irq_notify(unsigned int status, void *data)
{
	struct ovisp_camera_dev *camdev = (struct ovisp_camera_dev *)data;
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct ovisp_camera_frame *frame = &camdev->frame;
	struct ovisp_camera_buffer *buf = NULL;
	unsigned long flags;
	if (!capture->running)
		return 0;

	if (status & ISP_NOTIFY_DATA_DONE) {
		buf = NULL;
		spin_lock_irqsave(&camdev->slock, flags);
		if((status & ISP_NOTIFY_DATA_DONE0)){
			buf = capture->active[0];
			capture->active[0] = NULL;
		}else{
			buf = capture->active[1];
			capture->active[1] = NULL;
		}
		spin_unlock_irqrestore(&camdev->slock, flags);
		if(buf && buf->vb.state == VB2_BUF_STATE_ACTIVE){
			if(camdev->snapshot)
				buf->vb.v4l2_buf.field = frame->field;
			buf->vb.v4l2_buf.sequence = capture->out_frames++;
			do_gettimeofday(&buf->vb.v4l2_buf.timestamp);
			isp_dev_call(camdev->isp, g_outinfo, buf->priv);
			capture->last = buf;
			vb2_buffer_done(&buf->vb, VB2_BUF_STATE_DONE);
		}else
			capture->lose_frames++;

	}
	if(!(status & ISP_NOTIFY_UPDATE_BUF))
		return 0;
	if (status & ISP_NOTIFY_OVERFLOW){
		capture->error_frames++;
		if((status & ISP_NOTIFY_DROP_FRAME0)){
			buf = capture->active[0];
			capture->active[0] = NULL;
		}else{
			buf = capture->active[1];
			capture->active[1] = NULL;
		}
		list_add_tail(&(buf->list),&(capture->list));
	}
	if (status & ISP_NOTIFY_DATA_START){
		buf = NULL;
		capture->in_frames++;
		spin_lock_irqsave(&camdev->slock, flags);
		if (!list_empty(&capture->list)){
			buf = list_entry(capture->list.next,
				struct ovisp_camera_buffer, list);
			list_del(&buf->list);
		}
		spin_unlock_irqrestore(&camdev->slock, flags);
		if(buf){
			if((status & ISP_NOTIFY_DATA_START0)){
				capture->active[1] = buf;
				ovisp_camera_update_buffer(camdev, 1);
			}else{
				capture->active[0] = buf;
				ovisp_camera_update_buffer(camdev, 0);
			}
		}
	}

	if (status & ISP_NOTIFY_DROP_FRAME){
		buf = NULL;
		if(((status & ISP_NOTIFY_DROP_FRAME0) && capture->active[0] == NULL)
			|| ((status & ISP_NOTIFY_DROP_FRAME1) && capture->active[1] == NULL)){
			capture->drop_frames++;
			spin_lock_irqsave(&camdev->slock, flags);
			if (!list_empty(&capture->list)){
				buf = list_entry(capture->list.next,
						struct ovisp_camera_buffer, list);
				list_del(&buf->list);
			}
			spin_unlock_irqrestore(&camdev->slock, flags);
			if(buf){
				if((status & ISP_NOTIFY_DROP_FRAME0)){
					capture->active[0] = buf;
					ovisp_camera_update_buffer(camdev, 0);
				}else{
					capture->active[1] = buf;
					ovisp_camera_update_buffer(camdev, 1);
				}
			}
		}
	}

	return 0;
}

static int ovisp_vb2_queue_setup(struct vb2_queue *vq, const struct v4l2_format *fmt,unsigned int *nbuffers,
		unsigned int *nplanes, unsigned long sizes[],
		void *alloc_ctxs[])
{
	int ret = 0;
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vq);
	struct ovisp_camera_capture *capture = &camdev->capture;
	unsigned long size;

		ret = isp_dev_call(camdev->isp, g_size, &size);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to get buffer's size\n");
		return -EINVAL;
	}
	if (0 == *nbuffers)
		*nbuffers = 32;

	if(vq->memory == V4L2_MEMORY_MMAP){
		while (size * *nbuffers > OVISP_CAMERA_BUFFER_MAX)
			(*nbuffers)--;
	}
	*nplanes = 1;
	sizes[0] = size;
	alloc_ctxs[0] = camdev->alloc_ctx;
	ISP_PRINT(ISP_INFO,"*nbuffers = %d\n",*nbuffers);
	if (*nbuffers == 1)
		camdev->snapshot = 1;
	else
		camdev->snapshot = 0;

	INIT_LIST_HEAD(&capture->list);
	capture->active[0] = NULL;
	capture->active[1] = NULL;
	capture->running = 0;
	capture->drop_frames = 0;
	capture->lose_frames = 0;
	capture->error_frames = 0;
	capture->in_frames = 0;
	capture->out_frames = 0;

	return 0;
}

static int ovisp_vb2_buffer_init(struct vb2_buffer *vb)
{
	vb->v4l2_buf.reserved = ovisp_vb2_plane_paddr(vb, 0);
	return 0;
}

static int ovisp_vb2_buffer_prepare(struct vb2_buffer *vb)
{
	struct ovisp_camera_buffer *buf =
		container_of(vb, struct ovisp_camera_buffer, vb);
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vb->vb2_queue);
	int ret = 0;
	unsigned long size;

	ret = isp_dev_call(camdev->isp, g_size, &size);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to get buffer's size\n");
		return -EINVAL;
	}
	if (vb2_plane_size(vb, 0) < size) {
		ISP_PRINT(ISP_ERROR,"Data will not fit into plane (%lu < %lu)\n",
				vb2_plane_size(vb, 0), size);
		return -EINVAL;
	}

	vb2_set_plane_payload(&buf->vb, 0, size);

	vb->v4l2_buf.reserved = ovisp_vb2_plane_paddr(vb, 0);
	return 0;
}

static void ovisp_vb2_buffer_queue(struct vb2_buffer *vb)
{
	struct ovisp_camera_buffer *buf =
		container_of(vb, struct ovisp_camera_buffer, vb);
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vb->vb2_queue);
	struct ovisp_camera_capture *capture = &camdev->capture;
	unsigned long flags;

	spin_lock_irqsave(&camdev->slock, flags);
	list_add_tail(&buf->list, &capture->list);
#if 0
	ISP_PRINT(ISP_INFO,"%s:%d ---add_list---\n",__func__,__LINE__);
	if (!capture->active) {
		ISP_PRINT(ISP_INFO,"%s:%d ---active = NULL---\n",__func__,__LINE__);
		capture->active = buf;
		if (capture->running)
			ovisp_camera_enable_capture(camdev);
	}
#endif
	spin_unlock_irqrestore(&camdev->slock, flags);

}

static int ovisp_vb2_start_streaming(struct vb2_queue *vq)
{
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vq);
	if (ovisp_camera_active(camdev)){
		return -EBUSY;
	}

	return ovisp_camera_start_streaming(camdev);
}

static int ovisp_vb2_stop_streaming(struct vb2_queue *vq)
{
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vq);

	if (!ovisp_camera_active(camdev))
		return -EINVAL;

	return ovisp_camera_stop_streaming(camdev);
}

static void ovisp_vb2_lock(struct vb2_queue *vq)
{
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vq);
	mutex_lock(&camdev->mlock);
}

static void ovisp_vb2_unlock(struct vb2_queue *vq)
{
	struct ovisp_camera_dev *camdev = vb2_get_drv_priv(vq);
	mutex_unlock(&camdev->mlock);
}

static struct vb2_ops ovisp_vb2_qops = {
	.queue_setup		= ovisp_vb2_queue_setup,
	.buf_init		= ovisp_vb2_buffer_init,
	.buf_prepare		= ovisp_vb2_buffer_prepare,
	.buf_queue		= ovisp_vb2_buffer_queue,
	.wait_prepare		= ovisp_vb2_unlock,
	.wait_finish		= ovisp_vb2_lock,
	.start_streaming	= ovisp_vb2_start_streaming,
	.stop_streaming		= ovisp_vb2_stop_streaming,
};

static int ovisp_vidioc_querycap(struct file *file, void  *priv,
		struct v4l2_capability *cap)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	strcpy(cap->driver, "ovisp");
	strcpy(cap->card, "ovisp");
	strlcpy(cap->bus_info, camdev->v4l2_dev.name, sizeof(cap->bus_info));
	cap->version = OVISP_CAMERA_VERSION;
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	return 0;
}
/**
* ovisp_vidioc_g_priority() - get priority handler
* @file: file ptr
* @priv: file handle
* @prio: ptr to v4l2_priority structure
*/
static int ovisp_vidioc_g_priority(struct file *file, void *priv,
                          enum v4l2_priority *prio)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	*prio = v4l2_prio_max(&camdev->prio);
	return 0;
}
/**
* ovisp_vidioc_s_priority() - set priority handler
* @file: file ptr
* @priv: file handle
* @prio: ptr to v4l2_priority structure
*/
static int ovisp_vidioc_s_priority(struct file *file, void *priv, enum v4l2_priority p)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_fh *fh = priv;
	return v4l2_prio_change(&camdev->prio, &fh->prio, p);
}

static int ovisp_vidioc_enum_fmt_vid_cap(struct file *file, void  *priv,
		struct v4l2_fmtdesc *f)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	struct ovisp_camera_format *fmt;
	struct v4l2_mbus_framefmt mbus;
	int ret;

	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	if (csd->bypass) {
		ret = v4l2_subdev_call(csd->sd, video, enum_mbus_fmt, f->index, &mbus.code);
		if (ret && ret != -ENOIOCTLCMD)
			return -EINVAL;
		fmt = ovisp_camera_find_format(&mbus, NULL, -1);
	} else {
		fmt = ovisp_camera_find_format(NULL, NULL, f->index);
	}

	if (!fmt) {
		return -EINVAL;
	}
	strlcpy(f->description, fmt->name, sizeof(f->description));
	f->pixelformat = fmt->fourcc;
	return 0;
}

static int ovisp_vidioc_g_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	int ret = 0;
	ret = isp_dev_call(camdev->isp, g_fmt, f);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to get isp format\n");
		return -EINVAL;
	}
	return 0;
}

#define FUNC_LINE  ISP_PRINT(ISP_INFO,"%s,%d\n", __func__, __LINE__)
static int ovisp_vidioc_try_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_frame *frame = &camdev->frame;
	int ret;

	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	if (f->fmt.pix.field == V4L2_FIELD_ANY)
		f->fmt.pix.field = V4L2_FIELD_INTERLACED;
	else if (V4L2_FIELD_INTERLACED != f->fmt.pix.field)
		return -EINVAL;

	v4l_bound_align_image(&f->fmt.pix.width,
			48, OVISP_CAMERA_WIDTH_MAX, 2,
			&f->fmt.pix.height,
			32, OVISP_CAMERA_HEIGHT_MAX, 0, 0);

	if (f->fmt.pix.width  < 48 || f->fmt.pix.width  > OVISP_CAMERA_WIDTH_MAX ||
			f->fmt.pix.height < 32 || f->fmt.pix.height > OVISP_CAMERA_HEIGHT_MAX) {
		ISP_PRINT(ISP_ERROR,"Invalid format (%dx%d)\n",
				f->fmt.pix.width, f->fmt.pix.height);
		return -EINVAL;
	}


	ret = ovisp_camera_try_format(camdev, f);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Format(%dx%d,%x/%x) is unsupported\n",
				f->fmt.pix.width,
				f->fmt.pix.height,
				frame->ifmt.vfmt.dev_fourcc,
				frame->ifmt.vfmt.fourcc);
		return ret;
	}

	return 0;
}

static int ovisp_vidioc_s_fmt_vid_cap(struct file *file, void *priv,
		struct v4l2_format *f)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_frame *frame = &camdev->frame;
	struct vb2_queue *q = &camdev->vbq;
	struct ovisp_fh *fh = priv;
	int ret;

	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	/* check priority */
	ret = v4l2_prio_check(&camdev->prio, fh->prio);
	if(0 != ret)
		return ret;


	ret = ovisp_vidioc_try_fmt_vid_cap(file, priv, f);
	if (ret < 0)
		return ret;
	/*set isp format */
	ret = isp_dev_call(camdev->isp, s_fmt, &frame->ifmt);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		ISP_PRINT(ISP_ERROR,"Failed to set isp format\n");
		return -EINVAL;
	}
	if (vb2_is_streaming(q))
		return -EBUSY;

	return 0;
}

static int ovisp_vidioc_reqbufs(struct file *file, void *priv,
		struct v4l2_requestbuffers *p)
{
	int ret = 0;
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct vb2_queue *q = &camdev->vbq;
	ISP_PRINT(ISP_INFO,"%s[%d]\n", __func__, __LINE__);
	if(p->type != q->type){
		ISP_PRINT(ISP_ERROR,"%s[%d] req->type = %d, queue->type = %d\n", __func__, __LINE__,
					p->type, q->type);
		return -EINVAL;
	}
	if(p->memory == V4L2_MEMORY_USERPTR){
		if(p->count == 0){
			ret = isp_dev_call(camdev->isp, tlb_unmap_all_vaddr);
		}
		if(ret < 0){
			ISP_PRINT(ISP_ERROR," %s[%d] tlb operator failed!\n", __func__, __LINE__);
			return -EINVAL;
		}
	}else{
		ISP_PRINT(ISP_WARNING,"%s[%d] sorry, ISP driver cann't support V4L2_MEMORY_MMAP now!\n", __func__, __LINE__);
		return -EPERM;
	}
	return vb2_reqbufs(&camdev->vbq, p);
}

static int ovisp_vidioc_querybuf(struct file *file, void *priv,
		struct v4l2_buffer *p)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	return vb2_querybuf(&camdev->vbq, p);
}

static int ovisp_vidioc_qbuf(struct file *file, void *priv,
		struct v4l2_buffer *p)
{
	int ret = 0;
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	/* ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__); */
	if(p->memory == V4L2_MEMORY_USERPTR){
		dma_cache_sync(NULL,(void *)(p->m.userptr),p->length, DMA_FROM_DEVICE);
		ret = isp_dev_call(camdev->isp, tlb_map_one_vaddr,p->m.userptr,p->length);
		if(ret < 0){
			ISP_PRINT(ISP_ERROR,"%s[%d] tlb operator failed!\n", __func__, __LINE__);
			return -EINVAL;
		}
	}
	return vb2_qbuf(&camdev->vbq, p);
}

static int ovisp_vidioc_dqbuf(struct file *file, void *priv,
		struct v4l2_buffer *p)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	int status;
	/* ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__); */
	status = vb2_dqbuf(&camdev->vbq, p, file->f_flags & O_NONBLOCK);
	return status;
}

static int ovisp_vidioc_streamon(struct file *file, void *priv,
		enum v4l2_buf_type i)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	if (ovisp_camera_active(camdev))
		return -EBUSY;

	return vb2_streamon(&camdev->vbq, i);
}

static int ovisp_vidioc_streamoff(struct file *file, void *priv,
		enum v4l2_buf_type i)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	return vb2_streamoff(&camdev->vbq, i);
}

static int ovisp_vidioc_enum_input(struct file *file, void *priv,
		struct v4l2_input *inp)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_client *client;

	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	if (inp->index >= camdev->clients)
		return -EINVAL;

	client = camdev->csd[inp->index].client;

	inp->type = V4L2_INPUT_TYPE_CAMERA;
	inp->std = V4L2_STD_525_60;
	strncpy(inp->name, client->board_info->type, I2C_NAME_SIZE);

	return 0;
}

static int ovisp_vidioc_g_input(struct file *file, void *priv,
		unsigned int *index)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	*index = camdev->input;
	return 0;
}

static int ovisp_vidioc_s_input(struct file *file, void *priv,
		unsigned int index)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_fh *fh = priv;
	int ret;
	ISP_PRINT(ISP_INFO,"--%s:%d\n", __func__, __LINE__);
	if (ovisp_camera_active(camdev))
		return -EBUSY;

	if (index == camdev->input)
		return 0;

	if (index >= camdev->clients) {
		ISP_PRINT(ISP_ERROR,"Input(%d) exceeds chip maximum(%d)\n",
				index, camdev->clients);
		return -EINVAL;
	}
	/* check priority */
	ret = v4l2_prio_check(&camdev->prio, fh->prio);
	if(0 != ret)
		return ret;


	if (camdev->input >= 0) {
		/* Old client. */
		ret = ovisp_subdev_power_off(camdev,
				camdev->csd[camdev->input].client,
				camdev->input);
		if (ret) {
			ISP_PRINT(ISP_ERROR,"Failed to power off subdev\n");
			return ret;
		}
	}

	/* New client. */
	camdev->input = index;
	ret = isp_dev_call(camdev->isp, init, NULL);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to init isp\n");
		ret = -EINVAL;
		goto err;
	}

	ret = ovisp_subdev_power_on(camdev,
			camdev->csd[index].client, index);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to power on subdev\n");
		ret = -EIO;
		goto release_isp;
	}

	ISP_PRINT(ISP_INFO,"Select client %d\n", index);

	return 0;

release_isp:
	isp_dev_call(camdev->isp, release, NULL);
err:
	camdev->input = -1;
	return ret;
}
static int ovisp_flush_cache(struct ovisp_camera_dev *camdev, unsigned int addr,
				unsigned int len, enum v4l2_memory memory, enum dma_data_direction direction)
{
	int ret = 0;
	if(memory == V4L2_MEMORY_USERPTR){
		dma_cache_sync(NULL, (void *)addr, len, direction);
		ret = isp_dev_call(camdev->isp, tlb_map_one_vaddr, addr, len);
		if(ret < 0){
			ISP_PRINT(ISP_ERROR,"%s[%d] tlb operator failed!\n", __func__, __LINE__);
			return -EINVAL;
		}
	}else if(memory == V4L2_MEMORY_MMAP){
		dma_cache_wback_inv(addr,len);
	}else{
		ISP_PRINT(ISP_ERROR,"%s[%d] memory is invalid!\n", __func__, __LINE__);
		ret = -EINVAL;
	}
	return ret;
}
static int ovisp_acquire_photo(struct ovisp_camera_dev *camdev, struct v4l2_control *ctrl)
{
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	struct isp_format tmp_ifmt;
	struct isp_format save_ifmt;
	struct ovisp_camera_devfmt cfmt;
	struct v4l2_acquire_photo_parm parm;
	struct ovisp_camera_buffer *buf = NULL;
	struct ovisp_camera_format *fmt;
	struct vb2_buffer *vb = NULL;
	unsigned long irqsave;
	int ret = 0;

	ret = copy_from_user(&parm, (void __user *)(ctrl->value), sizeof(parm));
	if(ret){
		ISP_PRINT(ISP_ERROR,"Failed to copy_from_user!\n");
		return -EFAULT;
	}
	/* flush cache */
	ret = ovisp_flush_cache(camdev, parm.src.addr, parm.src.lenght, parm.src.memory, DMA_TO_DEVICE);
	if(ret){
		ISP_PRINT(ISP_ERROR,"Failed to flush cache!\n");
		return -EFAULT;
	}
	ret = ovisp_flush_cache(camdev, parm.dst.addr, parm.dst.lenght, parm.dst.memory, DMA_FROM_DEVICE);
	if(ret){
		ISP_PRINT(ISP_ERROR,"Failed to flush cache!\n");
		return -EFAULT;
	}

	if(parm.flags & V4L2_ACQUIRE_DELAY_PHOTO){
		ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
		/*to match the sensor supported format*/
		cfmt.vmfmt.width = parm.width;
		cfmt.vmfmt.height = parm.height;
		ret = v4l2_subdev_call(csd->sd, video, try_mbus_fmt, &cfmt.vmfmt);
		if (ret && ret != -ENOIOCTLCMD)
			return -EINVAL;

		fmt = ovisp_camera_find_format(&cfmt.vmfmt, NULL, -1);
		if (!fmt) {
			ISP_PRINT(ISP_ERROR,"Sensor output format (0x%08x) invalid\n",
					cfmt.vmfmt.code);
			return -EINVAL;
		}
		memset(&cfmt.fmt_data, 0, sizeof(cfmt.fmt_data));
		tmp_ifmt.vfmt.width = parm.width;
		tmp_ifmt.vfmt.height = parm.height;
		tmp_ifmt.vfmt.fourcc = parm.src.fourcc;
		tmp_ifmt.vfmt.dev_width = cfmt.vmfmt.width;
		tmp_ifmt.vfmt.dev_height = cfmt.vmfmt.height;
		tmp_ifmt.vfmt.dev_fourcc = fmt->fourcc;
		tmp_ifmt.fmt_data = &cfmt.fmt_data;
		/* save all flags of isp */
		ret = isp_dev_call(camdev->isp, save_flags, &save_ifmt);
		if (ret) {
			ISP_PRINT(ISP_ERROR,"%s[%d] save_flags failed!\n",__func__, __LINE__);
			return -EINVAL;
		}
		if(cfmt.vmfmt.width != save_ifmt.vfmt.dev_width
				|| cfmt.vmfmt.height != save_ifmt.vfmt.dev_height)
		{
			ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
			/* modify sensor output imagesize */
			ret = v4l2_subdev_call(csd->sd, video, s_stream, 0);
			ret = v4l2_subdev_call(csd->sd, video, s_mbus_fmt, &cfmt.vmfmt);
			if (ret && ret != -ENOIOCTLCMD)
				return -EINVAL;
			ret = v4l2_subdev_call(csd->sd, video, s_stream, 1);
		}
		ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
		spin_lock_irqsave(&camdev->slock, irqsave);
		buf = camdev->capture.last;
		spin_unlock_irqrestore(&camdev->slock, irqsave);
		ret = isp_dev_call(camdev->isp, bypass_capture, &tmp_ifmt, parm.src.addr);
		if (ret) {
			ISP_PRINT(ISP_ERROR,"%s[%d] bypass_capture failed!\n",__func__, __LINE__);
			return -EINVAL;
		}
		tmp_ifmt.vfmt.dev_width = parm.width;
		tmp_ifmt.vfmt.dev_height = parm.height;
		tmp_ifmt.vfmt.dev_fourcc = parm.src.fourcc;
		tmp_ifmt.vfmt.width = parm.width;
		tmp_ifmt.vfmt.height = parm.height;
		tmp_ifmt.vfmt.fourcc = parm.dst.fourcc;
		tmp_ifmt.vfmt.field = camdev->frame.field;
		ret = isp_dev_call(camdev->isp, process_raw, &tmp_ifmt, parm.src.addr, parm.dst.addr, buf->priv);
		if(cfmt.vmfmt.width != save_ifmt.vfmt.dev_width
				|| cfmt.vmfmt.height != save_ifmt.vfmt.dev_height)
		{
			cfmt.vmfmt.width = save_ifmt.vfmt.dev_width;
			cfmt.vmfmt.height = save_ifmt.vfmt.dev_height;
			/* modify sensor output imagesize */
			ret = v4l2_subdev_call(csd->sd, video, s_stream, 0);
			ret = v4l2_subdev_call(csd->sd, video, s_mbus_fmt, &cfmt.vmfmt);
			if (ret && ret != -ENOIOCTLCMD)
				return -EINVAL;
			ret = v4l2_subdev_call(csd->sd, video, s_stream, 1);
		}
		/* save all flags of isp */
		ret = isp_dev_call(camdev->isp, restore_flags, &save_ifmt);
		if (ret) {
			ISP_PRINT(ISP_ERROR,"%s[%d] save_flags failed!\n",__func__, __LINE__);
			return -EINVAL;
		}
	}
	if(parm.flags & V4L2_ACQUIRE_LIVING_PHOTO){
		tmp_ifmt.vfmt.dev_width = parm.width;
		tmp_ifmt.vfmt.dev_height = parm.height;
		tmp_ifmt.vfmt.dev_fourcc = parm.src.fourcc;
		tmp_ifmt.vfmt.width = parm.width;
		tmp_ifmt.vfmt.height = parm.height;
		tmp_ifmt.vfmt.fourcc = parm.dst.fourcc;
		tmp_ifmt.vfmt.field = camdev->frame.field;
		vb = camdev->vbq.bufs[parm.index];
		buf = container_of(vb, struct ovisp_camera_buffer, vb);
		ret = isp_dev_call(camdev->isp, process_raw, &tmp_ifmt, parm.src.addr, parm.dst.addr, buf->priv);
	}
	return ret;
}
static int ovisp_vidioc_s_ctrl(struct file *file, void *priv,
		struct v4l2_control *ctrl)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	int ret = 0;
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	switch(ctrl->id){
		case V4L2_CID_ACQUIRE_PHOTO:
			ret = ovisp_acquire_photo(camdev, ctrl);
			break;
		default:
			if (csd->bypass)
				ret = v4l2_subdev_call(csd->sd, core, s_ctrl, ctrl);
			else
				ret = isp_dev_call(camdev->isp, s_ctrl, ctrl);
		break;
	}

	return ret;
}

static int ovisp_vidioc_g_ctrl(struct file *file, void *priv,
		struct v4l2_control *ctrl)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	int ret = 0;

	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	if (csd->bypass)
		ret = v4l2_subdev_call(csd->sd, core, g_ctrl, ctrl);
	else
		ret = isp_dev_call(camdev->isp, g_ctrl, ctrl);

	return ret;
}

static int ovisp_vidioc_cropcap(struct file *file, void *priv,
		struct v4l2_cropcap *a)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	a->bounds.left			= 0;
	a->bounds.top			= 0;
	a->bounds.width			= csd->max_width;
	a->bounds.height		= csd->max_height;
	a->defrect			= a->bounds;
	a->type				= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	ISP_PRINT(ISP_INFO,"CropCap %dx%d\n", csd->max_width, csd->max_height);

	return 0;
}

static int ovisp_vidioc_g_crop(struct file *file, void *priv,
		struct v4l2_crop *a)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	a->c.left	= 0;
	a->c.top	= 0;
	a->c.width	= csd->max_width;
	a->c.height	= csd->max_height;
	a->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;

	return 0;
}

static int ovisp_vidioc_s_crop(struct file *file, void *priv,
		struct v4l2_crop *a)
{
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	return 0;
}

static int ovisp_vidioc_s_parm(struct file *file, void *priv,
		struct v4l2_streamparm *parm)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	int ret = 0;
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	if (csd->bypass)
		ret = v4l2_subdev_call(csd->sd, video, s_parm, parm);
	else
		ret = isp_dev_call(camdev->isp, s_parm, parm);

	return ret;

}

static int ovisp_vidioc_g_parm(struct file *file, void *priv,
		struct v4l2_streamparm *parm)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_camera_subdev *csd = &camdev->csd[camdev->input];
	int ret = 0;
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);

	if (csd->bypass)
		ret = v4l2_subdev_call(csd->sd, video, g_parm, parm);
	else
		ret = isp_dev_call(camdev->isp, g_parm, parm);

	return ret;
}

static int ovisp_v4l2_open(struct file *file)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_fh *fh;

	ISP_PRINT(ISP_INFO,"Open camera. refcnt %d\n", camdev->refcnt);
	camdev->first_init = 1;

	if (++camdev->refcnt == 1) {
		camdev->input = -1;
	}

	fh = kzalloc(sizeof(*fh), GFP_KERNEL);
	if(NULL == fh)
		goto fh_alloc_fail;
	file->private_data = fh;
	/* Initialize priority of this instance to default priority */
	fh->prio = V4L2_PRIORITY_DEFAULT;
	v4l2_prio_open(&camdev->prio, &fh->prio);
	ISP_PRINT(ISP_INFO,"%s==========%d\n", __func__, __LINE__);
	//	camdev->input = 0;
	return 0;
fh_alloc_fail:
	return -ENOMEM;
}

static int ovisp_v4l2_close(struct file *file)
{
	int ret = 0;
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	struct ovisp_fh *fh = file->private_data;
	struct v4l2_subdev *sd = camdev->csd->sd;
	ISP_PRINT(ISP_INFO,"Close camera. refcnt %d\n", camdev->refcnt);
	dump_sensor_exposure(sd);

	if (--camdev->refcnt == 0) {
		ovisp_camera_stop_streaming(camdev);
		vb2_queue_release(&camdev->vbq);
		ovisp_subdev_power_off(camdev,
				camdev->csd[camdev->input].client, camdev->input);
		isp_dev_call(camdev->isp, release, NULL);
		camdev->input = -1;
	}
	/* Close the priority */
	v4l2_prio_close(&camdev->prio, fh->prio);

	if(camdev->isp->tlb_flag){
		isp_dev_call(camdev->isp, tlb_unmap_all_vaddr);
	}
	return ret;
}

static unsigned int ovisp_v4l2_poll(struct file *file,
		struct poll_table_struct *wait)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	return vb2_poll(&camdev->vbq, file, wait);
}

static int ovisp_v4l2_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct ovisp_camera_dev *camdev = video_drvdata(file);
	return vb2_mmap(&camdev->vbq, vma);
}

static const struct v4l2_ioctl_ops ovisp_v4l2_ioctl_ops = {

	/* VIDIOC_QUERYCAP handler */
	.vidioc_querycap		= ovisp_vidioc_querycap,
	/* Priority handling */
	.vidioc_s_priority		= ovisp_vidioc_s_priority,
	.vidioc_g_priority		= ovisp_vidioc_g_priority,

	.vidioc_enum_fmt_vid_cap	= ovisp_vidioc_enum_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap		= ovisp_vidioc_g_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap		= ovisp_vidioc_try_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap		= ovisp_vidioc_s_fmt_vid_cap,

	/*frame management*/
	.vidioc_reqbufs             = ovisp_vidioc_reqbufs,
	.vidioc_querybuf            = ovisp_vidioc_querybuf,
	.vidioc_qbuf                = ovisp_vidioc_qbuf,
	.vidioc_dqbuf               = ovisp_vidioc_dqbuf,

	/**/
	.vidioc_enum_input          = ovisp_vidioc_enum_input,
	.vidioc_g_input             = ovisp_vidioc_g_input,
	.vidioc_s_input             = ovisp_vidioc_s_input,

	/*isp function, modified according to spec*/
	.vidioc_g_ctrl	            = ovisp_vidioc_g_ctrl,
	.vidioc_s_ctrl              = ovisp_vidioc_s_ctrl,
	.vidioc_cropcap             = ovisp_vidioc_cropcap,
	.vidioc_g_crop              = ovisp_vidioc_g_crop,
	.vidioc_s_crop              = ovisp_vidioc_s_crop,
	.vidioc_s_parm              = ovisp_vidioc_s_parm,
	.vidioc_g_parm              = ovisp_vidioc_g_parm,

	.vidioc_streamon            = ovisp_vidioc_streamon,
	.vidioc_streamoff           = ovisp_vidioc_streamoff,
};

static struct v4l2_file_operations ovisp_v4l2_fops = {
	.owner 		= THIS_MODULE,
	.open 		= ovisp_v4l2_open,
	.release 	= ovisp_v4l2_close,
	.poll		= ovisp_v4l2_poll,
	.unlocked_ioctl	= video_ioctl2,
	.mmap 		= ovisp_v4l2_mmap,
};

static struct video_device ovisp_camera = {
	.name = "ovisp-camera",
	.minor = -1,
	.release = video_device_release,
	.fops = &ovisp_v4l2_fops,
	.ioctl_ops = &ovisp_v4l2_ioctl_ops,
};
/* -------------------sysfs interface------------------- */
struct clk *debug_csi_clk;
static ssize_t debug_mipi_csi_init(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	printk("dump csi\n");
	/*clk init */
	debug_csi_clk = clk_get(NULL, "csi");
	if(IS_ERR(debug_csi_clk)) {
		printk("error: cannot get csi clock\n");
	}
	clk_enable(debug_csi_clk);

	csi_phy_start(0, 24000000, 2);
	return 0;
}
static ssize_t debug_mipi_csi_dump_regs(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	printk("dump regs\n");
	dump_csi_reg();
	return 0;
}
static ssize_t debug_set_mipi_lanes(struct device *dev,
		struct device_attribute *attr,
		char *buf, size_t count)
{
	int rc;
	unsigned long lanes;
	rc = strict_strtoul(buf, 0, &lanes);
	if (rc)
		return rc;

		printk("====[debug] set mipi lanes: %ld\n", lanes);
	if(lanes ==1 || lanes == 2) {
		printk("====[debug] set mipi lanes: %ld\n", lanes);
	} else {
		printk("unsupported lanes set\n");
		csi_set_on_lanes(lanes);
	}

	return count;
}
static ssize_t debug_mipi_csi_close(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	printk("csi close\n");
	/*clk init */
	csi_phy_release();
	clk_disable(debug_csi_clk);
	return 0;
}

static DEVICE_ATTR(mipi_csi_init, S_IRUSR | S_IRGRP | S_IROTH, debug_mipi_csi_init, NULL);
static DEVICE_ATTR(mipi_csi_dump, S_IRUSR | S_IRGRP | S_IROTH, debug_mipi_csi_dump_regs, NULL);
static DEVICE_ATTR(set_mipi_lanes, S_IWUSR | S_IWGRP, NULL, debug_set_mipi_lanes);
static DEVICE_ATTR(mipi_csi_close, S_IRUSR | S_IRGRP | S_IROTH, debug_mipi_csi_close, NULL);

static struct attribute *mipi_csi_attributes[] = {
	&dev_attr_mipi_csi_init.attr,
	&dev_attr_mipi_csi_dump.attr,
	&dev_attr_set_mipi_lanes.attr,
	&dev_attr_mipi_csi_close.attr,
	NULL
};
static const struct attribute_group mipi_csi_attr_group = {
	    .attrs = mipi_csi_attributes,
};



/* -------------------end sysfs interface--------------- */

static int ovisp_camera_probe(struct platform_device *pdev)
{
	struct ovisp_camera_dev *camdev;
	struct ovisp_camera_platform_data *pdata;
	struct video_device *vfd;
	struct isp_device *isp;
	struct vb2_queue *q;
	struct resource *res;
	int irq;
	int ret;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		ISP_PRINT(ISP_ERROR,"Platform data not set\n");
		return -EINVAL;
	}

	/*for csi debug*/
	ret = sysfs_create_group(&pdev->dev.kobj, &mipi_csi_attr_group);
	if (ret < 0)
		goto err_sysfs_create;


	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	if (!res || !irq) {
		ISP_PRINT(ISP_ERROR,"Not enough platform resources");
		return -ENODEV;
	}

	res = request_mem_region(res->start,
			res->end - res->start + 1, dev_name(&pdev->dev));
	if (!res) {
		ISP_PRINT(ISP_ERROR,"Not enough memory for resources\n");
		return -EBUSY;
	}

	camdev = kzalloc(sizeof(*camdev), GFP_KERNEL);
	if (!camdev) {
		ISP_PRINT(ISP_ERROR,"Failed to allocate camera device\n");
		ret = -ENOMEM;
		goto exit;
	}

	isp = kzalloc(sizeof(*isp), GFP_KERNEL);
	if (!isp) {
		ISP_PRINT(ISP_ERROR,"Failed to allocate isp device\n");
		ret = -ENOMEM;
		goto free_camera_dev;;
	}

	isp->irq = irq;
	isp->res = res;
	isp->dev = &pdev->dev;
	isp->pdata = pdata;
	isp->irq_notify = ovisp_camera_irq_notify;
	isp->data = camdev;

	ret = isp_device_init(isp);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Unable to init isp device.n");
		goto free_isp_dev;
	}

	snprintf(camdev->v4l2_dev.name, sizeof(camdev->v4l2_dev.name),
			"%s", dev_name(&pdev->dev));
	ret = v4l2_device_register(NULL, &camdev->v4l2_dev);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to register v4l2 device\n");
		ret = -ENOMEM;
		goto release_isp_dev;
	}

	spin_lock_init(&camdev->slock);

	camdev->isp = isp;
	camdev->dev = &pdev->dev;
	camdev->pdata = pdata;
	camdev->input = -1;
	camdev->refcnt = 0;
	camdev->frame.field = V4L2_FIELD_INTERLACED;
	camdev->first_init = 1;

#if (CAMERA_NEED_REGULATOR)
	camdev->camera_power = regulator_get(camdev->dev, "cpu_avdd");
	if(IS_ERR(camdev->camera_power)) {
		dev_warn(camdev->dev, "camera regulator missing\n");
		ret = -ENXIO;
		goto regulator_error;
	}
#endif

#if 0
	/* Initialize contiguous memory allocator */
	camdev->alloc_ctx = ovisp_vb2_init_ctx(camdev->dev);
	if (IS_ERR(camdev->alloc_ctx)) {
		ret = PTR_ERR(camdev->alloc_ctx);
		goto unreg_v4l2_dev;
	}
#endif

camdev->alloc_ctx = vb2_dma_contig_init_ctx(camdev->dev);
if (IS_ERR(camdev->alloc_ctx)) {
	    ret = PTR_ERR(camdev->alloc_ctx);
		goto unreg_v4l2_dev;
}

#ifdef OVISP_DEBUGTOOL_ENABLE
	camdev->offline.size = 10*1024*1024;
	camdev->offline.vaddr = dma_alloc_coherent(camdev->dev,
			camdev->offline.size,
			&(camdev->offline.paddr),
			GFP_KERNEL);

	ISP_PRINT(ISP_WARNING,"camdev->offline.vaddr:0x%08lx,camdev->offline.paddr:0x%08lx",
			(unsigned long)camdev->offline.vaddr,
			(unsigned long)camdev->offline.paddr);

	camdev->bracket.size = 10*1024*1024;
	camdev->bracket.vaddr = dma_alloc_coherent(camdev->dev,
			camdev->bracket.size,
			&(camdev->bracket.paddr),
			GFP_KERNEL);
	ISP_PRINT(ISP_WARNING, "camdev->bracket.vaddr:0x%08lx,camdev->bracket.paddr:0x%08lx",
			(unsigned long)camdev->bracket.vaddr,
			(unsigned long)camdev->bracket.paddr);
#endif

	ret = ovisp_camera_init_subdev(camdev);
	if (ret) {
		ISP_PRINT(ISP_ERROR,"Failed to register v4l2 device\n");
		ret = -ENOMEM;
		goto cleanup_ctx;
	}
	/* Initialize queue. */
	q = &camdev->vbq;
	memset(q, 0, sizeof(camdev->vbq));
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->drv_priv = camdev;
	q->buf_struct_size = sizeof(struct ovisp_camera_buffer);
	q->ops = &ovisp_vb2_qops;
	q->mem_ops = &ovisp_vb2_memops;
	q->timestamp_type = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	vb2_queue_init(q);

	mutex_init(&camdev->mlock);

	vfd = video_device_alloc();
	if (!vfd) {
		ISP_PRINT(ISP_ERROR,"Failed to allocate video device\n");
		ret = -ENOMEM;
		goto free_i2c;
	}

	memcpy(vfd, &ovisp_camera, sizeof(ovisp_camera));
	vfd->lock = &camdev->mlock;
	vfd->v4l2_dev = &camdev->v4l2_dev;
	vfd->debug = V4L2_DEBUG_IOCTL | V4L2_DEBUG_IOCTL_ARG;
	//vfd->debug = V4L2_DEBUG_IOCTL;
	camdev->vfd = vfd;

	ret = video_register_device(vfd, VFL_TYPE_GRABBER, -1);
	if (ret < 0) {
		ISP_PRINT(ISP_ERROR,"Failed to register video device\n");
		goto free_video_device;
	}

	video_set_drvdata(vfd, camdev);
	platform_set_drvdata(pdev, camdev);

	/* init v4l2_priority */
	v4l2_prio_init(&camdev->prio);

	ret = isp_debug_init();
	if(ret)
		goto fail_debug;
	return 0;
fail_debug:
free_video_device:
	video_device_release(vfd);
free_i2c:
	ovisp_camera_free_subdev(camdev);
cleanup_ctx:
	vb2_dma_contig_cleanup_ctx(camdev->alloc_ctx);
#if (CAMERA_NEED_REGULATOR)
regulator_error:
	regulator_put(camdev->camera_power);
#endif
unreg_v4l2_dev:
	v4l2_device_unregister(&camdev->v4l2_dev);
release_isp_dev:
	isp_device_release(camdev->isp);
free_isp_dev:
	kfree(camdev->isp);
free_camera_dev:
	kfree(camdev);
err_sysfs_create:
exit:
	return ret;

}

static int __exit ovisp_camera_remove(struct platform_device *pdev)
{
	struct ovisp_camera_dev *camdev = platform_get_drvdata(pdev);

#if (CAMERA_NEED_REGULATOR)
	regulator_put(camdev->camera_power);
#endif
	video_device_release(camdev->vfd);
	v4l2_device_unregister(&camdev->v4l2_dev);
	platform_set_drvdata(pdev, NULL);
#ifdef OVISP_DEBUGTOOL_ENABLE
	dma_free_coherent(camdev->dev, camdev->offline.size, camdev->offline.vaddr, (dma_addr_t)camdev->offline.paddr);
#endif
	vb2_dma_contig_cleanup_ctx(camdev->alloc_ctx);
//	ovisp_vb2_cleanup_ctx(camdev->alloc_ctx);
	ovisp_camera_free_subdev(camdev);
	isp_device_release(camdev->isp);
	isp_debug_deinit();
	kfree(camdev->isp);
	kfree(camdev);

	return 0;
}

#ifdef CONFIG_PM
static int ovisp_camera_suspend(struct device *dev)
{
	struct ovisp_camera_dev *camdev = dev_get_drvdata(dev);

	isp_dev_call(camdev->isp, suspend, NULL);

	return 0;
}

static int ovisp_camera_resume(struct device *dev)
{
	struct ovisp_camera_dev *camdev = dev_get_drvdata(dev);

	isp_dev_call(camdev->isp, resume, NULL);

	return 0;
}

static struct dev_pm_ops ovisp_camera_pm_ops = {
	.suspend = ovisp_camera_suspend,
	.resume = ovisp_camera_resume,
};
#endif

static struct platform_driver ovisp_camera_driver = {
	.probe = ovisp_camera_probe,
	.remove = __exit_p(ovisp_camera_remove),
	.driver = {
		.name = "ovisp-camera",
		.owner = THIS_MODULE,
#ifdef CONFIG_PM
		.pm = &ovisp_camera_pm_ops,
#endif
	},
};

static int __init ovisp_camera_init(void)
{
	return platform_driver_register(&ovisp_camera_driver);
}

static void __exit ovisp_camera_exit(void)
{
	platform_driver_unregister(&ovisp_camera_driver);
}

module_init(ovisp_camera_init);
module_exit(ovisp_camera_exit);

MODULE_DESCRIPTION("OVISP camera driver");
MODULE_LICENSE("GPL");
