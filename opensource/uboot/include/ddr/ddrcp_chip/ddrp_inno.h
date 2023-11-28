/*
 * Inno DDR PHY common data structure.
 *
 * Copyright (C) 2013 Ingenic Semiconductor Co.,Ltd
 * Author: Zoro <ykli@ingenic.cn>
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

#ifndef __DDRP_INNO_H__
#define __DDRP_INNO_H__

typedef union ddrp_rst {
	uint32_t d32;
	struct {
		unsigned int reserved0_1:2;
		unsigned int digital:1;
		unsigned int analog:1;
		unsigned int reserved4_31:28;
	}b;
}ddrp_rst_t;

#define PHY_RESET (0x0 & ~(1<<2 | 1<<3))

typedef union ddrp_mem_cfg {
	uint32_t d32;
	struct {
		unsigned int memsel:3;
		unsigned int brusel:1;
		unsigned int reserved5_31:28;
	}b;
}ddrp_mem_cfg_t;
/* } ddrp_memcfg_t; */

typedef union ddrp_dq_width {
	uint32_t d32;
	struct {
		unsigned int DQ_H:1;
		unsigned int DQ_L:1;
		unsigned int reserved2_31:30;
	}b;
}ddrp_dq_width_t;

typedef union ddrp_cl {
	uint32_t d32;
	struct {
		unsigned int CL:4;
	}b;
}ddrp_cl_t;

typedef union ddrp_cwl {
	uint32_t d32;
	struct {
		unsigned int CWL:4;
	}b;
}ddrp_cwl_t;

typedef union ddrp_pll_fbdiv {
	uint32_t d32;
	struct {
		unsigned int PLLDIV:8;
	}b;
}ddrp_pll_fbdiv_t;

typedef union ddrp_pll_ctrl {
	uint32_t d32;
	struct {
		unsigned int PLLPDEN:1;
		unsigned int PLLDIVH:1;
		unsigned int reserved2_31:30;
	}b;
}ddrp_ctrl_t;

typedef union ddrp_pll_pdiv {
	uint32_t d32;
	struct {
		unsigned int PLLPREDIV:5;
		unsigned int PLLPOSTDIV:3;
	}b;
}ddrp_pdiv_t;

typedef union ddrp_pll_lock {
	uint32_t d32;
	struct {
		unsigned int reserved0_6:7;
		unsigned int PLL_LOCK:1;
	}b;
}ddrp_lock_t;

typedef union ddrp_trainning_ctrl {
	uint32_t d32;
	struct {
		unsigned int DSACE:1;
		unsigned int DSCSE:1;
		unsigned int WLCTRL:1;
		unsigned int WLBYPASS:1;
	}b;
}ddrp_trainning_ctrl_t;

typedef union ddrp_calib_done {
	uint32_t d32;
	struct {
		unsigned int LDQCFA:1;
		unsigned int HDQCFA:1;
		unsigned int LDQCFB:1;
		unsigned int HDQCFB:1;
	}b;
}ddrp_done_t;

typedef union ddrp_calib_delay_al {
	uint32_t d32;
	struct {
		unsigned int DLLSELAL:3;
		unsigned int OPHCSELAL:1;
		unsigned int CYCLESELAL:3;
	}b;
}ddrp_delay_al_t;

typedef union ddrp_calib_delay_ah {
	uint32_t d32;
	struct {
		unsigned int DLLSELAH:3;
		unsigned int OPHCSELAH:1;
		unsigned int CYCLESELAH:3;
	}b;
}ddrp_delay_ah_t;

typedef union ddrp_calib_bypass_al {
	uint32_t d32;
	struct {
		unsigned int BP_DLLSELBH:3;
		unsigned int BP_OPHCSELBH:1;
		unsigned int BP_CYCLESELBH:3;
	}b;
}ddrp_bypass_al_t;

typedef union ddrp_calib_bypass_ah {
	uint32_t d32;
	struct {
		unsigned int BP_DLLSELBH:3;
		unsigned int BP_OPHCSELBH:1;
		unsigned int BP_CYCLESELBH:3;
	}b;
}ddrp_bypass_ah_t;

typedef union ddrp_wl_mode1 {
	uint32_t d32;
	struct {
		unsigned int WLMDL:8;
	}b;
}ddrp_wl_mode1_t;

typedef union ddrp_wl_mode2 {
	uint32_t d32;
	struct {
		unsigned int WLMDH:8;
	}b;
}ddrp_wl_mode2_t;

typedef union ddrp_wl_done {
	uint32_t d32;
	struct {
		unsigned int LWRLFA:1;
		unsigned int HWRLFA:1;
		unsigned int LWRLFB:1;
		unsigned int HWRLFB:1;
	}b;
}ddrp_wl_done_t;

typedef union ddrp_init_comp {
	uint32_t d32;
	struct {
		unsigned int NIT_COMP:1;
	}b;
}ddrp_init_comp_t;


struct ddrp_reg {
	ddrp_rst_t            rst;
	ddrp_mem_cfg_t        memcfg;
	ddrp_dq_width_t       dq_width;
	uint32_t              cl;
	uint32_t              cwl;
	ddrp_pll_fbdiv_t      pll_fbdiv;
	ddrp_ctrl_t           ctrl;
	ddrp_pdiv_t           pdiv;
	ddrp_lock_t           lock;
	ddrp_trainning_ctrl_t trainning_ctrl;
	ddrp_done_t           done;
	ddrp_delay_al_t       delay_al;
	ddrp_delay_ah_t       delay_ah;
	ddrp_bypass_al_t      bypass_al;
	ddrp_bypass_ah_t      bypass_ah;
	ddrp_wl_mode1_t       wl_mode1;
	ddrp_wl_mode2_t       wl_mode2;
	ddrp_wl_done_t        wl_done;
	ddrp_init_comp_t      init_comp;
};

#endif /* __DDRP_INNO_H__ */
