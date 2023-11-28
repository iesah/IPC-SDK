/* linux/drivers/video/jz_mipi_dsi/jz_mipi_dsi_lowlevel.c
 *
 * Ingenic SoC MIPI-DSI lowlevel driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <common.h>
#include <asm/io.h>
#include <jz_lcd/jz_dsim.h>
#include "jz_mipi_dsi_regs.h"
#include "jz_mipi_dsih_hal.h"

struct freq_ranges ranges[] = {
	{90, 0x00, 0x01}, /* 80-90  */
	{100, 0x10, 0x01},/* 90-100 */
	{110, 0x20, 0x01},/* 100-110 */
	{125, 0x01, 0x01},/* 110-125 */

	{140, 0x11, 0x01},/* 125-140 */
	{150, 0x21, 0x01},/* 140-150 */
	{160, 0x02, 0x01},/* 150-160 */
	{180, 0x12, 0x03},/* 160-180 */

	{200, 0x22, 0x03},/* 180-200 */
	{210, 0x03, 0x03},/* 200-210 */
	{240, 0x13, 0x03},/* 210-240 */
	{250, 0x23, 0x03},/* 240-250 */

	{270, 0x04, 0x07},/* 250-270 */
	{300, 0x14, 0x07},/* 270-300 */
	{330, 0x24, 0x07},/* 300-330 */
	{360, 0x15, 0x07},/* 330-360 */

	{400, 0x25, 0x07},/* 360-400 */
	{450, 0x06, 0x07},/* 400-450 */
	{500, 0x16, 0x07},/* 450-500 */
	{550, 0x07, 0x0f},/* 500-550 */
	{600, 0x17, 0x0f},/* 550-600 */

	{650, 0x08, 0x0f},/* 600-650 */
	{700, 0x18, 0x0f},/* 650-700 */
	{750, 0x09, 0x0f},/* 700-750 */
	{800, 0x19, 0x0f},/* 750-800 */

	{850, 0x0A, 0x0f},/* 800-850 */
	{900, 0x1A, 0x0f},/* 850-900 */
	{950, 0x2A, 0x0f},/* 900-950 */
	{1000, 0x3A, 0x0f}/* 950-1000 */
};

struct loop_band loop_bandwidth[] = {
	{32, 0x06, 0x10}, /* 12 - 32 */
	{64, 0x06, 0x10}, /* 33 - 64 */
	{128, 0x0C, 0x08},/* 65 - 128 */
	{256, 0x04, 0x04}, /* 129 - 256 */
	{512, 0x00, 0x01}, /* 257 - 512 */
	{768, 0x01, 0x01}, /* 513 - 768 */
	{1000, 0x02, 0x01} /* 769 - 1000 */
};

void jz_dsih_dphy_reset(struct dsi_device *dsi, int reset)
{

	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, reset, 1, 1);
}

void jz_dsih_dphy_stop_wait_time(struct dsi_device *dsi,
				 unsigned char no_of_byte_cycles)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_IF_CFG, no_of_byte_cycles, 8,
			     8);
}

void jz_dsih_dphy_no_of_lanes(struct dsi_device *dsi, unsigned char no_of_lanes)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_IF_CFG, no_of_lanes - 1, 0, 2);
}

void jz_dsih_dphy_clock_en(struct dsi_device *dsi, int en)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, en, 2, 1);
}

void jz_dsih_dphy_shutdown(struct dsi_device *dsi, int powerup)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, powerup, 0, 1);
}

void jz_dsih_dphy_ulpm_enter(struct dsi_device *dsi)
{
	/* PHY_STATUS[6:1] == 6'h00 */
	if (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 1, 6) == 0x0) {
		printf("MIPI D-PHY is already in ULPM state now\n");
		return;
	}
	/* PHY_RSTZ[3:0] = 4'hF */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, 0xF, 0, 4);
	/* PHY_ULPS_CTRL[3:0] = 4'h0 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_ULPS_CTRL, 0x0, 0, 4);
	/* PHY_TX_TRIGGERS[3:0] = 4'h0 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_TX_TRIGGERS, 0x0, 0, 4);
	/* PHY_STATUS[6:4] == 3'h3 */
	while (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 4, 3) != 0x3 ||
	       mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 2) != 0x1)
		;
	/* PHY_ULPS_CTRL [3:0] = 4'h5 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_ULPS_CTRL, 0x5, 0, 4);
	/* LPCLK_CTRL[1:0] = 2'h2 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_LPCLK_CTRL, 0x2, 0, 2);
	/* PHY_STATUS[6:0] == 7'h1 */
	while (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 7) != 0x1)
		;
	/* PHY_RSTZ[3] = 1'b0 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, 0x0, 3, 1);
	/* PHY_STATUS [0] == 1'b0 */
	while(mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 1) != 0x0)
		;
	printf("%s ...\n", __func__);
}

