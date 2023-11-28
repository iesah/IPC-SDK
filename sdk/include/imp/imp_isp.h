/*
 * IMP ISP header file.
 *
 * Copyright (C) 2022 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_ISP_H__
#define __IMP_ISP_H__

#include <stdbool.h>
#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * ISP模块头文件
 */

/**
 * @defgroup IMP_ISP
 * @ingroup imp
 * @brief 图像信号处理单元。主要包含图像效果设置、模式切换以及Sensor的注册添加删除等操作
 *
 * ISP模块与数据流无关，不需要进行Bind，仅作用于效果参数设置及Sensor控制。
 *
 * ISP模块的使能步骤如下：
 * @code
 * int32_t ret = 0;
 * ret = IMP_ISP_Open(); // step.1 创建ISP模块
 * if(ret < 0){
 *     printf("Failed to ISPInit\n");
 *     return -1;
 * }
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * sensor.cbus_type = SENSOR_CONTROL_INTERFACE_I2C; // OR SENSOR_CONTROL_INTERFACE_SPI
 * sensor.i2c = {
 * 	.type = "xxx", // I2C设备名字，必须和sensor驱动中struct i2c_device_id中的name一致。
 *	.addr = xx,	//I2C地址
 *	.i2c_adapter_id = xx, //sensor所在的I2C控制器ID
 * }
 * OR
 * sensor.spi = {
 *	.modalias = "xx", //SPI设备名字，必须和sensor驱动中struct spi_device_id中的name一致。
 *	.bus_num = xx, //SPI总线地址
 * }
 * ret = IMP_ISP_AddSensor(&sensor); //step.2 添加一个sensor，在此操作之前sensor驱动已经添加到内核。
 * if (ret < 0) {
 *     printf("Failed to Register sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableSensor(void); //step.3 使能sensor，现在sensor开始输出图像。
 * if (ret < 0) {
 *     printf("Failed to EnableSensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_EnableTuning(); //step.4 使能ISP tuning, 然后才能调用ISP调试接口。
 * if (ret < 0) {
 *     printf("Failed to EnableTuning\n");
 *     return -1;
 * }
 *
 * 调试接口请参考ISP调试接口文档。 //step.5 效果调试。
 *
 * @endcode
 * ISP模块的卸载步骤如下：
 * @code
 * int32_t ret = 0;
 * IMPSensorInfo sensor;
 * sensor.name = "xxx";
 * ret = IMP_ISP_DisableTuning(); //step.1 关闭ISP tuning
 * if (ret < 0) {
 *     printf("Failed to disable tuning\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DisableSensor(); //step.2 关闭sensor，现在sensor停止输出图像；在此操作前FrameSource必须全部关闭。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_DelSensor(&sensor); //step.3 删除sensor，在此操作前sensor必须关闭。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 *
 * ret = IMP_ISP_Close(); //step.4 清理ISP模块，在此操作前所有sensor都必须被删除。
 * if (ret < 0) {
 *     printf("Failed to disable sensor\n");
 *     return -1;
 * }
 * @endcode
 * 更多使用方法请参考Samples
 * @{
 */

#define GPIO_PA(n) (0 * 32 + (n))
#define GPIO_PB(n) (1 * 32 + (n))
#define GPIO_PC(n) (2 * 32 + (n))
#define GPIO_PD(n) (3 * 32 + (n))

/**
 * ISP功能开关
 */
typedef enum {
	IMPISP_TUNING_OPS_MODE_DISABLE,			/**< 不使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_ENABLE,			/**< 使能该模块功能 */
	IMPISP_TUNING_OPS_MODE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsMode;

/**
 * ISP功能模式
 */
typedef enum {
	IMPISP_TUNING_OPS_TYPE_AUTO,			/**< 该模块的操作为自动模式 */
	IMPISP_TUNING_OPS_TYPE_MANUAL,			/**< 该模块的操作为手动模式 */
	IMPISP_TUNING_OPS_TYPE_BUTT,			/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPTuningOpsType;

/**
* 摄像头标号
*/
typedef enum {
	IMPVI_MAIN = 0,            /**< 主摄像头 */
	IMPVI_SEC = 1,             /**< 次摄像头(暂不支持) */
	IMPVI_THR = 2,             /**< 第三摄像头(暂不支持) */
	IMPVI_BUTT,                /**< 用于判断参数有效性的值，必须小于此值 */
} IMPVI_NUM;

/**
* 摄像头控制总线类型枚举
*/
typedef enum {
	TX_SENSOR_CONTROL_INTERFACE_I2C = 1,	/**< I2C控制总线 */
	TX_SENSOR_CONTROL_INTERFACE_SPI,	/**< SPI控制总线 */
} IMPSensorControlBusType;

/**
* 摄像头控制总线类型是I2C时，需要配置的参数结构体
*/
typedef struct {
	char type[20];		/**< I2C设备名字，必须与摄像头驱动中struct i2c_device_id中name变量一致 */
	int32_t addr;		/**< I2C地址 */
	int32_t i2c_adapter_id;	/**< I2C控制器 */
} IMPI2CInfo;

/**
* 摄像头控制总线类型是SPI时，需要配置的参数结构体
*/
typedef struct {
	char modalias[32];      /**< SPI设备名字，必须与摄像头驱动中struct spi_device_id中name变量一致 */
	int32_t bus_num;        /**< SPI总线地址 */
} IMPSPIInfo;

/**
* 摄像头输入数据接口枚举
*/
typedef enum {
	IMPISP_SENSOR_VI_MIPI_CSI0 = 0,	/**< MIPI CSI 接口 */
	IMPISP_SENSOR_VI_DVP = 2,	/**< DVP接口 */
	IMPISP_SENSOR_VI_BUTT = 3,	/**< 用于判断参数有效性的值，必须小于此值 */
} IMPSensorVinType;

/**
* 摄像头输入时钟源选择
*/
typedef enum {
	IMPISP_SENSOR_MCLK0 = 0,	/**< MCLK0时钟源 */
	IMPISP_SENSOR_MCLK1 = 1,	/**< MCLK1时钟源 */
	IMPISP_SENSOR_MCLK2 = 2,	/**< MCLK2时钟源 */
	IMPISP_SENSOR_MCLK_BUTT = 3,	/**< 用于判断参数有效性的值，必须小于此值 */
} IMPSensorMclk;

/**
* 摄像头注册信息结构体
*/
typedef struct {
	char name[32];				/**< 摄像头名字 */
	IMPSensorControlBusType cbus_type;      /**< 摄像头控制总线类型 */
	union {
		IMPI2CInfo i2c;			/**< I2C总线信息 */
		IMPSPIInfo spi;			/**< SPI总线信息 */
	};
	int rst_gpio;		                /**< 摄像头reset接口链接的GPIO */
	int pwdn_gpio;		                /**< 摄像头power down接口链接的GPIO */
	int power_gpio;		                /**< 摄像头power 接口链接的GPIO，注意：现在没有启用该参数 */
	unsigned short sensor_id;               /**< 摄像头ID号 */
	IMPSensorVinType video_interface;	/**< 摄像头数据输入接口 */
	IMPSensorMclk mclk;			/**< 摄像头Mclk时钟源 */
	int default_boot;			/**< 摄像头默认启动setting */
} IMPSensorInfo;

/**
 * @fn int32_t IMP_ISP_Open(void)
 *
 * 打开ISP模块
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 创建ISP模块，准备向ISP添加sensor，并开启ISP效果调试功能。
 *
 * @attention 这个函数必须在添加sensor之前被调用。
 */
int32_t IMP_ISP_Open(void);

/**
 * @fn int32_t IMP_ISP_Close(void)
 *
 * 关闭ISP模块
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark ISP模块，ISP模块不再工作。
 *
 * @attention 在使用这个函数之前，必须保证所有FrameSource和效果调试功能已经关闭，所有sensor都被关闭删除.
 */
int32_t IMP_ISP_Close(void);

/**
 * @fn int32_t IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 添加一个sensor，用于向ISP模块提供数据源
 *
 * @param[in] num   需要添加sensor的标号
 * @param[in] pinfo 需要添加sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 添加一个摄像头，用于提供图像。
 *
 * @attention 在使用这个函数之前，必须保证摄像头驱动已经注册进内核.
 */
int32_t IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo);

/**
 * @fn int32_t IMP_ISP_DelSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 删除一个sensor
 *
 * @param[in] num   需要删除sensor的标号
 * @param[in] pinfo 需要删除sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 删除一个摄像头。
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经停止工作，即调用了IMP_ISP_DisableSensor函数.
 * @attention 多摄系统中，必须按照IMPVI_NUM 0-1-2顺序全部添加完成才能继续调用其他API接口，
 */
int32_t IMP_ISP_DelSensor(IMPVI_NUM num, IMPSensorInfo *pinfo);

/**
 * @fn int32_t IMP_ISP_EnableSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)
 *
 * 使能一个sensor
 *
 * @param[in] num   需要使能的sensor的标号
 * @param[in] pinfo 需要使能的sensor的信息指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 使能一个摄像头。
 *
 * @attention 在使用这个函数之前，必须保证此摄像头已经被添加，即调用了IMP_ISP_AddSensor函数.
 */
int32_t IMP_ISP_EnableSensor(IMPVI_NUM num, IMPSensorInfo *info);

/**
 * @fn int32_t IMP_ISP_DisableSensor(IMPVI_NUM num)
 *
 * 关闭一个sensor
 *
 * @param[in] num 需要关闭的sensor的标号
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 关闭一个摄像头，使之停止传输图像, 这样FrameSource无法输出图像，同时ISP也不能进行效果调试。
 *
 * @attention 在使用这个函数之前，必须保证所有FrameSource都已停止输出图像，同时效果调试也在不使能态.
 */
int32_t IMP_ISP_DisableSensor(IMPVI_NUM num);

/**
 * @fn int32_t IMP_ISP_EnableTuning(void)
 *
 * 使能ISP效果调试功能
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor被执行且返回成功.
 */
int32_t IMP_ISP_EnableTuning(void);

/**
 * @fn int32_t IMP_ISP_DisableTuning(void)
 *
 * 不使能ISP效果调试功能
 *
 * @param 无
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，必须保证在不使能sensor之前，先不使能ISP效果调试（即调用此函数）.
 */
int32_t IMP_ISP_DisableTuning(void);

/**
 * @fn int32_t IMP_ISP_SetDefaultBinPath(IMPVI_NUM num, char *path)
 *
 * 设置ISP bin文件默认路径
 *
 * @param[in] num   需要添加sensor的标号
 * @param[in] path  需要设置的bin文件绝对路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 设置启动时的图像效果bin文件绝对路径。
 *
 * @code
 * int ret = 0;
 * char path = "/etc/sensor/xxx-t41.bin";
 *
 * ret = IMP_ISP_SetDefaultBinPath(IMPVI_MAIN, &path);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_SetDefaultBinPath error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 这个函数必须在添加sensor之前被调用。
 */
int32_t IMP_ISP_SetDefaultBinPath(IMPVI_NUM num, char *path);

/**
 * @fn int32_t IMP_ISP_GetDefaultBinPath(IMPVI_NUM num, char *path)
 *
 * 获取ISP bin文件默认路径
 *
 * @param[in] num    需要添加sensor的标号
 * @param[in] path   需要设置的bin文件绝对路径
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 获取启动时的图像效果bin文件绝对路径。
 *
 * @code
 * int ret = 0;
 * char path[64];
 *
 * ret = IMP_ISP_GetDefaultBinPath(IMPVI_MAIN, &path);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_GetDefaultBinPath error !\n");
 * 	return -1;
 * }
 * printf("default bin path:%s\n", path);
 * @endcode
 *
 * @attention 这个函数必须在添加sensor之后被调用。
 */
int32_t IMP_ISP_GetDefaultBinPath(IMPVI_NUM num, char *path);

/**
 * @fn int32_t IMP_ISP_SetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable)
 *
 * ISP模块是否bypass
 *
 * @param[in] num       对应sensor的标号
 * @param[in] enable    是否bypass输出模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 如果此sensor需要bypass，需要在AddSensor之前就调用此接口。
 *
 * @code
 * int ret = 0;
 * IMPISPTuningOpsMode enable;
 *
 * enable = IMPISP_TUNING_OPS_MODE_ENABLE;
 * ret = IMP_ISP_SetISPBypass(IMPVI_MAIN, &enable);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_SetISPBypass error !\n");
 * 	return -1;
 * }
 * @endcode
 */
int32_t IMP_ISP_SetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable);

/**
 * @fn int32_t IMP_ISP_GetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable)
 *
 * ISP模块是否bypass
 *
 * @param[in] num       对应sensor的标号
 * @param[out] enable    是否bypass输出模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 需要在AddSensor之后调用此接口。
 *
 * @code
 * int ret = 0;
 * IMPISPTuningOpsMode enable;
 *
 * ret = IMP_ISP_GetISPBypass(IMPVI_MAIN, &enable);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_GetISPBypass error !\n");
 * 	return -1;
 * }
 * printf("isp bypass:%d\n", enable);
 * @endcode
 */
int32_t IMP_ISP_GetISPBypass(IMPVI_NUM num, IMPISPTuningOpsMode *enable);

/**
 * @fn int32_t IMP_ISP_WDR_ENABLE(IMPVI_NUM num, IMPISPTuningOpsMode *mode)
 *
 * 开关ISP WDR.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mode  ISP WDR 模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPTuningOpsMode mode;
 *
 * mode = IMPISP_TUNING_OPS_MODE_ENABLE;
 * ret = IMP_ISP_WDR_ENABLE(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_WDR_ENABLE error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 此函数第一次调用必须在添加sensor之前，即先调用此函数，然后调用IMP_ISP_AddSensor逐个添加sensor
 */
int32_t IMP_ISP_WDR_ENABLE(IMPVI_NUM num, IMPISPTuningOpsMode *mode);

