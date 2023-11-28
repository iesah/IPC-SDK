#include "ovisp-base.h"

#define ALL_SETTING_PATH "/data/ov8858/All_Setting.txt"
#define RAW_UV_DNS_SETTING_PATH "/data/ov8858/Raw_UvDns_Setting.txt"
#define LENC_SETTING_PATH "/data/ov8858/LENC_Setting.txt"
#define CURVE_SETTING_PATH "/data/ov8858/Curve_Setting.txt"
#define CCM_SETTING_PATH "/data/ov8858/CCM_Setting.txt"
#define BAND_SETTING_PATH "/data/ov8858/Band_Setting.txt"
#define AWB_SETTING_PATH "/data/ov8858/Awb_Setting.txt"
#define AF_SETTING_V2_2_2_PATH "/data/ov8858/AF_Setting_V2.2.2.txt"
#define AF_SETTING_14_PATH "/data/ov8858/AF_Setting_14.txt"
#define AEC_SETTING_PATH "/data/ov8858/Aec_Setting.txt"
// Read register by file
#define READ_REG_FILE "/data/ov8858/read_reg_file.txt"

void isp_setting_init(struct isp_device *isp)
{
	isp_reg_writeb(isp, 0x3f, 0x65000);
	calibrate_setting_by_file(isp, ALL_SETTING_PATH);
	calibrate_setting_by_file(isp, AWB_SETTING_PATH);
	calibrate_setting_by_file(isp, CCM_SETTING_PATH);
	calibrate_setting_by_file(isp, LENC_SETTING_PATH);
	calibrate_setting_by_file(isp, BAND_SETTING_PATH);
}