void jz_dsih_dphy_ulpm_exit(struct dsi_device *dsi)
{
	/* PHY_STATUS[6:1] == 6'h00 */
	if (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 1, 6) != 0x0) {
		printf("MIPI D-PHY is not in ULPM state now\n");
		return;
	}
	/* PHY_STATUS[0] == 1'b1 */
	if (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 1) == 0x1)
		goto step5;

	/* PHY_RSTZ [3] = 1'b1 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, 0x1, 3, 1);
	/* PHY_STATUS[0] == 1'b1 */
	while (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 1) != 0x1)
		;

step5:
	/* PHY_ULPS_CTRL[3:0] = 4'hF */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_ULPS_CTRL, 0xF, 0, 4);
	/* PHY_STATUS [5] == 1'b1 && PHY_STATUS [3] == 1'b1 */
	while(mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 5, 1) != 0x1 ||
          mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 3, 1) != 0x1)
		;
	/* Wait for 1 ms */
	mdelay(1);
	/* PHY_ULPS_CTRL [3:0] = 4'h0 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_ULPS_CTRL, 0x0, 0, 4);
	/* LPCLK_CTRL[1:0] = 2'h1 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_LPCLK_CTRL, 0x1, 0, 2);
	/* PHY_STATUS [6:4] == 3'h3 && PHY_STATUS [1:0] == 2'h1 */
	while (mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 4, 3) != 0x3 ||
           mipi_dsih_read_part(dsi, R_DSI_HOST_PHY_STATUS, 0, 2) != 0x1)
		;
	/* PHY_RSTZ [3] = 1'b0 */
	mipi_dsih_write_part(dsi, R_DSI_HOST_PHY_RSTZ, 0x0, 3, 1);
	printf("%s ...\n", __func__);
}

void jz_dsih_hal_power(struct dsi_device *dsi, int on)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_PWR_UP, on, 0, 1);
}

int jz_dsi_init_config(struct dsi_device *dsi)
{
	struct dsi_config *dsi_config;
	dsih_error_t err = OK;

	dsi_config = dsi->dsi_config;

	debug("jz_dsi_init_config\n");
	mipi_dsih_hal_dpi_color_mode_pol(dsi, !dsi_config->color_mode_polarity);
	mipi_dsih_hal_dpi_shut_down_pol(dsi, !dsi_config->shut_down_polarity);
	mipi_dsih_dphy_enable_auto_clk(dsi, dsi_config->auto_clklane_ctrl);
	err = mipi_dsih_phy_hs2lp_config(dsi, dsi_config->max_hs_to_lp_cycles);
	err |= mipi_dsih_phy_lp2hs_config(dsi, dsi_config->max_lp_to_hs_cycles);
	err |= mipi_dsih_phy_bta_time(dsi, dsi_config->max_bta_cycles);
	if (err) {
		return err;
	}

	mipi_dsih_hal_dpi_lp_during_hfp(dsi, 1);
	mipi_dsih_hal_dpi_lp_during_hbp(dsi, 1);

	mipi_dsih_hal_dpi_lp_during_vactive(dsi, 1);
	mipi_dsih_hal_dpi_lp_during_vfp(dsi, 1);
	mipi_dsih_hal_dpi_lp_during_vbp(dsi, 1);
	mipi_dsih_hal_dpi_lp_during_vsync(dsi, 1);
	mipi_dsih_hal_dcs_wr_tx_type(dsi, 0, 1);
	mipi_dsih_hal_dcs_wr_tx_type(dsi, 1, 1);
	mipi_dsih_hal_dcs_wr_tx_type(dsi, 3, 1);	/* long packet */
	mipi_dsih_hal_dcs_rd_tx_type(dsi, 0, 1);

	mipi_dsih_hal_gen_wr_tx_type(dsi, 0, 1);
	mipi_dsih_hal_gen_wr_tx_type(dsi, 1, 1);
	mipi_dsih_hal_gen_wr_tx_type(dsi, 2, 1);
	mipi_dsih_hal_gen_wr_tx_type(dsi, 3, 1);	/* long packet */
	mipi_dsih_hal_gen_rd_tx_type(dsi, 0, 1);
	mipi_dsih_hal_gen_rd_tx_type(dsi, 1, 1);
	mipi_dsih_hal_gen_rd_tx_type(dsi, 2, 1);
	mipi_dsih_hal_gen_rd_vc(dsi, 0);
	mipi_dsih_hal_gen_eotp_rx_en(dsi, 0);
	mipi_dsih_hal_gen_eotp_tx_en(dsi, 0);
	mipi_dsih_hal_bta_en(dsi, 0);
	mipi_dsih_hal_gen_ecc_rx_en(dsi, 0);
	mipi_dsih_hal_gen_crc_rx_en(dsi, 0);
	return OK;
}

