#ifndef __OVISP_DEBUGTOOL_H__
#define __OVISP_DEBUGTOOL_H__

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
#include <linux/rtc.h>
#include <linux/syscalls.h>
#include "ovisp-video.h"

/*define this for bypass*/
/*#define OVISP_DEBUGTOOL_ENABLE*/
#define OVISP_DEBUGTOOL_SAVE_PREVIEW_BUF


#define OVISP_DEBUGTOOL_GETRAW_FILENAME "/data/media/DCIM/Camera/camera_getraw"
#define OVISP_DEBUGTOOL_ISPSETTING_FILENAME "/data/media/DCIM/Camera/camera_isp_setting"
#define OVISP_DEBUGTOOL_FIRMWARE_FILENAME "/data/media/DCIM/Camera/camera_isp_firmware"


u8 ovisp_debugtool_get_flag(char* filename);
void ovisp_debugtool_save_yuv_file(struct ovisp_camera_dev* camdev);
int ovisp_debugtool_load_isp_setting(struct isp_device* isp, char* filename);
void ovisp_debugtool_save_raw_file(struct ovisp_camera_dev* camdev);
int ovisp_debugtool_save_file(char* filename, u8* buf, u32 len, int flag);
off_t ovisp_debugtool_filesize(char* filename);
int ovisp_debugtool_read_file(char* filename, u8* buf, off_t len);
int ovisp_debugtool_load_firmware(char* filename, u8* dest, u8* array_src, int array_len);

#endif /* __OVISP_DEBUGTOOL_H__ */