/**
 * @fn IMP_ISP_WDR_ENABLE_GET(IMPVI_NUM num, IMPISPTuningOpsMode *mode)
 *
 * 获取ISP WDR 模式.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mode ISP WDR 模式
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPTuningOpsMode mode;
 *
 * ret = IMP_ISP_WDR_ENABLE_GET(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_WDR_ENABLE_GET error !\n");
 * 	return -1;
 * }
 * printf("wdr enable:%d\n", mode);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_WDR_ENABLE_GET(IMPVI_NUM num, IMPISPTuningOpsMode *mode);

/**
 * Sensor寄存器
 */
typedef struct {
        uint32_t addr;   /**< 寄存器地址 */
        uint32_t value;  /**< 寄存器值 */
} IMPISPSensorRegister;

/**
 * @fn int32_t IMP_ISP_SetSensorRegister(IMPVI_NUM num, IMPISPSensorRegister *reg)
 *
 * 设置sensor一个寄存器的值
 *
 * @param[in] num   对应sensor的标号
 * @param[in] reg   寄存器属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接设置一个sensor寄存器的值。
 *
 * @code
 * int ret = 0;
 * IMPISPSensorRegister reg;
 *
 * reg.addr = 0x00;
 * reg.value = 0x00;
 * ret = IMP_ISP_SetSensorRegister(IMPVI_MAIN, *reg);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_SetSensorRegister error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int32_t IMP_ISP_SetSensorRegister(IMPVI_NUM num, IMPISPSensorRegister *reg);

/**
 * @fn int32_t IMP_ISP_GetSensorRegister(IMPVI_NUM num, IMPISPSensorRegister *reg)
 *
 * 获取sensor一个寄存器的值
 *
 * @param[in] num   对应sensor的标号
 * @param[in] reg   寄存器属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 可以直接获取一个sensor寄存器的值。
 *
 * @code
 * int ret = 0;
 * IMPISPSensorRegister reg;
 *
 * ret = IMP_ISP_GetSensorRegister(IMPVI_MAIN, *reg);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_GetSensorRegister error !\n");
 * 	return -1;
 * }
 * printf("addr:0x%x, value:0x%x\n", reg.addr, reg.value);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证摄像头已经被使能.
 */
int32_t IMP_ISP_GetSensorRegister(IMPVI_NUM num, IMPISPSensorRegister *reg);

/**
 * Sensor属性参数
 */
typedef struct {
	uint32_t hts;       /**< sensor hts */
	uint32_t vts;       /**< sensor vts */
	uint32_t fps;       /**< sensor 帧率 */
	uint32_t width;     /**< sensor输出宽度 */
	uint32_t height;    /**< sensor输出的高度 */
} IMPISPSENSORAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_GetSensorAttr(IMPVI_NUM num, IMPISPSENSORAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr sensor属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPSENSORAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetSensorAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetSensorAttr error !\n");
 * 	return -1;
 * }
 * printf("hts:%d, vts:%d, fps:%d/%d, width:%d, height:%d\n",
 * attr.hts, attr.vts, attr.fps >> 16, attr.fps & 0xffff, attr.width, attr.height);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetSensorAttr(IMPVI_NUM num, IMPISPSENSORAttr *attr);

/**
 * Sensor帧率
 */
typedef struct {
        uint32_t num;  /**< 帧率的分子参数 */
        uint32_t den;  /**< 帧率的分母参数 */
} IMPISPSensorFps;

/**
 * @fn int32_t IMP_ISP_Tuning_SetSensorFPS(IMPVI_NUM num, IMPISPSensorFps *fps)
 *
 * 设置摄像头输出帧率
 *
 * @param[in] num       对应sensor的标号
 * @param[in] fps       帧率属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPSensorFps fps;
 *
 * fps.num = 15;
 * fps.den = 1;
 * ret = IMP_ISP_Tuning_SetSensorFPS(IMPVI_MAIN, &fps);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetSensorFPS error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetSensorFPS(IMPVI_NUM num, IMPISPSensorFps *fps);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSensorFPS(IMPVI_NUM num, IMPISPSensorFps *fps)
 *
 * 获取摄像头输出帧率
 *
 * @param[in] num       对应sensor的标号
 * @param[in] fps       帧率属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPSensorFps fps;
 *
 * ret = IMP_ISP_Tuning_GetSensorFPS(IMPVI_MAIN, &fps);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetSensorFPS error !\n");
 * 	return -1;
 * }
 * printf("fps:%f\n", (float)fps.num/fps.den);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证IMP_ISP_EnableSensor 和 IMP_ISP_EnableTuning已被调用。
 * @attention 在使能帧通道开始传输数据之前必须先调用此函数获取摄像头默认帧率。
 */
int32_t IMP_ISP_Tuning_GetSensorFPS(IMPVI_NUM num, IMPISPSensorFps *fps);

/**
 * @fn int32_t IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void))
 *
 * 设置视频丢失功能。当出现sensor与主板的连接线路出现问题时，设置的回调函数会被执行。
 *
 * @param[in] cb 回调函数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetVideoDrop(void (*cb)(void));

/**
 * ISP Wait Frame irq 参数。
 */

typedef enum {
	IMPISP_IRQ_FD = 0,	/**< 帧结束 */
	IMPISP_IRQ_FS = 1,	/**< 帧开始 */
} IMPISPIrqType;

/**
 * ISP Wait Frame 参数。
 */
typedef struct {
	uint32_t timeout;		/**< 超时时间，单位ms */
	uint64_t cnt;			/**< Frame统计(该参数设置无效，只能获取) */
	IMPISPIrqType irqtype;          /**< Frame中断类型(该参数设置无效，只能获取)*/
} IMPISPWaitFrameAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_WaitFrameDone(IMPVI_NUM num, IMPISPWaitFrameAttr *attr)
 *
 * 等待帧结束
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr 等待帧结束属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWaitFrameAttr attr;
 *
 * attr.timeout = 100;
 * ret = IMP_ISP_Tuning_WaitFrameDone(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_WaitFrameDone error !\n");
 * 	return -1;
 * }
 * printf("this total number of frames: %d, irq type:%s\n",
 * attr->cnt, attr->irqtype ? "frame start" : "frame done");
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_WaitFrameDone(IMPVI_NUM num, IMPISPWaitFrameAttr *attr);

/**
 * ISP抗闪频功能模式结构体。
 */
typedef enum {
	IMPISP_ANTIFLICKER_DISABLE_MODE,	/**< 不使能ISP抗闪频功能 */
	IMPISP_ANTIFLICKER_NORMAL_MODE,         /**< 使能ISP抗闪频功能的正常模式，即曝光最小值为第一个step，不能达到sensor的最小值 */
	IMPISP_ANTIFLICKER_AUTO_MODE,           /**< 使能ISP抗闪频功能的自动模式，最小曝光可以达到sensor曝光的最小值 */
	IMPISP_ANTIFLICKER_BUTT,                /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPAntiflickerMode;

/**
 * ISP抗闪频属性参数结构体。
 */
typedef struct {
	IMPISPAntiflickerMode mode;             /**< ISP抗闪频功能模式选择 */
	uint8_t freq;                           /**< 设置抗闪的工频 */
} IMPISPAntiflickerAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr)
 *
 * 设置ISP抗闪频属性
 *
 * @param[in] num   对应sensor的标号
 * @param[in] pattr 设置参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAntiflickerAttr attr;
 *
 * attr.mode = IMPISP_ANTIFLICKER_NORMAL_MODE;
 * attr.freq = 50;
 * ret = IMP_ISP_Tuning_SetAntiFlickerAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAntiFlickerAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr)
 *
 * 获得ISP抗闪频属性
 *
 * @param[in] num   对应sensor的标号
 * @param[out] pattr 获取参数值指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAntiflickerAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetAntiFlickerAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAntiFlickerAttr error !\n");
 * 	return -1;
 * }
 * printf("antiflicker mode:%d, antiflicker freq:%d\n", attr.mode, attr.freq);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetAntiFlickerAttr(IMPVI_NUM num, IMPISPAntiflickerAttr *pattr);

/**
 * HV Flip 模式
 */