void jz_init_dsi(struct dsi_device *dsi)
{

	jz_dsih_hal_power(dsi, 0);
	jz_dsih_hal_power(dsi, 1);
	jz_dsi_init_config(dsi);
	jz_dsih_hal_power(dsi, 0);
	jz_dsih_hal_power(dsi, 1);
}

void jz_mipi_dsi_init_interrupt(struct dsi_device *dsi)
{
	/* fiexed */
}

int jz_dsi_video_coding(struct dsi_device *dsi,
			unsigned short *bytes_per_pixel_x100,
			unsigned char *video_size_step,
			unsigned short *video_size)
{
	struct video_config *video_config;
	dsih_error_t err_code = OK;
	video_config = dsi->video_config;

	switch (video_config->color_coding) {
	case COLOR_CODE_16BIT_CONFIG1:
	case COLOR_CODE_16BIT_CONFIG2:
	case COLOR_CODE_16BIT_CONFIG3:
		*bytes_per_pixel_x100 = 200;
		*video_size_step = 1;
		break;
	case COLOR_CODE_18BIT_CONFIG1:
	case COLOR_CODE_18BIT_CONFIG2:
		mipi_dsih_hal_dpi_18_loosely_packet_en(dsi,
						       video_config->is_18_loosely);
		*bytes_per_pixel_x100 = 225;
		if (!video_config->is_18_loosely) {	// 18bits per pixel and NOT loosely, packets should be multiples of 4
			*video_size_step = 4;
			//round up active H pixels to a multiple of 4
			for (; ((*video_size) % 4) != 0; (*video_size)++) {
				;
			}
		} else {
			*video_size_step = 1;
		}
		break;
	case COLOR_CODE_24BIT:
		*bytes_per_pixel_x100 = 300;	//burst mode
		*video_size_step = 1;	//no burst mode
		break;
	default:
		printf("invalid color coding\n");
		err_code = ERR_DSI_COLOR_CODING;
		break;
	}
	if (err_code == OK) {
		debug("******&&&&&&&&---video_config->color_coding:%d\n",
		       video_config->color_coding);
		err_code =
		    mipi_dsih_hal_dpi_color_coding(dsi,
						   video_config->color_coding);
	}
	if (err_code != OK) {
		return err_code;
	}
	return 0;

}

