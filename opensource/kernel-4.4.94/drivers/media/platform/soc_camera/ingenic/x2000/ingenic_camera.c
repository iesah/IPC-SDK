/*
 * Copyright (c) 2012 Ingenic Semiconductor Co., Ltd.
 *              http://www.ingenic.com/
 *
 * Core file for Ingenic Display Controller driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/dma.h>
#include <linux/videodev2.h>
#include <media/v4l2-common.h>
#include <media/v4l2-dev.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>
#include <media/soc_camera.h>
#include <media/soc_mediabus.h>
#include "ingenic_camera.h"
#include "mipi_csi.h"

static int frame_size_check_flag = 0;
static int showFPS = 0;

void cim_dump_reg(struct ingenic_camera_dev *pcdev)
{
	if(!pcdev) {
		printk("===>>%s,%d pcdev is NULL!\n",__func__,__LINE__);
		return;
	}
	printk("\n***************************************\n");
	printk("GLB_CFG			0x%08x\n", cim_readl(pcdev, GLB_CFG));
	printk("FRM_SIZE		0x%08x\n", cim_readl(pcdev, FRM_SIZE));
	printk("CROP_SITE		0x%08x\n", cim_readl(pcdev, CROP_SITE));
	printk("SCAN_CFG		0x%08x\n", cim_readl(pcdev, SCAN_CFG));
	printk("QOS_CTRL		0x%08x\n", cim_readl(pcdev, QOS_CTRL));
	printk("QOS_CFG			0x%08x\n", cim_readl(pcdev, QOS_CFG));
	printk("DLY_CFG			0x%08x\n", cim_readl(pcdev, DLY_CFG));
	printk("DES_ADDR		0x%08x\n", cim_readl(pcdev, DES_ADDR));
	printk("CIM_ST			0x%08x\n", cim_readl(pcdev, CIM_ST));
	printk("CIM_CLR_ST		0x%08x\n", cim_readl(pcdev, CIM_CLR_ST));
	printk("CIM_INTC		0x%08x\n", cim_readl(pcdev, CIM_INTC));
	printk("INT_FLAG		0x%08x\n", cim_readl(pcdev, INT_FLAG));
	printk("FRAME_ID		0x%08x\n", cim_readl(pcdev, FRAME_ID));
	printk("ACT_SIZE		0x%08x\n", cim_readl(pcdev, ACT_SIZE));
	printk("DBG_CGC			0x%08x\n", cim_readl(pcdev, DBG_CGC));
	printk("***************************************\n\n");
}

void cim_dump_reg_des(struct ingenic_camera_dev *pcdev)
{
	printk("\n***************************************\n");
	printk("DBG_DES next		0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DBG_DES WRBK_ADDR:	0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DBG_DES WRBK_STRD:	0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DBG_DES DES_INTC_t:	0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DBG_DES DES_CFG_t:	0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DES_HIST_CFG_t:		0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("HIST_WRBK_ADDR:		0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("SF_WRBK_ADDR:		0x%08x\n", cim_readl(pcdev, DBG_DES));
	printk("DBG_DMA:		0x%08x\n", cim_readl(pcdev, DBG_DMA));
	printk("***************************************\n\n");
}

void cim_dump_des(struct ingenic_camera_dev *pcdev)
{
	struct ingenic_camera_desc *desc = (struct ingenic_camera_desc *)pcdev->desc_vaddr;
	int i;
	printk("\n***************************************\n");
	for(i = 0; i < pcdev->buf_cnt; i++) {
		printk("print des %d\n",i);
		printk("next:			0x%08x\n", desc[i].next);
		printk("WRBK_ADDR:		0x%08x\n", desc[i].WRBK_ADDR);
		printk("WRBK_STRD:		0x%08x\n", desc[i].WRBK_STRD);
		printk("DES_INTC_t:		0x%08x\n", desc[i].DES_INTC_t.d32);
		printk("DES_CFG_t:		0x%08x\n", desc[i].DES_CFG_t.d32);
		printk("DES_HIST_CFG_t:		0x%08x\n", desc[i].DES_HIST_CFG_t.d32);
		printk("HIST_WRBK_ADDR:		0x%08x\n", desc[i].HIST_WRBK_ADDR);
		printk("SF_WRBK_ADDR:		0x%08x\n", desc[i].SF_WRBK_ADDR);
	}
	printk("***************************************\n\n");
}

static inline int timeval_sub_to_us(struct timeval lhs,
						struct timeval rhs)
{
	int sec, usec;
	sec = lhs.tv_sec - rhs.tv_sec;
	usec = lhs.tv_usec - rhs.tv_usec;

	return (sec*1000000 + usec);
}

static inline int time_us2ms(int us)
{
	return (us/1000);
}

static void calculate_frame_rate(void)
{
	static struct timeval time_now, time_last;
	unsigned int interval_in_us;
	unsigned int interval_in_ms;
	static unsigned int fpsCount = 0;

	switch(showFPS){
	case 1:
		fpsCount++;
		do_gettimeofday(&time_now);
		interval_in_us = timeval_sub_to_us(time_now, time_last);
		if ( interval_in_us > (USEC_PER_SEC) ) { /* 1 second = 1000000 us. */
			printk(" CIM FPS: %d\n",fpsCount);
			fpsCount = 0;
			time_last = time_now;
		}
		break;
	case 2:
		do_gettimeofday(&time_now);
		interval_in_us = timeval_sub_to_us(time_now, time_last);
		interval_in_ms = time_us2ms(interval_in_us);
		printk(" CIM tow frame interval ms: %d\n",interval_in_ms);
		time_last = time_now;
		break;
	default:
		if (showFPS > 2) {
			int d, f;
			fpsCount++;
			do_gettimeofday(&time_now);
			interval_in_us = timeval_sub_to_us(time_now, time_last);
			if (interval_in_us > USEC_PER_SEC * showFPS ) { /* 1 second = 1000000 us. */
				d = fpsCount / showFPS;
				f = (fpsCount * 10) / showFPS - d * 10;
				printk(" CIM FPS: %d.%01d\n", d, f);
				fpsCount = 0;
				time_last = time_now;
			}
		}
		break;
	}
}

static int ingenic_camera_querycap(struct soc_camera_host *ici, struct v4l2_capability *cap)
{
	strlcpy(cap->card, "ingenic-Camera", sizeof(cap->card));
	cap->version = VERSION_CODE;
	cap->device_caps = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;

	return 0;
}

static unsigned int ingenic_camera_poll(struct file *file, poll_table *pt)
{
	struct soc_camera_device *icd = file->private_data;

	return vb2_poll(&icd->vb2_vidq, file, pt);
}

static int ingenic_camera_alloc_desc(struct ingenic_camera_dev *pcdev, unsigned int count)
{
	pcdev->buf_cnt = count;
	pcdev->desc_vaddr = dma_alloc_coherent(pcdev->soc_host.v4l2_dev.dev,
			sizeof(*pcdev->desc_paddr) * pcdev->buf_cnt,
			(dma_addr_t *)&pcdev->desc_paddr, GFP_KERNEL);

	if (!pcdev->desc_paddr)
		return -ENOMEM;

	return 0;
}

static void ingenic_camera_free_desc(struct ingenic_camera_dev *pcdev)
{
	if(pcdev && pcdev->desc_vaddr) {
		dma_free_coherent(pcdev->soc_host.v4l2_dev.dev,
				sizeof(*pcdev->desc_paddr) * pcdev->buf_cnt,
				pcdev->desc_vaddr, (dma_addr_t )pcdev->desc_paddr);

		pcdev->desc_vaddr = NULL;
	}
}

static int ingenic_videobuf_setup(struct vb2_queue *vq, const void *parg,
				unsigned int *num_buffers, unsigned int *nplanes,
				unsigned int sizes[], void *alloc_ctxs[]) {
	struct soc_camera_device *icd = soc_camera_from_vb2q(vq);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	int size;

	if(pcdev->hist_en)
		size = icd->sizeimage + CIM_HIST_SIZE;
	else
		size = icd->sizeimage;

	if (!*num_buffers || *num_buffers > MAX_BUFFER_NUM)
		*num_buffers = MAX_BUFFER_NUM;

	if (size * *num_buffers > MAX_VIDEO_MEM)
		*num_buffers = MAX_VIDEO_MEM / size;

	*nplanes = 1;
	sizes[0] = size;
	alloc_ctxs[0] = pcdev->alloc_ctx;

	pcdev->start_streaming_called = 0;
	pcdev->sequence = 0;
	pcdev->active = NULL;

	if(ingenic_camera_alloc_desc(pcdev, *num_buffers))
		return -ENOMEM;
	dev_dbg(icd->parent, "%s, count=%d, size=%d\n", __func__,
		*num_buffers, size);

	return 0;
}

