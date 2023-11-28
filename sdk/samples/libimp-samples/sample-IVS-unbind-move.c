/*
 * sample-IVS-unbind-move.c
 *
 * Copyright (C) 2016 Ingenic Semiconductor Co.,Ltd
 */
#include <string.h>
#include <stdlib.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>
#include <imp/imp_ivs.h>
#include <imp/imp_ivs_move.h>
#include "sample-common.h"
#define TAG "Sample-IVS-unbind-move"
extern struct chn_conf chn[];

/**
 * @defgroup IMP_IVS
 * @ingroup imp
 * @brief IVS智能分析通用接口API(以下调试内容均可在 Ingenic-SDK-T××/include_cn/imp/imp_ivs_move.h　下查看)
 * @section concept 1 相关概念
 * IMP IVS 通过IVS通用接口API调用实例化的IMPIVSInterface以将智能分析算法嵌入到SDK中来分析SDK中的frame图像。
 * @subsection IMPIVSInterface 1.1 IMPIVSInterface
 * IMPIVSInterface 为通用算法接口，具体算法通过实现此接口并将其传给IMP IVS达到在SDK中运行具体算法的目的。
 * IMPIVSInterface 成员param为成员函数init的参数。
 * @section ivs_usage 2 使用方法
 * 函数的具体实现见sample示例文件
 *
 * STEP.1 初始化系统，可以直接调用范例中的sample_system_init()函数。
 * 整个应用程序只能初始化系统一次，若之前初始化了，这儿不需要再初始化。
 * @code
 * ret = IMP_ISP_Open(); //打开isp模块
 * ret = IMP_ISP_EnableTuning();	// 使能翻转，调试图像
 *	ret = IMP_ISP_SetCameraInputMode(&mode) //如果有多个sensor(最大支持三摄),设置多摄的模式(单摄请忽略)
 *	ret = IMP_ISP_AddSensor(IMPVI_MAIN, &sensor_info[*]) //添加sensor,在此操作之前sensor驱动已经添加到内核 (IMPVI_MAIN为主摄, IMPVI_SEC为次摄, IMPVI_THR为第三摄)
 * @endcode
 *
 * STEP.2 初始化framesource
 * 若算法所使用的framesource通道已创建，直接使用已经创建好的通道即可。
 * 若算法所使用的framesource通道未创建，可以调用范列中的sample_framesource_init(IVS_FS_CHN, &fs_chn_attr)进行创建。
 * @code
 * ret = IMP_FrameSource_CreateChn(chn[i].index, &chn[i].fs_chn_attr);	//创建通道
 * ret = IMP_FrameSource_SetChnAttr(chn[i].index, &chn[i].fs_chn_attr);	//设置通道属性
 * @endcode
 *
 * STEP.3 启动framesource。
 * @code
 *	IMP_FrameSource_SetFrameDepth(0, 0);	//设置最大图像深度，此接口用于设置某一通道缓存的视频图像帧数。当用户设置缓存多帧
 *	视频图像时,用户可以获取到一定数目的连续图像数据。若指定 depth 为 0,表示不需要系统为该通道缓存图像,故用户获取
 *	不到该通道图像数据。系统默认不为通道缓存图像,即 depth 默认为 0。
 *	ret = sample_framesource_streamon(IVS_FS_CHN);
 *	if (ret < 0) {
 *		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
 *		return -1;
 *	}
 * @endcode
 *
 * STEP.4 获取算法结果
 * Polling结果、获取结果和释放结果必须严格对应，不能中间有中断;
 * 只有Polling结果正确返回，获取到的结果才会被更新，否则获取到的结果无法预知。
 *
 * STEP.6~9 释放资源,关于资源的释放请按照示例代码对应的顺序使用。
 * @code
 *  sample_ivs_move_stop(2, inteface);
 *  sample_framesource_streamoff();
 *  sample_framesource_exit(IVS_FS_CHN);
 *  sample_system_exit();
 * @endcode
 */
