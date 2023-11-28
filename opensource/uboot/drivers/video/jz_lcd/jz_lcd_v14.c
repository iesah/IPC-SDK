/*
 * x1830/X2000 LCDC DRIVER
 *
 * Copyright (c) 2014 Ingenic Semiconductor Co.,Ltd
 * Author: Huddy <hyli@ingenic.cn>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/io.h>
#include <config.h>
#include <serial.h>
#include <common.h>
#include <lcd.h>
/*#include <asm/arch/lcdc.h>*/
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <jz_lcd/jz_lcd_v14.h>
#include "dpu_reg.h"

#include <asm/gpio.h>

#define DEBUG
#ifdef CONFIG_JZ_MIPI_DSI
#include <jz_lcd/jz_dsim.h>
#include "./jz_mipi_dsi/jz_mipi_dsi_regs.h"
#include "./jz_mipi_dsi/jz_mipi_dsih_hal.h"
struct dsi_device *dsi;
extern struct dsi_device jz_dsi;
struct dsi_phy dsi_phy;
void jz_dsi_init(struct dsi_device *dsi);
int jz_dsi_video_cfg(struct dsi_device *dsi);
#endif

struct jzfb_config_info lcd_config_info;
static int lcd_enable_state = 0;
void board_set_lcd_power_on(void);
void flush_cache_all(void);
/* void lcd_close_backlight(void); */
/* void lcd_set_backlight_level(int num); */
#define fb_write(addr,config)				\
	writel(config,DPU_BASE+addr)
#define fb_read(addr)				\
	readl(DPU_BASE+addr)

#ifdef DEBUG