typedef enum {
	IMPISP_FLIP_NORMAL_MODE = 0,	/**< 正常模式 */
	IMPISP_FLIP_H_MODE,				/**< 镜像模式 */
	IMPISP_FLIP_V_MODE,				/**< 翻转模式 */
	IMPISP_FLIP_HV_MODE,			/**< 镜像翻转模式 */
	IMPISP_FLIP_MODE_BUTT,          /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPHVFLIP;

/**
 * HV Flip属性
 */
typedef struct {
	IMPISPHVFLIP sensor_mode;   /**< sensor对应的flip模式 */
	IMPISPHVFLIP isp_mode[3];	/**< ISP每个通道对应的flip模式 */
} IMPISPHVFLIPAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetHVFLIP(IMPVI_NUM num, IMPISPHVFLIPAttr *attr)
 *
 * 设置HV Flip的模式.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr		flip属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPHVFLIPAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetHVFLIP(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetHVFLIP error !\n");
 * 	return -1;
 * }
 * attr.sensor_mode = IMPISP_FLIP_H_MODE;
 * attr.isp_mode[0] = IMPISP_FLIP_V_MODE;
 * ret = IMP_ISP_Tuning_SetHVFLIP(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetHVFLIP error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @remark 使用IVDC直通模块时，通道0没有ISP Vflip功能.
 * @remark sensor HVFLip功能与ISP HVFlip为前后级关系，禁止Sensor和ISP同时镜像或同时翻转
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetHVFLIP(IMPVI_NUM num, IMPISPHVFLIPAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetHVFLIP(IMPVI_NUM num, IMPISPHVFLIPAttr *attr)
 *
 * 获取HV Flip的模式.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] attr		flip属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPHVFLIPAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetHVFLIP(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetHVFLIP error !\n");
 * 	return -1;
 * }
 * printf("hvflip mode:...\n", ...);
 # @endcode
 *
 * @remark 使用IVDC直通模块时，通道0没有ISP Vflip功能.
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetHVFLIP(IMPVI_NUM num, IMPISPHVFLIPAttr *attr);

/**
 * ISP 工作模式配置，正常模式或夜视模式。
 */
typedef enum {
	IMPISP_RUNNING_MODE_DAY = 0,        /**< 正常模式 */
	IMPISP_RUNNING_MODE_NIGHT = 1,      /**< 夜视模式 */
	IMPISP_RUNNING_MODE_CUSTOM = 2,     /**< 定制模式 */
	IMPISP_RUNNING_MODE_BUTT,           /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPRunningMode;

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *mode)
 *
 * 设置ISP工作模式，正常模式或夜视模式或者定制；默认为正常模式。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mode  运行模式参数
 *
 * @remark ISP工作模式，如果选用定制模式需要添加特殊的bin文件，此模式可用于星光夜视的专门调节
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPRunningMode mode;
 *
 * if( it is during a night now){
 *  	mode = IMPISP_RUNNING_MODE_NIGHT
 * }else{
 * 	mode = IMPISP_RUNNING_MODE_DAY;
 * }
 * ret = IMP_ISP_Tuning_SetISPRunningMode(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPRunningMode error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *mode);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *pmode)
 *
 * 获取ISP工作模式。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] pmode 操作参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPRunningMode mode;
 *
 * mode = IMPISP_RUNNING_MODE_DAY;
 * ret = IMP_ISP_Tuning_GetISPRunningMode(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetISPRunningMode error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetISPRunningMode(IMPVI_NUM num, IMPISPRunningMode *pmode);

/**
 * @fn int32_t IMP_ISP_Tuning_SetBrightness(IMPVI_NUM num, unsigned char *bright)
 *
 * 设置ISP 综合效果图片亮度
 *
 * @param[in] num       对应sensor的标号
 * @param[in] bright    图片亮度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @code
 * int ret = 0;
 * unsigned char bright = 128;
 *
 * ret = IMP_ISP_Tuning_SetBrightness(IMPVI_MAIN, &bright);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetBrightness error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetBrightness(IMPVI_NUM num, unsigned char *bright);

/**
 * @fn int32_t IMP_ISP_Tuning_GetBrightness(IMPVI_NUM num, unsigned char *pbright)
 *
 * 获取ISP 综合效果图片亮度
 *
 * @param[in] num         对应sensor的标号
 * @param[out] pbright    图片亮度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加亮度，小于128降低亮度。
 *
 * @code
 * int ret = 0;
 * unsigned char bright;
 *
 * ret = IMP_ISP_Tuning_GetBrightness(IMPVI_MAIN, &bright);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetBrightness error !\n");
 * 	return -1;
 * }
 * printf("brightness:%d\n", bright);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetBrightness(IMPVI_NUM num, unsigned char *pbright);

/**
 * @fn int32_t IMP_ISP_Tuning_SetContrast(IMPVI_NUM num, unsigned char *contrast)
 *
 * 设置ISP 综合效果图片对比度
 *
 * @param[in] num 对应sensor的标号
 * @param[in] contrast 图片对比度参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @code
 * int ret = 0;
 * unsigned char contrast = 128;
 *
 * ret = IMP_ISP_Tuning_SetContrast(IMPVI_MAIN, &contrast);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, " error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetContrast(IMPVI_NUM num, unsigned char *contrast);

/**
 * @fn int32_t IMP_ISP_Tuning_GetContrast(IMPVI_NUM num, unsigned char *pcontrast)
 *
 * 获取ISP 综合效果图片对比度
 *
 * @param[in] num        对应sensor的标号
 * @param[out] pcontrast 图片对比度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加对比度，小于128降低对比度。
 *
 * @code
 * int ret = 0;
 * unsigned char contrast;
 *
 * ret = IMP_ISP_Tuning_GetContrast(IMPVI_MAIN, &contrast);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetContrast error !\n");
 * 	return -1;
 * }
 * printf("contrast:%d\n", contrast);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetContrast(IMPVI_NUM num, unsigned char *pcontrast);

 /**
  * @fn int32_t IMP_ISP_Tuning_SetSharpness(IMPVI_NUM num, unsigned char *sharpness)
 *
 * 设置ISP 综合效果图片锐度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] sharpness     图片锐度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @code
 * int ret = 0;
 * unsigned char sharpness = 128;
 *
 * IMP_ISP_Tuning_SetSharpness(IMPVI_MAIN, &sharpness);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetSharpness error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetSharpness(IMPVI_NUM num, unsigned char *sharpness);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSharpness(IMPVI_NUM num, unsigned char *psharpness)
 *
 * 获取ISP 综合效果图片锐度
 *
 * @param[in] num         对应sensor的标号
 * @param[out] psharpness 图片锐度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加锐度，小于128降低锐度。
 *
 * @code
 * int ret = 0;
 * unsigned char sharpness;
 *
 * IMP_ISP_Tuning_GetSharpness(IMPVI_MAIN, &sharpness);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetSharpness error !\n");
 * 	return -1;
 * }
 * printf("sharpness:%d\n", sharpness);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetSharpness(IMPVI_NUM num, unsigned char *psharpness);

/**
 * @fn int32_t IMP_ISP_Tuning_SetSaturation(IMPVI_NUM num, unsigned char *saturation)
 *
 * 设置ISP 综合效果图片饱和度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] saturation    图片饱和度参数值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @code
 * int ret = 0;
 * unsigned char saturation = 128;
 *
 * ret = IMP_ISP_Tuning_SetSaturation(IMPVI_MAIN, &saturation);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetSaturation error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetSaturation(IMPVI_NUM num, unsigned char *saturation);

/**
 * @fn int32_t IMP_ISP_Tuning_GetSaturation(IMPVI_NUM num, unsigned char *psaturation)
 *
 * 获取ISP 综合效果图片饱和度
 *
 * @param[in] num           对应sensor的标号
 * @param[in] saturation    图片饱和度参数指针
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128增加饱和度，小于128降低饱和度。
 *
 * @code
 * int ret = 0;
 * unsigned char saturation;
 *
 * ret = IMP_ISP_Tuning_GetSaturation(IMPVI_MAIN, &saturation);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetSaturation error !\n");
 * 	return -1;
 * }
 * printf("saturation:%d\n", saturation);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetSaturation(IMPVI_NUM num, unsigned char *psaturation);

/**
 * @fn int32_t IMP_ISP_Tuning_SetBcshHue(IMPVI_NUM num, unsigned char *hue)
 *
 * 设置图像的色调
 *
 * @param[in] num 对应sensor的标号
 * @param[in] hue 图像的色调参考值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128正向调节色调，小于128反向调节色调，调节范围0~255。
 *
 * @code
 * int ret = 0;
 * unsigned char hue = 128;
 *
 * ret = IMP_ISP_Tuning_SetBcshHue(IMPVI_MAIN, &hue);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetBcshHue error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_SetBcshHue(IMPVI_NUM num, unsigned char *hue);

/**
 * @fn int32_t IMP_ISP_Tuning_GetBcshHue(IMPVI_NUM num, unsigned char *hue)
 *
 * 获取图像的色调值。
 *
 * @param[in] num   对应sensor的标号
 * @param[out] hue  图像的色调参数指针。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 默认值为128，大于128代表正向调节色调，小于128代表反向调节色调，范围0~255。
 *
 * @code
 * int ret = 0;
 * unsigned char hue;
 *
 * ret = IMP_ISP_Tuning_GetBcshHue(IMPVI_MAIN, &hue);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetBcshHue error !\n");
 * 	return -1;
 * }
 * printf("bcsh hue:%d\n", hue);
 * @endcode
 *
 * @attention 在使用这个函数之前，必须保证ISP效果调试功能已使能.
 */
int32_t IMP_ISP_Tuning_GetBcshHue(IMPVI_NUM num, unsigned char *hue);

/**
 * ISP各个模块旁路开关
 */
typedef union {
	uint32_t key;                          /**< 各个模块旁路开关 */
	struct {
		uint32_t bitBypassBLC : 1;      /**< [0] */
		uint32_t bitBypassLSC : 1;      /**< [1] */
		uint32_t bitBypassAWB0 : 1;     /**< [2] */
		uint32_t bitBypassWDR : 1;      /**< [3] */
		uint32_t bitBypassDPC : 1;      /**< [4] */
		uint32_t bitBypassGIB : 1;      /**< [5] */
		uint32_t bitBypassAWB1 : 1;     /**< [6] */
		uint32_t bitBypassADR : 1;      /**< [7] */
		uint32_t bitBypassDMSC : 1;     /**< [8] */
		uint32_t bitBypassCCM : 1;      /**< [9] */
		uint32_t bitBypassGAMMA : 1;    /**< [10] */
		uint32_t bitBypassDEFOG : 1;    /**< [11] */
		uint32_t bitBypassCSC : 1;      /**< [12] */
		uint32_t bitBypassMDNS : 1;     /**< [13] */
		uint32_t bitBypassYDNS : 1;     /**< [14] */
		uint32_t bitBypassBCSH : 1;     /**< [15] */
		uint32_t bitBypassCLM : 1;      /**< [16] */
		uint32_t bitBypassYSP : 1;      /**< [17] */
		uint32_t bitBypassSDNS : 1;     /**< [18] */
		uint32_t bitBypassCDNS : 1;     /**< [19] */
		uint32_t bitBypassHLDC : 1;     /**< [20] */
		uint32_t bitBypassLCE : 1;      /**< [21] */
		uint32_t bitBypassTMO : 1;      /**< [22] */
		uint32_t bitBypassIRDPC : 1;    /**< [23] */
		uint32_t bitBypassIRGAMMA : 1;  /**< [24] */
		uint32_t bitRsv : 7;            /**< [25 ~ 31] */
	};
} IMPISPModuleCtl;

/**
 * @fn int32_t IMP_ISP_Tuning_SetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule)
 *
 * 设置ISP各个模块bypass功能
 *
 * @param[in] num           对应sensor的标号
 * @param[in] ispmodule     ISP各个模块bypass功能.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPModuleCtl ctl;
 *
 * ret = IMP_ISP_Tuning_GetModuleControl(IMPVI_MAIN, &ctl);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetModuleControl error !\n");
 * 	return -1;
 * }
 *
 * ctl.bitBypassBLC = 1;
 * //...
 *
 * ret = IMP_ISP_Tuning_SetModuleControl(IMPVI_MAIN, &ctl);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetModuleControl error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule);

/**
 * @fn int32_t IMP_ISP_Tuning_GetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule)
 *
 * 获取ISP各个模块bypass功能.
 *
 * @param[in] num           对应sensor的标号
 * @param[out] ispmodule    ISP各个模块bypass功能
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPModuleCtl ctl;
 *
 * ret = IMP_ISP_Tuning_GetModuleControl(IMPVI_MAIN, &ctl);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetModuleControl error !\n");
 * 	return -1;
 * }
 * printf("module conctrol:0x%x\n", ctl.key);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetModuleControl(IMPVI_NUM num, IMPISPModuleCtl *ispmodule);

/**
 * ISP 模块强度配置数组的下标
 */
typedef enum {
	IMP_ISP_MODULE_SINTER = 0, /**< 2D降噪下标 */
	IMP_ISP_MODULE_TEMPER,     /**< 3D降噪下标 */
	IMP_ISP_MODULE_DRC,        /**< 数字宽动态下标 */
	IMP_ISP_MODULE_DPC,        /**< 动态去坏点下标 (ps：默认强度128，强度越小，坏点越明显；强度越大，去坏点能力越好)*/
	IMP_ISP_MODULE_DEFOG,	   /**< 去雾模块的强度下标 */
	IMP_ISP_MODULE_BUTT,       /**< 用于判断参数有效性的值，必须大于此值 */
} IMPISPModuleRatioArrayList;

/**
 * ISP 模块强度配置单元
 */
typedef struct {
	IMPISPTuningOpsMode en;     /**< 模块强度配置功能使能 */
	uint8_t ratio;              /**< 模块强度配置功能强度，128为默认强度，大于128增加强度，小于128降低强度 */
} IMPISPRatioUnit;

/**
 * ISP 模块强度配置
 */
typedef struct {
	IMPISPRatioUnit ratio_attr[16];  /**< 各个模块强度配置功能属性 */
} IMPISPModuleRatioAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio)
 *
 * 设置各个模块的强度。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] ratio 各个模块的强度配置功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPModuleRatioAttr ratio;
 *
 * ret = IMP_ISP_Tuning_GetModule_Ratio(IMPVI_MAIN, &ratio);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetModule_Ratio error !\n");
 * 	return -1;
 * }
 *
 * ratio.ratio_attr[IMP_ISP_MODULE_SINTER].en = IMPISP_TUNING_OPS_MODE_ENABLE;
 * ratio.ratio_attr[IMP_ISP_MODULE_SINTER].ratio = 128;
 * //...
 *
 * ret = IMP_ISP_Tuning_SetModule_Ratio(IMPVI_MAIN, &ratio);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetModule_Ratio error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio);

/**
 * @fn int32_t IMP_ISP_Tuning_GetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio)
 *
 * 获取各个模块的强度。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] ratio        各个模块的强度配置功能属性.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPModuleRatioAttr ratio;
 *
 * ret = IMP_ISP_Tuning_GetModule_Ratio(IMPVI_MAIN, &ratio);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetModule_Ratio error !\n");
 * 	return -1;
 * }
 * printf("sinter en:%d, sinter ratio:%d\n", ratio.ratio_attr[IMP_ISP_MODULE_SINTER].en,
 * ratio.ratio_attr[IMP_ISP_MODULE_SINTER].ratio);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetModule_Ratio(IMPVI_NUM num, IMPISPModuleRatioAttr *ratio);

/**
 * ISP CSC转换矩阵标准与模式结构体
 */
typedef enum {
	IMP_ISP_CG_BT601_FULL,          /**< BT601 full range */
	IMP_ISP_CG_BT601_LIMITED,       /**< BT601 非full range */
	IMP_ISP_CG_BT709_FULL,          /**< BT709 full range */
	IMP_ISP_CG_BT709_LIMITED,       /**< BT709 非full range */
	IMP_ISP_CG_USER,                /**< 用户自定义模式 */
	IMP_ISP_CG_BUTT,                /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPCSCColorGamut;

/**
 * ISP CSC转换矩阵结构体
 */
typedef struct {
	float CscCoef[9];               /**< 3x3矩阵 */
	unsigned char CscOffset[2];     /**< [0] UV偏移值 [1] Y偏移值*/
	unsigned char CscClip[4];       /**< 分别为Y最大值，Y最大值，UV最大值，UV最小值 */
} IMPISPCscMatrix;

/**
 * ISP CSC属性结构体
 */
typedef struct {
	IMPISPCSCColorGamut ColorGamut;     /**< RGB转YUV的标准矩阵 */
	IMPISPCscMatrix Matrix;             /**< 客户自定义的转换矩阵 */
} IMPISPCSCAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc)
 *
 * 设置CSC属性.
 *
 * @param[in] num 对应sensor的标号
 * @param[in] csc CSC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPCSCAttr csc;
 *
 * memset(&csc, 0, sizeof(csc));
 * csc.ColorGamut = mode;
 *
 * if (csc.ColorGamut == IMP_ISP_CG_USER) {
 *      float CscCoef[] = {...};
 *      unsigned char CscOffset[] = {...};
 *      unsigned char CscClip[] = {...};
 *
 *      memcpy(csc.CscCoef, CscCoef, sizeof(CscCoef));
 *      memcpy(csc.CscOffset, CscOffset, sizeof(CscOffset));
 *      memcpy(csc.CscClip, CscClip, sizeof(CscClip));
 * }
 * ret = IMP_ISP_Tuning_SetISPCSCAttr(IMPVI_MAIN, &csc);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetISPCSCAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc);

/**
 * @fn int32_t IMP_ISP_Tuning_GetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc)
 *
 * 获取CSC属性.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] csc      CSC属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPCSCAttr csc;
 *
 * memset(&csc, 0, sizeof(csc));
 *
 * ret = IMP_ISP_Tuning_GetISPCSCAttr(IMPVI_MAIN, &csc);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetISPCSCAttr error !\n");
 * 	return -1;
 * }
 * printf("csc color gamut:%d\n", csc.ColorGamut);
 * if (csc.ColorGamut == IMP_ISP_CG_USER) {
 *      printf("CscCoef:%f ..., CscOffset:%d ..., CscClip:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetISPCSCAttr(IMPVI_NUM num, IMPISPCSCAttr *csc);

/**
 * ISP 颜色矩阵属性
 */
typedef struct {
	IMPISPTuningOpsMode ManualEn;       /**< 手动CCM使能 */
	IMPISPTuningOpsMode SatEn;          /**< 手动模式下饱和度使能 */
	float ColorMatrix[9];               /**< 颜色矩阵 */
} IMPISPCCMAttr;
/**
 * @fn int32_t IMP_ISP_Tuning_SetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm)
 *
 * 设置CCM属性.
 *
 * @param[in] num 对应sensor的标号
 * @param[in] ccm CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPCCMAttr attr;
 * float ColorMatrix[9] = {...};
 *
 * attr.ManualEn = IMPISP_TUNING_OPS_MODE_ENABLE;
 * attr.SatEn = IMPISP_TUNING_OPS_MODE_ENABLE;
 * memcpy(attr.ColorMatrix, ColorMatrix, sizeof(ColorMatrix));
 *
 * ret = IMP_ISP_Tuning_SetCCMAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetCCMAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm);

/**
 * @fn int32_t IMP_ISP_Tuning_GetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm)
 *
 * 获取CCM属性.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] ccm      CCM属性参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPCCMAttr attr;
 *
 * memset(&attr, 0x0, sizeof(IMPISPCCMAttr));
 * ret = IMP_ISP_Tuning_GetCCMAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetCCMAttr error !\n");
 * 	return -1;
 * }
 * printf("manual_en:%d, sat_en:%d\n", attr.ManualEn, attr.SatEn);
 * printf("color matrix:...", ...);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetCCMAttr(IMPVI_NUM num, IMPISPCCMAttr *ccm);

/**
 * ISP Gamma模式枚举
 */