static int ingenic_init_desc(struct vb2_buffer *vb2) {
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb2->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_camera_desc *desc;
	dma_addr_t dma_address;
	u32 index = vb2->index;
	u32 pixfmt = icd->current_fmt->host_fmt->fourcc;

	desc = (struct ingenic_camera_desc *) pcdev->desc_vaddr;

	dma_address = *(dma_addr_t *)vb2_plane_cookie(vb2, 0);
	if(!dma_address) {
		dev_err(icd->parent, "Failed to setup DMA address\n");
		return -ENOMEM;
	}

	desc[index].DES_CFG_t.data.ID = index;
	desc[index].WRBK_ADDR = dma_address;

	switch (pixfmt){
		case V4L2_PIX_FMT_RGB565:
			desc[index].DES_CFG_t.data.WRBK_FMT = WRBK_FMT_RGB565;
			break;
		case V4L2_PIX_FMT_RGB32:
			desc[index].DES_CFG_t.data.WRBK_FMT = WRBK_FMT_RGB888;
			break;
		case V4L2_PIX_FMT_YUYV:
			desc[index].DES_CFG_t.data.WRBK_FMT = WRBK_FMT_YUV422;
			break;
		case V4L2_PIX_FMT_GREY:
			desc[index].DES_CFG_t.data.WRBK_FMT = WRBK_FMT_MONO;
			break;
		default:
			dev_err(icd->parent, "No support format!\n");
			return -EINVAL;
	}

	if(!pcdev->desc_head &&
			!pcdev->desc_tail) {
		pcdev->desc_head = &desc[index];
		pcdev->desc_tail = &desc[index];
		desc[index].DES_INTC_t.data.EOF_MSK = 0;
		desc[index].DES_CFG_t.data.DES_END = 1;
		desc[index].next = 0;
	} else if(pcdev->desc_head != NULL &&
			pcdev->desc_tail != NULL) {
		pcdev->desc_tail->next = (dma_addr_t)(&pcdev->desc_paddr[index]);
		pcdev->desc_tail->DES_INTC_t.data.EOF_MSK = 1;
		pcdev->desc_tail->DES_CFG_t.data.DES_END = 0;
		pcdev->desc_tail = &desc[index];

		desc[index].DES_INTC_t.data.EOF_MSK = 0;
		desc[index].DES_CFG_t.data.DES_END = 1;
		desc[index].next = 0;
	}

	if(!pcdev->hist_en) {
		desc[index].DES_HIST_CFG_t.d32 = 0;
		desc[index].HIST_WRBK_ADDR = 0;
	} else {
		if(pixfmt != V4L2_PIX_FMT_YUYV &&
			pixfmt != V4L2_PIX_FMT_GREY) {
			dev_err(icd->parent,"fmt not support hist!\n");
			return -EINVAL;
		}
		desc[index].DES_HIST_CFG_t.HIST_EN = 1;
		desc[index].DES_HIST_CFG_t.GAIN_MUL = pcdev->hist_gain_mul;
		desc[index].DES_HIST_CFG_t.GAIN_ADD = pcdev->hist_gain_add;
		desc[index].HIST_WRBK_ADDR
			= desc[index].WRBK_ADDR + icd->sizeimage;
	}

	if(!pcdev->interlace) {
		desc[index].WRBK_STRD = icd->user_width;
		desc[index].SF_WRBK_ADDR = 0;
	} else {
		desc[index].WRBK_STRD = icd->user_width * 2;
		desc[index].SF_WRBK_ADDR =
			desc[index].WRBK_ADDR + icd->user_width * 2;
	}

	return 0;
}

static int check_platform_param(struct ingenic_camera_dev *pcdev,
			       unsigned char buswidth, unsigned long *flags)
{
	/*
	 * Platform specified synchronization and pixel clock polarities are
	 * only a recommendation and are only used during probing. The PXA270
	 * quick capture interface supports both.
	 */
	*flags = V4L2_MBUS_MASTER |
		V4L2_MBUS_HSYNC_ACTIVE_HIGH |
		V4L2_MBUS_HSYNC_ACTIVE_LOW |
		V4L2_MBUS_VSYNC_ACTIVE_HIGH |
		V4L2_MBUS_VSYNC_ACTIVE_LOW |
		V4L2_MBUS_DATA_ACTIVE_HIGH |
		V4L2_MBUS_PCLK_SAMPLE_RISING |
		V4L2_MBUS_PCLK_SAMPLE_FALLING;

	return 0;

}

static int ingenic_videobuf_init(struct vb2_buffer *vb2)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb2);
	struct ingenic_buffer *buf = container_of(vbuf, struct ingenic_buffer, vb);

	INIT_LIST_HEAD(&buf->list);

	return 0;
}

static int is_cim_disabled(struct ingenic_camera_dev *pcdev)
{
	return !(cim_readl(pcdev, CIM_ST) & CIM_ST_WORKING);
}

static void ingenic_buffer_queue(struct vb2_buffer *vb2)
{
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb2);
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb2->vb2_queue);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_buffer *buf = container_of(vbuf, struct ingenic_buffer, vb);
	struct ingenic_camera_desc *desc_v;
	struct ingenic_camera_desc *desc_p;
	unsigned long flags;
	unsigned int regval;
	int index = vb2->index;

	spin_lock_irqsave(&pcdev->lock, flags);

	list_add_tail(&buf->list, &pcdev->video_buffer_list);
	if (!pcdev->active) {
		pcdev->active = buf;
	}

	if(!pcdev->start_streaming_called) {
		goto out;
	}

	if((index > pcdev->buf_cnt) || (index < 0)) {
		dev_err(icd->parent,"Warning: index %d out of range!\n", index);
		goto out;
	}

	desc_v = (struct ingenic_camera_desc *) pcdev->desc_vaddr;
	desc_p = pcdev->desc_paddr;

	if(is_cim_disabled(pcdev) && list_is_singular(&pcdev->video_buffer_list)) {
		desc_v[index].DES_CFG_t.data.DES_END = 1;
		desc_v[index].DES_INTC_t.data.EOF_MSK = 0;

		pcdev->desc_head = &desc_v[index];
		pcdev->desc_tail = &desc_v[index];

		regval = (dma_addr_t)(&desc_p[index]);
		cim_writel(pcdev, regval, DES_ADDR);
		cim_writel(pcdev, CIM_CTRL_START, CIM_CTRL);
	} else {
		pcdev->desc_tail->next = (dma_addr_t)(&desc_p[index]);
		pcdev->desc_tail->DES_INTC_t.data.EOF_MSK = 1;
		pcdev->desc_tail->DES_CFG_t.data.DES_END = 0;
		pcdev->desc_tail = &desc_v[index];

		desc_v[index].DES_CFG_t.data.DES_END = 1;
		desc_v[index].DES_INTC_t.data.EOF_MSK = 0;
	}

out:
	spin_unlock_irqrestore(&pcdev->lock, flags);

	if(showFPS){
		calculate_frame_rate();
	}

	return;
}

static	int ingenic_buffer_prepare(struct vb2_buffer *vb)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(vb->vb2_queue);
	int ret = 0;

	dev_vdbg(icd->parent, "%s (vb=0x%p) 0x%p %lu\n", __func__,
		vb, vb2_plane_vaddr(vb, 0), vb2_get_plane_payload(vb, 0));

	vb2_set_plane_payload(vb, 0, icd->sizeimage);
	if (vb2_plane_vaddr(vb, 0) &&
	    vb2_get_plane_payload(vb, 0) > vb2_plane_size(vb, 0)) {
		ret = -EINVAL;
		return ret;
	}

	return 0;

}

static void ingenic_cim_start(struct ingenic_camera_dev *pcdev)
{
	struct vb2_buffer *vb2 = &pcdev->active->vb.vb2_buf;
	unsigned regval = 0;
	int index;

	BUG_ON(!pcdev->active);

	/* clear status register */
	cim_writel(pcdev, CIM_CLR_ALL, CIM_CLR_ST);

	/* configure dma desc addr*/
	index = vb2->index;
	regval = (dma_addr_t)(&(pcdev->desc_paddr[index]));
	cim_writel(pcdev, regval, DES_ADDR);

	cim_writel(pcdev, CIM_CTRL_START, CIM_CTRL);
}

static void ingenic_cim_stop(struct ingenic_camera_dev *pcdev)
{
	cim_writel(pcdev, CIM_CTRL_QCK_STOP, CIM_CTRL);
	cim_writel(pcdev, CIM_CLR_ALL, CIM_CLR_ST);
}

