/*
 * sample-Encoder-video-direct.c
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 *本文件所有调用的api具体说明均可在 proj/sdk-lv3/include/api/cn/imp/ 目录下头文件中查看
 *该sample支持最多三路数据，主码流使用ISP绘制OSD(支持直通)，次码流使用IPU绘制OSD，三路码流不绘制OSD，该sample仅作为
 *使用直通、ISPOSD、IPUOSD的示例
* */

#include <stdio.h>
#include <stdlib.h>
#include <imp/imp_log.h>
#include <imp/isp_osd.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>

#include "sample-common.h"

#include "logodata_100x100_bgra.h"

#define TAG "Sample-Encoder-OSD"

extern struct chn_conf chn[];
static int byGetFd = 0;
extern int direct_switch;
extern int gosd_enable;
IMPRgnHandle *pIpuHander;


#define ISPOSD
#ifdef ISPOSD
#define OSD_LETTER_NUM 20
#ifdef SUPPORT_RGB555LE
#include "bgramapinfo_rgb555le.h"
#else
#include "bgramapinfo.h"
#endif

static uint32_t *timeStampData_ipu;
static uint32_t *timeStampData_isp;

static IMPOSDRgnAttr rIspOsdAttr;
#endif


//ISPOSD 绘制的公共部分
int* sample_isposd_init(int chnNum)
{
	int *prHander = NULL;
	int timehandle = -1,logohandle = -1;
	prHander = malloc(2 * sizeof(int));
	if (!prHander) {
		IMP_LOG_ERR(TAG, "malloc() error !\n");
		return NULL;
	}

	timehandle = IMP_ISP_Tuning_CreateOsdRgn(chnNum, NULL);
	if(timehandle < 0)
	{
		IMP_LOG_ERR(TAG,"[%s][%d]IMP_ISP_Tuning_CreateOsdRgn err\n",__func__,__LINE__);
		return NULL;
	}

	logohandle = IMP_ISP_Tuning_CreateOsdRgn(chnNum, NULL);
	if(logohandle < 0)
	{
		IMP_LOG_ERR(TAG,"[%s][%d]IMP_ISP_Tuning_CreateOsdRgn err\n",__func__,__LINE__);
		return NULL;
	}

	prHander[0] = timehandle;
	prHander[1] = logohandle;

	IMP_LOG_INFO(TAG, "[%s]succeed!\n",__func__);

	return prHander;
}

int sample_isposd_exit(int *prHander)
{
	if(!prHander){
		IMP_LOG_ERR(TAG, "[%s][%d]prHander is null\n",__func__,__LINE__);
		return -1;
	}
	int chnNum = 0,showflg = 0;

	IMP_ISP_Tuning_ShowOsdRgn(chnNum,prHander[0],showflg);
	IMP_ISP_Tuning_DestroyOsdRgn(chnNum,prHander[0]);

	IMP_ISP_Tuning_ShowOsdRgn(chnNum,prHander[1],showflg);
	IMP_ISP_Tuning_DestroyOsdRgn(chnNum,prHander[1]);

	free(prHander);
	prHander = NULL;

	return 0;
}