typedef enum {
	IMP_ISP_GAMMA_CURVE_DEFAULT,    /**< 默认gamma模式 */
	IMP_ISP_GAMMA_CURVE_SRGB,       /**< 标准SRGB gamma模式 */
	IMP_ISP_GAMMA_CURVE_REC709,		/**< REC709 gamma模式 */
	IMP_ISP_GAMMA_CURVE_HDR,        /**< HDR Gamma模式 */
	IMP_ISP_GAMMA_CURVE_USER,       /**< 用户自定义gamma模式 */
	IMP_ISP_GAMMA_CURVE_BUTT,       /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPGammaCurveType;

/**
 * gamma属性结构体
 */
typedef struct {
	IMPISPGammaCurveType Curve_type; /**< gamma模式 */
	uint16_t gamma[129];		/**< gamma参数数组，有129个点 */
} IMPISPGammaAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma)
 *
 * 设置GAMMA参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] gamma gamma参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPGammaAttr attr;
 * uint16_t gamma[129] = {...};
 *
 * attr.Curve_type = mode;
 * if (attr.Curve_type == IMP_ISP_GAMMA_CURVE_USER) {
 *      memcpy(attr.gamma, gamma, sizeof(gamma));
 * }
 * ret = IMP_ISP_Tuning_SetGammaAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetGammaAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma);

/**
 * @fn int32_t IMP_ISP_Tuning_GetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma)
 *
 * 获取GAMMA参数.
 *
 * @param[in] num       对应sensor的标号
 * @param[out] gamma    gamma参数
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPGammaAttr attr;
 *
 * memset(&attr, 0, sizeof(IMPISPGammaAttr));
 * ret = IMP_ISP_Tuning_GetGammaAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetGammaAttr error !\n");
 * 	return -1;
 * }
 * printf("curve type:%d\n", attr.Curve_type);
 * if (attr.Curve_type == IMP_ISP_GAMMA_CURVE_USER) {
 *      printf("gamma:...", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetGammaAttr(IMPVI_NUM num, IMPISPGammaAttr *gamma);

/**
 * 统计值直方图统计色域结构体
 */
typedef enum {
	IMP_ISP_HIST_ON_RAW,    /**< Raw域 */
	IMP_ISP_HIST_ON_YUV,    /**< YUV域 */
} IMPISPHistDomain;

/**
 * 统计范围结构体
 */
typedef struct {
	unsigned int start_h;   /**< 横向起始点，单位为pixel AF统计值横向起始点：[1 ~ width]，且取奇数 */
	unsigned int start_v;   /**< 纵向起始点，单位为pixel AF统计值垂直起始点 ：[3 ~ height]，且取奇数 */
	unsigned char node_h;   /**< 横向统计区域块数 [12 ~ 15]*/
	unsigned char node_v;   /**< 纵向统计区域块数 [12 ~ 15]*/
} IMPISP3AStatisLocation;

/**
 * AE统计值属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode ae_sta_en;  /**< AE统计功能开关*/
	IMPISP3AStatisLocation local;   /**< AE统计位置(预留) */
	IMPISPHistDomain hist_domain;   /**< AE统计色域(预留) */
	unsigned char histThresh[4];    /**< AE直方图的分段 */
} IMPISPAEStatisAttr;

/**
 * AWB统计值属性结构体
 */
typedef enum {
	IMP_ISP_AWB_ORIGIN,     /**< 原始统计值 */
	IMP_ISP_AWB_LIMITED,    /**< 加限制条件后的统计值 */
} IMPISPAWBStatisMode;

/**
 * AWB统计值属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode awb_sta_en;         /**< AWB统计功能开关*/
	IMPISP3AStatisLocation local;		/**< AWB统计范围 */
	IMPISPAWBStatisMode mode;		/**< AWB统计属性(预留) */
} IMPISPAWBStatisAttr;

/**
 * AF统计属性结构体
 */
typedef struct {
	IMPISPTuningOpsMode af_sta_en;      /**< AF统计功能开关*/
	IMPISP3AStatisLocation local;       /**< AF统计范围 */
	unsigned char af_metrics_shift;     /**< AF统计值缩小参数 默认是0，1代表缩小2倍*/
	unsigned short af_delta;            /**< AF统计低通滤波器的权重 [0 ~ 64]*/
	unsigned short af_theta;            /**< AF统计高通滤波器的权重 [0 ~ 64]*/
	unsigned short af_hilight_th;       /**< AF高亮点统计阈值 [0 ~ 255]*/
	unsigned short af_alpha_alt;        /**< AF统计低通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
	unsigned short af_belta_alt;        /**< AF统计低通滤波器的水平与垂直方向的权重 [0 ~ 64]*/
} IMPISPAFStatisAttr;

/**
 * 统计信息属性结构体
 */
typedef struct {
	IMPISPAEStatisAttr ae;      /**< AE 统计信息属性 */
	IMPISPAWBStatisAttr awb;    /**< AWB 统计信息属性 */
	IMPISPAFStatisAttr af;      /**< AF 统计信息属性 */
} IMPISPStatisConfig;

/**
 * @fn int32_t IMP_ISP_Tuning_SetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config)
 *
 * 设置统计信息参数.
 *
 * @param[in] num               对应sensor的标号
 * @param[in] statis_config    统计信息属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPStatisConfig config;
 * unsigned char histThresh[4] = {...};
 *
 * ret = IMP_ISP_Tuning_GetStatisConfig(IMPVI_MAIN, &config);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetStatisConfig error !\n");
 * 	return -1;
 * }
 *
 * config.ae.ae_sta_en = ae_mode;
 * if (config.ae.ae_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      memcpy(config.ae.histThresh, histThresh, sizeof(histThresh));
 * }
 *
 * config.awb.awb_sta_en = awb_mode;
 * if (config.awb.awb_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      config.awb.local.start_h = 1;
 *      config.awb.local.start_v = 1;
 *      config.awb.local.node_h = 15;
 *      config.awb.local.node_v = 15;
 * }
 *
 * config.af.af_sta_en = af_mode;
 * if (config.af.af_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      config.af.local.start_h = 1;
 *      config.af.local.start_v = 3;
 *      config.af.local.node_h = 15;
 *      config.af.local.node_v = 15;
 *      config.af.af_metrics_shift = 0;
 *      config.af.af_delta = 1;
 *      config.af.af_theta = 1;
 *      config.af.af_hilight_th = 1;
 *      config.af.af_alpha_alt = 1;
 *      config.af.af_belta_alt = 1;
 * }
 *
 * ret = IMP_ISP_Tuning_SetStatisConfig(IMPVI_MAIN, &config);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetStatisConfig error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config);

/**
 *  @fn int32_t IMP_ISP_Tuning_GetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config)
 *
 * 获取统计信息参数.
 *
 * @param[in] num               对应sensor的标号
 * @param[out] statis_config    统计信息属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPStatisConfig config;
 *
 * ret = IMP_ISP_Tuning_GetStatisConfig(IMPVI_MAIN, &config);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetStatisConfig error !\n");
 * 	return -1;
 * }
 *
 * printf("ae config mode:%s\n", config.ae.ae_sta_en ? "enable" : "disable");
 * if (config.ae.ae_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("ae config:...", ...);
 * }
 *
 * printf("awb config mode:%s\n", config.awb.awb_sta_en ? "enable" : "disable");
 * if (config.awb.awb_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("awb config:...", ...);
 * }
 *
 * printf("af config mode:%s\n", config.af.af_sta_en ? "enable" : "disable");
 * if (config.af.af_sta_en == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("af config:...", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetStatisConfig(IMPVI_NUM num, IMPISPStatisConfig *statis_config);

/**
 * 权重信息
 */
typedef struct {
	unsigned char weight[15][15];    /**< 各区域权重信息 [0 ~ 8]*/
} IMPISPWeight;

/**
 * AE权重信息
 */
typedef struct {
	IMPISPTuningOpsMode roi_enable;    /**< 感兴趣区域权重设置使能(预留) */
	IMPISPTuningOpsMode weight_enable; /**< 全局权重设置使能 */
	IMPISPWeight ae_roi;               /**< 感兴趣区域权重值(0 ~ 8)(预留) */
	IMPISPWeight ae_weight;            /**< 全局权重值(0~ 8) */
} IMPISPAEWeightAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight)
 *
 * 设置统计信息参数.
 *
 * @param[in] num                对应sensor的标号
 * @param[out] ae_weight         权重信息
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEWeightAttr attr;
 * unsigned char weight[15*15] = {...};
 *
 * attr.weight_enable = IMPISP_TUNING_OPS_MODE_ENABLE;
 * memcpy(attr.ae_weight.weight, weight, sizeof(weight));
 *
 * ret = IMP_ISP_Tuning_SetAeWeight(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeWeight error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight)
 *
 * 获取统计信息参数.
 *
 * @param[in] num                对应sensor的标号
 * @param[out] ae_weight         权重信息
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEWeightAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetAeWeight(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeWeight error !\n");
 * 	return -1;
 * }
 *
 * if (attr.roi_enable == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("ae roi weight:...\n", attr.ae_roi.weight[...][...]);
 * }
 * if (attr.weight_enable == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("ae global weight:...\n", attr.ae_weight.weight[...][...]);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAeWeight(IMPVI_NUM num, IMPISPAEWeightAttr *ae_weight);

/**
 * 各区域统计信息
 */
typedef struct {
	uint32_t statis[15][15];    /**< 各区域统计信息*/
}  __attribute__((packed, aligned(1))) IMPISPStatisZone;

/**
 * AE统计信息
 */
typedef struct {
	unsigned short ae_hist_5bin[5];         /**< AE统计直方图bin值 [0 ~ 65535]*/
	uint32_t ae_hist_256bin[256];           /**< AE统计直方图bin值, 为每个bin的实际pixel数量*/
	IMPISPStatisZone ae_statis;             /**< AE统计信息 */
}  __attribute__((packed, aligned(1))) IMPISPAEStatisInfo;

/**
 * @fn int32_t IMP_ISP_Tuning_GetAeStatistics(IMPVI_NUM num, IMPISPAEStatisInfo *ae_statis)
 *
 * 获取AE统计信息.
 *
 * @param[in] num                对应sensor的标号
 * @param[out] ae_statis         AE统计信息
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEStatisInfo info;
 *
 * ret = IMP_ISP_Tuning_GetAeStatistics(IMPVI_MAIN, &statis);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeStatistics error !\n");
 * 	return -1;
 * }
 *
 * printf("ae hist 5bin:...\n", info.ae_hist_5bin[...]);
 * printf("ae hist 256bin:...\n, info.ae_hist_256bin[...]");
 * printf("ae statis:...\n", info.ae_statis.statis[...][...]);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAeStatistics(IMPVI_NUM num, IMPISPAEStatisInfo *ae_statis);

/**
 * AE曝光时间单位
 */
typedef enum {
	ISP_CORE_EXPR_UNIT_LINE,			/**< 单位为曝光行 */
	ISP_CORE_EXPR_UNIT_US,				/**< 单位为微秒 */
} IMPISPAEIntegrationTimeUnit;

/**
 * AE曝光信息
 */
typedef struct {
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
	IMPISPTuningOpsType AeMode;                         /**< AE Freezen使能 */
	IMPISPTuningOpsType AeIntegrationTimeMode;          /**< AE曝光手动模式使能 */
	IMPISPTuningOpsType AeAGainManualMode;              /**< AE Sensor 模拟增益手动模式使能 */
	IMPISPTuningOpsType AeDGainManualMode;              /**< AE Sensor数字增益手动模式使能 */
	IMPISPTuningOpsType AeIspDGainManualMode;	    /**< AE ISP 数字增益手动模式使能 */
	uint32_t AeIntegrationTime;                         /**< AE手动模式下的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	IMPISPTuningOpsType AeMinIntegrationTimeMode;       /**< AE最小曝光使能位(预留) */
	IMPISPTuningOpsType AeMinAGainMode;                 /**< AE最小模拟增益使能位 */
	IMPISPTuningOpsType AeMinDgainMode;                 /**< AE最小数字增益使能位(预留) */
	IMPISPTuningOpsType AeMinIspDGainMode;              /**< AE最小ISP数字增益使能位(预留) */
	IMPISPTuningOpsType AeMaxIntegrationTimeMode;       /**< AE最大曝光使能位 */
	IMPISPTuningOpsType AeMaxAGainMode;                 /**< AE最大sensor模拟增益使能位 */
	IMPISPTuningOpsType AeMaxDgainMode;                 /**< AE最大sensor数字增益使能位 */
	IMPISPTuningOpsType AeMaxIspDGainMode;              /**< AE最大ISP数字增益使能位 */
	uint32_t AeMinIntegrationTime;                      /**< AE最小曝光时间 */
	uint32_t AeMinAGain;                                /**< AE最小sensor模拟增益，单位是倍数 x 1024 */
	uint32_t AeMinDgain;                                /**< AE最小sensor数字增益，单位是倍数 x 1024 */
	uint32_t AeMinIspDGain;                             /**< AE最小ISP数字增益，单位是倍数 x 1024 */
	uint32_t AeMaxIntegrationTime;                      /**< AE最大曝光时间 */
	uint32_t AeMaxAGain;                                /**< AE最大sensor模拟增益，单位是倍数 x 1024 */
	uint32_t AeMaxDgain;                                /**< AE最大sensor数字增益，单位是倍数 x 1024 */
	uint32_t AeMaxIspDGain;                             /**< AE最大ISP数字增益，单位是倍数 x 1024 */

	/* WDR模式下短帧的AE 手动模式属性*/
	IMPISPTuningOpsType AeShortMode;                    /**< AE Freezen使能 */
	IMPISPTuningOpsType AeShortIntegrationTimeMode;     /**< AE曝光手动模式使能 */
	IMPISPTuningOpsType AeShortAGainManualMode;         /**< AE Sensor 模拟增益手动模式使能 */
	IMPISPTuningOpsType AeShortDGainManualMode;         /**< AE Sensor数字增益手动模式使能 */
	IMPISPTuningOpsType AeShortIspDGainManualMode;      /**< AE ISP 数字增益手动模式使能 */
	uint32_t AeShortIntegrationTime;                    /**< AE手动模式下的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	IMPISPTuningOpsType AeShortMinIntegrationTimeMode;  /**< AE最小曝光使能位(预留) */
	IMPISPTuningOpsType AeShortMinAGainMode;            /**< AE最小模拟增益使能位 */
	IMPISPTuningOpsType AeShortMinDgainMode;            /**< AE最小数字增益使能位(预留) */
	IMPISPTuningOpsType AeShortMinIspDGainMode;         /**< AE最小ISP数字增益使能位(预留) */
	IMPISPTuningOpsType AeShortMaxIntegrationTimeMode;  /**< AE最大曝光使能位 */
	IMPISPTuningOpsType AeShortMaxAGainMode;            /**< AE最大sensor模拟增益使能位 */
	IMPISPTuningOpsType AeShortMaxDgainMode;            /**< AE最大sensor数字增益使能位 */
	IMPISPTuningOpsType AeShortMaxIspDGainMode;         /**< AE最大ISP数字增益使能位 */
	uint32_t AeShortMinIntegrationTime;                 /**< AE最小曝光时间 */
	uint32_t AeShortMinAGain;                           /**< AE最小sensor模拟增益 */
	uint32_t AeShortMinDgain;                           /**< AE最小sensor数字增益 */
	uint32_t AeShortMinIspDGain;                        /**< AE最小ISP数字增益 */
	uint32_t AeShortMaxIntegrationTime;                 /**< AE最大曝光时间 */
	uint32_t AeShortMaxAGain;                           /**< AE最大sensor模拟增益 */
	uint32_t AeShortMaxDgain;                           /**< AE最大sensor数字增益 */
	uint32_t AeShortMaxIspDGain;                        /**< AE最大ISP数字增益 */

	uint32_t TotalGainDb;                               /**< AE total gain，单位为db(只读) */
	uint32_t TotalGainDbShort;                          /**< AE 短帧 total gain, 单位为db */
	uint64_t ExposureValue;                             /**< AE 曝光值，为integration time x again x dgain */
	uint32_t EVLog2;                                    /**< AE 曝光值，此值经过log运算 */
} IMPISPAEExprInfo;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo)
 *
 * 获取AE统计信息.
 *
 * @param[in] num                对应sensor的标号
 * @param[in] exprinfo           AE曝光信息
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEExprInfo info;
 *
 * ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeExprInfo error !\n");
 * 	return -1;
 * }
 *
 * //Main switch in manual mode.
 * info.AeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 *
 * info.AeIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.AeIntegrationTimeUnit = ISP_CORE_EXPR_UNIT_LINE;
 * info.AeIntegrationTime = 100;
 * info.AeAGainManualMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.AeAGain = 1024;
 *
 * info.AeMinIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.AeIntegrationTimeUnit = ISP_CORE_EXPR_UNIT_LINE;
 * info.AeMinIntegrationTime = 4;
 * info.AeMinAGainMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.AeMinAGain = 1024;
 *
 * //Main switch in short manual mode.
 * info.AeShortMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 *
 * info.AeShortIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.info.AeIntegrationTimeUnit = ISP_CORE_EXPR_UNIT_LINE;
 * ifno.AeShortIntegrationTime = 10;
 *
 * info.AeShortMinIntegrationTimeMode = IMPISP_TUNING_OPS_TYPE_MANUAL;
 * info.AeIntegrationTimeUnit = ISP_CORE_EXPR_UNIT_LINE;
 * info.AeShortMinIntegrationTime = 4;
 *
 * ret = IMP_ISP_Tuning_SetAeExprInfo(IMPVI_MAIN, &info);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeExprInfo error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo)
 *
 * 获取AE统计信息.
 *
 * @param[in] num                对应sensor的标号
 * @param[out] exprinfo          AE曝光信息
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEExprInfo info;
 *
 * ret = IMP_ISP_Tuning_GetAeExprInfo(IMPVI_MAIN, &info);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeExprInfo error !\n");
 * 	return -1;
 * }
 *
 * printf("ae integration time:%d\n", info.AeIntegrationTime);
 * printf("ae min integration time:%d\n", info.AeMinIntegrationTime);
 * printf("ae short integration time:%d\n", info.AeShortIntegrationTime);
 * printf("ae short min integration time:%d\n", info.AeShortMinIntegrationTime);
 * printf("total gain:%d, short total gain:%d, exposure:%d, ev:%d\n",
 * info.TotalGainDb, info.TotalGainDbShort, info.ExposureValue, info.EVLog2);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAeExprInfo(IMPVI_NUM num, IMPISPAEExprInfo *exprinfo);

