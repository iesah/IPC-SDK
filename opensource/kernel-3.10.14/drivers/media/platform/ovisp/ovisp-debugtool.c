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

#include "ovisp-isp.h"
#include "ovisp-video.h"
#include "ovisp-videobuf.h"
#include "ovisp-debugtool.h"
#include "ovisp-base.h"
#include "isp-debug.h"

#define STATE_LINE_BEGIN 0
#define STATE_LINE_IGNORE 1
#define STATE_LEFT 2
#define STATE_ARRAY_0_0 3
#define STATE_ARRAY_0_X 4
#define STATE_ARRAY_0_DATA0 5
#define STATE_ARRAY_0_DATA1 6
#define STATE_ARRAY_0_DATA2 7
#define STATE_ARRAY_0_DATA3 8
#define STATE_ARRAY_0_DATA4 9
#define STATE_ARRAY_0_END 10
#define STATE_ARRAY_1_0 11
#define STATE_ARRAY_1_X 12
#define STATE_ARRAY_1_DATA0 13
#define STATE_ARRAY_1_DATA1 14
#define STATE_LINE_END 15


u8 ovisp_debugtool_get_flag(char* filename)
{
	u8 flag = 0;
	struct file *file1 = NULL;
	mm_segment_t old_fs;

	if(NULL == filename)
		return 0;

	file1 = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, filename);
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file1->f_op->read(file1, (char *)&flag, 1, &file1->f_pos);
		set_fs(old_fs);
		filp_close(file1, NULL);
	}

	return flag;
}



void ovisp_debugtool_save_yuv_file(struct ovisp_camera_dev* camdev)
{
	struct file *file2 = NULL;
	mm_segment_t old_fs;
	char yuvfilename[100];
	struct timespec ts;
	struct rtc_time tm;
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct ovisp_camera_frame *frame = &camdev->frame;
	void *vaddr;
	int width;
	int height;

	memset(yuvfilename, 0, sizeof(yuvfilename));

	vaddr = vb2_plane_vaddr(&capture->last->vb, 0);
	width = frame->ifmt.vfmt.width;
	height = frame->ifmt.vfmt.height;

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);

	sprintf(yuvfilename, "/data/media/DCIM/Camera/%d.yuv",
			1);


	file2 = filp_open(yuvfilename, O_CREAT | O_RDWR, 0644);
	if (IS_ERR(file2)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, yuvfilename);
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file2->f_op->write(file2, (char *)(vaddr), width*height*2, &file2->f_pos);
		set_fs(old_fs);
		filp_close(file2, NULL);
	}


}


static u8 convert_ascii2num(u8 v)
{
	if (v >= '0'&& v <='9')
		return v - '0';
	else if (v >= 'a' && v <='f')
		return v - 'a' + 0xa;
	else if (v >= 'A' && v <='F')
		return v - 'A' + 0xa;
	else
		return 0xff;
}

