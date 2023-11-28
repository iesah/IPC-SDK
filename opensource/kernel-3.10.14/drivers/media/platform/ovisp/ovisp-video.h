#ifndef __OVISP_VIDEO_H__
#define __OVISP_VIDEO_H__

#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-core.h>
#include <mach/ovisp-v4l2.h>
//#include <mach/ovisp-memory.h>
#include <mach/camera.h>

#include "ovisp-isp.h"


#define OVISP_CAMERA_VERSION		KERNEL_VERSION(1, 0, 0)
#define OVISP_CAMERA_CLIENT_NUM		(2)
#define OVISP_CAMERA_WIDTH_MAX		(3264)
#define OVISP_CAMERA_HEIGHT_MAX		(2448)

#if defined(CONFIG_VIDEO_OVISP_BUFFER_STATIC)
#define OVISP_CAMERA_BUFFER_MAX		(ISP_MEMORY_SIZE)
#else
#define OVISP_CAMERA_BUFFER_MAX		(16 * 1024 * 1024)
#endif


struct ovisp_camera_buffer {
	struct vb2_buffer vb;
	struct list_head list;
	unsigned char priv[ISP_OUTPUT_INFO_LENS];
};

struct ovisp_camera_format {
	char *name;
	enum v4l2_mbus_pixelcode code;
	unsigned int fourcc;
	unsigned int depth;
};
struct ovisp_camera_frame {
	enum v4l2_field field;
	struct ovisp_camera_devfmt cfmt;
	struct isp_format ifmt;
};

struct ovisp_camera_capture {
	struct list_head list;
	struct ovisp_camera_buffer *active[2];
	struct ovisp_camera_buffer *last;
	unsigned int running;
	unsigned int drop_frames;
	unsigned int lose_frames;
	unsigned int error_frames;
	unsigned int in_frames;
	unsigned int out_frames;
};

struct ovisp_camera_subdev {
	struct ovisp_camera_client *client;
	struct i2c_adapter *i2c_adap;
	struct v4l2_subdev *sd;
	struct clk *mclk;
	struct isp_prop prop;
	unsigned int max_width;
	unsigned int max_height;
	int bypass;
};

struct ovisp_camera_offline {
	dma_addr_t paddr;
	void* vaddr;
	int size;
};

struct ovisp_camera_bracket {
	dma_addr_t paddr;
	void* vaddr;
	int size;
};

/* File handle structure */
struct ovisp_fh{
	enum v4l2_priority prio;
};
struct ovisp_camera_dev {
	struct device *dev;
	struct v4l2_prio_state prio;
	struct isp_device *isp;
	struct video_device *vfd;
	struct v4l2_device v4l2_dev;
	struct ovisp_camera_platform_data *pdata;
	struct ovisp_camera_subdev csd[OVISP_CAMERA_CLIENT_NUM];
	struct ovisp_camera_frame frame;
	struct ovisp_camera_capture capture;
	struct ovisp_camera_offline offline;
	struct ovisp_camera_bracket bracket;
	struct vb2_queue vbq;
	spinlock_t slock;
	struct mutex mlock;
	void *alloc_ctx;
	int clients;
	int snapshot;
	int refcnt;
	int input;
	int first_init;

	struct regulator * camera_power;
};

#endif /*__OVISP_VIDEO_H__*/