void update_time(void *p)
{
	int ret;
	IMP_Sample_OsdParam *pisposdparam = (IMP_Sample_OsdParam *)p;
	IMPIspOsdAttrAsm stISPOSDAsm = {0};
	/*generate time*/
	char DateStr[40];
	time_t currTime;
	struct tm *currDate;
	unsigned i = 0, j = 0;
	void *dateData = NULL;
	uint32_t *data = pisposdparam->ptimestamps;
	int timehandle = pisposdparam->phandles[0];
	while(1) {
		int penpos_t = 0;
		int fontadv = 0;

		time(&currTime);
		currDate = localtime(&currTime);
		memset(DateStr, 0, 40);
		strftime(DateStr, 40, "%Y-%m-%d %I:%M:%S", currDate);
		for (i = 0; i < OSD_LETTER_NUM; i++) {
			switch(DateStr[i]) {
				case '0' ... '9':
					dateData = (void *)gBgramap[DateStr[i] - '0'].pdata;
					fontadv = gBgramap[DateStr[i] - '0'].width;
					penpos_t += gBgramap[DateStr[i] - '0'].width;
					break;
				case '-':
					dateData = (void *)gBgramap[10].pdata;
					fontadv = gBgramap[10].width;
					penpos_t += gBgramap[10].width;
					break;
				case ' ':
					dateData = (void *)gBgramap[11].pdata;
					fontadv = gBgramap[11].width;
					penpos_t += gBgramap[11].width;
					break;
				case ':':
					dateData = (void *)gBgramap[12].pdata;
					fontadv = gBgramap[12].width;
					penpos_t += gBgramap[12].width;
					break;
				default:
					break;
			}
#ifdef SUPPORT_RGB555LE
			for (j = 0; j < OSD_REGION_HEIGHT; j++) {
				memcpy((void *)((uint16_t *)data + j*OSD_LETTER_NUM*OSD_REGION_WIDTH + penpos_t),
						(void *)((uint16_t *)dateData + j*fontadv), fontadv*sizeof(uint16_t));
			}
#else
			for (j = 0; j < OSD_REGION_HEIGHT; j++) {
				memcpy((void *)((uint32_t *)data + j*OSD_LETTER_NUM*OSD_REGION_WIDTH + penpos_t),
						(void *)((uint32_t *)dateData + j*fontadv), fontadv*sizeof(uint32_t));
			}

#endif
		}
#ifdef SUPPORT_RGB555LE
		stISPOSDAsm.type = ISP_OSD_REG_PIC;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = IMP_ISP_PIC_ARGB_1555;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
		stISPOSDAsm.stsinglepicAttr.pic.pinum =  timehandle;
		stISPOSDAsm.stsinglepicAttr.pic.osd_enable = 1;
		stISPOSDAsm.stsinglepicAttr.pic.osd_left = 10;
		stISPOSDAsm.stsinglepicAttr.pic.osd_top = 10;
		stISPOSDAsm.stsinglepicAttr.pic.osd_width = OSD_REGION_WIDTH * OSD_LETTER_NUM;
		stISPOSDAsm.stsinglepicAttr.pic.osd_height = OSD_REGION_HEIGHT;
		stISPOSDAsm.stsinglepicAttr.pic.osd_image = (char*)data;
		stISPOSDAsm.stsinglepicAttr.pic.osd_stride = OSD_REGION_WIDTH * OSD_LETTER_NUM * 2;
#else
		stISPOSDAsm.type = ISP_OSD_REG_PIC;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = IMP_ISP_PIC_ARGB_8888;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
		stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
		stISPOSDAsm.stsinglepicAttr.pic.pinum =  timehandle;
		stISPOSDAsm.stsinglepicAttr.pic.osd_enable = 1;
		stISPOSDAsm.stsinglepicAttr.pic.osd_left = 10;
		stISPOSDAsm.stsinglepicAttr.pic.osd_top = 10;
		stISPOSDAsm.stsinglepicAttr.pic.osd_width = OSD_REGION_WIDTH * OSD_LETTER_NUM;
		stISPOSDAsm.stsinglepicAttr.pic.osd_height = OSD_REGION_HEIGHT;
		stISPOSDAsm.stsinglepicAttr.pic.osd_image = (char*)data;
		stISPOSDAsm.stsinglepicAttr.pic.osd_stride = OSD_REGION_WIDTH * OSD_LETTER_NUM * 4;
#endif

		ret = IMP_ISP_Tuning_SetOsdRgnAttr(0, timehandle, &stISPOSDAsm);
		if(ret < 0) {
			IMP_LOG_ERR(TAG,"IMP_ISP_SetOSDAttr error\n");
			return ;
		}

		ret = IMP_ISP_Tuning_ShowOsdRgn(0, timehandle, 1);
		if(ret < 0) {
			IMP_LOG_ERR(TAG,"IMP_OSD_ShowRgn_ISP error\n");
			return ;
		}

		/*更新时间戳*/
		sleep(1);
	}

	return ;
}

