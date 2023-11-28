#ifndef __OVISP_BASE_H__
#define __OVISP_BASE_H__

#include <asm/io.h>
#include "ovisp-isp.h"
// add by wqyan
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

/* isp's type is (struct isp_device *) */
#define isp_dev_call(isp, f, args...)				\
	(!(isp) ? -ENODEV : (((isp)->ops && (isp)->ops->f) ?	\
						 (isp)->ops->f((isp) , ##args) : -ENOIOCTLCMD))

#define isp_reg_readl(isp, offset)	readl((isp)->base + (offset))

#define isp_reg_readw(isp, offset)	readw((isp)->base + (offset))

#define isp_reg_readb(isp, offset)	readb((isp)->base + (offset))

#define isp_reg_writel(isp, value, offset)	writel((value), (isp)->base + (offset))

#define isp_reg_writew(isp, value, offset)	writew((value), (isp)->base + (offset))

#define isp_reg_writeb(isp, value, offset)	writeb((value), (isp)->base + (offset))

void isp_firmware_writeb(struct isp_device * isp,
		unsigned char value, unsigned int offset);
unsigned char isp_firmware_readb(struct isp_device * isp,
		unsigned int offset);
int calibrate_setting_by_file(struct isp_device *isp, char *file_path);
int calibrate_read_by_file(struct isp_device *isp, char *file_path);
#endif /* __OVISP_BASE_H__*/