static int ingenic_cim_soft_reset(struct soc_camera_device *icd) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	int count = 4000;

	v4l2_subdev_call(sd, video, s_stream, 1);

	cim_writel(pcdev, CIM_CTRL_SOFT_RST, CIM_CTRL);
	while((!(cim_readl(pcdev,CIM_ST) & CIM_ST_SRST)) && count--) {
		udelay(1000);
	}

	if (count < 0) {
		dev_err(icd->parent, "cim soft reset failed!\n");
		v4l2_subdev_call(sd, video, s_stream, 0);
		return -ENODEV;
	}
	cim_writel(pcdev,CIM_CLR_SRST,CIM_CLR_ST);

	return 0;
}

static int ingenic_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_buffer *buf, *node;
	unsigned long flags;
	int ret;

	list_for_each_entry_safe(buf, node, &pcdev->video_buffer_list, list) {
		ret = ingenic_init_desc(&buf->vb.vb2_buf);
		if(ret) {
			dev_err(icd->parent, "%s:Desc initialization failed\n",
					__func__);
			return ret;
		}
	}

	ret = ingenic_cim_soft_reset(icd);
	if(ret)
		return ret;

	spin_lock_irqsave(&pcdev->lock, flags);
	ingenic_cim_start(pcdev);
	pcdev->start_streaming_called = 1;
	spin_unlock_irqrestore(&pcdev->lock, flags);

	return 0;
}

static void ingenic_stop_streaming(struct vb2_queue *q)
{
	struct soc_camera_device *icd = soc_camera_from_vb2q(q);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_buffer *buf, *node;
	unsigned long flags;

	spin_lock_irqsave(&pcdev->lock, flags);
	ingenic_cim_stop(pcdev);

	/* Release all active buffers */
	list_for_each_entry_safe(buf, node, &pcdev->video_buffer_list, list) {
		list_del_init(&buf->list);
		vb2_buffer_done(&buf->vb.vb2_buf, VB2_BUF_STATE_ERROR);
	}

	pcdev->start_streaming_called = 0;
	pcdev->desc_head = NULL;
	pcdev->desc_tail = NULL;
	pcdev->active = NULL;

	spin_unlock_irqrestore(&pcdev->lock, flags);
}

static struct vb2_ops ingenic_videobuf2_ops = {
	.buf_init		= ingenic_videobuf_init,
	.queue_setup		= ingenic_videobuf_setup,
	.buf_prepare		= ingenic_buffer_prepare,
	.buf_queue		= ingenic_buffer_queue,
	.start_streaming	= ingenic_start_streaming,
	.stop_streaming		= ingenic_stop_streaming,
	.wait_prepare		= vb2_ops_wait_prepare,
	.wait_finish		= vb2_ops_wait_finish,
};

static int ingenic_camera_init_videobuf2(struct vb2_queue *q, struct soc_camera_device *icd) {

	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	q->io_modes = VB2_MMAP | VB2_USERPTR;
	q->drv_priv = icd;
	q->buf_struct_size = sizeof(struct ingenic_buffer);
	q->ops = &ingenic_videobuf2_ops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;

	return vb2_queue_init(q);
}

static int ingenic_camera_cropcap(struct soc_camera_device *icd, struct v4l2_cropcap *a)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);

	return v4l2_subdev_call(sd, video, cropcap, a);
}

static int ingenic_camera_get_crop(struct soc_camera_device *icd, struct v4l2_crop *a)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_rect *rect = &a->c;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	int ret = 0;

	switch(img_sz->rsz_way) {
	case IMG_RESIZE_CROP:
		rect->left = img_sz->c_left;
		rect->top = img_sz->c_top;
		rect->width = img_sz->c_w;
		rect->height = img_sz->c_h;
		break;
	case IMG_NO_RESIZE:
		ret = v4l2_subdev_call(sd, video, g_crop, a);
		break;
	default:
		dev_err(icd->parent,"resize way not supported!\n");
		return -EINVAL;
	}

	return ret;
}

static int ingenic_camera_enum_framesizes(struct soc_camera_device *icd,
				   struct v4l2_frmsizeenum *fsize)
{
	int ret;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_subdev_frame_size_enum fse = {
		.index = fsize->index,
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};

	xlate = soc_camera_xlate_by_fourcc(icd, fsize->pixel_format);
	if (!xlate)
		return -EINVAL;
	fse.code = xlate->code;

	ret = v4l2_subdev_call(sd, pad, enum_frame_size, NULL, &fse);
	if (ret < 0)
		return ret;

	if (fse.min_width == fse.max_width &&
	    fse.min_height == fse.max_height) {
		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete.width = fse.min_width;
		fsize->discrete.height = fse.min_height;
		return 0;
	}
	fsize->type = V4L2_FRMSIZE_TYPE_CONTINUOUS;
	fsize->stepwise.min_width = fse.min_width;
	fsize->stepwise.max_width = fse.max_width;
	fsize->stepwise.min_height = fse.min_height;
	fsize->stepwise.max_height = fse.max_height;
	fsize->stepwise.step_width = 1;
	fsize->stepwise.step_height = 1;
	return 0;
}

static int ingenic_camera_check_crop(struct soc_camera_device *icd, struct v4l2_rect *r)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct v4l2_rect rect_w = *r;
	u32 pixfmt = icd->current_fmt->host_fmt->fourcc;

	if(pcdev->field != V4L2_FIELD_NONE) {
		dev_err(icd->parent, "field not supported by crop!\n");
		return -EINVAL;
	}

	if((rect_w.width + rect_w.left > 2046) ||
		(rect_w.height + rect_w.top > 2046)) {
		dev_err(icd->parent, "crop bounds check failed!\n");
		return -EINVAL;
	}

	v4l_bound_align_image(&rect_w.width, 100, 2046,
			(pixfmt == V4L2_PIX_FMT_YUYV ? 1 : 0),
			&rect_w.height, 100, 2046, 0, 0);
	if((rect_w.width != r->width) ||
		(rect_w.height != r->height)) {
		dev_err(icd->parent, "crop sizes check failed!\n");
		return -EINVAL;
	}

	if((pixfmt == V4L2_PIX_FMT_YUYV)
		&& (rect_w.left % 2)) {
		dev_err(icd->parent, "crop align check failed!\n");
		return -EINVAL;
	}

	return 0;
}

static int ingenic_camera_set_host_crop(struct soc_camera_device *icd, const struct v4l2_crop *a)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	struct v4l2_crop a_w = *a;
	struct v4l2_rect *rect_s = &a_w.c;
	struct v4l2_rect rect_w = *rect_s;
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_TRY,
	};
	struct v4l2_mbus_framefmt *mf = &format.format;
	unsigned int tmp;
	int width, height;
	int ret;


	ret = ingenic_camera_check_crop(icd, &rect_w);
	if (ret)
		return ret;

	width = img_sz->src_w;
	height = img_sz->src_h;

	if(((rect_w.left + rect_w.width) > width) ||
		((rect_w.top + rect_w.height) > height)) {
		dev_err(icd->parent, "please enum appropriate framesizes!\n");
		return -EINVAL;
	}

	ret = v4l2_subdev_call(sd, pad, get_fmt, NULL, &format);
	if (ret < 0)
		return ret;

	mf->width = width;
	mf->height = height;

	ret = v4l2_subdev_call(sd, pad, set_fmt, NULL, &format);
	if (ret < 0)
		return ret;

	if ((mf->width != width) ||
		(mf->height != height)) {
		dev_err(icd->parent, "please enum appropriate framesizes!\n");
		return -EINVAL;
	}

	if (mf->code != icd->current_fmt->code) {
		dev_err(icd->parent, "current size and format do not match!\n");
		return -EINVAL;
	}

	icd->user_width		= rect_s->width;
	icd->user_height	= rect_s->height;

	img_sz->rsz_way		= IMG_RESIZE_CROP;
	img_sz->src_w		= mf->width;
	img_sz->src_h		= mf->height;
	img_sz->c_left		= rect_w.left;
	img_sz->c_top		= rect_w.top;
	img_sz->c_w		= rect_s->width;
	img_sz->c_h		= rect_s->height;

	tmp = (img_sz->c_w << CIM_CROP_WIDTH_LBIT) |
		(img_sz->c_h << CIM_CROP_HEIGHT_LBIT);
	cim_writel(pcdev, tmp, FRM_SIZE);
	tmp = (img_sz->c_left << CIM_CROP_X_LBIT) |
		(img_sz->c_top << CIM_CROP_Y_LBIT);
	cim_writel(pcdev, tmp, CROP_SITE);

	return ret;
}