void jz_dsi_dpi_cfg(struct dsi_device *dsi, unsigned int *ratio_clock_xPF,
		    unsigned short *bytes_per_pixel_x100)
{
	struct video_config *video_config;
	unsigned int hs_timeout = 0;
	int counter = 0;

	video_config = dsi->video_config;
	debug("ratio_clock_xPF------------------------->%d\n",
	       *ratio_clock_xPF);
	mipi_dsih_hal_dpi_video_mode_type(dsi, video_config->video_mode);

	/*HSA+HBP+HACT+HFP * 1 */
	mipi_dsih_hal_dpi_hline(dsi,
				(unsigned
				 short)((video_config->h_total_pixels *
					 (*ratio_clock_xPF)) /
					PRECISION_FACTOR));
	mipi_dsih_hal_dpi_hbp(dsi,
			      ((video_config->h_back_porch_pixels *
				(*ratio_clock_xPF)) / PRECISION_FACTOR));
	mipi_dsih_hal_dpi_hsa(dsi,
			      ((video_config->h_sync_pixels *
				(*ratio_clock_xPF)) / PRECISION_FACTOR));
	mipi_dsih_hal_dpi_vactive(dsi, video_config->v_active_lines);
	mipi_dsih_hal_dpi_vfp(dsi,
			      video_config->v_total_lines -
			      (video_config->v_back_porch_lines +
			       video_config->v_sync_lines +
			       video_config->v_active_lines));
	mipi_dsih_hal_dpi_vbp(dsi, video_config->v_back_porch_lines);
	mipi_dsih_hal_dpi_vsync(dsi, video_config->v_sync_lines);
	debug("hline:%d\n",
	       (unsigned
		short)((video_config->h_total_pixels * (*ratio_clock_xPF)) /
		       PRECISION_FACTOR));
	debug("hbp:%d\n",
	       ((video_config->h_back_porch_pixels * (*ratio_clock_xPF)) /
		PRECISION_FACTOR));
	debug("hsa:%d\n",
	       ((video_config->h_sync_pixels * (*ratio_clock_xPF)) /
		PRECISION_FACTOR));
	debug("vactive:%d\n", video_config->v_active_lines);
	debug("vfp:%d\n",
	       video_config->v_total_lines - (video_config->v_back_porch_lines +
					      video_config->v_sync_lines +
					      video_config->v_active_lines));
	debug("vbp:%d\n", video_config->v_back_porch_lines);
	debug("vsync:%d\n", video_config->v_sync_lines);

	mipi_dsih_hal_dpi_hsync_pol(dsi, !video_config->h_polarity);	//active low
	mipi_dsih_hal_dpi_vsync_pol(dsi, !video_config->v_polarity);	//active low
	mipi_dsih_hal_dpi_dataen_pol(dsi, !video_config->data_en_polarity);	// active high
	// HS timeout timing
	hs_timeout =
	    ((video_config->h_total_pixels * video_config->v_active_lines) +
	     (DSIH_PIXEL_TOLERANCE * (*bytes_per_pixel_x100)) / 100);
	for (counter = 0x80; (counter < hs_timeout) && (counter > 2); counter--) {
		if ((hs_timeout % counter) == 0) {
			mipi_dsih_hal_timeout_clock_division(dsi, counter);
			mipi_dsih_hal_lp_rx_timeout(dsi,
						    (unsigned short)(hs_timeout
								     /
								     counter));
			mipi_dsih_hal_hs_tx_timeout(dsi,
						    (unsigned short)(hs_timeout
								     /
								     counter));
			break;
		}
	}

}

void jz_dsih_hal_tx_escape_division(struct dsi_device *dsi,
				    unsigned char tx_escape_division)
{
	mipi_dsih_write_part(dsi, R_DSI_HOST_CLKMGR_CFG, tx_escape_division, 0,
			     8);
}