int ovisp_debugtool_load_isp_setting(struct isp_device* isp, char* filename)
{
	unsigned int i;
	char v = 0;
	struct file *file1 = NULL;
	mm_segment_t old_fs;
	off_t file_len = 0;
	int state = STATE_LINE_BEGIN;
	unsigned int reg = 0;
	unsigned char value = 0;

	if(NULL == filename)
		return 0;

	file1 = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"no isp setting file use default \n");
		return 0;
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file_len = file1->f_dentry->d_inode->i_size;
		ISP_PRINT(ISP_INFO,"load isp setting from file: %s \n", filename);
		for (i = 0 ; i < file_len ; i++) {
			file1->f_op->read(file1, (char *)&v, 1, &file1->f_pos);
			if (' ' == v || '\t' == v)
				continue;
			if ('\n' == v) {
				state = STATE_LINE_BEGIN;
				continue;
			}

			if (STATE_LINE_BEGIN == state) {
				if ('{' == v)
					state = STATE_LEFT;
				if ('/' == v)
					state = STATE_LINE_IGNORE;
			} else if (STATE_LINE_IGNORE == state) {
				if ('\n' == v)
					state = STATE_LINE_BEGIN;
			} else if (STATE_LEFT == state) {
				if ('0' == v)
					state = STATE_ARRAY_0_0;
				else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_0 == state) {
				if ('x' == v || 'X' == v)
					state = STATE_ARRAY_0_X;
				else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_X == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
				  reg = ((u32)v)<<16;
					state = STATE_ARRAY_0_DATA0;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_DATA0 == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
				  reg |= ((u32)v)<<12;
					state = STATE_ARRAY_0_DATA1;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_DATA1 == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
				  reg |= ((u32)v)<<8;
					state = STATE_ARRAY_0_DATA2;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_DATA2 == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
					reg |= v<<4;
					state = STATE_ARRAY_0_DATA3;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_DATA3 == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
					reg |= v<<0;
					state = STATE_ARRAY_0_DATA4;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_DATA4 == state) {
				if (',' == v)
					state = STATE_ARRAY_0_END;
				else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_0_END == state) {
				if ('0' == v)
					state = STATE_ARRAY_1_0;
				else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_1_0 == state) {
				if ('x' == v || 'X' == v)
					state = STATE_ARRAY_1_X;
				else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_1_X == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
				  value = ((u32)v)<<4;
					state = STATE_ARRAY_1_DATA0;
				} else
					state = STATE_LINE_IGNORE;
			} else if (STATE_ARRAY_1_DATA0 == state) {
				v = convert_ascii2num(v);
				if (v >= 0 && v <= 0xf) {
					value |= v<<0;
					isp_reg_writeb(isp, value, reg);
				}
				state = STATE_LINE_IGNORE;
			}
		}
		set_fs(old_fs);
		filp_close(file1, NULL);
	}

	return 0;
}


void ovisp_debugtool_save_raw_file(struct ovisp_camera_dev* camdev)
{
	struct file *file1 = NULL;
	mm_segment_t old_fs;
	char filename[100];
	char yuvfilename[100];
	struct timespec ts;
	struct rtc_time tm;
	struct ovisp_camera_capture *capture = &camdev->capture;
	struct ovisp_camera_frame *frame = &camdev->frame;
	struct isp_device* isp=camdev->isp;
	void *vaddr;
	int width;
	int height;
	u8 raw_info[24];

	memset(filename, 0, sizeof(filename));
	memset(yuvfilename, 0, sizeof(yuvfilename));

	vaddr = vb2_plane_vaddr(&capture->last->vb, 0);
	width = frame->ifmt.vfmt.width;
	height = frame->ifmt.vfmt.height;

	getnstimeofday(&ts);
	rtc_time_to_tm(ts.tv_sec, &tm);
	sprintf(filename, "/data/media/DCIM/Camera/%d-%02d-%02d-%02d-%02d-%02d-%09lu.raw",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);
	sprintf(yuvfilename, "/data/media/DCIM/Camera/%d-%02d-%02d-%02d-%02d-%02d-%09lu.yuv",
			tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec, ts.tv_nsec);

	raw_info[0] = (u8)((width >> 8) & 0xff);
	raw_info[1] = (u8)(width & 0xff);
	raw_info[2] = (u8)((height >> 8) & 0xff);
	raw_info[3] = (u8)(height & 0xff);
	raw_info[4] = isp_reg_readb(isp, 0x1c170);	//gain 0
	raw_info[5] = isp_reg_readb(isp, 0x1c171);	//gain 1
	raw_info[6] = isp_reg_readb(isp, 0x1c16a);	//exposure 0
	raw_info[7] = isp_reg_readb(isp, 0x1c16b);	//exposure 1
	raw_info[8] = isp_reg_readb(isp, 0x1e944);	//vts 0
	raw_info[9] = isp_reg_readb(isp, 0x1e945);	//vts 1
	raw_info[10] = isp_reg_readb(isp, 0x1c734);	//awb gains section1
	raw_info[11] = isp_reg_readb(isp, 0x1c735);
	raw_info[12] = isp_reg_readb(isp, 0x1c736);
	raw_info[13] = isp_reg_readb(isp, 0x1c737);
	raw_info[14] = isp_reg_readb(isp, 0x1c738);
	raw_info[15] = isp_reg_readb(isp, 0x1c739);
	raw_info[16] = isp_reg_readb(isp, 0x65300);	//awb gains section2
	raw_info[17] = isp_reg_readb(isp, 0x65301);
	raw_info[18] = isp_reg_readb(isp, 0x65302);
	raw_info[19] = isp_reg_readb(isp, 0x65303);
	raw_info[20] = isp_reg_readb(isp, 0x65304);
	raw_info[21] = isp_reg_readb(isp, 0x65305);
	raw_info[22] = isp_reg_readb(isp, 0x65306);
	raw_info[23] = isp_reg_readb(isp, 0x65307);

	file1 = filp_open(filename, O_CREAT | O_RDWR, 0644);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, filename);
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file1->f_op->write(file1, (char *)camdev->offline.vaddr, width*height*2, &file1->f_pos);
		file1->f_op->write(file1, (char *)raw_info, sizeof(raw_info), &file1->f_pos);
		set_fs(old_fs);
		filp_close(file1, NULL);
	}

