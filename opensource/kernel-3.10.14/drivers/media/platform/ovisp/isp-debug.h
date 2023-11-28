#ifndef _ISP_DEBUG_H_
#define _ISP_DEBUG_H_

/* =================== switchs ================== */

/**
 * default debug level, if just switch ISP_WARNING
 * or ISP_INFO, this not effect DEBUG_REWRITE and
 * DEBUG_TIME_WRITE/READ
 **/
//#define OVISP_CSI_TEST
#define PRINT_LEVEL		ISP_WARNING
//#define PRINT_LEVEL		ISP_INFO
/* =================== print tools ================== */

#define ISP_INFO		0x0
#define ISP_WARNING		0x1
#define ISP_ERROR		0x2
#if 0
#define ISP_PRINT(level, ...) do { if (level >= PRINT_LEVEL) printk("[ISP] "__VA_ARGS__); \
	if(level >= ISP_ERROR) dump_stack();} while (0)
#else
#define ISP_PRINT(level, format, ...)		\
	isp_printf(level, format, ##__VA_ARGS__)
#endif
#define ISP_DEBUG(...) ISP_PRINT(ISP_INFO, __VA_ARGS__)

//extern unsigned int isp_print_level;
int isp_debug_init(void);
int isp_debug_deinit(void);
int isp_printf(unsigned int level, unsigned char *fmt, ...);
/* =================== debug isp interfaces ================== */
void dump_isp_top_register(struct isp_device * isp);
void dump_mac(struct isp_device * isp);
void dump_idi(struct isp_device * isp);
void dump_isp_debug_regs(struct isp_device * isp);
void dump_isp_range_regs(struct isp_device * isp, unsigned int addr, int count);
void dump_isp_configuration(struct isp_device * isp);
void dump_isp_firmware(struct isp_device * isp);
void dump_isp_syscontrol(struct isp_device * isp);
void dump_firmware_reg(struct isp_device * isp, int base, int num);
void __dump_isp_regs(struct isp_device * isp, int base, int end);
void dump_isp_configs(struct isp_device *isp);
void dump_tlb_regs(struct isp_device *isp);
void dump_command_reg(struct isp_device *isp);
void dump_i2c_regs(struct isp_device * isp);
void dump_isp_i2c_cmd(struct isp_device *isp, struct isp_i2c_cmd *cmd);

void dump_isp_set_para(struct isp_device *isp, struct isp_parm *iparm, unsigned short iformat, unsigned short oformat);
void dump_isp_cal_zoom(struct isp_device *isp);
void dump_isp_offline_process(struct isp_device *isp);
/* =================== debug csi interfaces ================== */
void dump_csi_reg(void);
void dump_isp_exposure(struct isp_device *isp);
void dump_isp_exposure_init(struct isp_device *isp);
void dump_sensor_exposure(struct v4l2_subdev *sd);
#endif /* _ISP_DEBUG_H_ */