/**
 * AE场景模式状态
 */
typedef enum {
	IMP_ISP_AE_SCENCE_AUTO,        /**< 自动模式 */
	IMP_ISP_AE_SCENCE_DISABLE,     /**< 关闭此场景模式 */
	TISP_AE_SCENCE_ROI_ENABLE,     /**< ROI 使能此场景模式 */
	TISP_AE_SCENCE_GLOBAL_ENABLE,  /**< GLOBAL 使能此场景模式 */
	IMP_ISP_AE_SCENCE_BUTT,        /**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPAEScenceMode;

/**
 * AE场景模式属性
 */
typedef struct {
	IMPISPAEScenceMode AeHLCEn;            /**< AE 强光抑制功能使能 */
	unsigned char AeHLCStrength;           /**< AE 强光抑制强度（0 ~ 10）*/
	IMPISPAEScenceMode AeBLCEn;            /**< AE 背光补偿功能使能 */
	unsigned char AeBLCStrength;           /**< AE 背光补偿强度（0 ~ 10） */
	IMPISPAEScenceMode AeTargetCompEn;     /**< AE 目标亮度补偿使能 */
	uint32_t AeTargetComp;                 /**< AE 目标亮度调节强度（0 ~ 255，小于128变暗，大于128变亮） */
	IMPISPAEScenceMode AeStartEn;          /**< AE 起始点功能使能 */
	uint32_t AeStartEv;                    /**< AE 起始点EV值 */

	uint32_t luma;                         /**< AE Luma值 */
	uint32_t luma_scence;                  /**< AE 场景Luma值 */
} IMPISPAEScenceAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr)
 *
 * 设置AE场景模式.
 *
 * @param[in] num                    对应sensor的标号
 * @param[in] scenceattr             AE场景模式设置
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEScenceAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeScenceAttr error !\n");
 * 	return -1;
 * }
 *
 * attr.AeHLCEn = TISP_AE_SCENCE_ROI_ENABLE;
 * attr.AeHLCStrength = 1;
 *
 * ret = IMP_ISP_Tuning_SetAeScenceAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAeScenceAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr)
 *
 * 获取AE场景模式信息.
 *
 * @param[in] num                    对应sensor的标号
 * @param[out] scenceattr            AE场景模式设置
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAEScenceAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAeScenceAttr error !\n");
 * 	return -1;
 * }
 * printf("ae hlcd mode:%d, strength:%d\n", attr.AeHLCEn, attr.AeHLCStrength);
 * printf("luma:%d, luma scence:%d\n", attr.luma, attr.luma_scence);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAeScenceAttr(IMPVI_NUM num, IMPISPAEScenceAttr *scenceattr);

/**
 * AWB统计信息
 */
typedef struct {
	IMPISPStatisZone awb_r;    /**< AWB R通道统计值 */
	IMPISPStatisZone awb_g;    /**< AWB G通道统计值 */
	IMPISPStatisZone awb_b;    /**< AWB B通道统计值 */
} IMPISPAWBStatisInfo;

/**
* @fn int32_t IMP_ISP_Tuning_GetAwbStatistics(IMPVI_NUM num, IMPISPAWBStatisInfo *awb_statis)
*
* 获取AWB统计值.
*
* @param[in]    num                    对应sensor的标号
* @param[out]   awb_statis             awb统计信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAwbStatistics(IMPVI_NUM num, IMPISPAWBStatisInfo *awb_statis);

/**
 * 白平衡增益属性
 */
typedef struct {
	uint32_t rgain;     /**< 白平衡R通道增益 */
	uint32_t bgain;     /**< 白平衡B通道增益 */
} IMPISPAWBGain;

/**
 * AWB 全局统计信息
 */
typedef struct {
	IMPISPAWBGain statis_weight_gain;	/**< 白平衡全局加权统计值 */
	IMPISPAWBGain statis_gol_gain;		/**< 白平衡全局统计值 */
} IMPISPAWBGlobalStatisInfo;

/**
* @fn int32_t IMP_ISP_Tuning_GetAwbGlobalStatistics(IMPVI_NUM num, IMPISPAWBGlobalStatisInfo *awb_statis)
*
* 获取AWB统计值.
*
* @param[in]    num                    对应sensor的标号
* @param[out]   awb_statis             awb统计信息
*
* @retval 0 成功
* @retval 非0 失败，返回错误码
*
* @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
*/
int32_t IMP_ISP_Tuning_GetAwbGlobalStatistics(IMPVI_NUM num, IMPISPAWBGlobalStatisInfo *awb_statis);

/**
 * 白平衡模式
 */
typedef enum {
	ISP_CORE_WB_MODE_AUTO = 0,			/**< 自动模式 */
	ISP_CORE_WB_MODE_MANUAL,			/**< 手动模式 */
	ISP_CORE_WB_MODE_DAY_LIGHT,			/**< 晴天 */
	ISP_CORE_WB_MODE_CLOUDY,			/**< 阴天 */
	ISP_CORE_WB_MODE_INCANDESCENT,                  /**< 白炽灯 */
	ISP_CORE_WB_MODE_FLOURESCENT,                   /**< 荧光灯 */
	ISP_CORE_WB_MODE_TWILIGHT,			/**< 黄昏 */
	ISP_CORE_WB_MODE_SHADE,				/**< 阴影 */
	ISP_CORE_WB_MODE_WARM_FLOURESCENT,              /**< 暖色荧光灯 */
	ISP_CORE_WB_MODE_COLORTEND,			/**< 自定义模式 */
} IMPISPAWBMode;

/**
 * 白平衡自定义模式属性
 */
typedef struct {
	IMPISPTuningOpsMode customEn;   /**< 白平衡自定义模式使能 */
	IMPISPAWBGain gainH;            /**< 白平衡高色温通道增益偏移 */
	IMPISPAWBGain gainM;            /**< 白平衡中色温通道增益偏移 */
	IMPISPAWBGain gainL;            /**< 白平衡低色温通道增益偏移 */
	uint32_t ct_node[4];            /**< 白平衡通道增益偏移的节点 */
} IMPISPAWBCustomModeAttr;

/**
 * 白平衡属性
 */
typedef struct isp_core_wb_attr{
	IMPISPAWBMode mode;                     /**< 白平衡模式 */
	IMPISPAWBGain gain_val;			/**< 白平衡通道增益，手动模式时有效 */
	IMPISPTuningOpsMode awb_frz;            /**< 白平衡frzzen 使能*/
	unsigned int ct;                        /**< 白平衡当前色温值 */
	IMPISPAWBCustomModeAttr custom;         /**< 白平衡自定义模式属性 */
	IMPISPTuningOpsMode awb_start_en;       /**< 白平衡收敛起始点使能 */
	IMPISPAWBGain awb_start;                /**< 白平衡收敛起始点 */
} IMPISPWBAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr)
 *
 * 设置AWB属性
 *
 * @param[in]    num                    对应sensor的标号
 * @param[in]    attr                   awb属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWBAttr attr;
 * uint32_t ct_node[4] = {...};
 *
 * ret = IMP_ISP_Tuning_GetAwbAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbAttr error !\n");
 * 	return -1;
 * }
 *
 * attr.mode = ISP_CORE_WB_MODE_MANUAL;
 * atrr.gain_val.rgain = 200;
 * atrr.gain_val.bgain = 200;
 *
 * attr.awb_frz = IMPISP_TUNING_OPS_MODE_ENABLE;
 * attr.ct = 200;
 *
 * attr.awb_start_en = IMPISP_TUNING_OPS_MODE_ENABLE;
 * attr.awb_start.rgain = 200;
 * attr.awb_start.bgain = 200;
 *
 * attr.custom.customEn = IMPISP_TUNING_OPS_MODE_ENABLE;
 * attr.custom.gainH.rgain = 100;
 * attr.custom.gainH.bgain = 100;
 * attr.custom.gainM.rgain = 100;
 * attr.custom.gainM.bgain = 100;
 * attr.custom.gainL.rgain = 100;
 * attr.custom.gainL.bgain = 100;
 * memcpy(attr.custom.ct_node, ct_node, sizeof(ct_node));
 *
 * ret = IMP_ISP_Tuning_SetAwbAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAwbAttr error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr);

 /**
 * @fn int32_t IMP_ISP_Tuning_GetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr)
 *
 * 获取AWB属性
 *
 * @param[in]    num                    对应sensor的标号
 * @param[out]   attr                   awb属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWBAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetAwbAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbAttr error !\n");
 * 	return -1;
 * }
 * printf("awb mode:%d, rgain:%d, bgain:%d\n", attr.mode, attr.gain_val.rgain, attr.gain_val.bgain);
 * printf("awb frzzen:%d, ct:%d\n", attr.awb_frz, attr.ct);
 * printf("awb start en:%d, start rgain:%d, start bgain:%d\n", attr.awb_start_en, attr.awb_start.rgain, attr.awb_start.bgain);
 * printf("awb custom en:%d", attr.custom.customEn);
 * if (attr.custom.customEn == IMPISP_TUNING_OPS_MODE_ENABLE) {
 *      printf("awb custom:...", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAwbAttr(IMPVI_NUM num, IMPISPWBAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight)
 *
 * 设置AWB统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[in] awb_weight    各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWeight weight;
 * unsigned char w[15*15] = {...};
 *
 * memcpy(weight.weight, w, sizeof(w));
 * ret = IMP_ISP_Tuning_SetAwbWeight(IMPVI_MAIN, &weight);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAwbWeight error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight)
 *
 * 获取AWB统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] awb_weight   各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWeight weight;
 *
 * ret = IMP_ISP_Tuning_GetAwbWeight(IMPVI_MAIN, &weight);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAwbWeight error !\n");
 * 	return -1;
 * }
 * printf("awb weight:...\n", ...);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAwbWeight(IMPVI_NUM num, IMPISPWeight *awb_weight);

/**
 * AF statistics info each area
 */