#if 0
void jz_dsih_dphy_configure(struct dsi_device *dsi,
			    unsigned char no_of_lanes, unsigned int output_freq)
{
	unsigned int loop_divider = 0;	/* (M) */
	unsigned int input_divider = 1;	/* (N) */
	unsigned char data[4];	/* maximum data for now are 4 bytes per test mode */
	unsigned char no_of_bytes = 0;
	unsigned char i = 0;	/* iterator */
	unsigned char range = 0;	/* ranges iterator */
	int flag = 0;
	struct dsi_phy *phy;
	phy = dsi->dsi_phy;
	debug("entry function :%s\n", __func__);
	if (output_freq < MIN_OUTPUT_FREQ) {
		return ERR_DSI_PHY_FREQ_OUT_OF_BOUND;
	}
	/* find M and N dividers */
	/* M :loop_divider, N: input_divider */
	for (input_divider = 1 + (phy->reference_freq / DPHY_DIV_UPPER_LIMIT); ((phy->reference_freq / input_divider) >= DPHY_DIV_LOWER_LIMIT) && (!flag); input_divider++) {	/* here the >= DPHY_DIV_LOWER_LIMIT is a phy constraint, formula should be above 1 MHz */
		if (((output_freq * input_divider) % (phy->reference_freq)) == 0) {	/*found the input_divider we want, but how can we be sure,
											 * for example ,now output_freq is 90000 , that's 90MHZ.
											 * if (90000 * input_divider) % fref == 0, and fref = 27000,
											 * then input_divider = 3;
											 * then loop_divider = 90000 * 3 / 27000 = 10;
											 * if input_divider = 6, then loop_divider is 20; flag = 1 exit loop.
											 */
			/* values found */
			loop_divider =
			    ((output_freq * input_divider) /
			     (phy->reference_freq));
			if (loop_divider >= 12) {
				flag = 1;
			}
		}
	}

	if ((!flag)
	    || ((phy->reference_freq / input_divider) < DPHY_DIV_LOWER_LIMIT)) {
		/* no exact value found in previous for loop */
		/* this solution is not favourable as jitter would be maximum */
		loop_divider = output_freq / DPHY_DIV_LOWER_LIMIT;
		input_divider = phy->reference_freq / DPHY_DIV_LOWER_LIMIT;
		/*make sure M is not overflow.M is mask 9bits(0 <= m <= 511)*/
		if (loop_divider > 511) {
			loop_divider = output_freq / (DPHY_DIV_LOWER_LIMIT * 2);
			input_divider = phy->reference_freq / (DPHY_DIV_LOWER_LIMIT * 2);
		}
	} else {		/* variable was incremented before exiting the loop */
		/*
		 * input_divider is 6 now,
		 * */
		input_divider--;
		/* now is 5.
		 * loop_divider is still 20.
		 * */
	}
	for (i = 0; (i < (sizeof(loop_bandwidth) / sizeof(loop_bandwidth[0])))
	     && (loop_divider > loop_bandwidth[i].loop_div); i++) {
		;
	}
	/* i = 0;
	 * */
	if (i >= (sizeof(loop_bandwidth) / sizeof(loop_bandwidth[0]))) {
		return ERR_DSI_PHY_FREQ_OUT_OF_BOUND;
	}
	/* get the PHY in power down mode (shutdownz=0) and reset it (rstz=0) to
	   avoid transient periods in PHY operation during re-configuration procedures. */
	jz_dsih_dphy_reset(dsi, 0);
	jz_dsih_dphy_clock_en(dsi, 0);
	jz_dsih_dphy_shutdown(dsi, 0);
	/* provide an initial active-high test clear pulse in TESTCLR  */
	//performs vendor-specific interface initialization
	mipi_dsih_dphy_test_clear(dsi, 1);
	mipi_dsih_dphy_test_clear(dsi, 0);
	/* find ranges */
	for (range = 0; (range < (sizeof(ranges) / sizeof(ranges[0])))
	     && ((output_freq / 1000) > ranges[range].freq); range++) {
		;
	}
	if (range >= (sizeof(ranges) / sizeof(ranges[0]))) {
		return ERR_DSI_PHY_FREQ_OUT_OF_BOUND;
	}
	/* set up board depending on environment if any */
	if (phy->bsp_pre_config != 0) {
		/*set master-phy output direction */
		phy->bsp_pre_config(dsi, 0);
	}
	/* setup digital part */
	/* hs frequency range [7]|[6:1]|[0] */
	data[0] = (0 << 7) | (ranges[range].hs_freq << 1) | 0;
	mipi_dsih_dphy_write(dsi, 0x44, data, 1);
	/* setup PLL */
	/* vco range  [7]|[6:3]|[2:1]|[0] */
	data[0] = (1 << 7) | (ranges[range].vco_range << 3) | (0 << 1) | 0;
	mipi_dsih_dphy_write(dsi, 0x10, data, 1);
	/* PLL  reserved|Input divider control|Loop Divider Control|Post Divider Ratio [7:6]|[5]|[4]|[3:0] */
	data[0] = (0x00 << 6) | (0x01 << 5) | (0x01 << 4) | (0x03 << 0);	/* post divider default = 0x03 - it is only used for clock out 2 */

	/* PLL post divider ratio and PLL input and divider ratios control */
	mipi_dsih_dphy_write(dsi, 0x19, data, 1);
	/* PLL Lock bypass|charge pump current [7:4]|[3:0] */
	data[0] = (0x00 << 4) | (loop_bandwidth[i].cp_current << 0);

	/* PLL CP control , PLL lock bypass for initialization and for ULP mode */
	mipi_dsih_dphy_write(dsi, 0x11, data, 1);
	/* bypass CP default|bypass LPF default| LPF resistor [7]|[6]|[5:0] */
	data[0] =
	    (0x01 << 7) | (0x01 << 6) | (loop_bandwidth[i].lpf_resistor << 0);
	mipi_dsih_dphy_write(dsi, 0x12, data, 1);
	/* PLL input divider ratio [7:0] */
	data[0] = input_divider - 1;

	/*PLL input divider ratio */
	mipi_dsih_dphy_write(dsi, 0x17, data, 1);
	no_of_bytes = 2;	/* pll loop divider (code 0x18) takes only 2 bytes (10 bits in data) */
	for (i = 0; i < no_of_bytes; i++) {
		data[i] =
		    ((unsigned char)((((loop_divider - 1) >> (5 * i)) & 0x1F) |
				     (i << 7)));
		/* 7 is dependent on no_of_bytes
		   make sure 5 bits only of value are written at a time */
	}
	/* *******PLL loop divider ratio - SET no|reserved|feedback divider [7]|[6:5]|[4:0] */
	mipi_dsih_dphy_write(dsi, 0x18, data, no_of_bytes);

	/*now recover the phy state as it before */
	jz_dsih_dphy_no_of_lanes(dsi, no_of_lanes);
	jz_dsih_dphy_stop_wait_time(dsi, 0x1C);
	jz_dsih_dphy_clock_en(dsi, 1);
	jz_dsih_dphy_shutdown(dsi, 1);
	jz_dsih_dphy_reset(dsi, 1);
	debug("configure master-phy is ok\n");
}
#endif

