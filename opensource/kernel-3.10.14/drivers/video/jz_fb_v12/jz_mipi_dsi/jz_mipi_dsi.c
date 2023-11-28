/* linux/drivers/video/jz_mipi_dsi/jz_mipi_dsi.c
 *
 * Ingenic SoC MIPI-DSI dsim driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/memory.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/notifier.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>
#include <linux/gpio.h>

#include <mach/jzfb.h>
#include "jz_mipi_dsi_lowlevel.h"
#include "jz_mipi_dsih_hal.h"
#include "jz_mipi_dsi_regs.h"

#define DSI_IOBASE      0x13014000
struct mipi_dsim_ddi {
	int				bus_id;
	struct list_head		list;
	struct mipi_dsim_lcd_device	*dsim_lcd_dev;
	struct mipi_dsim_lcd_driver	*dsim_lcd_drv;
};

static LIST_HEAD(dsim_ddi_list);

static DEFINE_MUTEX(mipi_dsim_lock);

void dump_dsi_reg(struct dsi_device *dsi);
static DEFINE_MUTEX(dsi_lock);

int jz_dsi_video_cfg(struct dsi_device *dsi)
{
	dsih_error_t err_code = OK;
	unsigned short bytes_per_pixel_x100 = 0;	/* bpp x 100 because it can be 2.25 */
	unsigned short video_size = 0;
	unsigned int ratio_clock_xPF = 0;	/* holds dpi clock/byte clock times precision factor */
	unsigned short null_packet_size = 0;
	unsigned char video_size_step = 1;
	unsigned int total_bytes = 0;
	unsigned int bytes_per_chunk = 0;
	unsigned int no_of_chunks = 0;
	unsigned int bytes_left = 0;
	unsigned int chunk_overhead = 0;

	struct video_config *video_config;
	video_config = dsi->video_config;

	/* check DSI controller dsi */
	if ((dsi == NULL) || (video_config == NULL)) {
		return ERR_DSI_INVALID_INSTANCE;
	}
	if(dsi->state == UBOOT_INITIALIZED) {
		/*no need to reconfig, just return*/
		printk("dsi has already been initialized by uboot!!\n");
		return OK;
	}
	if (dsi->state != INITIALIZED) {
		return ERR_DSI_INVALID_INSTANCE;
	}

	ratio_clock_xPF =
	    (video_config->byte_clock * PRECISION_FACTOR) /
	    (video_config->pixel_clock);
	video_size = video_config->h_active_pixels;
	/*  disable set up ACKs and error reporting */
	mipi_dsih_hal_dpi_frame_ack_en(dsi, video_config->receive_ack_packets);
	if (video_config->receive_ack_packets) {	/* if ACK is requested, enable BTA, otherwise leave as is */
		mipi_dsih_hal_bta_en(dsi, 1);
	}
	/*0:switch to high speed transfer, 1 low power mode */
	mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG, 0);
	/*0:enable video mode, 1:enable cmd mode */
	mipi_dsih_hal_gen_set_mode(dsi, 0);

	err_code =
	    jz_dsi_video_coding(dsi, &bytes_per_pixel_x100, &video_size_step,
				&video_size);
	if (err_code) {
		return err_code;
	}

	jz_dsi_dpi_cfg(dsi, &ratio_clock_xPF, &bytes_per_pixel_x100);

	/* TX_ESC_CLOCK_DIV must be less than 20000KHz */
	jz_dsih_hal_tx_escape_division(dsi, TX_ESC_CLK_DIV);

	/* video packetisation   */
	if (video_config->video_mode == VIDEO_BURST_WITH_SYNC_PULSES) {	/* BURST */
		//mipi_dsih_hal_dpi_null_packet_en(dsi, 0);
		mipi_dsih_hal_dpi_null_packet_size(dsi, 0);
		//mipi_dsih_hal_dpi_multi_packet_en(dsi, 0);
		err_code =
		    err_code ? err_code : mipi_dsih_hal_dpi_chunks_no(dsi, 1);
		err_code =
		    err_code ? err_code :
		    mipi_dsih_hal_dpi_video_packet_size(dsi, video_size);
		if (err_code != OK) {
			return err_code;
		}
		/* BURST by default, returns to LP during ALL empty periods - energy saving */
		mipi_dsih_hal_dpi_lp_during_hfp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_hbp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vactive(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vfp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vbp(dsi, 1);
		mipi_dsih_hal_dpi_lp_during_vsync(dsi, 1);
#ifdef CONFIG_DSI_DPI_DEBUG
		/*      D E B U G               */
		{
			pr_info("burst video");
			pr_info("h line time %d ,",
			       (unsigned
				short)((video_config->h_total_pixels *
					ratio_clock_xPF) / PRECISION_FACTOR));
			pr_info("video_size %d ,", video_size);
		}
#endif
	} else {		/* non burst transmission */
		null_packet_size = 0;
		/* bytes to be sent - first as one chunk */
		bytes_per_chunk =
		    (bytes_per_pixel_x100 * video_config->h_active_pixels) /
		    100 + VIDEO_PACKET_OVERHEAD;
		/* bytes being received through the DPI interface per byte clock cycle */
		total_bytes =
		    (ratio_clock_xPF * video_config->no_of_lanes *
		     (video_config->h_total_pixels -
		      video_config->h_back_porch_pixels -
		      video_config->h_sync_pixels)) / PRECISION_FACTOR;
		pr_debug("---->total_bytes:%d, bytes_per_chunk:%d\n", total_bytes,
		       bytes_per_chunk);
		/* check if the in pixels actually fit on the DSI link */
		if (total_bytes >= bytes_per_chunk) {
			chunk_overhead = total_bytes - bytes_per_chunk;
			/* overhead higher than 1 -> enable multi packets */
			if (chunk_overhead > 1) {
				for (video_size = video_size_step; video_size < video_config->h_active_pixels; video_size += video_size_step) {	/* determine no of chunks */
					if ((((video_config->h_active_pixels *
					       PRECISION_FACTOR) / video_size) %
					     PRECISION_FACTOR) == 0) {
						no_of_chunks =
						    video_config->
						    h_active_pixels /
						    video_size;
						bytes_per_chunk =
						    (bytes_per_pixel_x100 *
						     video_size) / 100 +
						    VIDEO_PACKET_OVERHEAD;
						if (total_bytes >=
						    (bytes_per_chunk *
						     no_of_chunks)) {
							bytes_left =
							    total_bytes -
							    (bytes_per_chunk *
							     no_of_chunks);
							break;
						}
					}
				}
				/* prevent overflow (unsigned - unsigned) */
				if (bytes_left >
				    (NULL_PACKET_OVERHEAD * no_of_chunks)) {
					null_packet_size =
					    (bytes_left -
					     (NULL_PACKET_OVERHEAD *
					      no_of_chunks)) / no_of_chunks;
					if (null_packet_size > MAX_NULL_SIZE) {	/* avoid register overflow */
						null_packet_size =
						    MAX_NULL_SIZE;
					}
				}
			} else {	/* no multi packets */
				no_of_chunks = 1;
#ifdef CONFIG_DSI_DPI_DEBUG
				/*      D E B U G               */
				{
					pr_info("no multi no null video");
					pr_info("h line time %d",
					       (unsigned
						short)((video_config->
							h_total_pixels *
							ratio_clock_xPF) /
						       PRECISION_FACTOR));
					pr_info("video_size %d", video_size);
				}
#endif
				/* video size must be a multiple of 4 when not 18 loosely */
				for (video_size = video_config->h_active_pixels;
				     (video_size % video_size_step) != 0;
				     video_size++) {
					;
				}
			}
		} else {
			pr_err
			    ("resolution cannot be sent to display through current settings");
			err_code = ERR_DSI_OVERFLOW;

		}
	}
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_chunks_no(dsi,
							      no_of_chunks);
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_video_packet_size(dsi,
								      video_size);
	err_code =
	    err_code ? err_code : mipi_dsih_hal_dpi_null_packet_size(dsi,
								     null_packet_size);

	// mipi_dsih_hal_dpi_null_packet_en(dsi, null_packet_size > 0? 1: 0);
	// mipi_dsih_hal_dpi_multi_packet_en(dsi, (no_of_chunks > 1)? 1: 0);
#ifdef  CONFIG_DSI_DPI_DEBUG
	/*      D E B U G               */
	{
		pr_info("total_bytes %d ,", total_bytes);
		pr_info("bytes_per_chunk %d ,", bytes_per_chunk);
		pr_info("bytes left %d ,", bytes_left);
		pr_info("null packets %d ,", null_packet_size);
		pr_info("chunks %d ,", no_of_chunks);
		pr_info("video_size %d ", video_size);
	}
#endif
	mipi_dsih_hal_dpi_video_vc(dsi, video_config->virtual_channel);
	jz_dsih_dphy_no_of_lanes(dsi, video_config->no_of_lanes);
	/* enable high speed clock */
	mipi_dsih_dphy_enable_hs_clk(dsi, 1);
	pr_info("video configure is ok!\n");


	return err_code;

}