static int sample_ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	IMP_IVS_MoveParam param;
	int i = 0, j = 0;
	memset(&param, 0, sizeof(IMP_IVS_MoveParam));
	param.skipFrameCnt = 1;
	param.frameInfo.width = FIRST_SENSOR_WIDTH_SECOND;
	param.frameInfo.height = FIRST_SENSOR_HEIGHT_SECOND;
	param.roiRectCnt = 1;
	for(i=0; i<param.roiRectCnt; i++){
	  param.sense[i] = 4;
	}
	/* printf("param.sense=%d, param.skipFrameCnt=%d, param.frameInfo.width=%d, param.frameInfo.height=%d\n", param.sense, param.skipFrameCnt, param.frameInfo.width, param.frameInfo.height); */
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
		  if((i==0)&&(j==0)){
			param.roiRect[j * 2 + i].p0.x = i * param.frameInfo.width /* / 2 */;
			param.roiRect[j * 2 + i].p0.y = j * param.frameInfo.height /* / 2 */;
			param.roiRect[j * 2 + i].p1.x = (i + 1) * param.frameInfo.width /* / 2 */ - 1;
			param.roiRect[j * 2 + i].p1.y = (j + 1) * param.frameInfo.height /* / 2 */ - 1;
			printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
		  }
		  else
		    {
		      	param.roiRect[j * 2 + i].p0.x = param.roiRect[0].p0.x;
			param.roiRect[j * 2 + i].p0.y = param.roiRect[0].p0.y;
			param.roiRect[j * 2 + i].p1.x = param.roiRect[0].p1.x;;
			param.roiRect[j * 2 + i].p1.y = param.roiRect[0].p1.y;;
			printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
		    }
		}
	}
	*interface = IMP_IVS_CreateMoveInterface(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}
static int sample_ivs_move_stop(int chn_num, IMPIVSInterface *interface)
{
	IMP_IVS_DestroyMoveInterface(interface);
	return 0;
}
#if 0
static int sample_ivs_set_sense(int chn_num, int sensor)
{
	int ret = 0;
	IMP_IVS_MoveParam param;
	int i = 0;
	ret = IMP_IVS_GetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", chn_num);
		return -1;
	}
	for( i = 0 ; i < param.roiRectCnt ; i++){
	  param.sense[i] = sensor;
	}
	ret = IMP_IVS_SetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", chn_num);
		return -1;
	}
	return 0;
}
#endif
int main(int argc, char *argv[])
{
	int i, ret;
	IMPIVSInterface *interface = NULL;
	//IMP_IVS_MoveParam param;
	IMP_IVS_MoveOutput *result = NULL;
	IMPFrameInfo frame;
	unsigned char * g_sub_nv12_buf_move = 0;
	chn[0].enable = 0;
	chn[1].enable = 1;
	int sensor_sub_width = 640;
	int sensor_sub_height = 360;
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
	g_sub_nv12_buf_move = (unsigned char *)malloc(sensor_sub_width * sensor_sub_height * 3 / 2);
	if (g_sub_nv12_buf_move == 0) {
		printf("error(%s,%d): malloc buf failed \n", __func__, __LINE__);
		return -1;
	}
	/* Step.3 framesource Stream On */
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
	/* Step.4 ivs move start */
	ret = sample_ivs_move_start(0, 2, &interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_start(0, 0) failed\n");
		return -1;
	}
	if(interface->init && ((ret = interface->init(interface)) < 0)) {
		IMP_LOG_ERR(TAG, "interface->init failed, ret=%d\n", ret);
		return -1;
	}
	/* Step.5 start to get ivs move result */
	for (i = 0; i < NR_FRAMES_TO_SAVE; i++) {

		ret = IMP_FrameSource_SnapFrame(1, PIX_FMT_NV12, sensor_sub_width, sensor_sub_height, g_sub_nv12_buf_move, &frame);
		if (ret < 0) {
			printf("%d get frame failed try again\n", 0);
			usleep(30*1000);
		}
		frame.virAddr = (unsigned int)g_sub_nv12_buf_move;
		if (interface->preProcessSync && ((ret = interface->preProcessSync(interface, &frame)) < 0)) {
			IMP_LOG_ERR(TAG, "interface->preProcessSync failed,ret=%d\n", ret);
			return -1;
		}
		if (interface->processAsync && ((ret = interface->processAsync(interface, &frame)) < 0)) {
			IMP_LOG_ERR(TAG, "interface->processAsync failed,ret=%d\n", ret);
			return -1;
		}
		if (interface->getResult && ((ret = interface->getResult(interface, (void **)&result)) < 0)) {
			IMP_LOG_ERR(TAG, "interface->getResult failed,ret=%d\n", ret);
			return -1;
		}
		IMP_LOG_INFO(TAG, "frame[%d], result->retRoi(%d,%d,%d,%d)\n", i, result->retRoi[0], result->retRoi[1], result->retRoi[2], result->retRoi[3]);
		//release moveresult
		if (interface->releaseResult && ((ret = interface->releaseResult(interface, (void *)result)) < 0)) {
		IMP_LOG_ERR(TAG, "interface->releaseResult failed ret=%d\n", ret);
			return -1;
		}
	}
	if(interface->exit < 0) {
		IMP_LOG_ERR(TAG, "interface->init failed, ret=%d\n", ret);
		return -1;
	}
	free(g_sub_nv12_buf_move);
	/* Step.6 ivs move stop */
	ret = sample_ivs_move_stop(2, interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_stop(0) failed\n");
		return -1;
	}
	/* Step.7 Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}
	/* Step.8 FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}
	/* Step.9 System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}
	return 0;
}