typedef struct {
	IMPISPStatisZone Af_Fir0;
	IMPISPStatisZone Af_Fir1;
	IMPISPStatisZone Af_Iir0;
	IMPISPStatisZone Af_Iir1;
	IMPISPStatisZone Af_YSum;
	IMPISPStatisZone Af_HighLumaCnt;
} IMPISPAFStatisInfo;
/**
 * @fn IMP_ISP_Tuning_GetAfStatistics(IMPVI_NUM num, IMPISPAFStatisInfo *af_statis)
 *
 * 获取AF统计值。
 *
 * @param[in]   num             对应sensor的标号
 * @param[out]  af_statis       AF统计值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAFStatisInfo info;
 *
 * ret = IMP_ISP_Tuning_GetAfStatistics(IMPVI_MAIN, &info);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAfStatistics error !\n");
 * 	return -1;
 * }
 * printf("af statis:...\n", ...);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAfStatistics(IMPVI_NUM num, IMPISPAFStatisInfo *af_statis);

/**
 * AF全局统计值信息
 */
 typedef struct {
	 uint32_t af_metrics;       /**< AF主统计值*/
	 uint32_t af_metrics_alt;   /**< AF次统计值*/
	 uint8_t af_frame_num;		/**< AF帧数*/
 } IMPISPAFMetricsInfo;

/**
 * @fn int32_t IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_NUM num, IMPISPAFMetricsInfo *metric)
 *
 * 获取AF统计值。
 *
 * @param[in]   num             对应sensor的标号
 * @param[out]  metric          AF全局统计值
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPAFMetricsInfo info;
 *
 * ret = IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_MAIN, &info);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAFMetricesInfo error !\n");
 * 	return -1;
 * }
 * printf("af metrics:%d, af metrics alt:%d, frame num:%d\n", info.af_metrics, info.af_metrics_alt, info.af_frame_num);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAFMetricesInfo(IMPVI_NUM num, IMPISPAFMetricsInfo *metric);

/**
 * @fn int32_t IMP_ISP_Tuning_SetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight)
 *
 * 设置AF统计区域的权重。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] af_weight 各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWeight af;
 * unsigned char weight[15*15] = {...};
 *
 * memcpy(af.weight, weight, sizeof(weight));
 * ret = IMP_ISP_Tuning_SetAfWeight(IMPVI_MAIN, &af);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAfWeight error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight)
 *
 * 获取AF统计区域的权重。
 *
 * @param[in] num           对应sensor的标号
 * @param[out] af_weight    各区域权重信息。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWeight af;
 *
 * ret = IMP_ISP_Tuning_GetAfWeight(IMPVI_MAIN, &af);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAfWeight error !\n");
 * 	return -1;
 * }
 * printf("af weight:...\n", ...);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAfWeight(IMPVI_NUM num, IMPISPWeight *af_weight);

/**
 * ISP AutoZoom Attribution
 */
typedef struct {
	int32_t zoom_chx_en[3];     /**< 数字自动对焦功能通道使能 */
	int32_t zoom_left[3];       /**< 自动对焦区域横向起始点，需要小于原始图像的宽度 */
	int32_t zoom_top[3];        /**< 自动对焦区域纵向起始点，需要小于原始图像的高度 */
	int32_t zoom_width[3];      /**< 自动对焦区域的宽度 */
	int32_t zoom_height[3];     /**< 自动对焦区域的高度 */
} IMPISPAutoZoom;

/**
 * @fn int32_t IMP_ISP_Tuning_SetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom)
 *
 * 设置自动对焦功能的属性。
 *
 * @param[in] num           对应sensor的标号
 * @param[in] ispautozoom   自动对焦功能的属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * //以1080P为例，ch0已开启
 * int ret = 0;
 * IMPISPAutoZoom zoom;
 *
 * zoom.zoom_chx_en[0] = 1;
 * zoom.zoom_left[0] = 10;
 * zoom.zoom_top[0] = 10;
 * zoom.zoom_width[0] = 640;
 * zoom.zoom_height[0] = 480;
 *
 * ret = IMP_ISP_Tuning_SetAutoZoom(IMPVI_MAIN, &zoom);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetAutoZoom error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom);

/**
 * @fn int32_t IMP_ISP_Tuning_GetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom)
 *
 * 获取自动对焦功能的属性。
 *
 * @param[in] num               对应sensor的标号
 * @param[out] ispautozoom      自动对焦功能的属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int i;
 * int ret = 0;
 * IMPISPAutoZoom zoom;
 *
 * ret = IMP_ISP_Tuning_GetAutoZoom(IMPVI_MAIN, &zoom);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetAutoZoom error !\n");
 * 	return -1;
 * }
 * for(i=0; i<3; i++){
 *      if (zoom.zoom_chx_en[i]) {
 *              printf("ch:%d, left:%d, top:%d, width:%d, height:%d\n", i,
 *              zoom.zoom_left[i], zoom.zoom_top[i], zoom.zoom_width[i], zoom.zoom_height[i]);
 *      }
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetAutoZoom(IMPVI_NUM num, IMPISPAutoZoom *ispautozoom);

/**
 * 填充数据类型
 */
typedef enum {
        IMPISP_MASK_TYPE_RGB = 0, /**< RGB */
        IMPISP_MASK_TYPE_YUV = 1, /**< YUV */
} IMPISP_MASK_TYPE;

/**
 * 填充数据
 */
typedef struct color_value {
	struct {
		unsigned char r_value;	/**< R 值 */
		unsigned char g_value;	/**< G 值 */
		unsigned char b_value;	/**< B 值 */
	} argb; 			/**< RGB */
	struct {
		unsigned char y_value;	/**< Y 值 */
		unsigned char u_value;	/**< U 值 */
		unsigned char v_value;	/**< V 值 */
	} ayuv;	        		/**< YUV */
} IMP_ISP_COLOR_VALUE;

/**
 * 每个通道的填充属性
 */
typedef struct isp_mask_block_par {
	uint8_t chx;              /**< 通道号(范围: 0~2) */
	uint8_t pinum;            /**< 块号(范围: 0~3) */
	uint8_t mask_en;          /**< 填充使能 */
	uint16_t mask_pos_top;    /**< 填充位置y坐标*/
	uint16_t mask_pos_left;   /**< 填充位置x坐标  */
	uint16_t mask_width;      /**< 填充数据宽度 */
	uint16_t mask_height;     /**< 填充数据高度 */
	IMPISP_MASK_TYPE mask_type;		/**< 填充数据类型 */
	IMP_ISP_COLOR_VALUE mask_value;  /**< 填充数据值 */
} IMPISPMaskBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetMaskBlock(IMPVI_NUM num, IMPISPMaskBlockAttr *mask)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] mask  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr block;
 *
 * if (en) {
 *      block.chx = 0;
 *      block.pinum = 0;
 *      block.mask_en = 1;
 *      block.mask_pos_top = 10;
 *      block.mask_pos_left = 100;
 *      block.mask_width = 200;
 *      block.mask_height = 200;
 *      block.mask_type = IMPISP_MASK_TYPE_YUV;
 *      block.mask_value.ayuv.y_value = 100;
 *      block.mask_value.ayuv.u_value = 100;
 *      block.mask_value.ayuv.v_value = 100;
 * } else {
 *      block.mask_en = 0;
 * }
 *
 * ret = IMP_ISP_Tuning_SetMaskBlock(IMPVI_MAIN, &block);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetMaskBlock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetMaskBlock(IMPVI_NUM num, IMPISPMaskBlockAttr *mask);

/**
 * @fn int32_t IMP_ISP_Tuning_GetMaskBlock(IMPVI_NUM num, IMPISPMaskBlockAttr *mask)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] mask 填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPMaskBlockAttr attr;
 *
 * attr.chx = 0;
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetMaskBlock(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetMaskBlock error !\n");
 * 	return -1;
 * }
 * printf("chx:%d, pinum:%d, en:%d\n", attr.chx, attr.pinum, attr.mask_en);
 * if (attr.mask_en) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetMaskBlock(IMPVI_NUM num, IMPISPMaskBlockAttr *mask);

/**
 * 填充图片格式
 */
typedef enum {
	IMP_ISP_PIC_ARGB_8888,  /**< ARGB8888 */
	IMP_ISP_PIC_ARGB_1555,  /**< ARBG1555 */
} IMPISPPICTYPE;

/**
 * 填充格式
 */
typedef enum {
	IMP_ISP_ARGB_TYPE_BGRA = 0,
	IMP_ISP_ARGB_TYPE_GBRA,
	IMP_ISP_ARGB_TYPE_BRGA,
	IMP_ISP_ARGB_TYPE_RBGA,
	IMP_ISP_ARGB_TYPE_GRBA,
	IMP_ISP_ARGB_TYPE_RGBA,

	IMP_ISP_ARGB_TYPE_ABGR = 8,
	IMP_ISP_ARGB_TYPE_AGBR,
	IMP_ISP_ARGB_TYPE_ABRG,
	IMP_ISP_ARGB_TYPE_AGRB,
	IMP_ISP_ARGB_TYPE_ARBG,
	IMP_ISP_ARGB_TYPE_ARGB,
} IMPISPARGBType;

/**
 * 填充图片参数
 */
typedef struct {
    uint8_t  pinum;			/**< 块号(范围: 0~7) */
	uint8_t  osd_enable;    /**< 填充功能使能 */
	uint16_t osd_left;      /**< 填充横向起始点 */
	uint16_t osd_top;       /**< 填充纵向起始点 */
	uint16_t osd_width;     /**< 填充宽度 */
	uint16_t osd_height;    /**< 填充高度 */
	char *osd_image;	/**< 填充图片首地址 */
	uint16_t osd_stride;    /**< 填充图片的对其宽度, 以字节为单位，例如320x240的RGBA8888图片osd_stride=320*4 */
} IMPISPOSDBlockAttr;

/**
   * 填充功能通道属性
    */