/* set all register settings to MIPI DSI controller. */
dsih_error_t jz_dsi_phy_cfg(struct dsi_device * dsi)
{
	dsih_error_t err = OK;
	err = jz_dsi_set_clock(dsi);
	if (err) {
		return err;
	}
	err = jz_init_dsi(dsi);
	if (err) {
		return err;
	}
	return OK;
}

int jz_dsi_phy_open(struct dsi_device *dsi)
{
	struct video_config *video_config;
	video_config = dsi->video_config;

	pr_debug("entry %s()\n", __func__);

	jz_dsih_dphy_reset(dsi, 0);
	jz_dsih_dphy_stop_wait_time(dsi, 0x1c);	/* 0x1c: */

	if (video_config->no_of_lanes == 2 || video_config->no_of_lanes == 1) {
		jz_dsih_dphy_no_of_lanes(dsi, video_config->no_of_lanes);
	} else {
		return ERR_DSI_OUT_OF_BOUND;
	}

	jz_dsih_dphy_clock_en(dsi, 1);
	jz_dsih_dphy_shutdown(dsi, 1);
	jz_dsih_dphy_reset(dsi, 1);

	dsi->dsi_phy->status = INITIALIZED;
	return OK;
}

void dump_dsi_reg(struct dsi_device *dsi)
{
	pr_info("dsi->dev: ===========>dump dsi reg\n");
	pr_info("dsi->dev: VERSION------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VERSION));
	pr_info("dsi->dev: PWR_UP:------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PWR_UP));
	pr_info("dsi->dev: CLKMGR_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CLKMGR_CFG));
	pr_info("dsi->dev: DPI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID));
	pr_info("dsi->dev: DPI_COLOR_CODING---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING));
	pr_info("dsi->dev: DPI_CFG_POL--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_CFG_POL));
	pr_info("dsi->dev: DPI_LP_CMD_TIM-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM));
	pr_info("dsi->dev: DBI_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_VCID));
	pr_info("dsi->dev: DBI_CFG------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CFG));
	pr_info("dsi->dev: DBI_PARTITIONING_EN:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_PARTITIONING_EN));
	pr_info("dsi->dev: DBI_CMDSIZE--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DBI_CMDSIZE));
	pr_info("dsi->dev: PCKHDL_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PCKHDL_CFG));
	pr_info("dsi->dev: GEN_VCID-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_VCID));
	pr_info("dsi->dev: MODE_CFG-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_MODE_CFG));
	pr_info("dsi->dev: VID_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG));
	pr_info("dsi->dev: VID_PKT_SIZE-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE));
	pr_info("dsi->dev: VID_NUM_CHUNKS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS));
	pr_info("dsi->dev: VID_NULL_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NULL_SIZE));
	pr_info("dsi->dev: VID_HSA_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME));
	pr_info("dsi->dev: VID_HBP_TIME-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME));
	pr_info("dsi->dev: VID_HLINE_TIME-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME));
	pr_info("dsi->dev: VID_VSA_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES));
	pr_info("dsi->dev: VID_VBP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES));
	pr_info("dsi->dev: VID_VFP_LINES------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES));
	pr_info("dsi->dev: VID_VACTIVE_LINES--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES));
	pr_info("dsi->dev: EDPI_CMD_SIZE------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE));
	pr_info("dsi->dev: CMD_MODE_CFG-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_MODE_CFG));
	pr_info("dsi->dev: GEN_HDR------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_HDR));
	pr_info("dsi->dev: GEN_PLD_DATA-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_GEN_PLD_DATA));
	pr_info("dsi->dev: CMD_PKT_STATUS-----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_CMD_PKT_STATUS));
	pr_info("dsi->dev: TO_CNT_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_TO_CNT_CFG));
	pr_info("dsi->dev: HS_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_RD_TO_CNT));
	pr_info("dsi->dev: LP_RD_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_RD_TO_CNT));
	pr_info("dsi->dev: HS_WR_TO_CNT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_HS_WR_TO_CNT));
	pr_info("dsi->dev: LP_WR_TO_CNT_CFG---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LP_WR_TO_CNT));
	pr_info("dsi->dev: BTA_TO_CNT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_BTA_TO_CNT));
	pr_info("dsi->dev: SDF_3D-------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D));
	pr_info("dsi->dev: LPCLK_CTRL---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_LPCLK_CTRL));
	pr_info("dsi->dev: PHY_TMR_LPCLK_CFG--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_LPCLK_CFG));
	pr_info("dsi->dev: PHY_TMR_CFG--------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TMR_CFG));
	pr_info("dsi->dev: PHY_RSTZ-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_RSTZ));
	pr_info("dsi->dev: PHY_IF_CFG---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_IF_CFG));
	pr_info("dsi->dev: PHY_ULPS_CTRL------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_ULPS_CTRL));
	pr_info("dsi->dev: PHY_TX_TRIGGERS----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TX_TRIGGERS));
	pr_info("dsi->dev: PHY_STATUS---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS));
	pr_info("dsi->dev: PHY_TST_CTRL0------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL0));
	pr_info("dsi->dev: PHY_TST_CTRL1------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_TST_CTRL1));
	pr_info("dsi->dev: INT_ST0------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST0));
	pr_info("dsi->dev: INT_ST1------------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_ST1));
	pr_info("dsi->dev: INT_MSK0-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK0));
	pr_info("dsi->dev: INT_MSK1-----------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_MSK1));
	pr_info("dsi->dev: INT_FORCE0---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE0));
	pr_info("dsi->dev: INT_FORCE1---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_INT_FORCE1));
	pr_info("dsi->dev: VID_SHADOW_CTRL----:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_SHADOW_CTRL));
	pr_info("dsi->dev: DPI_VCID_ACT-------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_VCID_ACT));
	pr_info("dsi->dev: DPI_COLOR_CODING_AC:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_COLOR_CODING_ACT));
	pr_info("dsi->dev: DPI_LP_CMD_TIM_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_DPI_LP_CMD_TIM_ACT));
	pr_info("dsi->dev: VID_MODE_CFG_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG_ACT));
	pr_info("dsi->dev: VID_PKT_SIZE_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_PKT_SIZE_ACT));
	pr_info("dsi->dev: VID_NUM_CHUNKS_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_NUM_CHUNKS_ACT));
	pr_info("dsi->dev: VID_HSA_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HSA_TIME_ACT));
	pr_info("dsi->dev: VID_HBP_TIME_ACT---:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HBP_TIME_ACT));
	pr_info("dsi->dev: VID_HLINE_TIME_ACT-:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_HLINE_TIME_ACT));
	pr_info("dsi->dev: VID_VSA_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VSA_LINES_ACT));
	pr_info("dsi->dev: VID_VBP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VBP_LINES_ACT));
	pr_info("dsi->dev: VID_VFP_LINES_ACT--:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VFP_LINES_ACT));
	pr_info("dsi->dev: VID_VACTIVE_LINES_ACT:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_VID_VACTIVE_LINES_ACT));
	pr_info("dsi->dev: SDF_3D_ACT---------:%08x\n",
		 mipi_dsih_read_word(dsi, R_DSI_HOST_SDF_3D_ACT));

}

void set_base_dir_tx(struct dsi_device *dsi, void *param)
{
	int i = 0;
	register_config_t phy_direction[] = {
		{0xb4, 0x02},
		{0xb8, 0xb0},
		{0xb8, 0x100b0},
		{0xb4, 0x00},
		{0xb8, 0x000b0},
		{0xb8, 0x0000},
		{0xb4, 0x02},
		{0xb4, 0x00}
	};
	i = mipi_dsih_write_register_configuration(dsi, phy_direction,
						   (sizeof(phy_direction) /
						    sizeof(register_config_t)));
	if (i != (sizeof(phy_direction) / sizeof(register_config_t))) {
		pr_err("ERROR setting up testchip %d", i);
	}
}

static void jz_mipi_update_cfg(struct dsi_device *dsi)
{
	int ret;
	int st_mask = 0;
	int retry = 20;
	dsi->state = NOT_INITIALIZED;
	ret = jz_dsi_phy_open(dsi);
	if (ret) {
		pr_err("open the phy failed!\n");
	}

	mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG,
				     0xffffff0);

	/*set command mode */
	mipi_dsih_write_word(dsi, R_DSI_HOST_MODE_CFG, 0x1);
	/*set this register for cmd size, default 0x6 */
	mipi_dsih_write_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE, 0x6);

	/*
	 * jz_dsi_phy_cfg:
	 * PLL programming, config the output freq to DEFAULT_DATALANE_BPS.
	 * */
	ret = jz_dsi_phy_cfg(dsi);
	if (ret) {
		pr_err("phy configigure failed!\n");
	}

#if 0
	if (dsi->video_config->no_of_lanes == 2)
		st_mask = 0x95;
	else
		st_mask = 0x15;

	/*checkout phy clk lock and  clklane, datalane stopstate  */
	while ((mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS) & st_mask) !=
			st_mask && retry--) {
		pr_info("phy status = %08x\n", mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS));
	}

	if (!retry){
		pr_err("wait for phy config failed!\n");
	}
#endif

	mipi_dsih_dphy_enable_hs_clk(dsi, 0);
	dsi->state = INITIALIZED;

}

static int jz_mipi_dsi_set_blank(struct dsi_device *dsi, int blank_mode)
{
	struct mipi_dsim_lcd_driver *client_drv = dsi->dsim_lcd_drv;
	struct mipi_dsim_lcd_device *client_dev = dsi->dsim_lcd_dev;
	dev_dbg("%s, %d :blank_mode:%x, dsi->suspended:%x\n", __func__, __LINE__, blank_mode, dsi->suspended);
	switch (blank_mode) {
	case DSI_BLANK_UNBLANK:
		if(!clk_is_enabled(dsi->clk)) {
			clk_enable(dsi->clk);
		}
		break;
	case DSI_BLANK_NORMAL:
		if(clk_is_enabled(dsi->clk)) {
			clk_disable(dsi->clk);
		}
		break;
	case DSI_BLANK_POWERDOWN:
		if (dsi->suspended)
			return 0;

		dsi->suspended = true;

		if (client_drv && client_drv->suspend)
			client_drv->suspend(client_dev);


		jz_dsih_dphy_clock_en(dsi, 0);
		jz_dsih_dphy_shutdown(dsi, 0);
		mipi_dsih_hal_power(dsi, 0);
		clk_disable(dsi->clk);


		break;
	case DSI_BLANK_POWERUP:
		if (!dsi->suspended)
			return 0;

		dsi->suspended = false;
		if(!clk_is_enabled(dsi->clk)) {
			clk_enable(dsi->clk);
		}

		//dump_dsi_reg(dsi);
		/* lcd panel power on. */
		if (client_drv && client_drv->power_on)
			client_drv->power_on(client_dev, 1);

		jz_mipi_update_cfg(dsi);
		/* set lcd panel sequence commands. */
		if (client_drv && client_drv->set_sequence)
			client_drv->set_sequence(client_dev);

		mipi_dsih_dphy_enable_hs_clk(dsi, 1);
		mipi_dsih_dphy_auto_clklane_ctrl(dsi, 1);

		mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG, 1);
		mipi_dsih_hal_power(dsi, 1);

		break;
	default:
		break;
	}

}
/* define MIPI-DSI Master operations. */
static struct dsi_master_ops jz_master_ops = {
	.video_cfg = jz_dsi_video_cfg,
	.cmd_write = write_command,	/*jz_dsi_wr_data, */
	.cmd_read = NULL,	/*jz_dsi_rd_data, */
	.set_early_blank_mode   = NULL, /*jz_mipi_dsi_early_blank_mode,*/
	.set_blank_mode         = NULL, /*jz_mipi_dsi_blank_mode,*/
	.set_blank				= jz_mipi_dsi_set_blank,
};

int mipi_dsi_register_lcd_device(struct mipi_dsim_lcd_device *lcd_dev)
{
	struct mipi_dsim_ddi *dsim_ddi;

	if (!lcd_dev->name) {
		pr_err("dsim_lcd_device name is NULL.\n");
		return -EFAULT;
	}

	dsim_ddi = kzalloc(sizeof(struct mipi_dsim_ddi), GFP_KERNEL);
	if (!dsim_ddi) {
		pr_err("failed to allocate dsim_ddi object.\n");
		return -ENOMEM;
	}

	dsim_ddi->dsim_lcd_dev = lcd_dev;

	mutex_lock(&mipi_dsim_lock);
	list_add_tail(&dsim_ddi->list, &dsim_ddi_list);
	mutex_unlock(&mipi_dsim_lock);

	return 0;
}

static struct mipi_dsim_ddi *mipi_dsi_find_lcd_device(
					struct mipi_dsim_lcd_driver *lcd_drv)
{
	struct mipi_dsim_ddi *dsim_ddi, *next;
	struct mipi_dsim_lcd_device *lcd_dev;

	mutex_lock(&mipi_dsim_lock);

	list_for_each_entry_safe(dsim_ddi, next, &dsim_ddi_list, list) {
		if (!dsim_ddi)
			goto out;

		lcd_dev = dsim_ddi->dsim_lcd_dev;
		if (!lcd_dev)
			continue;

		if ((strcmp(lcd_drv->name, lcd_dev->name)) == 0) {
			/**
			 * bus_id would be used to identify
			 * connected bus.
			 */
			dsim_ddi->bus_id = lcd_dev->bus_id;
			mutex_unlock(&mipi_dsim_lock);

			return dsim_ddi;
		}

		list_del(&dsim_ddi->list);
		kfree(dsim_ddi);
	}

out:
	mutex_unlock(&mipi_dsim_lock);

	return NULL;
}

int mipi_dsi_register_lcd_driver(struct mipi_dsim_lcd_driver *lcd_drv)
{
	struct mipi_dsim_ddi *dsim_ddi;

	if (!lcd_drv->name) {
		pr_err("dsim_lcd_driver name is NULL.\n");
		return -EFAULT;
	}

	dsim_ddi = mipi_dsi_find_lcd_device(lcd_drv);
	if (!dsim_ddi) {
		pr_err("mipi_dsim_ddi object not found.\n");
		return -EFAULT;
	}

	dsim_ddi->dsim_lcd_drv = lcd_drv;

	pr_info("registered panel driver(%s) to mipi-dsi driver.\n",
		lcd_drv->name);

	return 0;

}

static struct mipi_dsim_ddi *mipi_dsi_bind_lcd_ddi(
						struct dsi_device *dsim,
						const char *name)
{
	struct mipi_dsim_ddi *dsim_ddi, *next;
	struct mipi_dsim_lcd_driver *lcd_drv;
	struct mipi_dsim_lcd_device *lcd_dev;
	int ret;

	mutex_lock(&dsim->lock);

	list_for_each_entry_safe(dsim_ddi, next, &dsim_ddi_list, list) {
		lcd_drv = dsim_ddi->dsim_lcd_drv;
		lcd_dev = dsim_ddi->dsim_lcd_dev;

		pr_debug("dsi->dev: lcd_drv->name  = %s, name = %s\n",
				lcd_drv->name, name);
		if ((strcmp(lcd_drv->name, name) == 0)) {
			lcd_dev->master = dsim;

			dev_set_name(&lcd_dev->dev, "%s", lcd_drv->name);

			ret = device_register(&lcd_dev->dev);
			if (ret < 0) {
				pr_err("can't register %s, status %d\n",
					dev_name(&lcd_dev->dev), ret);
				mutex_unlock(&dsim->lock);
				return NULL;
			}

			dsim->dsim_lcd_dev = lcd_dev;
			dsim->dsim_lcd_drv = lcd_drv;

			mutex_unlock(&dsim->lock);

			return dsim_ddi;
		}else{
			pr_err("dsi->dev: lcd_drv->name is different with fb name\n");
		}
	}

	mutex_unlock(&dsim->lock);

	return NULL;
}

struct dsi_device * jzdsi_init(struct jzdsi_data *pdata)
{
	struct dsi_device *dsi;
	struct dsi_phy *dsi_phy;
	struct mipi_dsim_ddi *dsim_ddi;
	int retry = 5;
	int st_mask = 0;
	int ret = -EINVAL;
	pr_debug("entry %s()\n", __func__);

	dsi = (struct dsi_device *)kzalloc(sizeof(struct dsi_device), GFP_KERNEL);
	if (!dsi) {
		pr_err("dsi->dev: failed to allocate dsi object.\n");
		ret = -ENOMEM;
		goto err_dsi;
	}

	dsi_phy = (struct dsi_phy *)kzalloc(sizeof(struct dsi_phy), GFP_KERNEL);
	if (!dsi_phy) {
		pr_err("dsi->dev: failed to allocate dsi phy  object.\n");
		ret = -ENOMEM;
		goto err_dsi_phy;
	}
	dsi->state = NOT_INITIALIZED;
	dsi->dsi_config = &(pdata->dsi_config);

	dsi_phy->status = NOT_INITIALIZED;
	dsi_phy->reference_freq = REFERENCE_FREQ;
	dsi_phy->bsp_pre_config = set_base_dir_tx;
	dsi->dsi_phy = dsi_phy;
	dsi->video_config = &(pdata->video_config);
	dsi->video_config->pixel_clock = PICOS2KHZ(pdata->modes->pixclock);	// dpi_clock
	dsi->video_config->h_polarity = pdata->modes->sync & FB_SYNC_HOR_HIGH_ACT;
	dsi->video_config->h_active_pixels = pdata->modes->xres;
	dsi->video_config->h_sync_pixels = pdata->modes->hsync_len;
	dsi->video_config->h_back_porch_pixels = pdata->modes->left_margin;
	dsi->video_config->h_total_pixels = pdata->modes->xres + pdata->modes->hsync_len + pdata->modes->left_margin + pdata->modes->right_margin;
	dsi->video_config->v_active_lines = pdata->modes->yres;
	dsi->video_config->v_polarity =  pdata->modes->sync & FB_SYNC_VERT_HIGH_ACT;
	dsi->video_config->v_sync_lines = pdata->modes->vsync_len;
	dsi->video_config->v_back_porch_lines = pdata->modes->upper_margin;
	dsi->video_config->v_total_lines = pdata->modes->yres + pdata->modes->upper_margin + pdata->modes->lower_margin + pdata->modes->vsync_len;
	dsi->master_ops = &jz_master_ops;

	dsi->clk = clk_get(NULL, "dsi");
	if (IS_ERR(dsi->clk)) {
		pr_err("failed to get dsi clock source\n");
		goto err_put_clk;
	}
	clk_enable(dsi->clk);

	dsi->address = (unsigned int)ioremap(DSI_IOBASE, 0x190);
	if (!dsi->address) {
		pr_err("Failed to ioremap register dsi memory region\n");
		goto err_iounmap;
	}

	mutex_init(&dsi->lock);
	dsim_ddi = mipi_dsi_bind_lcd_ddi(dsi, pdata->modes->name);
	if (!dsim_ddi) {
		pr_err("dsi->dev: mipi_dsim_ddi object not found.\n");
		ret = -EINVAL;
		goto err_bind_lcd;
	}

	if (dsim_ddi->dsim_lcd_drv && dsim_ddi->dsim_lcd_drv->probe)
		dsim_ddi->dsim_lcd_drv->probe(dsim_ddi->dsim_lcd_dev);
	if (mipi_dsih_read_word(dsi, R_DSI_HOST_PWR_UP) & 0x1) {
		dsi->state = UBOOT_INITIALIZED;
	}

	if(dsi->state == UBOOT_INITIALIZED) {

		dsi->dsi_phy->status = INITIALIZED;
		/*
		 * ######## BUG to be fix #######
		 * Uboot and kernel parameters are different. if we place
		 * dsi->state = INITIALIZED. the lcd display abnormal.
		 *
		 * */
#ifdef CONFIG_JZ_MIPI_DBI
		dsi->state = INITIALIZED; /*must be here for set_sequence function*/
#endif
	} else {

		if (dsim_ddi->dsim_lcd_drv && dsim_ddi->dsim_lcd_drv->power_on)
			dsim_ddi->dsim_lcd_drv->power_on(dsim_ddi->dsim_lcd_dev, 1);

		ret = jz_dsi_phy_open(dsi);
		if (ret) {
			goto err_phy_open;
		}

		/*set command mode */
		mipi_dsih_write_word(dsi, R_DSI_HOST_MODE_CFG, 0x1);
		/*set this register for cmd size, default 0x6 */
		mipi_dsih_write_word(dsi, R_DSI_HOST_EDPI_CMD_SIZE, 0x6);

		/*
		 * jz_dsi_phy_cfg:
		 * PLL programming, config the output freq to DEFAULT_DATALANE_BPS.
		 * */
		ret = jz_dsi_phy_cfg(dsi);
		if (ret) {
			goto err_phy_cfg;
		}

		pr_debug("wait for phy config ready\n");
		if (dsi->video_config->no_of_lanes == 2)
			st_mask = 0x95;
		else
			st_mask = 0x15;

		/*checkout phy clk lock and  clklane, datalane stopstate  */
		while ((mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS) & st_mask) !=
				st_mask && retry) {
			retry--;
			pr_info("phy status = %08x\n", mipi_dsih_read_word(dsi, R_DSI_HOST_PHY_STATUS));
		}

		if (!retry)
			goto err_phy_state;

		dsi->state = INITIALIZED; /*must be here for set_sequence function*/

		mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG,
				0xffffff0);

		if (dsim_ddi->dsim_lcd_drv && dsim_ddi->dsim_lcd_drv->set_sequence){
			dsim_ddi->dsim_lcd_drv->set_sequence(dsim_ddi->dsim_lcd_dev);
		}else{
			pr_err("lcd mipi panel init failed!\n");
			goto err_panel_init;
		}

		mipi_dsih_dphy_enable_hs_clk(dsi, 1);
		mipi_dsih_dphy_auto_clklane_ctrl(dsi, 1);

		mipi_dsih_write_word(dsi, R_DSI_HOST_CMD_MODE_CFG, 1);
	}

	dsi->suspended = false;

#ifdef CONFIG_DSI_DPI_DEBUG	/*test pattern */
	/*
	 * Wether the uboot dsi on or not. if kernel config test Pattern. we enable it
	 * when LCDC driver call video config. the color bar will appear. if LCDC keep
	 * transfering data. the suspend resume will display a different color bar.
	 * */
	unsigned int tmp = 0;

	tmp = mipi_dsih_read_word(dsi, R_DSI_HOST_VID_MODE_CFG);
	tmp |= 1 << 16 | 0 << 20 | 1 << 24;
	mipi_dsih_write_word(dsi, R_DSI_HOST_VID_MODE_CFG, tmp);
#endif


	return dsi;
err_panel_init:
	pr_err("dsi->dev: lcd mipi panel init error\n");
err_phy_state:
	pr_err("dsi->dev: jz dsi phy state error\n");
err_phy_cfg:
	pr_err("dsi->dev: jz dsi phy cfg error\n");
err_phy_open:
	pr_err("dsi->dev: jz dsi phy open error\n");
err_bind_lcd:
	iounmap((void __iomem *)dsi->address);
err_iounmap:
	clk_put(dsi->clk);
err_put_clk:
	kfree(dsi_phy);
err_dsi_phy:
	kfree(dsi);
err_dsi:
	return NULL;
}

void jzdsi_remove(struct dsi_device *dsi)
{
	struct dsi_phy *dsi_phy;
	dsi_phy = dsi->dsi_phy;

	iounmap((void __iomem *)dsi->address);
	clk_disable(dsi->clk);
	clk_put(dsi->clk);
	kfree(dsi_phy);
	kfree(dsi);
}
