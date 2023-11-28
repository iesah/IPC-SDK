#include "ovisp-base.h"

#define ALL_SETTING_PATH "/etc/bf3116/All_Setting.txt"
#define RAW_UV_DNS_SETTING_PATH "/etc/bf3116/Raw_UvDns_Setting.txt"
#define LENC_SETTING_PATH "/etc/bf3116/LENC_Setting.txt"
#define CURVE_SETTING_PATH "/etc/bf3116/Curve_Setting.txt"
#define CCM_SETTING_PATH "/etc/bf3116/CCM_Setting.txt"
#define BAND_SETTING_PATH "/etc/bf3116/Band_Setting.txt"
#define AWB_SETTING_PATH "/etc/bf3116/Awb_Setting.txt"
#define AF_SETTING_V2_2_2_PATH "/etc/bf3116/AF_Setting_V2.2.2.txt"
#define AF_SETTING_14_PATH "/etc/bf3116/AF_Setting_14.txt"
#define AEC_SETTING_PATH "/etc/bf3116/Aec_Setting.txt"
#define RAW_TOP_SETTING_PATH "/etc/bf3116/Raw_Top_Setting.txt"
#define RAW_LINER_SETTING_PATH "/etc/bf3116/Raw_Liner_Setting.txt"
#define READ_REG_FILE "/etc/bf3116/read_reg_file.txt"


void isp_setting_init(struct isp_device *isp)
{
//	isp_reg_writeb(isp, 0x3f, 0x65000);
//	calibrate_setting_by_file(isp, RAW_TOP_SETTING_PATH);
//	calibrate_setting_by_file(isp, RAW_LINER_SETTING_PATH);

	/* calibrate_setting_by_file(isp, AEC_AGC_SETTING_PATH); */
	calibrate_setting_by_file(isp, AEC_SETTING_PATH);
	calibrate_setting_by_file(isp, ALL_SETTING_PATH);
//	calibrate_setting_by_file(isp, CURVE_SETTING_PATH);
	calibrate_setting_by_file(isp, AWB_SETTING_PATH);
//	calibrate_setting_by_file(isp, RAW_UV_DNS_SETTING_PATH);
	calibrate_setting_by_file(isp, CCM_SETTING_PATH);
	calibrate_setting_by_file(isp, LENC_SETTING_PATH);
//	calibrate_setting_by_file(isp, BAND_SETTING_PATH);
	calibrate_setting_by_file(isp, AF_SETTING_V2_2_2_PATH);
//	calibrate_setting_by_file(isp, AF_SETTING_14_PATH);
//	calibrate_read_by_file(isp, READ_REG_FILE);
}


