#include "ovisp-base.h"
#include "isp-debug.h"
#if 0
inline unsigned int isp_reg_readl(struct isp_device *isp,
		unsigned int offset)
{
#define pp 3
#if (pp == 0)
	return (((unsigned int)readb(isp->base + offset) << 24)
			| ((unsigned int)readb(isp->base + offset + 1) << 16)
			| ((unsigned int)readb(isp->base + offset + 2) << 8)
			| (unsigned int)readb(isp->base + offset + 3));
#elif (pp == 1)
	return (((unsigned int)readb(isp->base + offset))
			| ((unsigned int)readb(isp->base + offset + 1) << 8)
			| ((unsigned int)readb(isp->base + offset + 2) << 16)
			| (unsigned int)readb(isp->base + offset + 3) << 24);
#elif (pp == 3)
	return readl(isp->base + offset);
#endif
#undef	pp
}

inline unsigned short isp_reg_readw(struct isp_device *isp,
		unsigned int offset)
{
	return readw(isp->base + offset);
}

inline unsigned char isp_reg_readb(struct isp_device *isp,
		unsigned int offset)
{
	return readb(isp->base + offset);
}

inline void isp_reg_writel(struct isp_device *isp,
		unsigned int value, unsigned int offset)
{
	writel(value, isp->base + offset);
}

inline void isp_reg_writew(struct isp_device *isp,
		unsigned short value, unsigned int offset)
{
	writew(value, isp->base + offset);
}

inline void isp_reg_writeb(struct isp_device *isp,
		unsigned char value, unsigned int offset)
{
	writeb(value, isp->base + offset);
}
#endif
void isp_firmware_writeb(struct isp_device * isp,
		unsigned char value, unsigned int offset)
{
	int pos = offset % 4;
	isp_reg_writeb(isp, value, offset - pos + (3 - pos));
}
unsigned char isp_firmware_readb(struct isp_device * isp,
		unsigned int offset)
{
	int pos = offset % 4;
	return isp_reg_readb(isp, offset - pos + (3 -  pos));
}
/*=============================================================*/
unsigned int read_reg_array[1024] = {0};
unsigned int read_reg_pri_type[1024] = {0};
int read_reg_num = 0;

#define strtoul                     simple_strtoul
int calibrate_read_by_file(struct isp_device *isp, char *file_path)
{
	struct file *file = NULL;
	struct inode *inode = NULL;
	char *file_buf = NULL;
	loff_t fsize;
	mm_segment_t old_fs;
	loff_t *pos;
	OV_CALIBRATION_SETTING setting;
	int setting_num = 0;


	file = filp_open(file_path, O_RDONLY, 0);
	if (file < 0 || IS_ERR(file)) {
		ISP_PRINT(ISP_ERROR,"ISP: open %s file for isp calibrate read failed\n", file_path);
		return -1;
	}

	inode = file->f_dentry->d_inode;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = &(file->f_pos);
	file_buf = (char *)vmalloc(fsize+1);
	if( file_buf == NULL ) {
		ISP_PRINT(ISP_ERROR,"file_buf vmalloc is error %s %d\n", __func__, __LINE__);
		return -1;;
	}
	memset(file_buf, 0x00, fsize+1);
	vfs_read(file, file_buf, fsize, pos);
	filp_close(file, NULL);
	set_fs(old_fs);

	{
		char *buf_p = file_buf;
		char *p = NULL;
		read_reg_num = 0;
		do {
			p = strstr(buf_p, "{0x");
			if( p != NULL ) {
				setting.offset = strtoul(p+1, NULL, 0);
				p = strstr(p+3, ", 0x");
			}
			if( p != NULL ) {
				setting.val = strtoul(p+2, NULL, 0);

				if( setting.offset & 0x70000 ) {
					read_reg_array[read_reg_num] = setting.offset;
					read_reg_pri_type[read_reg_num] = setting.val;
					read_reg_num ++;
				} else
					ISP_PRINT(ISP_WARNING,"ISP: not support this register 0x%05x = 0x%02x\n",
											setting.offset, setting.val);
				setting_num++;
				buf_p = p+4;
			}
		} while(p != NULL);
		ISP_PRINT(ISP_INFO,"ISP: %s the read number of lines is %d\n", file_path, setting_num);
	}

	vfree(file_buf);
	return 0;
}

int calibrate_setting_by_file(struct isp_device *isp, char *file_path)
{
	struct file *file = NULL;
	struct inode *inode = NULL;
	char *file_buf = NULL;
	loff_t fsize;
	mm_segment_t old_fs;
	loff_t *pos;
	OV_CALIBRATION_SETTING setting;
	int setting_num = 0;


	file = filp_open(file_path, O_RDONLY, 0);
	if (file < 0 || IS_ERR(file)) {
		ISP_PRINT(ISP_ERROR,"ISP: open %s file for isp calibrate setting failed\n", file_path);
		return -1;
	}
	inode = file->f_dentry->d_inode;
	fsize = inode->i_size;
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = &(file->f_pos);
	file_buf = (char *)vmalloc(fsize+1);
	if( file_buf == NULL ) {
		ISP_PRINT(ISP_ERROR,"file_buf vmalloc is error %s %d\n", __func__, __LINE__);
		return -1;
	}
	memset(file_buf, 0x00, fsize+1);
	vfs_read(file, file_buf, fsize, pos);
	filp_close(file, NULL);
	set_fs(old_fs);

	{
		char *buf_p = file_buf;
		char *p = NULL;
		do {
			p = strstr(buf_p, "{0x");
			if( p != NULL ) {
				setting.offset = strtoul(p+1, NULL, 0);
				p = strstr(p+3, ", 0x");
			}
			if( p != NULL ) {
				setting.val = strtoul(p+2, NULL, 0);
				if( setting.offset & 0x10000 )
					isp_firmware_writeb(isp, setting.val, setting.offset);
				else if( setting.offset & 0x60000 )
					isp_reg_writeb(isp, setting.val, setting.offset);
				else
					ISP_PRINT(ISP_WARNING,"ISP: not support this register 0x%05x = 0x%02x\n",
											setting.offset, setting.val);
				setting_num++;
				buf_p = p+4;
			}
		} while(p != NULL);
		ISP_PRINT(ISP_INFO,"ISP: %s the setting number of lines is %d\n", file_path, setting_num);
	}

	vfree(file_buf);

	return 0;
}
