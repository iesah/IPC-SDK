#ifndef __ISP_CTRL_H__
#define __ISP_CTRL_H__

#include "ovisp-isp.h"

void isp_set_exposure_init(struct isp_device *isp);
extern int isp_set_exposure(struct isp_device *isp, int val);
extern int isp_set_exposure_manual(struct isp_device *isp);
extern int isp_set_gain(struct isp_device *isp, int val);
extern int isp_set_iso(struct isp_device *isp, int val);
extern int isp_set_contrast(struct isp_device *isp, int val);
extern int isp_set_saturation(struct isp_device *isp, int val);
extern int isp_set_scene(struct isp_device *isp, int val);
extern int isp_set_effect(struct isp_device *isp, int val);
extern int isp_set_do_white_balance(struct isp_device *isp, int val);
extern int isp_set_brightness(struct isp_device *isp, int val);
extern int isp_set_sharpness(struct isp_device *isp, int val);
extern int isp_set_flicker(struct isp_device *isp, int val);
extern int isp_set_hflip(struct isp_device *isp, int val);
extern int isp_set_vflip(struct isp_device *isp, int val);

/* add by xhshen */
extern int isp_set_auto_white_balance(struct isp_device *isp, int val);

int isp_set_red_balance(struct isp_device *isp, int val);
int isp_set_blue_balance(struct isp_device *isp, int val);
int isp_get_red_balance(struct isp_device *isp, int* val);
int isp_get_blue_balance(struct isp_device *isp, int* val);
#endif/*__ISP_CTRL_H__*/