static int ingenic_camera_set_client_crop(struct soc_camera_device *icd, const struct v4l2_crop *a)
{
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_TRY,
	};
	struct v4l2_mbus_framefmt *mf = &format.format;
	unsigned int tmp;
	int ret;

	ret = v4l2_subdev_call(sd, video, s_crop, a);
	if (ret < 0)
		return ret;

	/* The capture device might have changed its output  */
	ret = v4l2_subdev_call(sd, pad, get_fmt, NULL, &format);
	if (ret < 0)
		return ret;

	dev_dbg(icd->parent, "Sensor cropped %dx%d\n",
		mf->width, mf->height);

	icd->user_width		= mf->width;
	icd->user_height	= mf->height;

	img_sz->rsz_way		= IMG_NO_RESIZE;
	img_sz->src_w		= mf->width;
	img_sz->src_h		= mf->height;
	tmp = (img_sz->src_w << CIM_CROP_WIDTH_LBIT) |
		(img_sz->src_h << CIM_CROP_HEIGHT_LBIT);
	cim_writel(pcdev, tmp, FRM_SIZE);
	cim_writel(pcdev, 0, CROP_SITE);

	return ret;
}

static int ingenic_camera_set_crop(struct soc_camera_device *icd, const struct v4l2_crop *a)
{
	int ret;

	ret = ingenic_camera_set_client_crop(icd, a);
	if(!ret)
		return ret;
	/*sensor not support crop*/
	return ingenic_camera_set_host_crop(icd, a);
}

static int ingenic_camera_get_selection(struct soc_camera_device *icd, struct v4l2_selection *sel)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_cropcap a;
	int ret;

	if (sel->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	ret = v4l2_subdev_call(sd, video, cropcap, &a);
	if(ret)
		return ret;

	switch (sel->target) {
	case V4L2_SEL_TGT_CROP_BOUNDS:
		sel->r = a.bounds;
		return 0;
	case V4L2_SEL_TGT_CROP_DEFAULT:
		sel->r = a.defrect;
		return 0;
	}

	return -EINVAL;
}

static int ingenic_camera_set_selection(struct soc_camera_device *icd, struct v4l2_selection *sel)
{
	return 0;
}

static int ingenic_camera_try_fmt(struct soc_camera_device *icd, struct v4l2_format *f) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct v4l2_subdev_pad_config pad_cfg;
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_TRY,
	};
	struct v4l2_mbus_framefmt *mf = &format.format;
	__u32 pixfmt = pix->pixelformat;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_err(icd->parent,"Format %x not found\n", pix->pixelformat);
		return -EINVAL;
	}

	v4l_bound_align_image(&pix->width, 100, 2046,
			(pixfmt == V4L2_PIX_FMT_YUYV ? 1 : 0),
			&pix->height, 100, 2046, 0, 0);

	mf->width	= pix->width;
	mf->height	= pix->height;
	mf->field	= pix->field;
	mf->colorspace	= pix->colorspace;
	mf->code		= xlate->code;

	/* limit to sensor capabilities */
	ret = v4l2_subdev_call(sd, pad, set_fmt, &pad_cfg, &format);
	if (ret < 0)
		return ret;

	if ((mf->width < pix->width) ||
		(mf->height < pix->height)) {
		dev_err(icd->parent, "fmt not supported!\n");
		return -EINVAL;
	}

	pix->field	= mf->field;
	pix->colorspace	= mf->colorspace;

	return 0;
}

static int ingenic_camera_set_fmt(struct soc_camera_device *icd, struct v4l2_format *f) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	const struct soc_camera_format_xlate *xlate;
	struct v4l2_pix_format *pix = &f->fmt.pix;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	struct v4l2_subdev_format format = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
	};
	struct v4l2_mbus_framefmt *mf = &format.format;
	int ret;

	xlate = soc_camera_xlate_by_fourcc(icd, pix->pixelformat);
	if (!xlate) {
		dev_err(icd->parent, "Format %x not found\n", pix->pixelformat);
		return -EINVAL;
	}

	mf->width        = pix->width;
	mf->height       = pix->height;
	mf->field        = pix->field;
	mf->colorspace   = pix->colorspace;
	mf->code         = xlate->code;

	ret = v4l2_subdev_call(sd, pad, set_fmt, NULL, &format);
	if (ret < 0)
		return ret;

	if (mf->code != xlate->code)
		return -EINVAL;

	img_sz->src_w	= mf->width;
	img_sz->src_h	= mf->height;

	/*when set_crop before set_fmt, entry first if*/
	if(unlikely(img_sz->rsz_way == IMG_RESIZE_CROP)) {
		dev_err(icd->parent, "Set_crop should be behind set_fmt!\n");
		return -EINVAL;
	} else {
		img_sz->rsz_way = IMG_NO_RESIZE;
		if((pix->width != mf->width) || (pix->height != mf->height))
			printk("image will be crop by cim\n");
	}
	pix->field       = mf->field;
	pix->colorspace  = mf->colorspace;

	pcdev->field	 = mf->field;
	icd->current_fmt = xlate;

	return ret;
}

static int ingenic_get_dvp_mbus_config(struct soc_camera_device *icd, unsigned long *flags) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_PARALLEL,};
	unsigned long common_flags;
	unsigned long bus_flags = 0;
	int ret;

	check_platform_param(pcdev, icd->current_fmt->host_fmt->bits_per_sample,
				&bus_flags);

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		if(cfg.type != V4L2_MBUS_PARALLEL) {
			dev_err(icd->parent, "Device type error!\n");
			return -EINVAL;
		}
		common_flags = soc_mbus_config_compatible(&cfg,
				bus_flags);
		if (!common_flags) {
			dev_warn(icd->parent,
					"Flags incompatible: camera 0x%x, host 0x%lx\n",
					cfg.flags, bus_flags);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	} else {
		common_flags = bus_flags;
	}

	/* Make choises, based on platform preferences */

	if ((common_flags & V4L2_MBUS_VSYNC_ACTIVE_HIGH) &&
	    (common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW)) {
		if (pcdev->pdata->flags & INGENIC_CAMERA_VSYNC_HIGH)
			common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_LOW;
		else
			common_flags &= ~V4L2_MBUS_VSYNC_ACTIVE_HIGH;
	}

	if ((common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING) &&
	    (common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)) {
		if (pcdev->pdata->flags & INGENIC_CAMERA_PCLK_RISING)
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_FALLING;
		else
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_RISING;
	}

	if ((common_flags & V4L2_MBUS_HSYNC_ACTIVE_HIGH) &&
	    (common_flags & V4L2_MBUS_HSYNC_ACTIVE_LOW)) {
		if (pcdev->pdata->flags & INGENIC_CAMERA_HSYNC_HIGH)
			common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_LOW;
		else
			common_flags &= ~V4L2_MBUS_HSYNC_ACTIVE_HIGH;
	}

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_dbg(icd->parent, "camera s_mbus_config(0x%lx) returned %d\n",
			common_flags, ret);
		return ret;
	}

	*flags = common_flags;

	return 0;
}

static int ingenic_get_itu656_mbus_config(struct soc_camera_device *icd, unsigned long *flags) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_BT656,};
	unsigned long common_flags;
	unsigned long bus_flags = 0;
	int ret;


	bus_flags = V4L2_MBUS_PCLK_SAMPLE_RISING |
			V4L2_MBUS_PCLK_SAMPLE_FALLING |
			V4L2_MBUS_DATA_ACTIVE_HIGH |
			V4L2_MBUS_DATA_ACTIVE_LOW |
			V4L2_MBUS_MASTER |
			V4L2_MBUS_SLAVE;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		if(cfg.type != V4L2_MBUS_BT656) {
			dev_err(icd->parent, "Device type error!\n");
			return -EINVAL;
		}
		common_flags = soc_mbus_config_compatible(&cfg,
				bus_flags);
		if (!common_flags) {
			dev_warn(icd->parent,
					"Flags incompatible: camera 0x%x, host 0x%lx\n",
					cfg.flags, bus_flags);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	} else {
		common_flags = bus_flags;
	}

	/* Make choises, based on platform preferences */

	if ((common_flags & V4L2_MBUS_PCLK_SAMPLE_RISING) &&
	    (common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING)) {
		if (pcdev->pdata->flags & INGENIC_CAMERA_PCLK_RISING)
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_FALLING;
		else
			common_flags &= ~V4L2_MBUS_PCLK_SAMPLE_RISING;
	}

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_dbg(icd->parent, "camera s_mbus_config(0x%lx) returned %d\n",
			common_flags, ret);
		return ret;
	}

	*flags = common_flags;

	return 0;
}