void dump_dsi_reg(struct dsi_device *dsi)
{
	printf( "-----------dump dsi reg------------\n");
	printf( "VERSION------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VERSION));
	printf( "PWR_UP:------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PWR_UP));
	printf( "CLKMGR_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CLKMGR_CFG));
	printf( "DPI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID));
	printf( "DPI_COLOR_CODING---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING));
	printf( "DPI_CFG_POL--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_CFG_POL));
	printf( "DPI_LP_CMD_TIM-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM));
	printf( "DBI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_VCID));
	printf( "DBI_CFG------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CFG));
	printf( "DBI_PARTITIONING_EN:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_PARTITIONING_EN));
	printf( "DBI_CMDSIZE--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CMDSIZE));
	printf( "PCKHDL_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PCKHDL_CFG));
	printf( "GEN_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_VCID));
	printf( "MODE_CFG-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_MODE_CFG));
	printf( "VID_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG));
	printf( "VID_PKT_SIZE-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE));
	printf( "VID_NUM_CHUNKS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS));
	printf( "VID_NULL_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NULL_SIZE));
	printf( "VID_HSA_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME));
	printf( "VID_HBP_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME));
	printf( "VID_HLINE_TIME-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME));
	printf( "VID_VSA_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES));
	printf( "VID_VBP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES));
	printf( "VID_VFP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES));
	printf( "VID_VACTIVE_LINES--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES));
	printf( "EDPI_CMD_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE));
	printf( "CMD_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_MODE_CFG));
	printf( "GEN_HDR------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_HDR));
	printf( "GEN_PLD_DATA-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_PLD_DATA));
	printf( "CMD_PKT_STATUS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_PKT_STATUS));
	printf( "TO_CNT_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_TO_CNT_CFG));
	printf( "HS_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_RD_TO_CNT));
	printf( "LP_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_RD_TO_CNT));
	printf( "HS_WR_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_WR_TO_CNT));
	printf( "LP_WR_TO_CNT_CFG---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_WR_TO_CNT));
	printf( "BTA_TO_CNT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_BTA_TO_CNT));
	printf( "SDF_3D-------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D));
	printf( "LPCLK_CTRL---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LPCLK_CTRL));
	printf( "PHY_TMR_LPCLK_CFG--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_LPCLK_CFG));
	printf( "PHY_TMR_CFG--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_CFG));
	printf( "PHY_RSTZ-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_RSTZ));
	printf( "PHY_IF_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_IF_CFG));
	printf( "PHY_ULPS_CTRL------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_ULPS_CTRL));
	printf( "PHY_TX_TRIGGERS----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TX_TRIGGERS));
	printf( "PHY_STATUS---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS));
	printf( "PHY_TST_CTRL0------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL0));
	printf( "PHY_TST_CTRL1------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL1));
	printf( "INT_ST0------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST0));
	printf( "INT_ST1------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST1));
	printf( "INT_MSK0-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK0));
	printf( "INT_MSK1-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK1));
	printf( "INT_FORCE0---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE0));
	printf( "INT_FORCE1---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE1));
	printf( "VID_SHADOW_CTRL----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_SHADOW_CTRL));
	printf( "DPI_VCID_ACT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID_ACT));
	printf( "DPI_COLOR_CODING_AC:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING_ACT));
	printf( "DPI_LP_CMD_TIM_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM_ACT));
	printf( "VID_MODE_CFG_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG_ACT));
	printf( "VID_PKT_SIZE_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE_ACT));
	printf( "VID_NUM_CHUNKS_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS_ACT));
	printf( "VID_HSA_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME_ACT));
	printf( "VID_HBP_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME_ACT));
	printf( "VID_HLINE_TIME_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME_ACT));
	printf( "VID_VSA_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES_ACT));
	printf( "VID_VBP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES_ACT));
	printf( "VID_VFP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES_ACT));
	printf( "VID_VACTIVE_LINES_ACT:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES_ACT));
	printf( "SDF_3D_ACT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D_ACT));
}
static void dump_dc_reg(void)
{
	printf("-----------------dc_reg------------------\n");
	printf("DC_FRM_CFG_ADDR:    %lx\n",fb_read(DC_FRM_CFG_ADDR));
	printf("DC_FRM_CFG_CTRL:    %lx\n",fb_read(DC_FRM_CFG_CTRL));
	printf("DC_CTRL:            %lx\n",fb_read(DC_CTRL));
	printf("DC_LAYER0_CSC_MULT_YRV:    %lx\n",fb_read(DC_LAYER0_CSC_MULT_YRV));
	printf("DC_LAYER0_CSC_MULT_GUGV:   %lx\n",fb_read(DC_LAYER0_CSC_MULT_GUGV));
	printf("DC_LAYER0_CSC_MULT_BU:     %lx\n",fb_read(DC_LAYER0_CSC_MULT_BU));
	printf("DC_LAYER0_CSC_SUB_YUV:     %lx\n",fb_read(DC_LAYER0_CSC_SUB_YUV));
	printf("DC_LAYER1_CSC_MULT_YRV:    %lx\n",fb_read(DC_LAYER1_CSC_MULT_YRV));
	printf("DC_LAYER1_CSC_MULT_GUGV:   %lx\n",fb_read(DC_LAYER1_CSC_MULT_GUGV));
	printf("DC_LAYER1_CSC_MULT_BU:     %lx\n",fb_read(DC_LAYER1_CSC_MULT_BU));
	printf("DC_LAYER1_CSC_SUB_YUV:     %lx\n",fb_read(DC_LAYER1_CSC_SUB_YUV));
	printf("DC_ST:              %lx\n",fb_read(DC_ST));
	printf("DC_INTC:            %lx\n",fb_read(DC_INTC));
	printf("DC_INT_FLAG:	    %lx\n",fb_read(DC_INT_FLAG));
	printf("DC_COM_CONFIG:      %lx\n",fb_read(DC_COM_CONFIG));
	printf("DC_PCFG_RD_CTRL:    %lx\n",fb_read(DC_PCFG_RD_CTRL));
	printf("DC_OFIFO_PCFG:	    %lx\n",fb_read(DC_OFIFO_PCFG));
	printf("DC_DISP_COM:        %lx\n",fb_read(DC_DISP_COM));
	printf("-----------------dc_reg------------------\n");
}

static void dump_tft_reg(void)
{
	printf("----------------tft_reg------------------\n");
	printf("TFT_TIMING_HSYNC:   %lx\n",fb_read(DC_TFT_HSYNC));
	printf("TFT_TIMING_VSYNC:   %lx\n",fb_read(DC_TFT_VSYNC));
	printf("TFT_TIMING_HDE:     %lx\n",fb_read(DC_TFT_HDE));
	printf("TFT_TIMING_VDE:     %lx\n",fb_read(DC_TFT_VDE));
	printf("TFT_TRAN_CFG:       %lx\n",fb_read(DC_TFT_CFG));
	printf("TFT_ST:             %lx\n",fb_read(DC_TFT_ST));
	printf("----------------tft_reg------------------\n");
}

static void dump_slcd_reg(void)
{
	printf("---------------slcd_reg------------------\n");
	printf("SLCD_CFG:           %lx\n",fb_read(DC_SLCD_CFG));
	printf("SLCD_WR_DUTY:       %lx\n",fb_read(DC_SLCD_WR_DUTY));
	printf("SLCD_TIMING:        %lx\n",fb_read(DC_SLCD_TIMING));
	printf("SLCD_FRM_SIZE:      %lx\n",fb_read(DC_SLCD_FRM_SIZE));
	printf("SLCD_SLOW_TIME:     %lx\n",fb_read(DC_SLCD_SLOW_TIME));
	//printf("SLCD_CMD:           %lx\n",fb_read(DC_SLCD_CMD));
	printf("SLCD_ST:            %lx\n",fb_read(DC_SLCD_ST));
	printf("---------------slcd_reg------------------\n");
}

static void dump_frm_desc_reg(void)
{
	unsigned int ctrl;
	ctrl = fb_read(DC_CTRL);
	ctrl |= DC_DES_CNT_RST;
	fb_write(DC_CTRL, ctrl);

	printf("--------Frame Descriptor register--------\n");
	printf("FrameNextCfgAddr:   %lx\n",fb_read(DC_FRM_DES));
	printf("FrameSize:          %lx\n",fb_read(DC_FRM_DES));
	printf("FrameCtrl:          %lx\n",fb_read(DC_FRM_DES));
	printf("WritebackAddr:      %lx\n",fb_read(DC_FRM_DES));
	printf("WritebackStride:    %lx\n",fb_read(DC_FRM_DES));
	printf("Layer0CfgAddr:      %lx\n",fb_read(DC_FRM_DES));
	printf("Layer1CfgAddr:      %lx\n",fb_read(DC_FRM_DES));
	printf("Layer2CfgAddr:      %lx\n",fb_read(DC_FRM_DES));
	printf("Layer3CfgAddr:      %lx\n",fb_read(DC_FRM_DES));
	printf("LayCfgEn:	    %lx\n",fb_read(DC_FRM_DES));
	printf("InterruptControl:   %lx\n",fb_read(DC_FRM_DES));
	printf("--------Frame Descriptor register--------\n");
}

static void dump_layer_desc_reg(void)
{
	unsigned int ctrl;
	ctrl = fb_read(DC_CTRL);
	ctrl |= DC_DES_CNT_RST;
	fb_write(DC_CTRL, ctrl);

	printf("--------layer0 Descriptor register-------\n");
	printf("LayerSize:          %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerCfg:           %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerBufferAddr:    %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerScale:         %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerRotation:      %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerScratch:       %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerPos:           %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerResizeCoef_X:  %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerResizeCoef_Y:  %lx\n",fb_read(DC_LAY0_DES));
	printf("LayerStride:        %lx\n",fb_read(DC_LAY0_DES));
	printf("BufferAddr_UV:      %lx\n",fb_read(DC_LAY0_DES));
	printf("stride_UV:	    %lx\n",fb_read(DC_LAY0_DES));
	printf("--------layer0 Descriptor register-------\n");

	printf("--------layer1 Descriptor register-------\n");
	printf("LayerSize:          %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerCfg:           %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerBufferAddr:    %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerScale:         %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerRotation:      %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerScratch:       %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerPos:           %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerResizeCoef_X:  %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerResizeCoef_Y:  %lx\n",fb_read(DC_LAY1_DES));
	printf("LayerStride:        %lx\n",fb_read(DC_LAY1_DES));
	printf("BufferAddr_UV:      %lx\n",fb_read(DC_LAY1_DES));
	printf("stride_UV:	    %lx\n",fb_read(DC_LAY1_DES));
	printf("--------layer1 Descriptor register-------\n");
}

static void dump_frm_desc(struct jzfb_framedesc *framedesc, int index)
{
	printf("-------User Frame Descriptor index[%d]-----\n", index);
	printf("FramedescAddr:	    0x%x\n",(uint32_t)framedesc);
	printf("FrameNextCfgAddr:   0x%x\n",framedesc->FrameNextCfgAddr);
	printf("FrameSize:          0x%x\n",framedesc->FrameSize.d32);
	printf("FrameCtrl:          0x%x\n",framedesc->FrameCtrl.d32);
	printf("Layer0CfgAddr:      0x%x\n",framedesc->Layer0CfgAddr);
	printf("Layer1CfgAddr:      0x%x\n",framedesc->Layer1CfgAddr);
	printf("LayerCfgEn:	    0x%x\n",framedesc->LayCfgEn.d32);
	printf("InterruptControl:   0x%x\n",framedesc->InterruptControl.d32);
	printf("-------User Frame Descriptor index[%d]-----\n", index);
}

static void dump_layer_desc(struct jzfb_layerdesc *layerdesc, int row, int col)
{
	printf("------User layer Descriptor index[%d][%d]------\n", row, col);
	printf("LayerdescAddr:	    0x%x\n",(uint32_t)layerdesc);
	printf("LayerSize:          0x%x\n",layerdesc->LayerSize.d32);
	printf("LayerCfg:           0x%x\n",layerdesc->LayerCfg.d32);
	printf("LayerBufferAddr:    0x%x\n",layerdesc->LayerBufferAddr);
	printf("LayerScale:         0x%x\n",layerdesc->LayerScale.d32);
	printf("LayerPos:           0x%x\n",layerdesc->LayerPos.d32);
	printf("LayerStride:        0x%x\n",layerdesc->LayerStride);
	printf("------User layer Descriptor index[%d][%d]------\n", row, col);
}

void dump_lay_cfg(struct jzfb_lay_cfg * lay_cfg, int index)
{
	printf("------User disp set index[%d]------\n", index);
	printf("lay_en:		   0x%x\n",lay_cfg->lay_en);
	printf("lay_z_order:	   0x%x\n",lay_cfg->lay_z_order);
	printf("pic_witdh:	   0x%x\n",lay_cfg->pic_width);
	printf("pic_heght:	   0x%x\n",lay_cfg->pic_height);
	printf("disp_pos_x:	   0x%x\n",lay_cfg->disp_pos_x);
	printf("disp_pos_y:	   0x%x\n",lay_cfg->disp_pos_y);
	printf("g_alpha_en:	   0x%x\n",lay_cfg->g_alpha_en);
	printf("g_alpha_val:	   0x%x\n",lay_cfg->g_alpha_val);
	printf("color:		   0x%x\n",lay_cfg->color);
	printf("format:		   0x%x\n",lay_cfg->format);
	printf("stride:		   0x%x\n",lay_cfg->stride);
	printf("------User disp set index[%d]------\n", index);
}
#endif


static void dump_lcdc_registers(void)
{
	dump_dsi_reg(dsi);
	dump_dc_reg();
	dump_tft_reg();
	dump_slcd_reg();
	dump_frm_desc_reg();
	dump_layer_desc_reg();
}

static void dump_desc(struct jzfb_config_info *info)
{
	int i, j;
	for(i = 0; i < MAX_DESC_NUM; i++) {
		for(j = 0; j < MAX_LAYER_NUM; j++) {
			dump_layer_desc(info->layerdesc[i][j], i, j);
		}
		dump_frm_desc(info->framedesc[i], i);
	}
}

struct jzfb_colormode {
	uint32_t mode;
	uint32_t color;
	uint32_t bits_per_pixel;
	uint32_t nonstd;
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
};

static struct jzfb_colormode jzfb_colormodes[] = {
	{
		.mode = LAYER_CFG_FORMAT_RGB888,
		.color = LAYER_CFG_COLOR_RGB,
		.bits_per_pixel = 32,
		.nonstd = 0,
#ifdef CONFIG_ANDROID
		.color = LAYER_CFG_COLOR_BGR,
		.red	= { .length = 8, .offset = 0, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 16, .msb_right = 0 },
#else
		.color = LAYER_CFG_COLOR_RGB,
		.red	= { .length = 8, .offset = 16, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 0, .msb_right = 0 },
#endif
		.transp	= { .length = 0, .offset = 0, .msb_right = 0 },
	}, {
		.mode = LAYER_CFG_FORMAT_ARGB8888,
		.bits_per_pixel = 32,
		.nonstd = 0,
#ifdef CONFIG_ANDROID
		.color = LAYER_CFG_COLOR_BGR,
		.red	= { .length = 8, .offset = 0, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 16, .msb_right = 0 },
#else
		.color = LAYER_CFG_COLOR_RGB,
		.red	= { .length = 8, .offset = 16, .msb_right = 0 },
		.green	= { .length = 8, .offset = 8, .msb_right = 0 },
		.blue	= { .length = 8, .offset = 0, .msb_right = 0 },
#endif
		.transp	= { .length = 8, .offset = 24, .msb_right = 0 },
	}, {
		.mode = LAYER_CFG_FORMAT_RGB555,
		.color = LAYER_CFG_COLOR_RGB,
		.bits_per_pixel = 16,
		.nonstd = 0,
		.red	= { .length = 5, .offset = 10, .msb_right = 0 },
		.green	= { .length = 5, .offset = 5, .msb_right = 0 },
		.blue	= { .length = 5, .offset = 0, .msb_right = 0 },
		.transp	= { .length = 0, .offset = 0, .msb_right = 0 },
	}, {
		.mode = LAYER_CFG_FORMAT_ARGB1555,
		.color = LAYER_CFG_COLOR_RGB,
		.bits_per_pixel = 16,
		.nonstd = 0,
		.red	= { .length = 5, .offset = 10, .msb_right = 0 },
		.green	= { .length = 5, .offset = 5, .msb_right = 0 },
		.blue	= { .length = 5, .offset = 0, .msb_right = 0 },
		.transp	= { .length = 1, .offset = 15, .msb_right = 0 },
	}, {
		.mode = LAYER_CFG_FORMAT_RGB565,
		.color = LAYER_CFG_COLOR_RGB,
		.bits_per_pixel = 16,
		.nonstd = 0,
		.red	= { .length = 5, .offset = 11, .msb_right = 0 },
		.green	= { .length = 6, .offset = 5, .msb_right = 0 },
		.blue	= { .length = 5, .offset = 0, .msb_right = 0 },
		.transp	= { .length = 0, .offset = 0, .msb_right = 0 },
	}, {
		.mode = LAYER_CFG_FORMAT_YUV422,
		.bits_per_pixel = 16,
		.nonstd = LAYER_CFG_FORMAT_YUV422,
	},
};

static void jzfb_colormode_to_var(struct fb_var_screeninfo *var,
		struct jzfb_colormode *color)
{
	var->bits_per_pixel = color->bits_per_pixel;
	var->nonstd = color->nonstd;
	var->red = color->red;
	var->green = color->green;
	var->blue = color->blue;
	var->transp = color->transp;
}

static bool cmp_var_to_colormode(struct fb_var_screeninfo *var,
		struct jzfb_colormode *color)
{
	bool cmp_component(struct fb_bitfield *f1, struct fb_bitfield *f2)
	{
		return f1->length == f2->length &&
			f1->offset == f2->offset &&
			f1->msb_right == f2->msb_right;
	}

	if (var->bits_per_pixel == 0 ||
			var->red.length == 0 ||
			var->blue.length == 0 ||
			var->green.length == 0)
		return 0;

	return var->bits_per_pixel == color->bits_per_pixel &&
		cmp_component(&var->red, &color->red) &&
		cmp_component(&var->green, &color->green) &&
		cmp_component(&var->blue, &color->blue) &&
		cmp_component(&var->transp, &color->transp);
}

struct jzfb_framedesc g_framedesc[MAX_DESC_NUM] __attribute__((aligned(256)));
struct jzfb_layerdesc g_layerdesc[MAX_DESC_NUM][MAX_LAYER_NUM] __attribute__((aligned(256)));

static int jzfb_alloc_devmem(struct jzfb_config_info *info)
{
	int i, j;
	for(i = 0; i < MAX_DESC_NUM; i++) {
		/* uncached addr*/
		info->framedesc[i] = ((unsigned int)&g_framedesc[i]) | 0xa0000000;
		/* phys addr */
		info->framedesc_phys[i] = virt_to_phys((unsigned int)info->framedesc[i]);

		for(j = 0; j < MAX_LAYER_NUM; j++) {
			info->layerdesc[i][j] = ((unsigned int)&g_layerdesc[i][j]) | 0xa0000000;
			info->layerdesc_phys[i][j] = virt_to_phys((unsigned int)info->layerdesc[i][j]);

			info->vidmem[i][j] = info->screen;
			info->vidmem_phys[i][j] = virt_to_phys((unsigned int)info->screen);
		}
	}
}

static int jzfb_check_frm_cfg(struct jzfb_config_info *info, struct jzfb_frm_cfg *frm_cfg)
{
	struct jzfb_lay_cfg *lay_cfg;
	struct fb_videomode *mode = info->modes;

	lay_cfg = frm_cfg->lay_cfg;

	if((!lay_cfg[0].lay_en) ||
	   (lay_cfg[0].lay_z_order != 1) ||
	   (lay_cfg[0].lay_z_order == lay_cfg[1].lay_z_order) ||
	   (lay_cfg[0].pic_width > mode->xres) ||
	   (lay_cfg[0].pic_width == 0) ||
	   (lay_cfg[0].pic_height > mode->yres) ||
	   (lay_cfg[0].pic_height == 0) ||
	   (lay_cfg[0].disp_pos_x > mode->xres) ||
	   (lay_cfg[0].disp_pos_y > mode->yres) ||
	   (lay_cfg[0].color > LAYER_CFG_COLOR_BGR) ||
	   (lay_cfg[0].stride > 4096)) {
		printf("%s frame[0] cfg value is err!\n",__func__);
		return -22;
	}

	switch (lay_cfg[0].format) {
	case LAYER_CFG_FORMAT_RGB555:
	case LAYER_CFG_FORMAT_ARGB1555:
	case LAYER_CFG_FORMAT_RGB565:
	case LAYER_CFG_FORMAT_RGB888:
	case LAYER_CFG_FORMAT_ARGB8888:
		break;
	default:
		printf("%s frame[0] cfg value is err!\n",__func__);
		return -22;
	}

	if(lay_cfg[1].lay_en) {
		if((lay_cfg[1].lay_z_order != 0) ||
		   (lay_cfg[1].pic_width > mode->xres) ||
		   (lay_cfg[1].pic_width == 0) ||
		   (lay_cfg[1].pic_height > mode->yres) ||
		   (lay_cfg[1].pic_height == 0) ||
		   (lay_cfg[1].disp_pos_x > mode->xres) ||
		   (lay_cfg[1].disp_pos_y > mode->yres) ||
		   (lay_cfg[1].color > LAYER_CFG_COLOR_BGR) ||
		   (lay_cfg[1].stride > 4096)) {
			printf("%s frame[1] cfg value is err!\n",__func__);
			return -22;
		}
		switch (lay_cfg[1].format) {
		case LAYER_CFG_FORMAT_RGB555:
		case LAYER_CFG_FORMAT_ARGB1555:
		case LAYER_CFG_FORMAT_RGB565:
		case LAYER_CFG_FORMAT_RGB888:
		case LAYER_CFG_FORMAT_ARGB8888:
			break;
		default:
			printf("%s frame[0] cfg value is err!\n",__func__);
			return -22;
		}
	}

	return 0;
}
	/*unsigned int wb_buf[1024];*/

static int jzfb_desc_init(struct jzfb_config_info *info, int frm_num)
{
	struct fb_videomode *mode = info->modes;
	struct jzfb_frm_mode *frm_mode;
	struct jzfb_frm_cfg *frm_cfg;
	struct jzfb_lay_cfg *lay_cfg;
	struct jzfb_framedesc **framedesc;
	int frm_num_mi, frm_num_ma;
	int frm_size;
	int i, j;
	int ret = 0;

	framedesc = info->framedesc;
	frm_mode = &info->current_frm_mode;
	frm_cfg = &frm_mode->frm_cfg;
	lay_cfg = frm_cfg->lay_cfg;

	ret = jzfb_check_frm_cfg(info, frm_cfg);
	if(ret) {
		printf("%s configure framedesc[%d] error!\n", __func__, frm_num);
		return ret;
	}

	if(frm_num == FRAME_CFG_ALL_UPDATE) {
		frm_num_mi = 0;
		frm_num_ma = MAX_DESC_NUM;
	} else {
		if(frm_num < 0 || frm_num > MAX_DESC_NUM) {
			printf("framedesc num err!\n");
			return -22;
		}
		frm_num_mi = frm_num;
		frm_num_ma = frm_num + 1;
	}

	for(i = frm_num_mi; i < frm_num_ma; i++) {
		framedesc[i]->FrameNextCfgAddr = info->framedesc_phys[i];
		framedesc[i]->FrameSize.b.width = mode->xres;
		framedesc[i]->FrameSize.b.height = mode->yres;
		framedesc[i]->FrameCtrl.d32 = FRAME_CTRL_DEFAULT_SET ;
		framedesc[i]->Layer0CfgAddr = info->layerdesc_phys[i][0];
		framedesc[i]->Layer1CfgAddr = info->layerdesc_phys[i][1];
		framedesc[i]->LayCfgEn.b.lay0_en = lay_cfg[0].lay_en;
		framedesc[i]->LayCfgEn.b.lay1_en = lay_cfg[1].lay_en;
		framedesc[i]->LayCfgEn.b.lay0_z_order = lay_cfg[0].lay_z_order;
		framedesc[i]->LayCfgEn.b.lay1_z_order = lay_cfg[1].lay_z_order;
		/*framedesc[i]->WritebackAddr = wb_buf;
		framedesc[i]->FrameCtrl.b.bit1_keep0 = 1;  writeback*/

		framedesc[i]->FrameCtrl.b.stop = 0;
		framedesc[i]->InterruptControl.d32 = DC_SOS_MSK;
	}

	frm_size = mode->xres * mode->yres;
	info->frm_size = frm_size * info->var.bits_per_pixel >> 3;
	info->screen_size = info->frm_size;

	for(i = frm_num_mi; i < frm_num_ma; i++) {
		for(j =0; j < MAX_LAYER_NUM; j++) {
			if(!lay_cfg[j].lay_en)
				continue;
			info->layerdesc[i][j]->LayerSize.b.width = lay_cfg[j].pic_width;
			info->layerdesc[i][j]->LayerSize.b.height = lay_cfg[j].pic_height;
			info->layerdesc[i][j]->LayerPos.b.x_pos = lay_cfg[j].disp_pos_x;
			info->layerdesc[i][j]->LayerPos.b.y_pos = lay_cfg[j].disp_pos_y;
			info->layerdesc[i][j]->LayerCfg.b.g_alpha_en = lay_cfg[j].g_alpha_en;
			info->layerdesc[i][j]->LayerCfg.b.g_alpha = lay_cfg[j].g_alpha_val;
			info->layerdesc[i][j]->LayerCfg.b.color = lay_cfg[j].color;
			info->layerdesc[i][j]->LayerCfg.b.domain_multi = lay_cfg[j].domain_multi;
			info->layerdesc[i][j]->LayerCfg.b.format = lay_cfg[j].format;
			info->layerdesc[i][j]->LayerStride = lay_cfg[j].stride;

			info->vidmem_phys[i][j] = info->vidmem_phys[0][0] +
						   i * info->frm_size +
						   j * info->frm_size * MAX_DESC_NUM;
			info->vidmem[i][j] = info->vidmem[0][0] +
						   i * info->frm_size +
						   j * info->frm_size * MAX_DESC_NUM;
			info->layerdesc[i][j]->LayerBufferAddr = info->vidmem_phys[i][j];
			info->layerdesc[i][j]->BufferAddr_UV = info->vidmem_phys[i][j] + lay_cfg[j].pic_width * lay_cfg[j].pic_height;
			info->layerdesc[i][j]->stride_UV = lay_cfg[j].stride;

		}

	}

	for(i = frm_num_mi; i < frm_num_ma; i++) {
		frm_mode->update_st[i] = FRAME_CFG_UPDATE;
	}

	return 0;
}

#if defined(CONFIG_LCD_LOGO)
static void fbmem_set(void *_ptr, unsigned short val, unsigned count)
{
	int bpp = NBITS(panel_info.vl_bpix);
	if(bpp == 16){
		unsigned short *ptr = _ptr;

		while (count--)
			*ptr++ = val;
	} else if (bpp == 32){
		int val_32;
		int alpha, rdata, gdata, bdata;
		unsigned int *ptr = (unsigned int *)_ptr;

		alpha = 0xff;

		rdata = val >> 11;
		rdata = (rdata << 3) | 0x7;

		gdata = (val >> 5) & 0x003F;
		gdata = (gdata << 2) | 0x3;

		bdata = val & 0x001F;
		bdata = (bdata << 3) | 0x7;

		if (lcd_config_info.fmt_order == FORMAT_X8B8G8R8) {
			val_32 =
				(alpha << 24) | (bdata << 16) | (gdata << 8) | rdata;
			/*fixed */
		} else if (lcd_config_info.fmt_order == FORMAT_X8R8G8B8) {
			val_32 =
				(alpha << 24) | (rdata << 16) | (gdata << 8) | bdata;
		}

		while (count--){
			*ptr++ = val_32;
		}
	}
}
/* 565RLE image format: [count(2 bytes), rle(2 bytes)] */
void rle_plot_biger(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height;
	unsigned int write_count;
	int flag_bit = (bpp == 16) ? 1 : 2;
	unsigned short *photo_ptr = (unsigned short *)src_buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;

	write_count = photo_ptr[0];
	while (photo_size > 0) {
		while (dis_height > 0) {
			dis_width = photo_width < vm_width ? photo_width : vm_width;
			while (dis_width > 0) {
				if (photo_size < 0)
					break;
				fbmem_set(lcd_fb, photo_ptr[1], write_count);
				lcd_fb += write_count * flag_bit;
				photo_ptr += 2;
				photo_size -= 2;
				write_count = photo_ptr[0];
				dis_width -= write_count;
			}
			if (dis_width < 0) {
				photo_ptr -= 2;
				photo_size += 2;
				lcd_fb += dis_width * flag_bit;
				write_count = - dis_width;
			}
			int extra_width = photo_width - vm_width;
			while (extra_width > 0) {
				photo_ptr += 2;
				photo_size -= 2;
				write_count = photo_ptr[0];
				extra_width -= write_count;
			}
			if (extra_width < 0) {
				photo_ptr -= 2;
				photo_size += 2;
				write_count = -extra_width;
			}
			dis_height -= 1;
		}
		if (dis_height <= 0)
			break;
	}
	return;
}

#ifdef CONFIG_LOGO_EXTEND
#ifndef  CONFIG_PANEL_DIV_LOGO
#define  CONFIG_PANEL_DIV_LOGO   (3)
#endif
void rle_plot_extend(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short compress_tmp_count, compress_tmp_val;
	unsigned short *photo_tmp_ptr;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int rate0, rate1, extend_rate;
	int i, 	test;

	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	rate0 =  vm_width/photo_ptr[0] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[0])/CONFIG_PANEL_DIV_LOGO : 1;
	rate1 =  vm_height/photo_ptr[1] > CONFIG_PANEL_DIV_LOGO ? (vm_width/photo_ptr[1])/CONFIG_PANEL_DIV_LOGO : 1;
	extend_rate = rate0 < rate1 ?  rate0 : rate1;
	debug("logo extend rate = %d\n", extend_rate);

	photo_width = photo_ptr[0] * extend_rate;
	photo_height = photo_ptr[1] * extend_rate;
	photo_size = ( photo_ptr[3] << 16 | photo_ptr[2]) * extend_rate; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = compress_tmp_count= photo_ptr[0] * extend_rate;
	compress_val = compress_tmp_val= photo_ptr[1];
	photo_tmp_ptr = photo_ptr;
	write_count = 0;
	debug("0> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
	while (photo_size > 0) {
		if (eheight > 0) {
			lcd_fb += eheight * vm_width * flag_bit;
		}
		while (dis_height > 0) {
			for(i = 0; i < extend_rate && dis_height > 0; i++){
				debug("I am  here\n");
				photo_ptr = photo_tmp_ptr;
				debug("1> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
				compress_count = compress_tmp_count;
				compress_val = compress_tmp_val;

				dis_width = photo_width < vm_width ? photo_width : vm_width;
				if (ewidth > 0) {
					lcd_fb += ewidth*flag_bit;
				}
				while (dis_width > 0) {
					if (photo_size < 0)
						break;
					write_count = compress_count;
					if (write_count > dis_width)
						write_count = dis_width;

					fbmem_set(lcd_fb, compress_val, write_count);
					lcd_fb += write_count * flag_bit;
					if (compress_count > write_count) {
						compress_count = compress_count - write_count;
					} else {
						photo_ptr += 2;
						photo_size -= 2;
						compress_count = photo_ptr[0] * extend_rate;
						compress_val = photo_ptr[1];
					}

					dis_width -= write_count;
				}

				if (ewidth > 0) {
					lcd_fb += ewidth * flag_bit;
				} else {
					int xwidth = -ewidth;
					while (xwidth > 0) {
						unsigned write_count = compress_count;

						if (write_count > xwidth)
							write_count = xwidth;

						if (compress_count > write_count) {
							compress_count = compress_count - write_count;
						} else {
							photo_ptr += 2;
							photo_size -= 2;
							compress_count = photo_ptr[0];
							compress_val = photo_ptr[1];
						}
						xwidth -= write_count;
					}

				}
				dis_height -= 1;
			}
			photo_tmp_ptr = photo_ptr;
			debug("2> photo_ptr = %08x, photo_tmp_ptr = %08x\n", photo_ptr, photo_tmp_ptr);
			compress_tmp_count = compress_count;
			compress_tmp_val = compress_val;
		}

		if (eheight > 0) {
			lcd_fb += eheight * vm_width *flag_bit;
		}
		if (dis_height <= 0)
			return;
	}
	return;
}
#endif

void rle_plot_smaller(unsigned short *src_buf, unsigned short *dst_buf, int bpp)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int dis_width, dis_height, ewidth, eheight;
	unsigned short compress_count, compress_val, write_count;
	unsigned short *photo_ptr;
	unsigned short *lcd_fb;
	int flag_bit = 1;
	if(bpp == 16){
		flag_bit = 1;
	}else if(bpp == 32){
		flag_bit = 2;
	}

	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;
	photo_ptr = (unsigned short *)src_buf;
	lcd_fb = (unsigned short *)dst_buf;

	photo_width = photo_ptr[0];
	photo_height = photo_ptr[1];
	photo_size = photo_ptr[3] << 16 | photo_ptr[2]; 	//photo size
	debug("photo_size =%d photo_width = %d, photo_height = %d\n", photo_size,
	      photo_width, photo_height);
	photo_ptr += 4;

	dis_height = photo_height < vm_height ? photo_height : vm_height;
	ewidth = (vm_width - photo_width)/2;
	eheight = (vm_height - photo_height)/2;
	compress_count = photo_ptr[0];
	compress_val = photo_ptr[1];
	write_count = 0;
	while (photo_size > 0) {
		if (eheight > 0) {
			lcd_fb += eheight * vm_width * flag_bit;
		}
		while (dis_height > 0) {
			dis_width = photo_width < vm_width ? photo_width : vm_width;
			if (ewidth > 0) {
				lcd_fb += ewidth*flag_bit;
			}
			while (dis_width > 0) {
				if (photo_size < 0)
					break;
				write_count = compress_count;
				if (write_count > dis_width)
					write_count = dis_width;

				fbmem_set(lcd_fb, compress_val, write_count);
				lcd_fb += write_count * flag_bit;
				if (compress_count > write_count) {
					compress_count = compress_count - write_count;
				} else {
					photo_ptr += 2;
					photo_size -= 2;
					compress_count = photo_ptr[0];
					compress_val = photo_ptr[1];
				}

				dis_width -= write_count;
			}

			if (ewidth > 0) {
				lcd_fb += ewidth * flag_bit;
			} else {
				int xwidth = -ewidth;
				while (xwidth > 0) {
					unsigned write_count = compress_count;

					if (write_count > xwidth)
						write_count = xwidth;

					if (compress_count > write_count) {
						compress_count = compress_count - write_count;
					} else {
						photo_ptr += 2;
						photo_size -= 2;
						compress_count = photo_ptr[0];
						compress_val = photo_ptr[1];
					}
					xwidth -= write_count;
				}

			}
			dis_height -= 1;
		}

		if (eheight > 0) {
			lcd_fb += eheight * vm_width *flag_bit;
		}
		if (dis_height <= 0)
			return;
	}
	return;
}

void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
	int vm_width, vm_height;
	int photo_width, photo_height, photo_size;
	int flag;
	int bpp;


	unsigned short *photo_ptr = (unsigned short *)buf;
	unsigned short *lcd_fb = (unsigned short *)dst_buf;
	bpp = NBITS(panel_info.vl_bpix);
	vm_width = panel_info.vl_col;
	vm_height = panel_info.vl_row;

	flag =  photo_ptr[0] * photo_ptr[1] - vm_width * vm_height;
	if(flag <= 0){
#ifdef CONFIG_LOGO_EXTEND
		rle_plot_extend(photo_ptr, lcd_fb, bpp);
#else
		rle_plot_smaller(photo_ptr, lcd_fb, bpp);
#endif
	}else if(flag > 0){
		rle_plot_biger(photo_ptr, lcd_fb, bpp);
	}
	return;
}

#else
void rle_plot(unsigned short *buf, unsigned char *dst_buf)
{
}
#endif
void fb_fill(void *logo_addr, void *fb_addr, int count)
{
	//memcpy(logo_buf, fb_addr, count);
	int i;
	int *dest_addr = (int *)fb_addr;
	int *src_addr = (int *)logo_addr;

	for(i = 0; i < count; i = i + 4){
		*dest_addr =  *src_addr;
		src_addr++;
		dest_addr++;
	}

}

static void jzfb_cmp_start()
{
	if(!(fb_read(DC_ST) & DC_FRM_WORKING)) {
		fb_write(DC_FRM_CFG_CTRL, DC_FRM_START);
		printf("Composer  enabled.\n");
	} else {
		printf("Composer has enabled.\n");
	}
}

static void jzfb_tft_start(void)
{

	if(!(fb_read(DC_TFT_ST) & DC_TFT_WORKING)) {
		fb_write(DC_CTRL, DC_TFT_START);
	} else {
		printf("TFT has enabled.\n");
	}
}

static int wait_dc_state(uint32_t state, uint32_t flag)
{
	unsigned long timeout = 20000;
	while(((!(fb_read(DC_ST) & state)) == flag) && timeout) {
		timeout--;
		udelay(10);
	}

	if(timeout <= 0) {
		printf("LCD wait state timeout! state = %d, DC_ST = 0x%x\n", state, DC_ST);
		return -1;
	}
	return 0;
}


static void tft_timing_init(struct fb_videomode *modes) {
	uint32_t hps, hpe, vps, vpe;
	uint32_t hds, hde, vds, vde;

	hps = modes->hsync_len;
	hpe = hps + modes->left_margin + modes->xres + modes->right_margin;
	vps = modes->vsync_len;
	vpe = vps + modes->upper_margin + modes->yres + modes->lower_margin;

	hds = modes->hsync_len + modes->left_margin;
	hde = hds + modes->xres;
	vds = modes->vsync_len + modes->upper_margin;
	vde = vds + modes->yres;

	fb_write(DC_TFT_HSYNC, (hps << DC_HPS_LBIT) | (hpe << DC_HPE_LBIT));
	fb_write(DC_TFT_VSYNC, (vps << DC_VPS_LBIT) | (vpe << DC_VPE_LBIT));
	fb_write(DC_TFT_HDE, (hds << DC_HDS_LBIT) | (hde << DC_HDE_LBIT));
	fb_write(DC_TFT_VDE, (vds << DC_VDS_LBIT) | (vde << DC_VDE_LBIT));
}

void tft_cfg_init(struct jzfb_tft_config *tft_config) {
	uint32_t tft_cfg;
	uint32_t lcd_cgu;

	lcd_cgu = *(volatile unsigned int *)(0xb0000064);
	if(tft_config->pix_clk_inv) {
		lcd_cgu |= (0x1 << 26);
	} else {
		lcd_cgu &= ~(0x1 << 26);
	}
	*(volatile unsigned int *)(0xb0000064) = lcd_cgu;

	tft_cfg = fb_read(DC_TFT_CFG);
	if(tft_config->pix_clk_inv) {
		tft_cfg |= DC_PIX_CLK_INV;
	} else {
		tft_cfg &= ~DC_PIX_CLK_INV;
	}

	if(tft_config->de_dl) {
		tft_cfg |= DC_DE_DL;
	} else {
		tft_cfg &= ~DC_DE_DL;
	}

	if(tft_config->sync_dl) {
		tft_cfg |= DC_SYNC_DL;
	} else {
		tft_cfg &= ~DC_SYNC_DL;
	}
	if(tft_config->vsync_dl) {
		tft_cfg |= DC_VSYNC_DL;
	} else {
		tft_cfg &= ~DC_VSYNC_DL;
	}

	tft_cfg &= ~DC_COLOR_EVEN_MASK;
	switch(tft_config->color_even) {
		case TFT_LCD_COLOR_EVEN_RGB:
			tft_cfg |= DC_EVEN_RGB;
			break;
		case TFT_LCD_COLOR_EVEN_RBG:
			tft_cfg |= DC_EVEN_RBG;
			break;
		case TFT_LCD_COLOR_EVEN_BGR:
			tft_cfg |= DC_EVEN_BGR;
			break;
		case TFT_LCD_COLOR_EVEN_BRG:
			tft_cfg |= DC_EVEN_BRG;
			break;
		case TFT_LCD_COLOR_EVEN_GBR:
			tft_cfg |= DC_EVEN_GBR;
			break;
		case TFT_LCD_COLOR_EVEN_GRB:
			tft_cfg |= DC_EVEN_GRB;
			break;
		default:
			printf("err!\n");
			break;
	}

	tft_cfg &= ~DC_COLOR_ODD_MASK;
	switch(tft_config->color_odd) {
		case TFT_LCD_COLOR_ODD_RGB:
			tft_cfg |= DC_ODD_RGB;
			break;
		case TFT_LCD_COLOR_ODD_RBG:
			tft_cfg |= DC_ODD_RBG;
			break;
		case TFT_LCD_COLOR_ODD_BGR:
			tft_cfg |= DC_ODD_BGR;
			break;
		case TFT_LCD_COLOR_ODD_BRG:
			tft_cfg |= DC_ODD_BRG;
			break;
		case TFT_LCD_COLOR_ODD_GBR:
			tft_cfg |= DC_ODD_GBR;
			break;
		case TFT_LCD_COLOR_ODD_GRB:
			tft_cfg |= DC_ODD_GRB;
			break;
		default:
			printf("err!\n");
			break;
	}

	tft_cfg &= ~DC_MODE_MASK;
	switch(tft_config->mode) {
		case TFT_LCD_MODE_PARALLEL_888:
			tft_cfg |= DC_MODE_PARALLEL_888;
			break;
		case TFT_LCD_MODE_PARALLEL_666:
			tft_cfg |= DC_MODE_PARALLEL_666;
			break;
		case TFT_LCD_MODE_PARALLEL_565:
			tft_cfg |= DC_MODE_PARALLEL_565;
			break;
		case TFT_LCD_MODE_SERIAL_RGB:
			tft_cfg |= DC_MODE_SERIAL_8BIT_RGB;
			break;
		case TFT_LCD_MODE_SERIAL_RGBD:
			tft_cfg |= DC_MODE_SERIAL_8BIT_RGBD;
			break;
		default:
			printf("err!\n");
			break;
	}
	fb_write(DC_TFT_CFG, tft_cfg);
}

static int jzfb_tft_set_par(struct jzfb_config_info *info)
{
	struct fb_videomode *mode = info->modes;

	tft_timing_init(mode);
	tft_cfg_init(info->tft_config);

	return 0;
}

static void slcd_send_mcu_command(struct jzfb_config_info *info,unsigned long cmd)
{
	int count = 10000;
	uint32_t slcd_cfg;

	while ((fb_read(DC_SLCD_ST) & DC_SLCD_ST_BUSY) && count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}

	slcd_cfg = fb_read(DC_SLCD_CFG);
	fb_write(DC_SLCD_CFG, (slcd_cfg & ~DC_FMT_EN));
	fb_write(DC_SLCD_REG_IF, DC_SLCD_REG_IF_FLAG_CMD | (cmd & ~DC_SLCD_REG_IF_FLAG_MASK));
}

static void slcd_send_mcu_data(struct jzfb_config_info *info,unsigned long data)
{
	int count = 10000;
	uint32_t slcd_cfg;

	while ((fb_read(DC_SLCD_ST) & DC_SLCD_ST_BUSY) && count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}

	slcd_cfg = fb_read(DC_SLCD_CFG);
	fb_write(DC_SLCD_CFG, (slcd_cfg | DC_FMT_EN));
	fb_write(DC_SLCD_REG_IF, DC_SLCD_REG_IF_FLAG_DATA | (data & ~DC_SLCD_REG_IF_FLAG_MASK));
}

static void slcd_send_mcu_prm(struct jzfb_config_info *info,unsigned long data)
{
	int count = 10000;
	uint32_t slcd_cfg;

	while ((fb_read(DC_SLCD_ST) & DC_SLCD_ST_BUSY) && count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}

	slcd_cfg = fb_read(DC_SLCD_CFG);
	fb_write(DC_SLCD_CFG, (slcd_cfg & ~DC_FMT_EN));
	fb_write(DC_SLCD_REG_IF, DC_SLCD_REG_IF_FLAG_PRM | (data & ~DC_SLCD_REG_IF_FLAG_MASK));
}

void lcd_enable(void)
{
	if (lcd_enable_state == 0) {
		jzfb_cmp_start();
	}

	lcd_enable_state = 1;
	return;
}

static void lcd_disable(struct jzfb_config_info *info, stop_mode_t stop_md)
{
	if(stop_md == QCK_STOP) {
		fb_write(DC_CTRL, DC_QCK_STP_CMP);
		wait_dc_state(DC_WORKING, 0);
	} else {
		fb_write(DC_CTRL, DC_GEN_STP_CMP);
	}

	lcd_enable_state = 0;
	return;
}

static void wait_slcd_busy()
{
	int count = 100000;
	while ((fb_read(DC_SLCD_ST) & DC_SLCD_ST_BUSY)
			&& count--) {
		udelay(10);
	}
	if (count < 0) {
		serial_puts("SLCDC wait busy state wrong\n");
	}
}


static void jzfb_slcd_mcu_init(struct jzfb_config_info *info)
{

	unsigned int is_enabled, i;
	unsigned long tmp;
	struct jzfb_smart_config *smart_config;
	struct smart_lcd_data_table *data_table;
	uint32_t length_data_table;
	stop_mode_t stop_md;

	if (info->lcd_type != LCD_TYPE_SLCD)
		return;
	is_enabled = lcd_enable_state;
	/* if (!is_enabled) { */
	/* 	lcd_enable(); */
	/* } */
	smart_config = info->smart_config;
	data_table = smart_config->data_table;
	length_data_table = smart_config->length_data_table;

	if (length_data_table &&data_table) {
		for (i = 0; i < length_data_table; i++) {
			switch (data_table[i].type) {
				case SMART_CONFIG_DATA:
					slcd_send_mcu_data(info,data_table[i].value);
					break;
				case SMART_CONFIG_PRM:
					slcd_send_mcu_prm(info,data_table[i].value);
					break;
				case SMART_CONFIG_CMD:
					slcd_send_mcu_command(info,data_table[i].value);
					break;
				case SMART_CONFIG_UDELAY:
					udelay(data_table[i].value);
					break;
				default:
					serial_puts("Unknow SLCD data type\n");
					break;
			}
		}
	}
}



void slcd_set_mcu_register(struct jzfb_config_info *info,unsigned long cmd, unsigned long data)
{
	slcd_send_mcu_command(info, cmd);
	slcd_send_mcu_data(info, data);
}


void slcd_cfg_init(struct jzfb_smart_config *smart_config)
{
	uint32_t slcd_cfg;
	slcd_cfg = fb_read(DC_SLCD_CFG);
	slcd_cfg &= ~DC_RDY_SWITCH;
	slcd_cfg &= ~DC_CS_EN;

	if(smart_config->te_mipi_switch){
		slcd_cfg |= DC_TE_MIPI_SWITCH;
	} else {
		slcd_cfg &= ~DC_TE_MIPI_SWITCH;
	}

	if(smart_config->dc_md) {
		slcd_cfg |= DC_DC_MD;
	} else {
		slcd_cfg &= ~DC_DC_MD;
	}

	if(smart_config->wr_md) {
		slcd_cfg |= DC_WR_DP;
	} else {
		slcd_cfg &= ~DC_WR_DP;
	}

	slcd_cfg &= ~DC_DBI_TYPE_MASK;
	switch(smart_config->smart_type){
	case SMART_LCD_TYPE_8080:
		slcd_cfg |= DC_DBI_TYPE_B_8080;
		break;
	case SMART_LCD_TYPE_6800:
		slcd_cfg |= DC_DBI_TYPE_A_6800;
		break;
	case SMART_LCD_TYPE_SPI_3:
		slcd_cfg |= DC_DBI_TYPE_C_SPI_3;
		break;
	case SMART_LCD_TYPE_SPI_4:
		slcd_cfg |= DC_DBI_TYPE_C_SPI_4;
		break;
	default:
		printf("err!\n");
		break;
	}

	slcd_cfg &= ~DC_DATA_FMT_MASK;
	switch(smart_config->pix_fmt) {
	case SMART_LCD_FORMAT_888:
		slcd_cfg |= DC_DATA_FMT_888;
		break;
	case SMART_LCD_FORMAT_666:
		slcd_cfg |= DC_DATA_FMT_666;
		break;
	case SMART_LCD_FORMAT_565:
		slcd_cfg |= DC_DATA_FMT_565;
		break;
	default:
		printf("err!\n");
		break;
	}

	slcd_cfg &= ~DC_DWIDTH_MASK;
	switch(smart_config->dwidth) {
	case SMART_LCD_DWIDTH_8_BIT:
		slcd_cfg |= DC_DWIDTH_8BITS;
		break;
	case SMART_LCD_DWIDTH_9_BIT:
		slcd_cfg |= DC_DWIDTH_9BITS;
		break;
	case SMART_LCD_DWIDTH_16_BIT:
		slcd_cfg |= DC_DWIDTH_16BITS;
		break;
	case SMART_LCD_DWIDTH_18_BIT:
		slcd_cfg |= DC_DWIDTH_18BITS;
		break;
	case SMART_LCD_DWIDTH_24_BIT:
		slcd_cfg |= DC_DWIDTH_24BITS;
		break;
	default:
		printf("err!\n");
		break;
	}

	slcd_cfg &= ~DC_CWIDTH_MASK;
	switch(smart_config->cwidth) {
	case SMART_LCD_CWIDTH_8_BIT:
		slcd_cfg |= DC_CWIDTH_8BITS;
		break;
	case SMART_LCD_CWIDTH_9_BIT:
		slcd_cfg |= DC_CWIDTH_9BITS;
		break;
	case SMART_LCD_CWIDTH_16_BIT:
		slcd_cfg |= DC_CWIDTH_16BITS;
		break;
	case SMART_LCD_CWIDTH_18_BIT:
		slcd_cfg |= DC_CWIDTH_18BITS;
		break;
	case SMART_LCD_CWIDTH_24_BIT:
		slcd_cfg |= DC_CWIDTH_24BITS;
		break;
	default:
		printf("err!\n");
		break;
	}

	fb_write(DC_SLCD_CFG, slcd_cfg);

	return;
}

static int slcd_timing_init(struct fb_videomode *mode)
{
	uint32_t width = mode->xres;
	uint32_t height = mode->yres;
	uint32_t dhtime = 0;
	uint32_t dltime = 0;
	uint32_t chtime = 0;
	uint32_t cltime = 0;
	uint32_t tah = 0;
	uint32_t tas = 0;
	uint32_t slowtime = 0;

	/*frm_size*/
	fb_write(DC_SLCD_FRM_SIZE,
		  ((width << DC_SLCD_FRM_H_SIZE_LBIT) |
		   (height << DC_SLCD_FRM_V_SIZE_LBIT)));

	/* wr duty */
	fb_write(DC_SLCD_WR_DUTY,
		  ((dhtime << DC_DSTIME_LBIT) |
		   (dltime << DC_DDTIME_LBIT) |
		   (chtime << DC_CSTIME_LBIT) |
		   (cltime << DC_CDTIME_LBIT)));

	/* slcd timing */
	fb_write(DC_SLCD_TIMING,
		  ((tah << DC_TAH_LBIT) |
		  (tas << DC_TAS_LBIT)));

	/* slow time */
	fb_write(DC_SLCD_SLOW_TIME, slowtime);

	return 0;

}

static int jzfb_slcd_set_par(struct jzfb_config_info *info)
{
	int i,j;
	struct fb_videomode *mode = info->modes;

	slcd_cfg_init(info->smart_config);
	slcd_timing_init(mode);
	return 0;
}

static void disp_common_init(struct jzfb_config_info *info)
{
	uint32_t disp_com;

	disp_com = fb_read(DC_DISP_COM);
	disp_com &= ~DC_DP_IF_SEL;
	if(info->lcd_type == LCD_TYPE_SLCD) {
		disp_com |= DC_DISP_COM_SLCD;
	} else if(info->lcd_type == LCD_TYPE_MIPI_SLCD) {
		disp_com |= DC_DISP_COM_MIPI_SLCD;
	} else {
		disp_com |= DC_DISP_COM_TFT;
	}
	if(info->dither_enable) {
		disp_com |= DC_DP_DITHER_EN;
		disp_com &= ~DC_DP_DITHER_DW_MASK;
		disp_com |= info->dither.dither_red
			     << DC_DP_DITHER_DW_RED_LBIT;
		/*disp_com |= info->dither.dither_red
			     << DC_DP_DITHER_DW_RED_HBIT;*/
		disp_com |= info->dither.dither_green
			    << DC_DP_DITHER_DW_GREEN_LBIT;
		/*disp_com |= info->dither.dither_green
			    << DC_DP_DITHER_DW_GREEN_HBIT;*/
		disp_com |= info->dither.dither_blue
			    << DC_DP_DITHER_DW_BLUE_LBIT;
		/*disp_com |= info->dither.dither_blue
			    << DC_DP_DITHER_DW_BLUE_HBIT;*/
	} else {
		disp_com &= ~DC_DP_DITHER_EN;
	}

	fb_write(DC_DISP_COM, disp_com);
}

static void common_cfg_init(void)
{
	unsigned com_cfg = 0;

	com_cfg = fb_read(DC_COM_CONFIG);
	/*Keep COM_CONFIG reg first bit 0 */
	com_cfg &= ~DC_OUT_SEL;

	/* set burst length 32*/
	com_cfg &= ~DC_BURST_LEN_BDMA_MASK;
	com_cfg |= DC_BURST_LEN_BDMA_32;

	fb_write(DC_COM_CONFIG, com_cfg);
}

static void jzfb_videomode_to_var(struct fb_var_screeninfo *var,
		const struct fb_videomode *mode)
{
	var->xres = mode->xres;
	var->yres = mode->yres;
	var->xres_virtual = mode->xres;
	var->yres_virtual = mode->yres * MAX_DESC_NUM * MAX_LAYER_NUM;
	var->xoffset = 0;
	var->yoffset = 0;
	var->left_margin = mode->left_margin;
	var->right_margin = mode->right_margin;
	var->upper_margin = mode->upper_margin;
	var->lower_margin = mode->lower_margin;
	var->hsync_len = mode->hsync_len;
	var->vsync_len = mode->vsync_len;
	var->sync = mode->sync;
	var->vmode = mode->vmode & FB_VMODE_MASK;
	var->pixclock = mode->pixclock;
}

static int jzfb_check_colormode(struct fb_var_screeninfo *var, uint32_t *mode)
{
	int i;

	if (var->nonstd) {
		for (i = 0; i < ARRAY_SIZE(jzfb_colormodes); ++i) {
			struct jzfb_colormode *m = &jzfb_colormodes[i];
			if (var->nonstd == m->nonstd) {
				jzfb_colormode_to_var(var, m);
				*mode = m->mode;
				return 0;
			}
		}

		return -22;
	}

	for (i = 0; i < ARRAY_SIZE(jzfb_colormodes); ++i) {
		struct jzfb_colormode *m = &jzfb_colormodes[i];
		if (cmp_var_to_colormode(var, m)) {
			jzfb_colormode_to_var(var, m);
			*mode = m->mode;
			return 0;
		}
	}

	return -22;
}

static int jzfb_update_frm_mode(struct jzfb_config_info *info)
{
	struct fb_videomode *mode;
	struct jzfb_frm_mode *frm_mode;
	struct jzfb_frm_cfg *frm_cfg;
	struct jzfb_lay_cfg *lay_cfg;
	struct fb_var_screeninfo *var = &info->var;
	int i;

	mode = info->modes;

	frm_mode = &info->current_frm_mode;
	frm_cfg = &frm_mode->frm_cfg;
	lay_cfg = frm_cfg->lay_cfg;

	/*Only set layer0 work*/
	lay_cfg[0].lay_en = 1;
	lay_cfg[0].lay_z_order = 1;
	lay_cfg[1].lay_en = 0;
	lay_cfg[1].lay_z_order = 0;

	for(i = 0; i < MAX_LAYER_NUM; i++) {
		lay_cfg[i].pic_width = mode->xres;
		lay_cfg[i].pic_height = mode->yres;
		lay_cfg[i].disp_pos_x = 0;
		lay_cfg[i].disp_pos_y = 0;
		lay_cfg[i].g_alpha_en = 0;
		lay_cfg[i].g_alpha_val = 0xff;
		lay_cfg[i].color = jzfb_colormodes[0].color;
		lay_cfg[i].format = jzfb_colormodes[0].mode;
		lay_cfg[i].domain_multi = 1;
		lay_cfg[i].stride = mode->xres;
	}

	info->current_frm_desc = 0;

	return 0;
}

static int jz_lcd_init_mem(void *lcdbase, struct jzfb_config_info *info)
{
	unsigned long palette_mem_size;

	info->screen = (unsigned long)lcdbase;
	jzfb_alloc_devmem(info);

	return 0;
}

static int slcd_pixel_refresh_times(struct jzfb_config_info *info)
{

	switch(info->smart_config->smart_type){
	case SMART_LCD_TYPE_8080:
	case SMART_LCD_TYPE_6800:
		break;
	case SMART_LCD_TYPE_SPI_3:
		return 9;
	case SMART_LCD_TYPE_SPI_4:
		return 8;
	default:
		printf("%s %d err!\n",__func__,__LINE__);
		break;
	}

	switch(info->smart_config->pix_fmt) {
	case SMART_LCD_FORMAT_888:
		return 3;
	case SMART_LCD_FORMAT_565:
		return 2;
	default:
		printf("%s %d err!\n",__func__,__LINE__);
		break;
	}

	return 1;
}

static int calc_refresh_ratio(struct jzfb_config_info *info)
{
	struct jzfb_smart_config *smart_config;
	smart_config = info->smart_config;

	switch(smart_config->smart_type){
	case SMART_LCD_TYPE_8080:
	case SMART_LCD_TYPE_6800:
		break;
	case SMART_LCD_TYPE_SPI_3:
		return 9;
	case SMART_LCD_TYPE_SPI_4:
		return 8;
	default:
		printf("%s %d err!\n",__func__,__LINE__);
		break;
	}

	switch(smart_config->pix_fmt) {
	case SMART_LCD_FORMAT_888:
		if(smart_config->dwidth == SMART_LCD_DWIDTH_8_BIT)
			return 3;
		if(smart_config->dwidth == SMART_LCD_DWIDTH_24_BIT)
			return 1;
	case SMART_LCD_FORMAT_565:
		if(smart_config->dwidth == SMART_LCD_DWIDTH_8_BIT)
			return 2;
		if(smart_config->dwidth == SMART_LCD_DWIDTH_16_BIT)
			return 1;
	default:
		printf("%s %d err!\n",__func__,__LINE__);
		break;
	}

	return 1;
}

static void refresh_pixclock_auto_adapt(struct jzfb_config_info *info)
{
	struct fb_videomode *mode;
	uint16_t hds, vds;
	uint16_t hde, vde;
	uint16_t ht, vt;
	struct fb_var_screeninfo *var = &info->var;
	unsigned long rate;
	unsigned int refresh_ratio = 1;

	mode = info->modes;
	if (mode == NULL) {
		printf("%s error: get video mode failed\n", __func__);
	}

	hds = mode->hsync_len + mode->left_margin;
	hde = hds + mode->xres;
	ht = hde + mode->right_margin;

	vds = mode->vsync_len + mode->upper_margin;
	vde = vds + mode->yres;
	vt = vde + mode->lower_margin;

	if (info->lcd_type == LCD_TYPE_SLCD) {
		refresh_ratio = calc_refresh_ratio(info);
	}

	hds = mode->hsync_len + mode->left_margin;
	hde = hds + mode->xres;
	ht = hde + mode->right_margin;

	vds = mode->vsync_len + mode->upper_margin;
	vde = vds + mode->yres;
	vt = vde + mode->lower_margin;

	if(mode->refresh){
		rate = mode->refresh * vt * ht * refresh_ratio;

		mode->pixclock = KHZ2PICOS(round_up(rate, 1000)/1000);
		var->pixclock = mode->pixclock;
	}else if(mode->pixclock){
		rate = PICOS2KHZ(mode->pixclock) * 1000;
		mode->refresh = rate / vt / ht / refresh_ratio;
	}else{
		printf("%s error:lcd important config info is absenced\n",__func__);
	}

	debug("mode->refresh: %d, mode->pixclock: %d, rate: %ld, modex->pixclock: %d\n", mode->refresh, mode->pixclock, rate, mode->pixclock);
}

static int jzfb_set_fix_par(struct jzfb_config_info *info)
{
	uint32_t disp_com;

	disp_common_init(info);

	common_cfg_init();

	fb_write(DC_CLR_ST, 0x01FFFFFE);

	disp_com = fb_read(DC_DISP_COM);
	if (info->lcd_type == LCD_TYPE_SLCD) {
		fb_write(DC_DISP_COM, disp_com | DC_DISP_COM_SLCD);
		jzfb_slcd_set_par(info);
	}else if(info->lcd_type == LCD_TYPE_MIPI_SLCD){
		fb_write(DC_DISP_COM, disp_com | DC_DISP_COM_MIPI_SLCD);
		jzfb_slcd_set_par(info);
	}else if(info->lcd_type == LCD_TYPE_MIPI_TFT){
		fb_write(DC_DISP_COM, disp_com | DC_DISP_COM_TFT);
		jzfb_tft_set_par(info);
	} else {
		fb_write(DC_DISP_COM, disp_com | DC_DISP_COM_TFT);
		jzfb_tft_set_par(info);
	}

	return 0;
}

static void dctrl_tlb_invalidate_ch(struct jzfb_config_info *info , int ch)
{
	int val;
	val = ch & 0xF;
	/*Invalid for ch layers.*/
	fb_write(DC_TLB_TLBC, val);
}

static void dctrl_tlb_enable(struct jzfb_config_info *info)
{
	struct jzfb_frm_mode *frm_mode;
	struct jzfb_frm_cfg *frm_cfg;
	struct jzfb_lay_cfg *lay_cfg;
	unsigned int glbc, val;
	int i;

	frm_mode = &info->current_frm_mode;
	frm_cfg = &frm_mode->frm_cfg;
	lay_cfg = frm_cfg->lay_cfg;

	val = fb_read(DC_TLB_GLBC);
	glbc = val;

	 for(i = 0; i < MAX_LAYER_NUM; i++) {
		 if(lay_cfg[i].lay_en) {
			 if(lay_cfg[i].tlb_en != (glbc>>i & 0x1)) {
				 glbc = lay_cfg[i].tlb_en ?
					 (glbc | (0x1 << i)) : (glbc & ~(0x1 << i));
			 }
		} else {
			glbc &= ~(0x1 << i);
		}
	}
	if(val != glbc) {
		dctrl_tlb_invalidate_ch(info, glbc);
		if(val > glbc) {
			info->tlb_disable_ch = glbc;
		} else {
			fb_write(DC_TLB_GLBC, glbc);
		}
	}

}

static void dctrl_tlb_configure(struct jzfb_config_info *info)
{
	unsigned int tlbv = 0;

	tlbv |= (1 << DC_CNM_LBIT);
	tlbv |= (1 << DC_GCN_LBIT);

	fb_write(DC_TLB_TLBV, tlbv);
}

static void dctrl_tlb_disable(struct jzfb_config_info *info)
{
	/*Invalid and disable for all layers.*/
	fb_write(DC_TLB_TLBC, DC_CH0_INVLD | DC_CH1_INVLD |
			DC_CH2_INVLD | DC_CH3_INVLD);
	fb_write(DC_TLB_GLBC, 0);
}

static int jzfb_set_par(struct jzfb_config_info *info)
{
	struct fb_videomode *mode = info->modes;
	struct jzfb_frmdesc_msg *desc_msg;
	uint32_t colormode;
	int ret;
	int i;
	unsigned int intc = 0;

	jzfb_set_fix_par(info);
	jzfb_videomode_to_var(&info->var, mode);

	jzfb_colormode_to_var(&info->var, &jzfb_colormodes[0]);

	ret = jzfb_check_colormode(&info->var, &colormode);
	if(ret) {
		printf("Check colormode failed!\n");
		return  ret;
	}

	if(colormode != LAYER_CFG_FORMAT_YUV422) {
		for(i = 0; i < MAX_LAYER_NUM; i++) {
			info->current_frm_mode.frm_cfg.lay_cfg[i].format = colormode;
		}
	} else {
		info->current_frm_mode.frm_cfg.lay_cfg[0].format = colormode;
	}

	jzfb_update_frm_mode(info);

	ret = jzfb_desc_init(info, FRAME_CFG_ALL_UPDATE);
	if(ret) {
		printf("Desc init err!\n");
		return ret;
	}

	if(lcd_config_info.lcd_type == LCD_TYPE_MIPI_SLCD || LCD_TYPE_TFT || LCD_TYPE_MIPI_TFT) {
		fb_write(DC_FRM_CFG_ADDR, info->framedesc_phys[info->current_frm_desc]);
	}
	if(lcd_config_info.lcd_type == LCD_TYPE_SLCD){
	/* printf("---------------slcd_reg------------------\n"); */
	/* printf("SLCD_CFG:           %lx\n",fb_read(DC_SLCD_CFG)); */
	/* printf("SLCD_WR_DUTY:       %lx\n",fb_read(DC_SLCD_WR_DUTY)); */
	/* printf("SLCD_TIMING:        %lx\n",fb_read(DC_SLCD_TIMING)); */
	/* printf("SLCD_FRM_SIZE:      %lx\n",fb_read(DC_SLCD_FRM_SIZE)); */
	/* printf("SLCD_SLOW_TIME:     %lx\n",fb_read(DC_SLCD_SLOW_TIME)); */
	/* printf("SLCD_CMD:           %lx\n",fb_read(DC_SLCD_CMD)); */
	/* printf("SLCD_ST:            %lx\n",fb_read(DC_SLCD_ST)); */
	/* printf("---------------slcd_reg------------------\n"); */
		jzfb_slcd_mcu_init(info);
	}

#ifdef CONFIG_JZ_MIPI_DSI
	switch (lcd_config_info.lcd_type) {
		case LCD_TYPE_MIPI_TFT:
			jz_dsi_mode_cfg(dsi,1);
			break;
		case LCD_TYPE_MIPI_SLCD:
			jz_dsi_mode_cfg(dsi,0);
			break;
	}

#endif
	intc = DC_EOD_MSK | DC_SDA_MSK | DC_UOT_MSK | DC_SOC_MSK | DC_OOW_MSK | DC_EOW_MSK | DC_SOS_MSK | DC_STOP_SRD_ACK;
	fb_write(DC_INTC,intc);

	return 0;
}

void lcd_ctrl_init(void *lcd_base)
{
	unsigned long pixel_clock_rate;

	/* init registers base address */
	memcpy(&lcd_config_info , &jzfb1_init_data , sizeof(jzfb1_init_data));
	lcd_config_info.lcdbaseoff = 0;

#ifdef CONFIG_JZ_MIPI_DSI
	dsi = &jz_dsi;
	dsi->dsi_phy = &dsi_phy;
#endif

	lcd_set_flush_dcache(1);

	refresh_pixclock_auto_adapt(&lcd_config_info);
	pixel_clock_rate = PICOS2KHZ(lcd_config_info.modes->pixclock) * 1000;

	/* smart lcd WR freq = (lcd pixel clock)/2 */
	if (lcd_config_info.lcd_type == LCD_TYPE_SLCD) {
		pixel_clock_rate *= 2;
	}

	printf("pixel_clock_rate = %d\n",pixel_clock_rate);
	clk_set_rate(LCD, pixel_clock_rate);

	/*lcd_close_backlight();*/
	panel_pin_init();

#ifdef CONFIG_LCD_FORMAT_X8B8G8R8
	lcd_config_info.fmt_order = FORMAT_X8B8G8R8;
#else
	lcd_config_info.fmt_order = FORMAT_X8R8G8B8;
#endif

	jz_lcd_init_mem(lcd_base, &lcd_config_info);

	panel_power_on();

#ifdef CONFIG_JZ_MIPI_DSI
	dsi->bpp_info = lcd_config_info.bpp;
	jz_dsi_init(dsi);
	panel_init_sequence(dsi);

	*(volatile unsigned int *)0xb0004118 = 0x7e;
	*(volatile unsigned int *)0xb0004198 = 0x7e;
	*(volatile unsigned int *)0xb0004218 = 0x7e;
	*(volatile unsigned int *)0xb0004298 = 0x7e;
	*(volatile unsigned int *)0xb0004318 = 0x7e;
#endif
	jzfb_set_par(&lcd_config_info);
	return;
}

void lcd_show_board_info(void)
{
	return;
}

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
	return;
}