static void draw_pic(int handle)
{
	int ret = 0;
	IMPIspOsdAttrAsm stISPOSDAsm;
	stISPOSDAsm.type = ISP_OSD_REG_PIC;
	stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_type = IMP_ISP_PIC_ARGB_8888;
	stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
	stISPOSDAsm.stsinglepicAttr.chnOSDAttr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_DISABLE;
	stISPOSDAsm.stsinglepicAttr.pic.pinum =  handle;
	stISPOSDAsm.stsinglepicAttr.pic.osd_enable = 1;
	stISPOSDAsm.stsinglepicAttr.pic.osd_left = 100;
	stISPOSDAsm.stsinglepicAttr.pic.osd_top = 100;
	stISPOSDAsm.stsinglepicAttr.pic.osd_width = 100;
	stISPOSDAsm.stsinglepicAttr.pic.osd_height = 100;
	stISPOSDAsm.stsinglepicAttr.pic.osd_image = (void *)logodata_100x100_bgra;
	stISPOSDAsm.stsinglepicAttr.pic.osd_stride = 100*4;

	ret = IMP_ISP_Tuning_SetOsdRgnAttr(0, handle, &stISPOSDAsm);
	if(ret < 0) {
		IMP_LOG_ERR(TAG,"IMP_ISP_Tuning_SetOsdRgnAttr error\n");
		return ;
	}

	ret = IMP_ISP_Tuning_ShowOsdRgn(0,  handle, 1);
	if(ret < 0) {
		IMP_LOG_ERR(TAG,"IMP_ISP_Tuning_ShowOsdRgn error\n");
		return ;
	}

}

static void draw_narmal(IMPOsdRgnType type)
{
	int ret = 0;
	memset(&rIspOsdAttr, 0, sizeof(IMPOSDRgnAttr));

	if(OSD_REG_ISP_LINE_RECT == type)
	{
		rIspOsdAttr.type = OSD_REG_ISP_LINE_RECT;
		rIspOsdAttr.osdispdraw.stDrawAttr.pinum = 0;
		rIspOsdAttr.osdispdraw.stDrawAttr.type = IMP_ISP_DRAW_LINE;
		rIspOsdAttr.osdispdraw.stDrawAttr.color_type = IMPISP_MASK_TYPE_YUV;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.enable = 1;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.startx = 200;    /*绘制竖线*/
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.starty = 200;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.endx = 500;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.endy = 200;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.color.ayuv.y_value = 255;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.color.ayuv.u_value = 0;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.color.ayuv.v_value = 0;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.width = 5;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.line.alpha = 4; /*范围为[0,4],数值越小越透明*/

		ret = IMP_OSD_SetRgnAttr_ISP(0, &rIspOsdAttr, 0);
		if(ret < 0){
			IMP_LOG_ERR(TAG,"[%s][%d]IMP_OSD_SetRgnAttr_ISP err\n",__func__,__LINE__);
		}
	}

	if(OSD_REG_ISP_LINE_RECT == type)
	{
		rIspOsdAttr.type = OSD_REG_ISP_LINE_RECT;
		rIspOsdAttr.osdispdraw.stDrawAttr.pinum = 1;
		rIspOsdAttr.osdispdraw.stDrawAttr.type = IMP_ISP_DRAW_WIND;
		rIspOsdAttr.osdispdraw.stDrawAttr.color_type = IMPISP_MASK_TYPE_YUV;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.enable = 1;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.left = 900;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.top = 200;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.width = 300;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.height = 300;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.y_value = 0;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.u_value = 255;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.color.ayuv.v_value = 0;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.line_width = 3;
		rIspOsdAttr.osdispdraw.stDrawAttr.cfg.wind.alpha = 4; /*范围为[0,4]*/

		ret = IMP_OSD_SetRgnAttr_ISP(0, &rIspOsdAttr, 0);
		if(ret < 0){
			IMP_LOG_ERR(TAG,"[%s][%d]IMP_OSD_SetRgnAttr_ISP err\n",__func__,__LINE__);
		}
	}

	if(OSD_REG_ISP_COVER == type)
	{
		rIspOsdAttr.type = OSD_REG_ISP_COVER;
		rIspOsdAttr.osdispdraw.stCoverAttr.chx = 0;
		rIspOsdAttr.osdispdraw.stCoverAttr.pinum = 0;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_en = 1;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_pos_top = 1000;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_pos_left = 600;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_width = 300;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_height = 300;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_type = IMPISP_MASK_TYPE_RGB;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_value.argb.r_value = 0;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_value.argb.g_value = 0;
		rIspOsdAttr.osdispdraw.stCoverAttr.mask_value.argb.b_value = 255;

		ret = IMP_OSD_SetRgnAttr_ISP(0, &rIspOsdAttr, 0);
		if(ret < 0){
			IMP_LOG_ERR(TAG,"[%s][%d]IMP_OSD_SetRgnAttr_ISP err\n",__func__,__LINE__);
		}
	}

	return ;
}