struct dphy_pll_range {
	unsigned int start_clk_sel;
	unsigned int output_freq0;	/*start freq in same resolution*/
	unsigned int output_freq1;	/*end freq in same resolution*/
	unsigned int resolution;
};

struct dphy_pll_range dphy_pll_table[] = {
	{0,   63750000,  93125000,   312500},
	{95,  93750000,  186250000,  625000},
	{244, 187500000, 372500000,  1250000},
	{393, 375000000, 745000000,  2500000},
	{542, 750000000, 2750000000UL, 5000000},
};

dsih_error_t jz_dsih_dphy_configure_x2000(struct dsi_device *dsi,
				    unsigned char no_of_lanes,
				    unsigned int output_freq)
{
	int i;
	struct dphy_pll_range *pll;
	unsigned int pll_clk_sel = 0xffffffff;
	for(i = 0; i < ARRAY_SIZE(dphy_pll_table); i++) {
		pll = &dphy_pll_table[i];
		if(output_freq >= pll->output_freq0 && output_freq <= pll->output_freq1) {
			pll_clk_sel = pll->start_clk_sel + (output_freq - pll->output_freq0) / pll->resolution;
			break;
		}
	}
	if(pll_clk_sel == 0xffffffff) {
		printf("can not find appropriate pll freq set for dsi phy! output_freq: %d\n", output_freq);
		return ERR_DSI_PHY_FREQ_OUT_OF_BOUND;
	}

	debug("before setting dsi phy: pll_clk_sel: %x\n", readl(dsi->dsi_phy->address + 0x64));
	writel(pll_clk_sel, (unsigned int *)(dsi->dsi_phy->address + 0x64));		/* pll_clk_sel */
	debug("after setting dsi phy: pll_clk_sel: %x, output_freq: %d\n", readl(dsi->dsi_phy->address + 0x64), output_freq);

	return OK;
}


static unsigned char calc_x2500_dphy_fbdiv(struct dsi_device* dsi)
{
	unsigned int real_mipi_clk = 0;
	real_mipi_clk = dsi->real_mipiclk / 1000000;	//  hz --- Mhz
	unsigned char fbdiv = 0;
	debug("%s   real_mipiclk = %d >>>>>>>>>>>>>>>>>>>>>>>>>>> \n",__func__,real_mipi_clk);
	fbdiv =  (real_mipi_clk + 20) * 4 / 12 + 1;
	return (fbdiv < 70)?70 : fbdiv;
}