static int ingenic_get_csi2_mbus_config(struct soc_camera_device *icd,
		unsigned long *flags) {
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct v4l2_mbus_config cfg = {.type = V4L2_MBUS_CSI2,};
	unsigned long common_flags;
	unsigned long csi2_flags;
	int ret;

	csi2_flags = V4L2_MBUS_CSI2_CONTINUOUS_CLOCK |
			V4L2_MBUS_CSI2_NONCONTINUOUS_CLOCK |
			V4L2_MBUS_CSI2_2_LANE |
			V4L2_MBUS_CSI2_1_LANE;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	if (!ret) {
		if(cfg.type != V4L2_MBUS_CSI2) {
			dev_err(icd->parent, "Device type error!\n");
			return -EINVAL;
		}
		common_flags = soc_mbus_config_compatible(&cfg, csi2_flags);
		if (!common_flags) {
			dev_warn(icd->parent,
					"Flags incompatible: camera 0x%x, host 0x%lx\n",
					cfg.flags, csi2_flags);
			return -EINVAL;
		}
	} else if (ret != -ENOIOCTLCMD) {
		return ret;
	} else {
		common_flags = csi2_flags;
	}

	if ((common_flags & V4L2_MBUS_CSI2_1_LANE) &&
	    (common_flags & V4L2_MBUS_CSI2_2_LANE)) {
		if (pcdev->pdata->flags & INGENIC_CAMERA_CSI2_2_LANE)
			common_flags &= ~V4L2_MBUS_CSI2_1_LANE;
		else
			common_flags &= ~V4L2_MBUS_CSI2_2_LANE;
	}

	if (!common_flags)
		return -EINVAL;

	cfg.flags = common_flags;
	ret = v4l2_subdev_call(sd, video, s_mbus_config, &cfg);
	if (ret < 0 && ret != -ENOIOCTLCMD) {
		dev_dbg(icd->parent, "camera s_mbus_config(0x%lx) returned %d\n",
			common_flags, ret);
		return ret;
	}

	*flags = common_flags;

	return 0;
}

static struct soc_camera_device *ctrl_to_icd(struct v4l2_ctrl *ctrl)
{
	return container_of(ctrl->handler, struct soc_camera_device,
							ctrl_handler);
}

static int ingenic_camera_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct soc_camera_device *icd = ctrl_to_icd(ctrl);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	struct ingenic_camera_desc *desc;
	unsigned int tmp;
	int i;

	switch (ctrl->id) {
	case INGENIC_CID_CROP_WAY:
		img_sz->crop_way = ctrl->val;
		break;
	case INGENIC_CID_HIST_EN:
		pcdev->hist_en = 1;
		break;
	case INGENIC_CID_HIST_GAIN_ADD:
		pcdev->hist_gain_add = ctrl->val;
		desc = (struct ingenic_camera_desc *) pcdev->desc_vaddr;
		if(!pcdev->hist_en || !desc) {
			dev_dbg(icd->parent, "setting hist_add are not satisfied!\n");
			return 0;
		}
		for(i = 0; i < pcdev->buf_cnt; i++)
			desc[i].DES_HIST_CFG_t.GAIN_ADD = pcdev->hist_gain_add;
		break;
	case INGENIC_CID_HIST_GAIN_MUL:
		pcdev->hist_gain_mul = ctrl->val;
		desc = (struct ingenic_camera_desc *) pcdev->desc_vaddr;
		if(!pcdev->hist_en || !desc) {
			dev_dbg(icd->parent, "setting hist_mul are not satisfied!\n");
			return 0;
		}
		for(i = 0; i < pcdev->buf_cnt; i++)
			desc[i].DES_HIST_CFG_t.GAIN_MUL = pcdev->hist_gain_mul;
		break;
	default:
		dev_err(icd->parent, "ctrl id not supported!\n");
		break;
	}
	return 0;
}

static const struct v4l2_ctrl_ops ingenic_camera_ctrl_ops = {
	.s_ctrl = ingenic_camera_s_ctrl,
};

static const struct v4l2_ctrl_config img_crop_way = {
	.ops	= &ingenic_camera_ctrl_ops,
	.id	= INGENIC_CID_CROP_WAY,
	.name	= "crop way",
	.type	= V4L2_CTRL_TYPE_INTEGER,
	.def	= CROP_SPECIFY_ZONE,
	.min	= CROP_SPECIFY_ZONE,
	.max	= CROP_BASE_MAX_PIC,
	.step	= 1,
};

static const struct v4l2_ctrl_config hist_en = {
	.ops	= &ingenic_camera_ctrl_ops,
	.id	= INGENIC_CID_HIST_EN,
	.type	= V4L2_CTRL_TYPE_BUTTON,
	.name	= "histgram enable",
};

static const struct v4l2_ctrl_config hist_gain_add = {
	.ops	= &ingenic_camera_ctrl_ops,
	.id	= INGENIC_CID_HIST_GAIN_ADD,
	.name	= "hist gain add",
	.type	= V4L2_CTRL_TYPE_INTEGER,
	.def	= 0,
	.min	= 0,
	.max	= 0xff,
	.step	= 1,
};

static const struct v4l2_ctrl_config hist_gain_mul = {
	.ops	= &ingenic_camera_ctrl_ops,
	.id	= INGENIC_CID_HIST_GAIN_MUL,
	.name	= "hist gain mul",
	.type	= V4L2_CTRL_TYPE_INTEGER,
	.def	= 0x20,
	.min	= 0,
	.max	= 0xff,
	.step	= 1,
};

static const struct soc_mbus_pixelfmt ingenic_camera_formats[] = {
	[0] = {
		.fourcc			= V4L2_PIX_FMT_RGB32,
		.name			= "RGB888/32bpp",
		.bits_per_sample	= 8,
		.packing		= SOC_MBUS_PACKING_EXTEND32,
		.order			= SOC_MBUS_ORDER_LE,
	},
};


static int ingenic_camera_get_formats(struct soc_camera_device *icd, unsigned int idx,
				  struct soc_camera_format_xlate *xlate)
{
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct device *dev = icd->parent;
	struct ingenic_image_sz *img_sz;
	struct v4l2_subdev_mbus_code_enum code = {
		.which = V4L2_SUBDEV_FORMAT_ACTIVE,
		.index = idx,
	};
	const struct soc_mbus_pixelfmt *fmt;
	int formats = 0, ret;
	int buswidth;

	ret = v4l2_subdev_call(sd, pad, enum_mbus_code, NULL, &code);
	if (ret < 0)
		/* No more formats */
		return 0;

	fmt = soc_mbus_get_fmtdesc(code.code);
	if (!fmt) {
		dev_dbg(icd->parent,
			 "Unsupported format code #%u: %d\n", idx, code.code);
		return 0;
	}

	buswidth = fmt->bits_per_sample;
	if (buswidth > 8) {
		dev_dbg(icd->parent, "bits-per-sample %d for code %x unsupported\n",
				buswidth, code.code);
		return 0;
	}

	if (!icd->host_priv) {
		struct v4l2_mbus_config cfg;
		ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
		if(!ret)
			pcdev->dat_if = cfg.type;
		else
			return ret;
		v4l2_ctrl_new_custom(&icd->ctrl_handler, &img_crop_way, NULL);
		v4l2_ctrl_new_custom(&icd->ctrl_handler, &hist_en, NULL);
		v4l2_ctrl_new_custom(&icd->ctrl_handler, &hist_gain_add, NULL);
		v4l2_ctrl_new_custom(&icd->ctrl_handler, &hist_gain_mul, NULL);
		if (icd->ctrl_handler.error)
			return icd->ctrl_handler.error;
		img_sz = kzalloc(sizeof(*img_sz), GFP_KERNEL);
		if(!img_sz)
			return -ENOMEM;
		icd->host_priv = img_sz;
	}

	switch (code.code) {
	case MEDIA_BUS_FMT_UYVY8_2X8:
	case MEDIA_BUS_FMT_VYUY8_2X8:
	case MEDIA_BUS_FMT_YUYV8_2X8:
	case MEDIA_BUS_FMT_YVYU8_2X8:
	case MEDIA_BUS_FMT_Y8_1X8:
		break;
	case MEDIA_BUS_FMT_RGB565_2X8_LE:
		formats++;
		if (xlate) {
			xlate->host_fmt	= &ingenic_camera_formats[0];
			xlate->code	= code.code;
			xlate++;
			dev_dbg(dev, "Providing format %s using code %d\n",
				ingenic_camera_formats[0].name, code.code);
		}
		break;
	default:
		dev_dbg(dev, "Providing code %d not support\n", code.code);
			return 0;
	}

	/* Generic pass-through */
	formats++;
	if (xlate) {
		xlate->host_fmt	= fmt;
		xlate->code	= code.code;
		xlate++;
		dev_dbg(dev, "Format %s pass-through mode\n",
			fmt->name);
	}

