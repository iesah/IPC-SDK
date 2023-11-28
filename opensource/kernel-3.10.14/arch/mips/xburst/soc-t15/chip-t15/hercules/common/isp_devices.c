#include <mach/tx_isp.h>

#include "board_base.h"
struct tx_isp_subdev_platform_data tx_isp_subdev_video_in = {
	.grp_id = TX_ISP_VIDEO_IN_GRP_IDX,
	.clks = NULL,
	.clk_num = 0,
};
/*
	the platform data of CSI.
	the last unit should be NULL.
*/
struct tx_isp_subdev_clk_info tx_csi_clk_info[] = {
	{"csi", DUMMY_CLOCK_RATE},
};
struct tx_isp_subdev_platform_data tx_isp_subdev_csi = {
	.grp_id = TX_ISP_CSI_GRP_IDX,
	.clks = tx_csi_clk_info,
	.clk_num = ARRAY_SIZE(tx_csi_clk_info),
};
/*
	the platform data of VIC.
	the last unit should be NULL.
*/
struct tx_isp_subdev_platform_data tx_isp_subdev_vic = {
	.grp_id = TX_ISP_VIC_GRP_IDX,
	.clks = NULL,
	.clk_num = 0,
};
/*
	the platform data of ISP.
	the last unit should be NULL.
*/
struct tx_isp_subdev_clk_info isp_core_clk_info[] = {
	{"cgu_isp", 60000000},
	{"isp", DUMMY_CLOCK_RATE},
};
struct tx_isp_subdev_platform_data tx_isp_subdev_core = {
	.grp_id = TX_ISP_CORE_GRP_IDX,
	.clks = isp_core_clk_info,
	.clk_num = ARRAY_SIZE(isp_core_clk_info),
};