typedef struct {
	IMPISPPICTYPE osd_type;                        /**< 填充图片类型  */
	IMPISPARGBType osd_argb_type;                  /**< 填充格式  */
	IMPISPTuningOpsMode osd_pixel_alpha_disable;   /**< 填充像素Alpha禁用功能使能  */
} IMPISPOSDAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr)
 *
 * 设置填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * attr.osd_type = IMP_ISP_PIC_ARGB_8888;
 * attr.osd_argb_type = IMP_ISP_ARGB_TYPE_BGRA;
 * attr.osd_pixel_alpha_disable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *
 * if(ret){
 * 	IMP_LOG_ERR(TAG, " error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr)
 *
 * 获取填充参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  填充参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDAttr attr;
 *
 * ret = IMP_ISP_Tuning_GetOSDAttr(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDAttr error !\n");
 * 	return -1;
 * }
 * printf("type:%d, argb_type:%d, mode:%d\n", attr.osd_type,
 * attr.osd_argb_type, attr.osd_pixel_alpha_disable);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDAttr(IMPVI_NUM num, IMPISPOSDAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_SetOSDBlock(IMPVI_NUM num, IMPISPOSDBlockAttr *attr)
 *
 * 设置OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr block;
 *
 * block.pinum = pinum;
 * block.osd_enable = enable;
 * block.osd_left = left / 2 * 2;
 * block.osd_top = top / 2 * 2;
 * block.osd_width = width;
 * block.osd_height = height;
 * block.osd_image = image;
 * block.osd_stride = stride;
 *
 * ret = IMP_ISP_Tuning_SetOSDBlock(IMPVI_MAIN, &block);
 * if(ret){
 * 	imp_log_err(tag, "imp_isp_tuning_setosdblock error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetOSDBlock(IMPVI_NUM num, IMPISPOSDBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetOSDBlock(IMPVI_NUM num, IMPISPOSDBlockAttr *attr)
 *
 * 获取OSD参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr OSD参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPOSDBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetOSDBlock(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetOSDBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, en:%d\n", attr.pinum, attr.osd_enable);
 * if (attr.osd_enable) {
 *      printf("top:%d, left:%d ...\n", ...);
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetOSDBlock(IMPVI_NUM num, IMPISPOSDBlockAttr *attr);

/**
 * 画窗功能属性
 */
typedef struct {
	uint8_t  enable;           /**< 画窗功能使能 */
	uint16_t left;             /**< 画窗功能横向起始点 */
	uint16_t top;              /**< 画窗功能纵向起始点 */
	uint16_t width;            /**< 画窗宽度 */
	uint16_t height;           /**< 画窗高度 */
	IMP_ISP_COLOR_VALUE color; /**< 画窗颜色 */
	uint8_t  line_width;	   /**< 窗口边框宽度 */
	uint8_t  alpha;            /**< 宽口边框alpha（3bit） */
}IMPISPDrawWindAttr;

/**
 * 画四角窗功能属性
 */
typedef struct {
	uint8_t  enable;           /**< 画四角窗功能使能 */
	uint16_t left;             /**< 画四角窗功能横向起始点 */
	uint16_t top;              /**< 画四角窗功能纵向起始点 */
	uint16_t width;            /**< 画四角窗宽度 */
	uint16_t height;           /**< 画四角窗高度 */
	IMP_ISP_COLOR_VALUE color; /**< 画四角窗颜色 */
	uint8_t  line_width;       /**< 画四角窗边框宽度 */
	uint8_t  alpha;            /**< 四角窗边框alpha （3bit） */
	uint16_t extend;           /**< 四角窗边框长度 */
} IMPISPDrawRangAttr;

/**
 * 画线功能属性
 */
typedef struct {
	uint8_t  enable;               /**< 画线功能使能 */
	uint16_t startx;               /**< 画线横向起始点 */
	uint16_t starty;               /**< 画线纵向起始点 */
	uint16_t endx;                 /**< 画线横向结束点 */
	uint16_t endy;                 /**< 画线纵向结束点 */
	IMP_ISP_COLOR_VALUE color;     /**< 线条颜色 */
	uint8_t  width;                /**< 线宽 */
	uint8_t  alpha;                /**< 线条Alpha值 */
} IMPISPDrawLineAttr;

/**
 * 画图功能类型
 */
typedef enum {
	IMP_ISP_DRAW_WIND,              /**< 画框 */
	IMP_ISP_DRAW_RANGE,             /**< 画四角窗 */
	IMP_ISP_DRAW_LINE,              /**< 画线 */
} IMPISPDrawType;

/**
 * 画图功能属性
 */
typedef struct {
	uint8_t pinum;                      /**< 块号(范围: 0~19) */
	IMPISPDrawType type;                /**< 画图类型 */
	IMPISP_MASK_TYPE color_type;		/**< 填充数据类型 */
	union {
		IMPISPDrawWindAttr wind;		/**< 画框属性 */
		IMPISPDrawRangAttr rang;		/**< 画四角窗属性 */
		IMPISPDrawLineAttr line;		/**< 画线属性 */
	} cfg;								/**< 画图属性 */
} IMPISPDrawBlockAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SetDrawBlock(IMPVI_NUM num, IMPISPDrawBlockAttr *attr)
 *
 * 设置绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)2;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.line.enable = en;
 * block.cfg.line.startx = left / 2 * 2;
 * block.cfg.line.starty = top / 2 * 2;
 * block.cfg.line.endx = w / 2 * 2;
 * block.cfg.line.endy = h / 2 * 2;
 * block.cfg.line.color.ayuv.y_value = y;
 * block.cfg.line.color.ayuv.u_value = u;
 * block.cfg.line.color.ayuv.v_value = v;
 * block.cfg.line.width = lw / 2 * 2;
 * block.cfg.line.alpha = alpha;
 * IMP_ISP_Tuning_SetDrawBlock(IMPVI_MAIN, &block);
 *
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)0;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.wind.enable = en;
 * block.cfg.wind.left = left / 2 * 2;
 * block.cfg.wind.top = top / 2 * 2;
 * block.cfg.wind.width = w / 2 * 2;
 * block.cfg.wind.height = h / 2 * 2;
 * block.cfg.wind.color.ayuv.y_value = y;
 * block.cfg.wind.color.ayuv.u_value = u;
 * block.cfg.wind.color.ayuv.v_value = v;
 * block.cfg.wind.line_width = lw / 2 * 2;
 * block.cfg.wind.alpha = alpha;
 *
 * IMP_ISP_Tuning_SetDrawBlock(IMPVI_MAIN, &block);
 * IMPISPDrawBlockAttr block;
 *
 * block.pinum = pinum;
 * block.type = (IMPISPDrawType)1;
 * block.color_type = (IMPISP_MASK_TYPE)ctype;
 * block.cfg.rang.enable = en;
 * block.cfg.rang.left = left / 2 * 2;
 * block.cfg.rang.top = top / 2 * 2;
 * block.cfg.rang.width = w / 2 * 2;
 * block.cfg.rang.height = h / 2 * 2;
 * block.cfg.rang.color.ayuv.y_value = y;
 * block.cfg.rang.color.ayuv.u_value = u;
 * block.cfg.rang.color.ayuv.v_value = v;
 * block.cfg.rang.line_width = lw / 2 * 2;
 * block.cfg.rang.alpha = alpha;
 * block.cfg.rang.extend = extend / 2 * 2;
 *
 * IMP_ISP_Tuning_SetDrawBlock(IMPVI_MAIN, &block);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetDrawBlock(IMPVI_NUM num, IMPISPDrawBlockAttr *attr);

/**
 * @fn int32_t IMP_ISP_Tuning_GetDrawBlock(IMPVI_NUM num, IMPISPDrawBlockAttr *attr)
 *
 * 获取绘图功能参数.
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr  绘图功能参数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPDrawBlockAttr attr;
 *
 * attr.pinum = 0;
 * ret = IMP_ISP_Tuning_GetDrawBlock(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetDrawBlock error !\n");
 * 	return -1;
 * }
 * printf("pinum:%d, type:%d, color type:%d\n", attr.pinum, attr.type, attr.color_type);
 * switch (attr.type) {
 *      case IMP_ISP_DRAW_WIND:
 *          printf("enable:%d\n", attr.wind.enable);
 *          if (attr.wind.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_RANGE:
 *          printf("enable:%d\n", attr.rang.enable);
 *          if (attr.rang.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      case IMP_ISP_DRAW_LINE:
 *          printf("enable:%d\n", attr.line.enable);
 *          if (attr.line.enable) {
 *              printf("left:%d, ...\n", ...);
 *          }
 *          break;
 *      default:
 *          break;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetDrawBlock(IMPVI_NUM num, IMPISPDrawBlockAttr *attr);

/**
 * 客户自定义自动曝光库的AE初始属性
 */
typedef struct {
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */

	/* WDR模式下长帧或者线性模式下的AE属性*/
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeMinIntegrationTime;                      /**< AE最小曝光时间 */
	uint32_t AeMinAGain;                                /**< AE最小sensor模拟增益 */
	uint32_t AeMinDgain;                                /**< AE最小sensor数字增益 */
	uint32_t AeMinIspDGain;                             /**< AE最小ISP数字增益 */
	uint32_t AeMaxIntegrationTime;                      /**< AE最大曝光时间 */
	uint32_t AeMaxAGain;                                /**< AE最大sensor模拟增益 */
	uint32_t AeMaxDgain;                                /**< AE最大sensor数字增益 */
	uint32_t AeMaxIspDGain;                             /**< AE最大ISP数字增益 */

	/* WDR模式下短帧的AE属性*/
	uint32_t AeShortIntegrationTime;                    /**< AE的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeShortMinIntegrationTime;                 /**< AE最小曝光时间 */
	uint32_t AeShortMinAGain;                           /**< AE最小sensor模拟增益 */
	uint32_t AeShortMinDgain;                           /**< AE最小sensor数字增益 */
	uint32_t AeShortMinIspDGain;                        /**< AE最小ISP数字增益 */
	uint32_t AeShortMaxIntegrationTime;                 /**< AE最大曝光时间 */
	uint32_t AeShortMaxAGain;                           /**< AE最大sensor模拟增益 */
	uint32_t AeShortMaxDgain;                           /**< AE最大sensor数字增益 */
	uint32_t AeShortMaxIspDGain;                        /**< AE最大ISP数字增益 */

	uint32_t fps;                                       /**< sensor 帧率 */
	IMPISPAEStatisAttr AeStatis;						/**< AE统计属性 */
} IMPISPAeInitAttr;

/**
 * 客户自定义自动曝光库的AE信息
 */
typedef struct {
	IMPISPAEStatisInfo ae_info;							/**< AE统计值 */
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/
	uint32_t AeShortIntegrationTime;                    /**< AE的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t Wdr_mode;									/**< 当前是否WDR模式*/
	IMPISPSENSORAttr sensor_attr;						/**< Sensor基本属性*/
}  __attribute__((packed, aligned(1))) IMPISPAeInfo;

/**
 * 客户自定义自动曝光库的AE属性
 */
typedef struct {
	uint32_t change;                                    /**< 是否更新AE参数 */
	IMPISPAEIntegrationTimeUnit AeIntegrationTimeUnit;  /**< AE曝光时间单位 */
	uint32_t AeIntegrationTime;                         /**< AE的曝光值 */
	uint32_t AeAGain;                                   /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeDGain;                                   /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeIspDGain;                                /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t AeShortIntegrationTime;                    /**< AE手动模式下的曝光值 */
	uint32_t AeShortAGain;                              /**< AE Sensor 模拟增益值，单位是倍数 x 1024 */
	uint32_t AeShortDGain;                              /**< AE Sensor数字增益值，单位是倍数 x 1024 */
	uint32_t AeShortIspDGain;                           /**< AE ISP 数字增益值，单位倍数 x 1024*/

	uint32_t luma;			       				/**< AE Luma值 */
	uint32_t luma_scence;		       				/**< AE 场景Luma值 */
} IMPISPAeAttr;

/**
 * 客户自定义自动曝光库的AE通知属性
 */
typedef enum {
	IMPISP_AE_NOTIFY_FPS_CHANGE,						/**< 帧率变更 */
} IMPISPAeNotify;

/**
 * 客户自定义自动曝光库的AE回调函数
 */
typedef struct {
	void *priv_data;																	/**< 私有数据地址 */
	int (*open)(void *priv_data, IMPISPAeInitAttr *AeInitAttr);                         /**< 自定义AE库开始接口 */
	void (*close)(void *priv_data);													    /**< 自定义AE库关闭接口 */
	void (*handle)(void *priv_data, const IMPISPAeInfo *AeInfo, IMPISPAeAttr *AeAttr);  /**< 自定义AE库的处理接口 */
	int (*notify)(void *priv_data, IMPISPAeNotify notify, void *data);                  /**< 自定义AE库的通知接口 */
} IMPISPAeAlgoFunc;

/**
 * @fn int32_t IMP_ISP_SetAeAlgoFunc(IMPVI_NUM num, IMPISPAeAlgoFunc *ae_func)
 *
 * 客户自定义自动曝光库的注册接口
 *
 * @param[in] num       对应sensor的标号
 * @param[in] ae_func   客户自定义AE库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)接口之后立刻调用。
 */
int32_t IMP_ISP_SetAeAlgoFunc(IMPVI_NUM num, IMPISPAeAlgoFunc *ae_func);

/**
 * 客户自定义自动白平衡库的AWB初始属性
 */
typedef struct {
	IMPISPAWBStatisAttr AwbStatis;							/**< AWB的初始统计属性 */
} IMPISPAwbInitAttr;

/**
 * 客户自定义自动白平衡库的AWB信息
 */
typedef struct {
	uint32_t cur_r_gain;								/**< 白平衡R通道增益 */
	uint32_t cur_b_gain;								/**< 白平衡B通道增益 */
	uint32_t r_gain_statis;								/**< 白平衡全局统计值r_gain */
	uint32_t b_gain_statis;								/**< 白平衡全局加权统计值b_gain */
	uint32_t r_gain_wei_statis;							/**< 白平衡全局加权统计值r_gain */
	uint32_t b_gain_wei_statis;							/**< 白平衡全局加权统计值b_gain */
	IMPISPAWBStatisInfo awb_statis;						/**< 白平衡区域统计值 */
}__attribute__((packed, aligned(1))) IMPISPAwbInfo;

/**
 * 客户自定义自动白平衡库的AWB属性
 */
typedef struct {
	uint32_t change;					/**< 是否更新AWB参数 */
	uint32_t r_gain;						/**< AWB参数 r_gain */
	uint32_t b_gain;						/**< AWB参数 b_gain */
	uint32_t ct;						    /**< 当前色温 */
} IMPISPAwbAttr;

/**
 * 客户自定义自动白平衡库的AWB通知属性
 */
typedef enum {
	IMPISP_AWB_NOTIFY_MODE_CHANGE,           /**< 当前AWB模式变化 */
} IMPISPAwbNotify;

/**
 * 客户自定义自动白平衡库的AWB回调函数
 */
typedef struct {
	void *priv_data;																		/**< 私有数据地址 */
	int (*open)(void *priv_data, IMPISPAwbInitAttr *AwbInitAttr);                           /**< 自定义AWB库开始接口 */
	void (*close)(void *priv_data);                                                         /**< 自定义AWB库关闭接口 */
	void (*handle)(void *priv_data, const IMPISPAwbInfo *AwbInfo, IMPISPAwbAttr *AwbAttr);  /**< 自定义AWB库的处理接口 */
	int (*notify)(void *priv_data, IMPISPAwbNotify notify, void *data);                     /**< 自定义AWB库的通知接口 */
} IMPISPAwbAlgoFunc;

/**
 * @fn int32_t IMP_ISP_SetAwbAlgoFunc(IMPVI_NUM num, IMPISPAwbAlgoFunc *awb_func)
 *
 * 客户自定义自动白平衡库的注册接口.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] awb_func  客户自定义AWB库注册函数.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 此函数需要在调用IMP_ISP_AddSensor(IMPVI_NUM num, IMPSensorInfo *pinfo)接口之后立刻调用。
 */
int32_t IMP_ISP_SetAwbAlgoFunc(IMPVI_NUM num, IMPISPAwbAlgoFunc *awb_func);

/**
 * Tuning bin文件功能属性
 */
typedef struct {
	IMPISPTuningOpsMode enable;	 /**< Switch bin功能开关 */
	char bname[128];			 /**< bin文件的绝对路径 */
} IMPISPBinAttr;

/**
 * @fn int32_t IMP_ISP_Tuning_SwitchBin(IMPVI_NUM num, IMPISPBinAttr *attr)
 *
 * 切换Bin文件.
 *
 * @param[in] num      对应sensor的标号
 * @param[in] attr     需要切换的bin文件属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPBinAttr attr;
 * char name[] = "/etc/sensor/xxx-t41.bin"
 *
 * attr.enable = IMPISP_TUNING_OPS_MODE_ENABLE;
 * memcpy(attr.bname, name, sizeof(name));
 * ret = IMP_ISP_Tuning_SwitchBin(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SwitchBin error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SwitchBin(IMPVI_NUM num, IMPISPBinAttr *attr);

/**
 * WDR输出模式
 */
typedef enum {
        IMPISP_WDR_OUTPUT_MODE_FUS_FRAME,	/**< 混合模式 */
        IMPISP_WDR_OUTPUT_MODE_LONG_FRAME,	/**< 长帧模式 */
        IMPISP_WDR_OUTPUT_MODE_SHORT_FRAME,	/**< 短帧模式 */
        IMPISP_WDR_OUTPUT_MODE_BUTT,		/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPWdrOutputMode;

/**
 * @fn int32_t IMP_ISP_Tuning_SetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode)
 *
 * 设置WDR图像输出模式。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] mode	输出模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWdrOutputMode mode;
 *
 * mode = IMPISP_WDR_OUTPUT_MODE_FUS_FRAME;
 * ret = IMP_ISP_Tuning_SetWdrOutputMode(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetWdrOutputMode error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode);

/**
 * @fn int32_t IMP_ISP_Tuning_GetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode)
 *
 * 获取WDR图像输出模式。
 *
 * @param[in] num       对应sensor的标号
 * @param[out] mode	输出模式.
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPWdrOutputMode mode;
 *
 * ret = IMP_ISP_Tuning_GetWdrOutputMode(IMPVI_MAIN, &mode);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_GetWdrOutputMode error !\n");
 * 	return -1;
 * }
 * printf("wdr output mode:%d\n", mode);
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_GetWdrOutputMode(IMPVI_NUM num, IMPISPWdrOutputMode *mode);

/**
 * 丢帧参数
 */
typedef struct {
	IMPISPTuningOpsMode enable;	/**< 使能标志 */
        uint8_t lsize;			/**< 总数量(范围:0~31) */
        uint32_t fmark;			/**< 位标志(1输出，0丢失) */
} IMPISPFrameDrop;

/**
 * 丢帧属性
 */
typedef struct {
	IMPISPFrameDrop fdrop[3];	/**< 各个通道的丢帧参数 */
} IMPISPFrameDropAttr;

/**
 * @fn int32_t IMP_ISP_SetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr)
 *
 * 设置丢帧属性。
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr	丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @code
 * int i;
 * int ret = 0;
 * IMPISPFrameDropAttr attr;
 *
 * for (i=0; i<3; i++) {
 *     if (en[i]) {
 *         attr.fdrop[i].enable = IMPISP_TUNING_OPS_MODE_ENABLE;
 *         attr.fdrop[i].lsize = 3;
 *         attr.fdrop[i].fmark = 0x5;
 *     } else {
 *         attr.fdrop[i].enable = IMPISP_TUNING_OPS_MODE_DISABLE;
 *     }
 *
 * }
 * ret = IMP_ISP_SetFrameDrop(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_SetFrameDrop error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_SetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr)
 *
 * 获取丢帧属性。
 *
 * @param[in] num       对应sensor的标号
 * @param[out] attr	丢帧属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark 每接收(lsize+1)帧就会丢(fmark无效位数)帧。
 * @remark 例如：lsize=3,fmark=0x5(每4帧丢第2和第4帧)
 *
 * @code
 * int i;
 * int ret = 0;
 * IMPISPFrameDropAttr attr;
 *
 * ret = IMP_ISP_GetFrameDrop(IMPVI_MAIN, &attr);
 * if(ret){
 *     IMP_LOG_ERR(TAG, "IMP_ISP_GetFrameDrop error !\n");
 *     return -1;
 * }
 * for (i=0; i<3; i++) {
 *     printf("ch:%d, enable:%d\n", i, attr.fdrop[i].enable);
 *     if (attr.fdrop[i].enable) {
 *         printf("lsize:%d, fmark:0x%x\n", ...);
 *     }
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_Open已被调用。
 */
int32_t IMP_ISP_GetFrameDrop(IMPVI_NUM num, IMPISPFrameDropAttr *attr);

/**
 * 缩放模式
 */
typedef enum {
        IMPISP_SCALER_FITTING_CRUVE,
        IMPISP_SCALER_FIXED_WEIGHT,
        IMPISP_SCALER_BUTT,
} IMPISPScalerMode;

/**
 * 缩放效果参数
 */
typedef struct {
        uint8_t chx;		/*通道 0~2*/
        IMPISPScalerMode mode;	/*缩放方法*/
        uint8_t level;		/*缩放清晰度等级 范围0~128*/
} IMPISPScalerLvAttr;

/**
 * @fn IMP_ISP_Tuning_SetScalerLv(IMPVI_NUM num, IMPISPScalerLvAttr *attr)
 *
 * Set Scaler 缩放的方法及等级.
 *
 * @param[in] num       对应sensor的标号
 * @param[in] attr	mscaler等级属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 * IMPISPScalerLvAttr attr;
 *
 * attr.chx = 0;
 * attr.mode = IMPISP_SCALER_FIXED_WEIGHT;
 * attr.level = 64;
 * ret = IMP_ISP_Tuning_SetScalerLv(IMPVI_MAIN, &attr);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_Tuning_SetScalerLv error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_Tuning_SetScalerLv(IMPVI_NUM num, IMPISPScalerLvAttr *attr);

/**
 * @fn IMP_ISP_StartNightMode(IMPVI_NUM num)
 *
 * 起始夜视模式.
 *
 * @param[in] num       对应sensor的标号
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @code
 * int ret = 0;
 *
 * ret = IMP_ISP_StartNightMode(IMPVI_MAIN);
 * if(ret){
 * 	IMP_LOG_ERR(TAG, "IMP_ISP_StartNightMode error !\n");
 * 	return -1;
 * }
 * @endcode
 *
 * @attention 使用范围：IMP_ISP_AddSensor之前。
 */
int32_t IMP_ISP_StartNightMode(IMPVI_NUM num);

/**
 * ISP内部通道类型
 */
typedef enum {
	TX_ISP_INTERNAL_CHANNEL_ISP_DMA0,	/**< ISP通道0(仅获取，设置无效) */
	TX_ISP_INTERNAL_CHANNEL_ISP_DMA1,	/**< ISP通道1(仅获取，设置无效) */
	TX_ISP_INTERNAL_CHANNEL_ISP_DMA2,	/**< ISP通道2(仅获取，设置无效) */
	TX_ISP_INTERNAL_CHANNEL_ISP_IR,		/**< ISP IR通道 */
	TX_ISP_INTERNAL_CHANNEL_VIC_DMA0,	/**< ISP bypass通道0 */
	TX_ISP_INTERNAL_CHANNEL_VIC_DMA1,	/**< ISP bypass通道1 */
	TX_ISP_INTERNAL_CHANNEL_VIC_BUTT,	/**< 用于判断参数的有效性，参数大小必须小于这个值 */
} IMPISPInternalChnType;

/**
 * ISP内部通道
 */
typedef struct {
	IMPISPInternalChnType type;	/**< ISP内部通道选择 */
	uint8_t vc_index;			/**< bypass模式下mipi虚拟通道号 */
} IMPISPInternalChn;

/**
 * ISP内部通道属性
 */
typedef struct {
	IMPISPInternalChn ch[3];	/**< 每个ISP输出通道对应的内部通道属性 */
} IMPISPInternalChnAttr;

/**
 * @fn int32_t IMP_ISP_SetInternalChnAttr(IMPVI_NUM num, IMPISPInternalChnAttr *attr)
 *
 * 设置通道属性。
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr	通道属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark ISP DMA通道类型只能获取，设置不会生效
 * @remark 一个内部通道不能被多个外部通道绑定
 * @remark 使用请参阅sample-ISP-InternalChn.c
 *
 * @attention 在使用这个函数之前，必须先使用IMP_ISP_GetInternalChnAttr获取属性。
 */
int32_t IMP_ISP_SetInternalChnAttr(IMPVI_NUM num, IMPISPInternalChnAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetInternalChnAttr(IMPVI_NUM num, IMPISPInternalChnAttr *attr)
 *
 * 获取通道属性。
 *
 * @param[in] num   对应sensor的标号
 * @param[out] attr	通道属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @remark ISP DMA通道类型只能获取，设置不会生效
 * @remark 使用请参阅sample-ISP-InternalChn.c
 *
 * @attention 这个函数必须要IMP_ISP_Open之前调用。
 */
int32_t IMP_ISP_GetInternalChnAttr(IMPVI_NUM num, IMPISPInternalChnAttr *attr);

#define MAXSUPCHNMUN 1  /*ISPOSD最大支持一个通道*/
#define MAXISPOSDPIC 8  /*ISPOSD每个通道支持绘制的最大图片个数,目前最大只支持8个*/

/**
 * 区域状态
 */
typedef enum {
	IMP_ISP_OSD_RGN_FREE,   /*ISPOSD区域未创建或者释放*/
	IMP_ISP_OSD_RGN_BUSY,   /*ISPOSD区域已创建*/
}IMPIspOsdRngStat;

/**
 * 模式选择
 */
typedef enum {
	ISP_OSD_REG_INV       = 0, /**< 未定义的 */
	ISP_OSD_REG_PIC       = 1, /**< ISP绘制图片*/
}IMPISPOSDType;
typedef struct IMPISPOSDNode IMPISPOSDNode;

/**
   * 填充功能通道属性
    */
typedef struct {
	IMPISPOSDAttr chnOSDAttr;					  /**< 填充功能通道属性 */
	IMPISPOSDBlockAttr pic;                       /**< 填充图片属性，每个通道最多可以填充8张图片 */
} IMPISPOSDSingleAttr;

/**
 * ISPOSD属性集合
 */
typedef struct {
	IMPISPOSDType type;
	union {
	IMPISPOSDSingleAttr stsinglepicAttr;/*pic 类型的ISPOSD*/
	};
}IMPIspOsdAttrAsm;

/**
 * @fn int IMP_ISP_Tuning_SetOsdPoolSize(int size)
 *
 * 创建ISPOSD使用的rmem内存大小
 *
 * @param[in]
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_SetOsdPoolSize(int size);

/**
 * @fn int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 创建ISPOSD区域
 *
 * @param[in] chn通道号，IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_CreateOsdRgn(int chn,IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 设置ISPOSD 通道区域的属性
 *
 * @param[in] chn通道号，handle号 IMPIspOsdAttrAsm 结构体指针
 *
 * @retval 0 成功
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_SetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr)
 *
 * 获取ISPOSD 通道号中的区域属性
 *
 * @param[in] chn 通道号，handle号，IMPOSDRgnCreateStat 结构体指针
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_GetOsdRgnAttr(int chn,int handle, IMPIspOsdAttrAsm *pIspOsdAttr);

/**
 * @fn int IMP_ISP_Tuning_ShowOsdRgn( int chn,int handle, int showFlag)
 *
 * 设置ISPOSD通道号中的handle对应的显示状态
 *
 * @param[in] chn通道号，handle号，showFlag显示状态(0:关闭显示，1:开启显示)
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_ShowOsdRgn(int chn,int handle, int showFlag);

/**
 * @fn int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle)
 *
 * 销毁通道中对应的handle节点
 *
 * @param[in] chn通道号，handle号
 *
 * @retval 0 成功
 * @retval 非0 失败
 *
 * @remarks 无。
 *
 * @attention 无。
 */
int IMP_ISP_Tuning_DestroyOsdRgn(int chn,int handle);

/**
 * @fn int32_t IMP_ISP_SetVicDoneCbFunc(void (*cb)(void))
 *
 * 当isp vic每完成一帧数据，设置的回调函数会被执行。
 *
 * @param[in] cb 回调函数。
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetVicDoneCbFunc(void (*cb)(void));

typedef enum {
	IMPISP_TUNING_CSCCR_DEFAULT_MODE,    /**< 默认模式*/
	IMPISP_TUNING_CSCCR_STRETCH_MODE,    /**< STRETCH模式*/
	IMPISP_TUNING_CSCCR_COMPRESS_MODE,   /**< COMPRESS模式*/
	IMPISP_TUNING_CSCCR_CUSTOMER_MODE,   /**< 自定义模式*/
} IMPISPCsccrMode;

typedef struct{
	uint8_t input_y_min;
	uint8_t input_y_max;
	uint8_t output_y_min;
	uint8_t output_y_max;
	uint8_t input_uv_min;
	uint8_t input_uv_max;
	uint8_t output_uv_min;
	uint8_t output_uv_max;
}IMPISPCsccrValue;

typedef struct {
	IMPISPCsccrMode mode;
	IMPISPCsccrValue parameters;
}IMPISPCsccrModeAttr;


/**
 * @fn int32_t IMP_ISP_SetCsccrMode(IMPVI_NUM, IMPISPCscrModeAttr *attr);
 *
 * 设置CSCR的工作模式（注意：parameters只有在自定义模式下才需要配置，其他模式不需要去设置参数）
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr	通道属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_SetCsccrMode(IMPVI_NUM num, IMPISPCsccrModeAttr *attr);

/**
 * @fn int32_t IMP_ISP_GetCsccrMode(IMPVI_NUM, IMPISPCscrModeAttr *attr);
 *
 * @param[in] num   对应sensor的标号
 * @param[in] attr	通道属性
 *
 * @retval 0 成功
 * @retval 非0 失败，返回错误码
 *
 * @attention 在使用这个函数之前，IMP_ISP_EnableTuning已被调用。
 */
int32_t IMP_ISP_GetCsccrMode(IMPVI_NUM num, IMPISPCsccrModeAttr *attr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
 * @}
 */

#endif /* __IMP_ISP_H__ */