	return formats;
}

static void ingenic_camera_put_formats(struct soc_camera_device *icd)
{
	kfree(icd->host_priv);
	icd->host_priv = NULL;
}

static int ingenic_camera_set_bus_param(struct soc_camera_device *icd) {
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_image_sz *img_sz = icd->host_priv;
	struct v4l2_subdev *sd = soc_camera_to_subdev(icd);
	struct v4l2_mbus_config cfg;
	unsigned long common_flags;
	unsigned long csi2_flags;
	unsigned long glb_cfg = 0;
	unsigned long intc = 0;
	unsigned long tmp;
	int csi2_lanes;
	int ret;

	ret = v4l2_subdev_call(sd, video, g_mbus_config, &cfg);
	pcdev->dat_if = cfg.type;

	if(pcdev->dat_if == V4L2_MBUS_CSI2) {
		ret = ingenic_get_csi2_mbus_config(icd, &csi2_flags);
		if(ret) {
			dev_err(icd->parent, "get csi mbus_config failed!\n");
			return ret;
		}

		if(csi2_flags & V4L2_MBUS_CSI2_2_LANE)
			csi2_lanes = 2;
		else if(csi2_flags & V4L2_MBUS_CSI2_1_LANE)
			csi2_lanes = 1;
		else {
			dev_err(icd->parent, "csi lanes not supported!\n");
			return -EINVAL;
		}

		if(icd->current_fmt->code == MEDIA_BUS_FMT_Y8_1X8 &&
				csi2_lanes == 2) {
			dev_err(icd->parent, "grey not support two lanes!\n");
			return ret;
		}

		ret = csi_phy_start(0, 24000000, csi2_lanes);
		if(ret < 0) {
			dev_err(icd->parent, "csi starting failed!\n");
			return ret;
		}

		/***Csi2 interface requirements***/
		common_flags = V4L2_MBUS_MASTER |
			V4L2_MBUS_HSYNC_ACTIVE_HIGH |
			V4L2_MBUS_VSYNC_ACTIVE_HIGH |
			V4L2_MBUS_PCLK_SAMPLE_RISING;

	} else if(pcdev->dat_if == V4L2_MBUS_PARALLEL) {
		ret = ingenic_get_dvp_mbus_config(icd, &common_flags);
		if(ret) {
			dev_err(icd->parent, "Get dvp mbus_config failed!\n");
			return ret;
		}
	} else if(pcdev->dat_if == V4L2_MBUS_BT656) {
		ret = ingenic_get_itu656_mbus_config(icd, &common_flags);
		if(ret) {
			dev_err(icd->parent, "Get bt656 mbus_config failed!\n");
			return ret;
		}
		if(common_flags & V4L2_MBUS_SLAVE) {
			dev_err(icd->parent, "bt656_mbus slave mode not supported!\n");
			return ret;
		}
	} else {
		dev_err(icd->parent, "cim interface not supported!\n");
		return -EINVAL;
	}

	glb_cfg = cim_readl(pcdev,GLB_CFG);

	/*PCLK Polarity Set*/
	glb_cfg = (common_flags & V4L2_MBUS_PCLK_SAMPLE_FALLING) ?
		(glb_cfg | GLB_CFG_DE_PCLK) : (glb_cfg & (~GLB_CFG_DE_PCLK));

	/*VSYNC Polarity Set*/
	glb_cfg = (common_flags & V4L2_MBUS_VSYNC_ACTIVE_LOW) ?
		(glb_cfg | GLB_CFG_DL_VSYNC) : (glb_cfg & (~GLB_CFG_DL_VSYNC));

	/*HSYNC Polarity Set*/
	glb_cfg = (common_flags & V4L2_MBUS_HSYNC_ACTIVE_LOW) ?
		(glb_cfg | GLB_CFG_DL_HSYNC) : (glb_cfg & (~GLB_CFG_DL_HSYNC));

	glb_cfg &= ~GLB_CFG_C_ORDER_MASK;
	glb_cfg &= ~GLB_CFG_ORG_FMT_MASK;
	if(pcdev->dat_if != V4L2_MBUS_BT656) {
		switch(icd->current_fmt->code) {
		case MEDIA_BUS_FMT_RGB565_2X8_LE:
			glb_cfg |= (GLB_CFG_ORG_FMT_RGB565 | GLB_CFG_C_ORDER_RGB);
			break;
		case MEDIA_BUS_FMT_UYVY8_2X8:
			glb_cfg |= (GLB_CFG_ORG_FMT_YUYV422 | GLB_CFG_C_ORDER_UYVY);
			break;
		case MEDIA_BUS_FMT_VYUY8_2X8:
			glb_cfg |= (GLB_CFG_ORG_FMT_YUYV422 | GLB_CFG_C_ORDER_VYUY);
			break;
		case MEDIA_BUS_FMT_YUYV8_2X8:
			glb_cfg |= (GLB_CFG_ORG_FMT_YUYV422 | GLB_CFG_C_ORDER_YUYV);
			break;
		case MEDIA_BUS_FMT_YVYU8_2X8:
			glb_cfg |= (GLB_CFG_ORG_FMT_YUYV422 | GLB_CFG_C_ORDER_YVYU);
			break;
		case MEDIA_BUS_FMT_Y8_1X8:
			glb_cfg |= GLB_CFG_ORG_FMT_MONO;
			break;
		default:
			dev_err(icd->parent, "Providing code %d not support\n",
					icd->current_fmt->code);
			return -EINVAL;
		}
	} else {
		unsigned int sf_cfg = 0;
		if(icd->current_fmt->code != MEDIA_BUS_FMT_YUYV8_2X8) {
			dev_err(icd->parent, "Providing code %d not support\n",
					icd->current_fmt->code);
		}
		glb_cfg |= (GLB_CFG_ORG_FMT_ITU656 | GLB_CFG_C_ORDER_YUYV);

		if(pcdev->field == V4L2_FIELD_NONE) {
			pcdev->interlace = 0;
		} else {
			switch(pcdev->field) {
			case V4L2_FIELD_INTERLACED_TB:
			case V4L2_FIELD_INTERLACED:
				sf_cfg &= ~CIM_F_ORDER;
				break;
			case V4L2_FIELD_INTERLACED_BT:
				sf_cfg |= CIM_F_ORDER;
				break;
			default:
				dev_err(icd->parent,
					"itu656 unsupported field");
				break;
			}
			sf_cfg |= CIM_SCAN_MD;
			sf_cfg &= ~CIM_SF_HEIGHT_MASK;
			sf_cfg |= ((img_sz->src_h / 2) & 0xfff);
			pcdev->interlace = 1;
		}
		cim_writel(pcdev, sf_cfg, SCAN_CFG);
	}

	glb_cfg |= GLB_CFG_BURST_LEN_32;
	glb_cfg |= GLB_CFG_AUTO_RECOVERY;
	if(frame_size_check_flag == 1)
		glb_cfg |= GLB_CFG_SIZE_CHK;
	else
		glb_cfg &= ~GLB_CFG_SIZE_CHK;

#ifdef CONFIG_CAMERA_USE_SNAPSHOT
		tmp = CONFIG_SNAPSHOT_PULSE_WIDTH;
		if(!tmp) {
			tmp = 0x08;
		}
		glb_cfg |= GLB_CFG_DAT_MODE;
		glb_cfg &= ~GLB_CFG_EXPO_WIDTH_MASK;
		glb_cfg |= (((tmp-1) & 0x7) << GLB_CFG_EXPO_WIDTH_LBIT);

		tmp = CONFIG_SNAPSHOT_FRAME_DELAY & 0xffffff;
		tmp |= CIM_DLY_EN;
		tmp |= CIM_DLY_MD;
		cim_writel(pcdev, tmp, DLY_CFG);
#else
		cim_writel(pcdev, 0, DLY_CFG);
#endif

	if(pcdev->dat_if == V4L2_MBUS_CSI2)
		glb_cfg |= GLB_CFG_DAT_IF_SEL;
	else
		glb_cfg &= ~GLB_CFG_DAT_IF_SEL;


	if(img_sz->rsz_way == IMG_NO_RESIZE) {
		tmp = (img_sz->src_w << CIM_CROP_WIDTH_LBIT) |
			(img_sz->src_h << CIM_CROP_HEIGHT_LBIT);
		cim_writel(pcdev, tmp, FRM_SIZE);
		cim_writel(pcdev, 0, CROP_SITE);
	} else if(img_sz->rsz_way == IMG_RESIZE_CROP){
		if(pcdev->field != V4L2_FIELD_NONE) {
			dev_err(icd->parent, "field not supported by crop!\n");
			return -EINVAL;
		}
		tmp = (img_sz->c_w << CIM_CROP_WIDTH_LBIT) |
			(img_sz->c_h << CIM_CROP_HEIGHT_LBIT);
		cim_writel(pcdev, tmp, FRM_SIZE);
		tmp = (img_sz->c_left << CIM_CROP_Y_LBIT) |
			(img_sz->c_top << CIM_CROP_X_LBIT);
		cim_writel(pcdev, tmp, CROP_SITE);
	} else {
		dev_err(icd->parent, "resize way err!!\n");
		return -EINVAL;
	}
	/* enable stop of frame interrupt. */
	intc |= CIM_INTC_MSK_EOW;

	/* enable end of frame interrupt,
	 * Work together with DES_INTC.EOF_MSK,
	 * only when both are 1 will generate interrupt. */
	intc |= CIM_INTC_MSK_EOF;

	/* enable general stop of frame interrupt. */
	intc |= CIM_INTC_MSK_GSA;

	/* enable rxfifo overflow interrupt */
	intc |= CIM_INTC_MSK_OVER;

	/* enable size check err */
	if(frame_size_check_flag == 1)
		intc |= CIM_INTC_MSK_SZ_ERR;
	else
		intc &= ~CIM_INTC_MSK_SZ_ERR;

	cim_writel(pcdev, 0, DBG_CGC);
	cim_writel(pcdev, glb_cfg, GLB_CFG);
	cim_writel(pcdev, intc, CIM_INTC);
	cim_writel(pcdev, 0, QOS_CTRL);

	return 0;
}