void* isp_osd_thread(void *arg)
{
	IMP_Sample_OsdParam *pisposdparam = (IMP_Sample_OsdParam*)arg;

	/*绘制线、框、矩形遮挡*/
	draw_narmal(OSD_REG_ISP_LINE_RECT);

	IMP_LOG_INFO(TAG,"ISP OSD draw_narmal success\n");

	/*绘制图片，注意绘制图片类型的ISP接口和绘制线、框、矩形遮挡的接口有所区别*/
	draw_pic(pisposdparam->phandles[1]);

	IMP_LOG_INFO(TAG,"ISP OSD draw_pic success\n");

	/*绘制时间戳*/
	update_time(pisposdparam);

	IMP_LOG_INFO(TAG,"ISP OSD time success\n");

	return NULL;
}

IMPRgnHandle *sample_ipuosd_init(int grpNum)
{
	int ret;
	IMPRgnHandle *prHander = NULL;
	IMPRgnHandle rHanderFont = 0;
	IMPRgnHandle rHanderLogo = 0;

	prHander = malloc(5 * sizeof(IMPRgnHandle));
	if (prHander <= 0) {
		IMP_LOG_ERR(TAG, "malloc() error !\n");
		return NULL;
	}

	rHanderFont = IMP_OSD_CreateRgn(NULL);
	if (rHanderFont == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn TimeStamp error !\n");
		return NULL;
	}
	//query osd rgn create status
	IMPOSDRgnCreateStat stStatus;
	memset(&stStatus,0x0,sizeof(IMPOSDRgnCreateStat));
	ret = IMP_OSD_RgnCreate_Query(rHanderFont,&stStatus);
	if(ret < 0){
		IMP_LOG_ERR(TAG, "IMP_OSD_RgnCreate_Query error !\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderFont, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrFont;
	memset(&rAttrFont, 0, sizeof(IMPOSDRgnAttr));
	rAttrFont.type = OSD_REG_PIC;
	rAttrFont.rect.p0.x = 10;
	rAttrFont.rect.p0.y = 10;
	rAttrFont.rect.p1.x = rAttrFont.rect.p0.x + 20 * OSD_REGION_WIDTH- 1;	//p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrFont.rect.p1.y = rAttrFont.rect.p0.y + OSD_REGION_HEIGHT - 1;
#ifdef SUPPORT_RGB555LE
	rAttrFont.fmt = PIX_FMT_RGB555LE;
#else
	rAttrFont.fmt = PIX_FMT_BGRA;
#endif
	rAttrFont.data.picData.pData = NULL;
	ret = IMP_OSD_SetRgnAttr(rHanderFont, &rAttrFont);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr TimeStamp error !\n");
		return NULL;
	}

	IMPOSDGrpRgnAttr grAttrFont;

	if (IMP_OSD_GetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrFont, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrFont.show = 1;

	/* Disable Font global alpha, only use pixel alpha. */
	grAttrFont.gAlphaEn = 1;
	grAttrFont.fgAlhpa = 0xff;
	grAttrFont.layer = 3;
	if (IMP_OSD_SetGrpRgnAttr(rHanderFont, grpNum, &grAttrFont) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}

	//实际上自己show配置的就是grAttrFont.show的值
	ret = IMP_OSD_ShowRgn(prHander[0], grpNum, 1);
	if (ret != 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_ShowRgn() timeStamp error\n");
		return NULL;
	}

	//logo
	rHanderLogo = IMP_OSD_CreateRgn(NULL);
	if (rHanderLogo == INVHANDLE) {
		IMP_LOG_ERR(TAG, "IMP_OSD_CreateRgn Logo error !\n");
		return NULL;
	}

	ret = IMP_OSD_RegisterRgn(rHanderLogo, grpNum, NULL);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IVS IMP_OSD_RegisterRgn failed\n");
		return NULL;
	}

	IMPOSDRgnAttr rAttrLogo;
	memset(&rAttrLogo, 0, sizeof(IMPOSDRgnAttr));
	int picw = 100;
	int pich = 100;
	rAttrLogo.type = OSD_REG_PIC;
	rAttrLogo.rect.p0.x = 0;
	rAttrLogo.rect.p0.y = 0;

	//p0 is start，and p1 well be epual p0+width(or heigth)-1
	rAttrLogo.rect.p1.x = rAttrLogo.rect.p0.x+picw-1;
	rAttrLogo.rect.p1.y = rAttrLogo.rect.p0.y+pich-1;
	rAttrLogo.fmt = PIX_FMT_BGRA;
	rAttrLogo.data.picData.pData = logodata_100x100_bgra;
	ret = IMP_OSD_SetRgnAttr(rHanderLogo, &rAttrLogo);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetRgnAttr Logo error !\n");
		return NULL;
	}
	IMPOSDGrpRgnAttr grAttrLogo;

	if (IMP_OSD_GetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_GetGrpRgnAttr Logo error !\n");
		return NULL;

	}
	memset(&grAttrLogo, 0, sizeof(IMPOSDGrpRgnAttr));
	grAttrLogo.show = 1;
	/* Set Logo global alpha to 0x7f, it is semi-transparent. */
	grAttrLogo.gAlphaEn = 1;
	grAttrLogo.fgAlhpa = 0x7f;
	grAttrLogo.layer = 2;

	if (IMP_OSD_SetGrpRgnAttr(rHanderLogo, grpNum, &grAttrLogo) < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_SetGrpRgnAttr Logo error !\n");
		return NULL;
	}

	prHander[0] = rHanderFont;
	prHander[1] = rHanderLogo;

	if(IMP_OSD_Start(grpNum) < 0){
		IMP_LOG_ERR(TAG, "IMP_OSD_Start error !\n");
		return NULL;
	}

	IMP_LOG_INFO(TAG, "[%s]succeed!\n",__func__);

	return prHander;
}