/*
	file2 = filp_open(yuvfilename, O_CREAT | O_RDWR, 0644);
	if (IS_ERR(file2)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, yuvfilename);
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file2->f_op->write(file2, (char *)vaddr, width*height*2, &file2->f_pos);
		set_fs(old_fs);
		filp_close(file2, NULL);
	}
*/
}

int ovisp_debugtool_save_file(char* filename, u8* buf, u32 len, int flag)
{
	struct file* file1;
	mm_segment_t old_fs;

	if(!(flag & O_APPEND))
		flag = 0;
	else
		flag = O_APPEND;

	file1 = filp_open(filename, O_CREAT | O_WRONLY | flag, 0644);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, filename);
		return -1;
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file1->f_op->write(file1, buf, len, &file1->f_pos);
		set_fs(old_fs);
		filp_close(file1, NULL);
	}

	return 0;
}

off_t ovisp_debugtool_filesize(char* filename)
{
	struct file *file1 = NULL;

	if(NULL == filename)
		return 0;

	file1 = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, filename);
		return 0;
	} else {
		return file1->f_dentry->d_inode->i_size;
	}
}

int ovisp_debugtool_read_file(char* filename, u8* buf, off_t len)
{
	struct file *file1 = NULL;
	mm_segment_t old_fs;
	off_t file_len = 0;
	int real_len = 0;

	if(NULL == filename)
		return 0;

	file1 = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(file1)) {
		ISP_PRINT(ISP_ERROR,"%s: open file: %s error!\n", __FUNCTION__, filename);
		return 0;
	} else {
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		file_len = file1->f_dentry->d_inode->i_size;
		if(file_len < len)
			len = file_len;
		real_len = file1->f_op->read(file1, (char *)buf, len, &file1->f_pos);
		set_fs(old_fs);
		filp_close(file1, NULL);
	}

	return real_len;
}

int ovisp_debugtool_load_firmware(char* filename, u8* dest, u8* array_src, int array_len)
{
	int len;
	u8* buf;
	int real_len = 0;
	len = ovisp_debugtool_filesize(filename);

	if(0 == len){
		ISP_PRINT(ISP_WARNING,"%s: enable ovisp debug tool, but %s file size is 0, use array instead \n", __func__, filename);
		memcpy(dest, array_src, array_len);
	}else{
		buf = kmalloc(len, GFP_KERNEL);
		real_len = ovisp_debugtool_read_file(filename, buf, len);
		if((real_len != array_len) || memcmp(buf, array_src, real_len)){
			ISP_PRINT(ISP_WARNING,"%s: isp_firmware array is different with %s \n", __func__, filename);
		}
		memcpy(dest, buf, real_len);
		kfree(buf);
		ISP_PRINT(ISP_INFO,"firmware %s file size:%d, loaded:%d \n", filename, len, real_len);
		//ovisp_debugtool_save_file("/data/media/DCIM/Camera/camera_isp_firmware_array", array_src, array_len , 0);
	}

	return real_len;
}