static void ingenic_camera_activate(struct ingenic_camera_dev *pcdev) {
	int ret = -1;

	if(pcdev->clk) {
		ret = clk_prepare_enable(pcdev->clk);
	}
	if(pcdev->mipi_clk) {
		ret = clk_prepare_enable(pcdev->mipi_clk);
	}
}

static void ingenic_camera_deactivate(struct ingenic_camera_dev *pcdev) {
	if( pcdev->mipi_clk) {
		clk_disable_unprepare(pcdev->mipi_clk);
	}
	if(pcdev->clk) {
		clk_disable_unprepare(pcdev->clk);
	}
}

static int ingenic_camera_add_device(struct soc_camera_device *icd) {
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;
	struct ingenic_image_sz *img_sz = icd->host_priv;

	if (pcdev->icd)
		return -EBUSY;

	dev_dbg(icd->parent, "ingenic Camera driver attached to camera %d\n",
			icd->devnum);

	pcdev->icd = icd;

	if(img_sz)
		img_sz->rsz_way = IMG_NO_RESIZE;

	ingenic_camera_activate(pcdev);

	return 0;
}

static void ingenic_camera_remove_device(struct soc_camera_device *icd) {
	struct soc_camera_host *ici = to_soc_camera_host(icd->parent);
	struct ingenic_camera_dev *pcdev = ici->priv;

	BUG_ON(icd != pcdev->icd);

	if(pcdev->dat_if == V4L2_MBUS_CSI2)
		csi_phy_stop(0);

	ingenic_camera_deactivate(pcdev);
	ingenic_camera_free_desc(pcdev);
	dev_dbg(icd->parent, "ingenic Camera driver detached from camera %d\n",
			icd->devnum);

	pcdev->icd = NULL;
	pcdev->hist_en = 0;
}

static int ingenic_camera_wakeup(struct ingenic_camera_dev *pcdev) {
	struct ingenic_buffer *buf = pcdev->active;
	struct vb2_v4l2_buffer *vbuf = &pcdev->active->vb;
	int id = cim_readl(pcdev, FRAME_ID);

	if(!buf || vbuf->vb2_buf.index != id) {
		dev_err(pcdev->dev, "cim buf synchronous problems!\n");
		return -EINVAL;
	}

	list_del_init(&buf->list);
	v4l2_get_timestamp(&vbuf->timestamp);
	vbuf->sequence = pcdev->sequence++;
	vb2_buffer_done(&vbuf->vb2_buf, VB2_BUF_STATE_DONE);

	return 0;
}

static irqreturn_t ingenic_camera_irq_handler(int irq, void *data) {
	struct ingenic_camera_dev *pcdev = (struct ingenic_camera_dev *)data;
	struct ingenic_camera_desc *desc_p;
	unsigned long status = 0;
	unsigned long flags = 0;
	unsigned long regval = 0;

	if(!pcdev->icd)
		return IRQ_HANDLED;

	spin_lock_irqsave(&pcdev->lock, flags);

	/* read interrupt flag register */
	status = cim_readl(pcdev,INT_FLAG);
	if (!status) {
		dev_err(pcdev->dev, "cim irq_flag is NULL! \n");
		goto out;
	}

	if(status & CIM_INT_FLAG_EOW) {
		cim_writel(pcdev, CIM_INT_FLAG_EOW, CIM_CLR_ST);

		if(status & CIM_INT_FLAG_EOF) {
			cim_writel(pcdev, CIM_INT_FLAG_EOF, CIM_CLR_ST);
		}

		if(ingenic_camera_wakeup(pcdev))
			goto out;

		if (list_empty(&pcdev->video_buffer_list)) {
			pcdev->active = NULL;
			goto out;
		}

		/* start next dma frame. */
		desc_p = pcdev->desc_paddr;
		regval = (unsigned long)(pcdev->desc_head->next);
		cim_writel(pcdev, regval, DES_ADDR);
		cim_writel(pcdev, CIM_CTRL_START, CIM_CTRL);

		pcdev->active =
			list_entry(pcdev->video_buffer_list.next, struct ingenic_buffer, list);
		pcdev->desc_head =
			(struct ingenic_camera_desc *)UNCAC_ADDR(phys_to_virt(regval));
		goto out;
	}

	if(status & CIM_INT_FLAG_EOF) {
		cim_writel(pcdev, CIM_INT_FLAG_EOF, CIM_CLR_ST);

		if(ingenic_camera_wakeup(pcdev))
			goto out;

		pcdev->active =
			list_entry(pcdev->video_buffer_list.next, struct ingenic_buffer, list);
		pcdev->desc_head =
			(struct ingenic_camera_desc *)UNCAC_ADDR(phys_to_virt(pcdev->desc_head->next));

		goto out;
	}

	if (status & CIM_INT_FLAG_SZ_ERR) {
		unsigned int val;
		val = cim_readl(pcdev, ACT_SIZE);
		cim_writel(pcdev, CIM_CLR_SZ_ERR, CIM_CLR_ST);
		dev_err(pcdev->dev, "cim size err! w = %d h = %d\n", val&0x3ff, val>>16&0x3ff);
		goto out;
	}

	if (status & CIM_INT_FLAG_OVER) {
		cim_writel(pcdev, CIM_INT_FLAG_OVER, CIM_CLR_ST);
		dev_err(pcdev->dev, "cim overflow! \n");
		goto out;
	}

	if(status & CIM_INT_FLAG_GSA) {
		cim_writel(pcdev, CIM_INT_FLAG_GSA, CIM_CLR_ST);
		printk("cim general stop! \n");
		goto out;
	}

	if(status & CIM_INT_FLAG_SOF) {
		cim_writel(pcdev, CIM_INT_FLAG_SOF, CIM_CLR_ST);
		goto out;
	}

out:
	spin_unlock_irqrestore(&pcdev->lock, flags);
	return IRQ_HANDLED;
}

static ssize_t frame_size_check_r(struct device *dev, struct device_attribute *attr, char *buf)
{
	snprintf(buf, 100, " 0: disable frame size check.\n 1: enable frame size check.\n");
	return 100;
}

static ssize_t frame_size_check_w(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
        if (count != 2)
                return -EINVAL;

        if (*buf == '0') {
                frame_size_check_flag = 0;
        } else if (*buf == '1') {
                frame_size_check_flag = 1;
        } else {
                return -EINVAL;
        }

        return count;
}

static ssize_t fps_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	printk("\n-----you can choice print way:\n");
	printk("Example: echo NUM > show_fps\n");
	printk("NUM = 0: close fps statistics\n");
	printk("NUM = 1: print recently fps\n");
	printk("NUM = 2: print interval between last and this queue buffers\n");
	printk("NUM > 2: print fps after NUM second\n");
	return 0;
}

static ssize_t fps_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	int num = 0;
	num = simple_strtoul(buf, NULL, 0);
	if(num < 0){
		printk("\n--please 'cat show_fps' to view using the method\n\n");
		return n;
	}
	showFPS = num;
	return n;
}