int sample_ipuosd_exit(IMPRgnHandle *ipuhandls,int grpNum)
{
	int ret = 0;
	ret = IMP_OSD_UnRegisterRgn(ipuhandls[0], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Cover error\n");
	}

	ret = IMP_OSD_UnRegisterRgn(ipuhandls[1], grpNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_OSD_UnRegisterRgn Rect error\n");
	}

	IMP_OSD_DestroyRgn(ipuhandls[0]);
	IMP_OSD_DestroyRgn(ipuhandls[1]);

	IMP_OSD_Stop(grpNum);

	free(ipuhandls);
	ipuhandls = NULL;

	return 0;
}

/*IPUOSD 更新时间戳线程*/
static void *update_thread(void *p)
{
	IMP_Sample_OsdParam *pipuosdpam = (IMP_Sample_OsdParam*)p;

	/*generate time*/
	char DateStr[40];
	time_t currTime;
	struct tm *currDate;
	unsigned i = 0, j = 0;
	void *dateData = NULL;
	int handle = pipuosdpam->phandles[0];
	uint32_t *data = pipuosdpam->ptimestamps;
	IMPOSDRgnAttrData rAttrData;

	while(1) {
			int penpos_t = 0;
			int fontadv = 0;

			time(&currTime);
			currDate = localtime(&currTime);
			memset(DateStr, 0, 40);
			strftime(DateStr, 40, "%Y-%m-%d %I:%M:%S", currDate);
			for (i = 0; i < OSD_LETTER_NUM; i++) {
				switch(DateStr[i]) {
					case '0' ... '9':
						dateData = (void *)gBgramap[DateStr[i] - '0'].pdata;
						fontadv = gBgramap[DateStr[i] - '0'].width;
						penpos_t += gBgramap[DateStr[i] - '0'].width;
						break;
					case '-':
						dateData = (void *)gBgramap[10].pdata;
						fontadv = gBgramap[10].width;
						penpos_t += gBgramap[10].width;
						break;
					case ' ':
						dateData = (void *)gBgramap[11].pdata;
						fontadv = gBgramap[11].width;
						penpos_t += gBgramap[11].width;
						break;
					case ':':
						dateData = (void *)gBgramap[12].pdata;
						fontadv = gBgramap[12].width;
						penpos_t += gBgramap[12].width;
						break;
					default:
						break;
				}
#ifdef SUPPORT_RGB555LE
				for (j = 0; j < OSD_REGION_HEIGHT; j++) {
					memcpy((void *)((uint16_t *)data + j*OSD_LETTER_NUM*OSD_REGION_WIDTH + penpos_t),
							(void *)((uint16_t *)dateData + j*fontadv), fontadv*sizeof(uint16_t));
				}
#else
				for (j = 0; j < OSD_REGION_HEIGHT; j++) {
					memcpy((void *)((uint32_t *)data + j*OSD_LETTER_NUM*OSD_REGION_WIDTH + penpos_t),
							(void *)((uint32_t *)dateData + j*fontadv), fontadv*sizeof(uint32_t));
				}

#endif
			}
			rAttrData.picData.pData = data;
			IMP_OSD_UpdateRgnAttrData(handle, &rAttrData);

			sleep(1);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i = 0, ret = 0;
	direct_switch = 1;	/*ivdc resource*/
	gosd_enable = 3; 	/*need rmem*/
	int chnNum = 0;
	int grpNum = 1;	/*as ipu resource*/
	int *pisphandles = NULL,*pipuhandles = NULL;
	IMPCell osdcell = {DEV_ID_OSD, grpNum, 0};

    if (argc >= 2) {
        byGetFd = atoi(argv[1]);
    }

	/* Step.1 System init */
	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/*Step.3 IPU OSD init，this sample show second chn*/
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable && 1 == i) {
			ret = IMP_OSD_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}

	/* Step.4 Encoder init */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}

	ret = sample_encoder_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.5 Bind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		/*除次码流外，其他码流走ISPOSD或者直接编码，次码流运行IPUOSD，需要绑定OSD模块*/
		if (chn[i].enable && 1 != i) {
			ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		}else if (chn[i].enable && 1 == i){
			ret = IMP_System_Bind(&chn[i].framesource_chn, &osdcell);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind FrameSource channel0 and OSD failed\n");
				return -1;
			}

			ret = IMP_System_Bind(&osdcell, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind OSD and Encoder failed\n");
				return -1;
			}
		}
	}

	/* Step.6 ISPOSD and IPUOSD init */
	pipuhandles = sample_ipuosd_init(grpNum);
	if (pipuhandles <= 0) {
		IMP_LOG_ERR(TAG, "OSD init failed\n");
		return -1;
	}

	pisphandles = sample_isposd_init(chnNum);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample osd init isp failed\n");
		return -1;
	}