static void init_dsi_phy_x2500(struct dsi_device *dsi)
{
	unsigned int temp = 0xffffffff;

	//power on ,reset, set pin_enable_ck/0/1/* of lanes to be used to high level and others to low
	debug("%s,%d  step0 do nothing now.\n", __func__, __LINE__);

	//step1
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x0C);
	temp &= ~0xff;
	temp |= 0x01;	//prediv
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x0C));
	if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x0C) & 0xff) != 0x01) {
		printf("%s,%d reg write error. Step1\n", __func__, __LINE__);
	}
	debug("reg:0x03, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x0C));
	//step2
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x10);
	temp &= ~0xff;
	temp |= calc_x2500_dphy_fbdiv(dsi);	//fbdiv
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x10));
	debug("reg:0x04, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x10));
	mdelay(20);
	//step3
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x04);
	temp &= ~0xff;
	temp |= 0xe4;		//PLL LDO
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x04));
	if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x04) & 0xff) != 0xe4) {
		printf("%s,%d reg write error. Step3\n", __func__, __LINE__);
	}
	debug("reg:0x01, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x04));
	//step4
	if (dsi->video_config->no_of_lanes == 4) {
		temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x00);
		temp &= ~0xff;
		temp |= 0x7d;     //4lane
		writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x00));
		if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x00) & 0xff) != 0x7d) {
			printf("%s,%d reg write error. Step4\n", __func__, __LINE__);
		}
		/* printf("%s>>>>>>>>>>>>>>>>>>>>>>>> config as 4 lane \n",__func__); */
	}
	else if (dsi->video_config->no_of_lanes == 2){
		temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x00);
		temp &= ~0xff;
		temp |= 0x4d;     //2lane
		writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x00));
		if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x00) & 0xff) != 0x4d) {
			printf("%s,%d reg write error. Step4\n", __func__, __LINE__);
		}
	}
	debug("reg:0x00, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x00));
	//step5
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x04);
	temp &= ~0xff;
	temp |= 0xe0;
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x04));
	if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x04) & 0xff) != 0xe0) {
		printf("%s,%d reg write error. Step5\n", __func__, __LINE__);
	}
	debug("reg:0x01, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x04));

	//step6
	mdelay(20);   //at lease 20ms, shortening need validation

	//step7
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x80);
	temp &= ~0xff;
	temp |= 0x1e;
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x80));
	if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x80) & 0xff) != 0x1e) {
		printf("%s,%d reg write error. Step7\n", __func__, __LINE__);
	}
	debug("reg:0x20, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x80));
	mdelay(5);
	//step8
	temp = *(volatile unsigned int*)(dsi->dsi_phy->address+0x80);
	temp &= ~0xff;
	temp |= 0x1f;
	writel(temp, (volatile unsigned int*)(dsi->dsi_phy->address+0x80));
	if ((*(volatile unsigned int*)(dsi->dsi_phy->address+0x80) & 0xff) != 0x1f) {
		printf("%s,%d reg write error. Step8\n", __func__, __LINE__);
	}
	debug("reg:0x20, value:0x%x\n", *(volatile unsigned int*)(dsi->dsi_phy->address+0x80));
	//step9
	mdelay(10);

	debug("%s,%d  dsi phy init over now...\n", __func__, __LINE__);
}

void jz_dsi_set_clock(struct dsi_device *dsi)
{
#ifdef CONFIG_X2000_V12 || CONFIG_M300
	jz_dsih_dphy_configure_x2000(dsi, dsi->video_config->no_of_lanes,
			       dsi->video_config->byte_clock * 8 * 1000);
#else
	init_dsi_phy_x2500(dsi);
#endif
	jz_dsih_dphy_stop_wait_time(dsi, 0x1C);
	jz_dsih_dphy_clock_en(dsi, 1);
	jz_dsih_dphy_shutdown(dsi, 1);
	jz_dsih_dphy_reset(dsi, 1);
	jz_dsih_hal_tx_escape_division(dsi, 7);
	return OK;
}