extern void dump_csi_reg(void);
static ssize_t cim_dump_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct jz_camera_dev *pcdev = (struct jz_camera_dev *)dev_get_drvdata(dev);

	cim_dump_reg(pcdev);
	cim_dump_reg_des(pcdev);
	dump_csi_reg();
	return 0;
}

static ssize_t cim_dump_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t n)
{
	struct jz_camera_dev *pcdev = (struct jz_camera_dev *)dev_get_drvdata(dev);
	int num = 0;
	num = simple_strtoul(buf, NULL, 0);
	if(num < 0) {
		printk("error input\n");
		return n;
	}
	int csi2_lanes = num;
	int ret = 0;

	ret = csi_phy_start(0, 24000000, csi2_lanes);
	return n;
}

/**********************cim_debug***************************/
static DEVICE_ATTR(frame_size_check, S_IRUGO|S_IWUSR, frame_size_check_r, frame_size_check_w);
static DEVICE_ATTR(show_fps, S_IRUGO|S_IWUSR, fps_show, fps_store);
static DEVICE_ATTR(cim_dump, S_IRUGO|S_IWUSR, cim_dump_show, cim_dump_store);

static struct attribute *cim_debug_attrs[] = {
	&dev_attr_frame_size_check.attr,
	&dev_attr_show_fps.attr,
	&dev_attr_cim_dump.attr,
	NULL,
};

const char cim_group_name[] = "debug";
static struct attribute_group cim_debug_attr_group = {
	.name	= cim_group_name,
	.attrs	= cim_debug_attrs,
};

static struct soc_camera_host_ops ingenic_soc_camera_host_ops = {
	.owner = THIS_MODULE,
	.add = ingenic_camera_add_device,
	.remove = ingenic_camera_remove_device,
	.set_fmt = ingenic_camera_set_fmt,
	.try_fmt = ingenic_camera_try_fmt,
	.get_formats = ingenic_camera_get_formats,
	.put_formats	= ingenic_camera_put_formats,
	.init_videobuf2 = ingenic_camera_init_videobuf2,
	.poll = ingenic_camera_poll,
	.querycap = ingenic_camera_querycap,
	.cropcap  = ingenic_camera_cropcap,
	.get_crop = ingenic_camera_get_crop,
	.set_crop = ingenic_camera_set_crop,
	.get_selection = ingenic_camera_get_selection,
	.set_selection = ingenic_camera_set_selection,
	.enum_framesizes = ingenic_camera_enum_framesizes,
	.set_bus_param = ingenic_camera_set_bus_param,
};

static int __init ingenic_camera_probe(struct platform_device *pdev) {
	int err = 0, ret = 0;
	unsigned int irq;
	struct resource *res;
	void __iomem *base;
	struct ingenic_camera_dev *pcdev;

	/* malloc */
	pcdev = kzalloc(sizeof(*pcdev), GFP_KERNEL);
	if (!pcdev) {
		dev_err(&pdev->dev, "Could not allocate pcdev\n");
		err = -ENOMEM;
		goto err_kzalloc;
	}

	/* resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		dev_err(&pdev->dev, "Could not get resource!\n");
		err = -ENODEV;
		goto err_get_resource;
	}

	/* irq */
	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev, "Could not get irq!\n");
		err = -ENODEV;
		goto err_get_irq;
	}

	/*get cim clk*/
	pcdev->clk = devm_clk_get(&pdev->dev, "gate_cim");
	if (IS_ERR(pcdev->clk)) {
		err = PTR_ERR(pcdev->clk);
		dev_err(&pdev->dev, "%s:can't get clk %s\n", __func__, "cim");
		goto err_clk_get_cim;
	}

	/*get mipi clk*/
	pcdev->mipi_clk = devm_clk_get(&pdev->dev, "gate_mipi");
	if (IS_ERR(pcdev->mipi_clk)) {
		err = PTR_ERR(pcdev->mipi_clk);
		dev_err(&pdev->dev, "%s:can't get mipi_clk %s\n", __func__, "mipi");
		goto err_clk_get_mipi;
	}

	/* Request the regions. */
	if (!request_mem_region(res->start, resource_size(res), DRIVER_NAME)) {
		err = -EBUSY;
		goto err_request_mem_region;
	}
	base = ioremap(res->start, resource_size(res));
	if (!base) {
		err = -ENOMEM;
		goto err_ioremap;
	}

	spin_lock_init(&pcdev->lock);
	INIT_LIST_HEAD(&pcdev->video_buffer_list);

	pcdev->dev = &pdev->dev;
	pcdev->res = res;
	pcdev->irq = irq;
	pcdev->base = base;
	pcdev->active = NULL;

	/* request irq */
	err = devm_request_irq(&pdev->dev, pcdev->irq, ingenic_camera_irq_handler, 0,
			dev_name(&pdev->dev), pcdev);
	if(err) {
		dev_err(&pdev->dev, "request irq failed!\n");
		goto err_request_irq;
	}

	pcdev->alloc_ctx = vb2_dma_contig_init_ctx(&pdev->dev);
	if (IS_ERR(pcdev->alloc_ctx)) {
		ret = PTR_ERR(pcdev->alloc_ctx);
		goto err_alloc_ctx;
	}
	pcdev->soc_host.drv_name        = DRIVER_NAME;
	pcdev->soc_host.ops             = &ingenic_soc_camera_host_ops;
	pcdev->soc_host.priv            = pcdev;
	pcdev->soc_host.v4l2_dev.dev    = &pdev->dev;
	pcdev->soc_host.nr              = pdev->id; /* use one cim0 or cim1 */

	err = soc_camera_host_register(&pcdev->soc_host);
	if (err)
		goto err_soc_camera_host_register;

	ret = sysfs_create_group(&pcdev->dev->kobj, &cim_debug_attr_group);
	if (ret) {
		dev_err(&pdev->dev, "device create sysfs group failed\n");

		ret = -EINVAL;
		goto err_free_file;
	}


	dev_dbg(&pdev->dev, "ingenic Camera driver loaded!\n");

	return 0;

err_free_file:
	sysfs_remove_group(&pcdev->dev->kobj, &cim_debug_attr_group);
err_soc_camera_host_register:
	vb2_dma_contig_cleanup_ctx(pcdev->alloc_ctx);
err_alloc_ctx:
	free_irq(pcdev->irq, pcdev);
err_request_irq:
	iounmap(base);
err_ioremap:
	release_mem_region(res->start, resource_size(res));
err_request_mem_region:
	devm_clk_put(&pdev->dev, pcdev->mipi_clk);
err_clk_get_mipi:
	devm_clk_put(&pdev->dev, pcdev->clk);
err_clk_get_cim:
err_get_irq:
err_get_resource:
	kfree(pcdev);
err_kzalloc:
	return err;

}

static int __exit ingenic_camera_remove(struct platform_device *pdev)
{
	struct soc_camera_host *soc_host = to_soc_camera_host(&pdev->dev);
	struct ingenic_camera_dev *pcdev = container_of(soc_host,
					struct ingenic_camera_dev, soc_host);
	struct resource *res;

	free_irq(pcdev->irq, pcdev);

	vb2_dma_contig_cleanup_ctx(pcdev->alloc_ctx);

	devm_clk_put(pcdev->dev, pcdev->clk);
	devm_clk_put(pcdev->dev, pcdev->mipi_clk);

	soc_camera_host_unregister(soc_host);

	sysfs_remove_group(&pcdev->dev->kobj, &cim_debug_attr_group);

	iounmap(pcdev->base);

	res = pcdev->res;
	release_mem_region(res->start, resource_size(res));

	kfree(pcdev);

	dev_dbg(&pdev->dev, "ingenic Camera driver unloaded\n");

	return 0;
}

static const struct of_device_id ingenic_camera_of_match[] = {
	{ .compatible = "ingenic,cim" },
	{ }
};
MODULE_DEVICE_TABLE(of, ingenic_camera_of_match);

static struct platform_driver ingenic_camera_driver = {
	.remove		= __exit_p(ingenic_camera_remove),
	.driver		= {
		.name	= DRIVER_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = ingenic_camera_of_match,
	},
};

static int __init ingenic_camera_init(void) {
	/*
	 * platform_driver_probe() can save memory,
	 * but this Driver can bind to one device only.
	 */
	return platform_driver_probe(&ingenic_camera_driver, ingenic_camera_probe);
}

static void __exit ingenic_camera_exit(void) {
	return platform_driver_unregister(&ingenic_camera_driver);
}

late_initcall(ingenic_camera_init);
module_exit(ingenic_camera_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wangchunlei <chunlei.wang@ingenic.cn>");
MODULE_DESCRIPTION("ingenic Soc Camera Host Driver");
MODULE_ALIAS("a ingenic-cim platform");