#ifdef SUPPORT_RGB555LE
	timeStampData_ipu = malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint16_t));
	timeStampData_isp = malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint16_t));
#else
	timeStampData_ipu = malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint32_t));
	timeStampData_isp = malloc(OSD_LETTER_NUM * OSD_REGION_HEIGHT * OSD_REGION_WIDTH * sizeof(uint32_t));
#endif

	/* Step.5 Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}

	IMP_Sample_OsdParam ipuosdparam;
	ipuosdparam.phandles = pipuhandles;
	ipuosdparam.ptimestamps = timeStampData_ipu;
	pthread_t ipuosdtid;
	ret = pthread_create(&ipuosdtid, NULL, update_thread, &ipuosdparam);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error %d\n", __LINE__);
		return -1;
	}

	/*isp osd*/
	IMP_Sample_OsdParam isposdparam;
	isposdparam.phandles = pisphandles;
	isposdparam.ptimestamps = timeStampData_isp;
	pthread_t isposdtid;
	pthread_create(&isposdtid, NULL, isp_osd_thread, &isposdparam);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error %d\n", __LINE__);
		return -1;
	}

	/* Step.6 Get stream */
    if (byGetFd) {
        ret = sample_get_video_stream_byfd();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Get video stream byfd failed\n");
            return -1;
        }
    } else {
        ret = sample_get_video_stream();
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "Get video stream failed\n");
            return -1;
        }
    }

	ret = sample_isposd_exit(pisphandles);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error\n");
		return -1;
	}

	ret = sample_ipuosd_exit(pipuhandles,grpNum);
	if (ret) {
		IMP_LOG_ERR(TAG, "thread create error\n");
		return -1;
	}

	pthread_cancel(ipuosdtid);
	pthread_join(ipuosdtid, NULL);
	pthread_cancel(isposdtid);
	pthread_join(isposdtid, NULL);
	free(timeStampData_ipu);
	free(timeStampData_isp);

	/* Exit sequence as follow */
	/* Step.a Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}


	/* Step.b UnBind */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable && 1 != i) {
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
		} else if(chn[i].enable && 1 == i) {
			ret = IMP_System_UnBind(&osdcell, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "Bind OSD and Encoder failed\n");
				return -1;
			}

			ret = IMP_System_UnBind(&chn[i].framesource_chn, &osdcell);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource and OSD failed\n");
				return -1;
			}
		}
	}

	/* Step.c Encoder exit */
	ret = sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}

	/* Step.d ipuosd exit */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable && 1 == i) {
			ret = IMP_OSD_DestroyGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", chn[i].index);
				return -1;
			}
		}
	}

	/* Step.e FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.f System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	return 0;
}
