/*
 * DDR driver for Synopsys DWC DDR PHY.
 * Used by Jz4775, JZ4780...
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

/*#define DEBUG*/
/*#define DEBUG_READ_WRITE */
/*#define CONFIG_DDR_DEBUG 1 */
#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>
#include <asm/cacheops.h>
#include <asm/dma-default.h>

#include <asm/io.h>
#include <asm/arch/clk.h>
#include "ddr_debug.h"
#define ddr_hang() do{						\
		printf("%s %d\n",__FUNCTION__,__LINE__);	\
		hang();						\
	}while(0)

DECLARE_GLOBAL_DATA_PTR;

#ifdef  CONFIG_DDR_DEBUG
#define FUNC_ENTER() printf("%s enter.\n",__FUNCTION__);
#define FUNC_EXIT() printf("%s exit.\n",__FUNCTION__);

static void dump_ddrc_register(void)
{
	printf("DDRC_STATUS         0x%x\n", ddr_readl(DDRC_STATUS));
	printf("DDRC_CFG            0x%x\n", ddr_readl(DDRC_CFG));
	printf("DDRC_CTRL           0x%x\n", ddr_readl(DDRC_CTRL));
	printf("DDRC_LMR            0x%x\n", ddr_readl(DDRC_LMR));
	printf("DDRC_DLP            0x%x\n", ddr_readl(DDRC_DLP));
	printf("DDRC_TIMING1        0x%x\n", ddr_readl(DDRC_TIMING(1)));
	printf("DDRC_TIMING2        0x%x\n", ddr_readl(DDRC_TIMING(2)));
	printf("DDRC_TIMING3        0x%x\n", ddr_readl(DDRC_TIMING(3)));
	printf("DDRC_TIMING4        0x%x\n", ddr_readl(DDRC_TIMING(4)));
	printf("DDRC_TIMING5        0x%x\n", ddr_readl(DDRC_TIMING(5)));
	printf("DDRC_REFCNT         0x%x\n", ddr_readl(DDRC_REFCNT));
	printf("DDRC_AUTOSR_CNT     0x%x\n", ddr_readl(DDRC_AUTOSR_CNT));
	printf("DDRC_AUTOSR_EN      0x%x\n", ddr_readl(DDRC_AUTOSR_EN));
	printf("DDRC_MMAP0          0x%x\n", ddr_readl(DDRC_MMAP0));
	printf("DDRC_MMAP1          0x%x\n", ddr_readl(DDRC_MMAP1));
	printf("DDRC_REMAP1         0x%x\n", ddr_readl(DDRC_REMAP(1)));
	printf("DDRC_REMAP2         0x%x\n", ddr_readl(DDRC_REMAP(2)));
	printf("DDRC_REMAP3         0x%x\n", ddr_readl(DDRC_REMAP(3)));
	printf("DDRC_REMAP4         0x%x\n", ddr_readl(DDRC_REMAP(4)));
	printf("DDRC_REMAP5         0x%x\n", ddr_readl(DDRC_REMAP(5)));
	printf("DDRC_DWCFG          0x%x\n", ddr_readl(DDRC_DWCFG));
	printf("DDRC_DWSTATUS          0x%x\n", ddr_readl(DDRC_DWSTATUS));

	printf("DDRC_HREGPRO        0x%x\n", ddr_readl(DDRC_HREGPRO));
	printf("DDRC_PREGPRO        0x%x\n", ddr_readl(DDRC_PREGPRO));
	printf("DDRC_CGUC0          0x%x\n", ddr_readl(DDRC_CGUC0));
	printf("DDRC_CGUC1          0x%x\n", ddr_readl(DDRC_CGUC1));

}

static void dump_ddrp_register(void)
{
	printf("DDRP_INNOPHY_INNO_PHY_RST    0x%x\n", ddr_readl(DDRP_INNOPHY_INNO_PHY_RST	));
	printf("DDRP_INNOPHY_MEM_CFG         0x%x\n", ddr_readl(DDRP_INNOPHY_MEM_CFG		));
	printf("DDRP_INNOPHY_TRAINING_CTRL   0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_CTRL	));
	printf("DDRP_INNOPHY_CALIB_DELAY_AL  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL	));
	printf("DDRP_INNOPHY_CALIB_DELAY_AH  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH	));
	printf("DDRP_INNOPHY_CALIB_DELAY_BL  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL	));
	printf("DDRP_INNOPHY_CALIB_DELAY_BH  0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH	));
	printf("DDRP_INNOPHY_CL              0x%x\n", ddr_readl(DDRP_INNOPHY_CL				));
	printf("DDRP_INNOPHY_CWL             0x%x\n", ddr_readl(DDRP_INNOPHY_CWL			));
	printf("DDRP_INNOPHY_WL_DONE         0x%x\n", ddr_readl(DDRP_INNOPHY_WL_DONE		));
	printf("DDRP_INNOPHY_CALIB_DONE      0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE		));
	printf("DDRP_INNOPHY_PLL_LOCK        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_LOCK		));
	printf("DDRP_INNOPHY_PLL_FBDIV       0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_FBDIV		));
	printf("DDRP_INNOPHY_PLL_CTRL        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_CTRL		));
	printf("DDRP_INNOPHY_PLL_PDIV        0x%x\n", ddr_readl(DDRP_INNOPHY_PLL_PDIV		));

    	printf("DDRP_INNOPHY_TRAINING_2c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_2c	));
	printf("DDRP_INNOPHY_TRAINING_3c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_3c	));
	printf("DDRP_INNOPHY_TRAINING_4c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_4c	));
	printf("DDRP_INNOPHY_TRAINING_5c     0x%x\n", ddr_readl(DDRP_INNOPHY_TRAINING_5c	));
}

static void dump_inno_driver_strength_register(void)
{
    	printf("inno reg:0x42 = 0x%x\n", readl(0xb3011108));
    	printf("inno reg:0x41 = 0x%x\n", readl(0xb3011104));
    	printf("inno cmd io driver strenth pull_down                = 0x%x\n", readl(0xb30112c0));
    	printf("inno cmd io driver strenth pull_up                  = 0x%x\n", readl(0xb30112c4));

    	printf("inno clk io driver strenth pull_down                = 0x%x\n", readl(0xb30112c8));
    	printf("inno clk io driver strenth pull_up                  = 0x%x\n", readl(0xb30112cc));

    	printf("Channel A data io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011300));
    	printf("Channel A data io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011304));
    	printf("Channel A data io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb3011340));
   	printf("Channel A data io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb3011344));
    	printf("Channel B data io ODT DQ[7:0]  pull_down               = 0x%x\n", readl(0xb3011380));
    	printf("Channel B data io ODT DQ[7:0]  pull_up                 = 0x%x\n", readl(0xb3011384));
    	printf("Channel B data io ODT DQ[15:8] pull_down               = 0x%x\n", readl(0xb30113c0));
    	printf("Channel B data io ODT DQ[15:8] pull_up                 = 0x%x\n", readl(0xb30113c4));

    	printf("Channel A data io driver strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011308));
    	printf("Channel A data io driver strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb301130c));
    	printf("Channel A data io driver strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb3011348));
    	printf("Channel A data io driver strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb301134c));
    	printf("Channel B data io driver strenth DQ[7:0]  pull_down    = 0x%x\n", readl(0xb3011388));
    	printf("Channel B data io driver strenth DQ[7:0]  pull_up      = 0x%x\n", readl(0xb301138c));
    	printf("Channel B data io driver strenth DQ[15:8] pull_down    = 0x%x\n", readl(0xb30113c8));
    	printf("Channel B data io driver strenth DQ[15:8] pull_up      = 0x%x\n", readl(0xb30113cc));

}

static void dump_ddr_phy_cfg_per_bit_de_skew_register(void)
{
    u32 idx = 0;

	for (idx = 0; idx <= 0x1b; idx++) {
		printf("PHY REG-0x100+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x100+idx)*4)));
	}

	for (idx = 0; idx <= 0xa; idx++) {
		printf("PHY REG-0x120+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x120+idx)*4)));
	}
	for (idx = 0xb; idx <= 0x15; idx++) {
		printf("PHY REG-0x120+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x120+idx)*4)));
	}
	for (idx = 0; idx <= 0xa; idx++) {
		printf("PHY REG-0x1a0+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x1a0+idx)*4)));
	}
	for (idx = 0xb; idx <= 0x15; idx++) {
		printf("PHY REG-0x1a0+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x1a0+idx)*4)));
	}
	for (idx = 0x0; idx <= 0x1b; idx++) {
		printf("PHY REG-0x100+0x%x :  0x%x \n",idx, readl(0xb3011000+((0x100+idx)*4)));
	}
}
#else
#define FUNC_ENTER()
#define FUNC_EXIT()
#define dump_ddrc_register()
#define dump_ddrp_register()
#define dump_inno_driver_strength_register()
#define dump_ddr_phy_cfg_per_bit_de_skew_register()
#endif

static void mem_remap(void)
{
	int i;
	unsigned int remap_array[] = REMMAP_ARRAY;
	for(i = 0;i < ARRAY_SIZE(remap_array);i++)
	{
		ddr_writel(remap_array[i],DDRC_REMAP(i+1));
	}
}
#define ddr_writel(value, reg)	writel((value), DDRC_BASE + reg)

static void ddr_phyreg_set_range(u32 offset, u32 startbit, u32 bitscnt, u32 value)
{
	u32 reg = 0;
	u32 mask = 0;
	mask = ((0xffffffff>>startbit)<<(startbit))&((0xffffffff<<(32-startbit-bitscnt))>>(32-startbit-bitscnt));
	reg = readl(DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
	reg = (reg&(~mask))|((value<<startbit)&mask);
	//printf("value = %x, reg = %x, mask = %x", value, reg, mask);
	writel(reg, DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
}

static u32 ddr_phyreg_get(u32 offset)
{
	return  readl(DDRC_BASE+DDR_PHY_OFFSET+(offset*4));
}

static void ddrc_post_init(void)
{
	ddr_writel(DDRC_REFCNT_VALUE, DDRC_REFCNT);
#ifdef CONFIG_DDR_TYPE_DDR3
	mem_remap();
#endif
	debug("DDRC_STATUS: %x\n",ddr_readl(DDRC_STATUS));
	ddr_writel(DDRC_CTRL_VALUE, DDRC_CTRL);

	ddr_writel(DDRC_CGUC0_VALUE, DDRC_CGUC0);
	ddr_writel(DDRC_CGUC1_VALUE, DDRC_CGUC1);

}

static void ddrc_prev_init(void)
{
	FUNC_ENTER();
	/* DDRC CFG init*/
	/* /\* DDRC CFG init*\/ */
	/* ddr_writel(DDRC_CFG_VALUE, DDRC_CFG); */

    	/* DDRC timing init*/
	ddr_writel(DDRC_TIMING1_VALUE, DDRC_TIMING(1));
	ddr_writel(DDRC_TIMING2_VALUE, DDRC_TIMING(2));
	ddr_writel(DDRC_TIMING3_VALUE, DDRC_TIMING(3));
	ddr_writel(DDRC_TIMING4_VALUE, DDRC_TIMING(4));
	ddr_writel(DDRC_TIMING5_VALUE, DDRC_TIMING(5));

	/* DDRC memory map configure*/
	ddr_writel(DDRC_MMAP0_VALUE, DDRC_MMAP0);
	ddr_writel(DDRC_MMAP1_VALUE, DDRC_MMAP1);

	/* ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); */
	ddr_writel(DDRC_CTRL_VALUE & ~(7 << 12), DDRC_CTRL);

	FUNC_EXIT();
}

static enum ddr_type get_ddr_type(void)
{
	int type;
	ddrc_cfg_t ddrc_cfg;
	ddrc_cfg.d32 = DDRC_CFG_VALUE;
	switch(ddrc_cfg.b.TYPE){
	case 3:
		type = LPDDR;
		break;
	case 4:
		type = DDR2;
		break;
	case 5:
		type = LPDDR2;
		break;
	case 6:
		type = DDR3;
		break;
	default:
		type = UNKOWN;
		debug("unsupport ddr type!\n");
		ddr_hang();
	}
	return type;
}

static void ddrc_reset_phy(void)
{
	FUNC_ENTER();
	ddr_writel(0xf << 20, DDRC_CTRL);
	mdelay(1);
	ddr_writel(0x8 << 20, DDRC_CTRL);  //dfi_reset_n low for innophy
	mdelay(1);
	FUNC_EXIT();
}

static struct jzsoc_ddr_hook *ddr_hook = NULL;
void register_ddr_hook(struct jzsoc_ddr_hook * hook)
{
	ddr_hook = hook;
}

static void ddrp_pll_init(void)
{
	unsigned int val;
	unsigned int timeout = 1000000;

	/* fbdiv={reg21[0],reg20[7:0]};
	 * pre_div = reg22[4:0];
	 * post_div = 2^reg22[7:5];
	 * fpll_2xclk = fpll_refclk * fbdiv / ( post_div * pre_div);
	 *			  = fpll_refclk * 20 / ( 1 * 10 ) = fpll_refclk * 2;
	 * fpll_1xclk = fpll_2xclk / 2 = fpll_refclk;
	 * Use: IN:fpll_refclk, OUT:fpll_1xclk */

	/* reg20; bit[7:0]fbdiv M[7:0] */

	val = ddr_readl(DDRP_INNOPHY_PLL_FBDIV);
	val &= ~(0xff);
	val |= 0x10;
	ddr_writel(val, DDRP_INNOPHY_PLL_FBDIV);

	/* reg21; bit[7:2]reserved; bit[1]PLL reset; bit[0]fbdiv M8 */
	val = ddr_readl(DDRP_INNOPHY_PLL_CTRL);
	val &= ~(0xff);
	val |= 0x1a;
	ddr_writel(val, DDRP_INNOPHY_PLL_CTRL);

	/* reg22; bit[7:5]post_div; bit[4:0]pre_div */
	val = ddr_readl(DDRP_INNOPHY_PLL_PDIV);
	val &= ~(0xff);
	val |= 0x4;
	ddr_writel(val, DDRP_INNOPHY_PLL_PDIV);

	/* reg21; bit[7:2]reserved; bit[1]PLL reset; bit[0]fbdiv M8 */
	val = ddr_readl(DDRP_INNOPHY_PLL_CTRL);
	val &= ~(0xff);
	val |= 0x18;
	ddr_writel(val, DDRP_INNOPHY_PLL_CTRL);

	/* reg1f; bit[7:4]reserved; bit[3:0]DQ width
	 * DQ width bit[3:0]:0x1:8bit,0x3:16bit,0x7:24bit,0xf:32bit */
	val = ddr_readl(DDRP_INNOPHY_DQ_WIDTH);
	val &= ~(0xf);
#if CONFIG_DDR_DW32
	val |= 0xf; /* 32bit */
#else
	val |= DDRP_DQ_WIDTH_DQ_H | DDRP_DQ_WIDTH_DQ_L; /* 0x3:16bit */
#endif
	ddr_writel(val, DDRP_INNOPHY_DQ_WIDTH);

	/* reg1; bit[7:5]reserved; bit[4]burst type; bit[3:2]reserved; bit[1:0]DDR mode */
	val = ddr_readl(DDRP_INNOPHY_MEM_CFG);
	val &= ~(0xff);
#ifdef CONFIG_DDR_TYPE_DDR3
	val |= 0x10; /* burst 8 */
#elif defined(CONFIG_DDR_TYPE_DDR2)
	val |= 0x11; /* burst 8 ddr2 */
#endif
	ddr_writel(val, DDRP_INNOPHY_MEM_CFG);

	/* bit[3]reset digital core; bit[2]reset analog logic; bit[0]Reset Initial status
	 * other reserved */
	val = ddr_readl(DDRP_INNOPHY_INNO_PHY_RST);
	val &= ~(0xff);
	val |= 0x0d;
	ddr_writel(val, DDRP_INNOPHY_INNO_PHY_RST);

	/* CWL */
	val = ddr_readl(DDRP_INNOPHY_CWL);
	val &= ~(0xf);
	val |= DDRP_CWL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CWL);

	/* CL */
	val = ddr_readl(DDRP_INNOPHY_CL);
	val &= ~(0xf);
	val |= DDRP_CL_VALUE;
	ddr_writel(val, DDRP_INNOPHY_CL);

	/* AL */
	val = ddr_readl(DDRP_INNOPHY_AL);
	val &= ~(0xf);
	val = 0x0;
	ddr_writel(val, DDRP_INNOPHY_AL);

	while(!(ddr_readl(DDRP_INNOPHY_PLL_LOCK) & 1 << 3) && timeout--);
	if(!timeout) {
		printf("DDRP_INNOPHY_PLL_LOCK time out!!!\n");
		hang();
	}
	//mdelay(20);
}

union ddrp_calib {
	/** raw register data */
	uint8_t d8;
	/** register bits */
	struct {
		unsigned dllsel:3;
		unsigned ophsel:1;
		unsigned cyclesel:3;
		unsigned reserved7:1;
	} calib;					/* calib delay/bypass al/ah */
};
static void ddrp_software_writeleveling_tx(void)
{
    int k,i,j,a,b,middle;
    int ret = -1;
    unsigned int addr = 0xa2000000,val;
    unsigned int reg_val;
    unsigned int ddbuf[8] = {0};
    unsigned int pass_invdelay_total[64] = {0};
    unsigned int pass_cnt = 0;
    unsigned int offset_addr_tx[4] = {0xb3011484,0xb30114b0,0xb3011684,0xb30116b0}; //A_DQ0 , A_DQ8, B_DQ0, B_DQ8 rx invdelay control register

    //ddr_writel((0x8|ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)), DDRP_INNOPHY_TRAINING_CTRL);

    writel(0x8, 0xb30114a4); //A_DQS0
    writel(0x8, 0xb30114d0); //A_DQS1
    writel(0x8, 0xb30116a4); //B_DQS0
    writel(0x8, 0xb30116d0); //B_DQS1

    writel(0x8, 0xb30114a8); //A_DQSB0
    writel(0x8, 0xb30114d4); //A_DQSB1
    writel(0x8, 0xb30116a8); //B_DQSB0
    writel(0x8, 0xb30116d4); //B_DQSB1

    for (j=0; j<=0x3; j++) {
        for(k=0; k<=0x7; k++) { // DQ0~DQ7
            pass_cnt = 0;
            for(i=0; i<=0x4; i++) { //invdelay from 0 to 3f

                reg_val = readl(offset_addr_tx[j]+k*4) & ~0x3f;
                writel((i | reg_val) , (offset_addr_tx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_tx[j]+k*2 is the addr

                //mdelay(5);
			    for(a = 0; a < 0xff; a++) {

                    val = 0x12345678 + (j + k + i + a)*4;

                    *(volatile unsigned int *)(addr + a * 4) = val;

                    if(((*(volatile unsigned int *)(addr + a * 4)) & 0xff000000) != (val & 0xff000000) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x00ff0000) != (val & 0x00ff0000) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x0000ff00) != (val & 0x0000ff00) && ((*(volatile unsigned int *)(addr + a * 4)) & 0x000000ff) != (val & 0x000000ff)) {
                        printf(" AL fail VALUE 0x%x  sVALUE 0x%x error \n", val ,(*(volatile unsigned int *)(addr + i * 4)));
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    pass_cnt++;
                }
            }

            middle = pass_cnt / 2;
            writel((pass_invdelay_total[middle] | reg_val), (offset_addr_tx[j]+k*4));    //this means step 3
            printf("\nTX  chan%d DQ%d small_value = %d big value %d  size = 0x%x middle = %x\n",j, k, pass_invdelay_total[0], pass_invdelay_total[pass_cnt-1], (pass_invdelay_total[pass_cnt-1] - pass_invdelay_total[0]), pass_invdelay_total[middle]);
        }
    }
}
static void ddrp_software_writeleveling_rx(void)
{
    int k,i,j,a,b,c,d,e,middle;
    int ret = -1;
    unsigned int addr = 0xa1000000;
    unsigned int val[0xff],value,tmp;
    unsigned int reg_val;
    unsigned int ddbuf[8] = {0};
    unsigned int pass_invdelay_total[64] = {0};
    unsigned int pass_dqs_total[64] = {0};
    unsigned int pass_dqs_value[64] = {0};
    unsigned int dq[8],middle_dq=0,max_dqs = 0;
    unsigned int pass_cnt = 0, pass_cnt_dqs = 0;
    unsigned int offset_addr_rx[4] = {0xb3011504,0xb3011530,0xb3011704,0xb3011730}; //A_DQ0 , A_DQ8, B_DQ0, B_DQ8 rx invdelay control register
    unsigned int offset_addr_dqs[4] = {0xb3011524,0xb3011550,0xb3011724,0xb3011750}; //A_DQS0 , A_DQS1, B_DQS0, B_DQS1 rx invdelay control register


    ddr_writel((0x8|ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)), DDRP_INNOPHY_TRAINING_CTRL);
#if 1

    /* this is set default value */
    value = 0x1f;
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x20)*4);///DM0
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x2b)*4);///DM1

    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x20)*4);///DM0
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x2b)*4);///DM1
    //RX DQ
    for (i = 0x21; i <= 0x28;i++) {
	    ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
	    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
    }

    //DQS
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x29)*4);///DQS0
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x34)*4);///DQS1

	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x29)*4);///DQS0
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x34)*4);///DQS1
    for (i = 0x2c; i <= 0x33;i++) {
	    ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
	    ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
    }

#endif
#if 1
    for(j=0; j<=0x3; j++) {
        for(k=0; k<=0x7; k++) { // DQ0~DQ7
            pass_cnt = 0;

            for(i=0x20; i<=0x3f; i++) { //invdelay from 0 to 63
                reg_val = 0;
                reg_val = readl(offset_addr_rx[j]+k*4) & ~0x3f;
                writel((i | reg_val), (offset_addr_rx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_rx[j]+k*4 is the addr

                //mdelay(5);
                for(a = 0; a < 0xff; a ++) {
                    val[a] = 0x12345678 + (j + k + i + a)*4;
                    *(volatile unsigned int *)(addr + a * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*2) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*3) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*4) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*5) * 4) = val[a];
                    *(volatile unsigned int *)(addr + (a + 0xff*6) * 4) = val[a];
                }

                //flush_dcache_range(addr, addr+(0xff*4));
		        //flush_l2cache_range(addr, addr+(0xff*4));

                for(a = 0; a < 0xff; a ++) {
                    if(((*(volatile unsigned int *)(addr + a * 4)) != val[a]) && ((*(volatile unsigned int *)(addr + (a + 0xff*3) * 4)) != val[a])) {
                        //printf("want = 0x%x value1 = 0x%x  value2 = 0x%x value3 = 0x%x value4 = 0x%x \n", val1, tmp, tmp1, tmp2, tmp3);
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    //printf("\nRX  chan%d DQ%d pass_value = %d \n",j, k, i);
                    pass_cnt++;
                } else {
                    printf("##### this is the big :2222 pass_cnt = %d\n", pass_cnt);
                    break;
                }

            }
            for(i=0x1f; i>=0x0; i--) { //invdelay from 0 to 63
                reg_val = 0;
                reg_val = readl(offset_addr_rx[j]+k*4) & ~0x3f;
                writel((i | reg_val), (offset_addr_rx[j]+k*4)); // set invdelay, i is invdelay value , offset_addr_rx[j]+k*4 is the addr

                //mdelay(5);
                for(a = 0; a < 0xff; a ++) {
                    val[a] = 0x12345678 + (j + k + i + a)*4;
                    *(volatile unsigned int *)(addr + a * 4) = val[a];
                }
                //flush_dcache_range(addr, addr+(0xff*4));
		        //flush_l2cache_range(addr, addr+(0xff*4));

                for(a = 0; a < 0xff; a ++) {
                    if((*(volatile unsigned int *)(addr + a * 4)) != val[a]) {
                        //printf("want = 0x%x value1 = 0x%x  value2 = 0x%x value3 = 0x%x value4 = 0x%x \n", val1, tmp, tmp1, tmp2, tmp3);
                        break;
                    }
                }

                if(a == 0xff) {
                    pass_invdelay_total[pass_cnt] = i;
                    //printf("\nRX  chan%d DQ%d pass_value = %d \n",j, k, i);
                    pass_cnt++;
                } else {
                    printf("##### this is the small:2222 pass_cnt = %d\n", pass_cnt);
                    break;
                }
            }

            for(a=0; a<pass_cnt-1; a++) {
                for(b=0; b<pass_cnt-a-1; b++) {
                    if(pass_invdelay_total[b] > pass_invdelay_total[b+1]) {
                        tmp= pass_invdelay_total[b];
                        pass_invdelay_total[b] = pass_invdelay_total[b+1];
                        pass_invdelay_total[b+1] = tmp;
                    }
                }
            }
            for(a=0; a<pass_cnt;a++) {
                printf("##### pass_invdelay_total[%d] value %d\n", a, pass_invdelay_total[a]);
            }
            printf("##### this is the small:3333\n");
            middle = pass_cnt / 2;
            writel(pass_invdelay_total[middle], (offset_addr_rx[j]+k*4));    //this means step 3
            printf("\nRX  chan%d DQ%d small_value = %d big value %d  size = 0x%x middle = %x\n",j, k, pass_invdelay_total[0], pass_invdelay_total[pass_cnt-1], (pass_invdelay_total[pass_cnt-1] - pass_invdelay_total[0]), pass_invdelay_total[middle]);
        }
    }

#endif
}
static void ddrp_hardware_calibration(void)
{
	unsigned int val;
	unsigned int timeout = 1000000;

#if 0
#ifdef CONFIG_DDR_TYPE_DDR3
	unsigned int i = 0;
	ddr_writel(0x4, DDRP_INNOPHY_INNO_WR_LEVEL1);
	ddr_writel(0x40, DDRP_INNOPHY_INNO_WR_LEVEL2);
	ddr_writel(0xa4, DDRP_INNOPHY_TRAINING_CTRL);
	while (((ddr_readl(DDRP_INNOPHY_WL_DONE) & 0xf) != 0xf) && timeout--);
	if(!timeout) {
		printf("timeout:INNOPHY_WL_DONE %x\n", ddr_readl(DDRP_INNOPHY_WL_DONE));
		hang();
	}
	ddr_writel(0xa0, DDRP_INNOPHY_TRAINING_CTRL);
#endif
#endif

#ifdef CONFIG_DDR_TYPE_DDR3
    	ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)|0x1, DDRP_INNOPHY_TRAINING_CTRL);
	do
	{
		val = ddr_readl(DDRP_INNOPHY_CALIB_DONE);
	} while (((val & 0xf) != 0xf) && timeout--);

	if(!timeout) {
		printf("timeout:INNOPHY_CALIB_DONE %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
		hang();
	}
   	ddr_writel(ddr_readl(DDRP_INNOPHY_TRAINING_CTRL)&(~0x1), DDRP_INNOPHY_TRAINING_CTRL);
#else
	ddr_writel(0xa9, DDRP_INNOPHY_TRAINING_CTRL);
	do
	{
		val = ddr_readl(DDRP_INNOPHY_CALIB_DONE);
	} while (((val & 0xf) != 0xf) && timeout--);

	if(!timeout) {
		printf("timeout:INNOPHY_CALIB_DONE %x\n", ddr_readl(DDRP_INNOPHY_CALIB_DONE));
		hang();
	}
	ddr_writel(0xa8, DDRP_INNOPHY_TRAINING_CTRL);
#endif
	{
		union ddrp_calib al, ah, bl, bh;
		al.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL_RESULT);
		//printf("auto :CALIB_AL: dllsel %x, ophsel %x, cyclesel %x\n", al.calib.dllsel, al.calib.ophsel, al.calib.cyclesel);
		ah.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH_RESULT);
		//printf("auto:CAHIB_AH: dllsel %x, ophsel %x, cyclesel %x\n", ah.calib.dllsel, ah.calib.ophsel, ah.calib.cyclesel);
		bl.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL_RESULT);
		//printf("auto :CALIB_BL: dllsel %x, ophsel %x, cyclesel %x\n", bl.calib.dllsel, bl.calib.ophsel, bl.calib.cyclesel);
		bh.d8 = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH_RESULT);
		//printf("auto:CAHIB_BH: dllsel %x, ophsel %x, cyclesel %x\n", bh.calib.dllsel, bh.calib.ophsel, bh.calib.cyclesel);
	}
}

static void ddr_phy_cfg_driver_odt(void)
{
	/* ddr phy driver strength and  ODT config */

#ifdef CONFIG_T40A
	/* cmd */
	ddr_phyreg_set_range(0xb0, 0, 5, 0xf);
	ddr_phyreg_set_range(0xb1, 0, 5, 0xf);
	/* ck */
	ddr_phyreg_set_range(0xb2, 0, 5, 0x11); //pull down
	ddr_phyreg_set_range(0xb3, 0, 5, 0x11);//pull up

	/* DQ ODT config */
	u32 dq_odt0 = 0x3;
	u32 dq_odt1 = 0x3;
	/* Channel A */
	ddr_phyreg_set_range(0xc0, 0, 5, dq_odt0);
	ddr_phyreg_set_range(0xc1, 0, 5, dq_odt0);
	ddr_phyreg_set_range(0xd0, 0, 5, dq_odt0);
	ddr_phyreg_set_range(0xd1, 0, 5, dq_odt0);
	/* Channel B */
	ddr_phyreg_set_range(0xe0, 0, 5, dq_odt1);
	ddr_phyreg_set_range(0xe1, 0, 5, dq_odt1);
	ddr_phyreg_set_range(0xf0, 0, 5, dq_odt1);
	ddr_phyreg_set_range(0xf1, 0, 5, dq_odt1);

	/* driver strength config */
	u32 dq_ds = 0x11;
	ddr_phyreg_set_range(0xc2, 0, 5, dq_ds);//pull down
	ddr_phyreg_set_range(0xc3, 0, 5, dq_ds);//pull up
	ddr_phyreg_set_range(0xd2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xd3, 0, 5, dq_ds);

	ddr_phyreg_set_range(0xe2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xe3, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xf2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xf3, 0, 5, dq_ds);

#elif defined (CONFIG_T40XP)

	/* cmd */
	ddr_phyreg_set_range(0xb0, 0, 5, 0xf);
	ddr_phyreg_set_range(0xb1, 0, 5, 0xf);
	/* ck */
	ddr_phyreg_set_range(0xb2, 0, 5, 0x11);//pull down
	ddr_phyreg_set_range(0xb3, 0, 5, 0x11);//pull up

	/* DQ ODT config */
	u32 dq_odt = 0x3;
	/* Channel A */
	ddr_phyreg_set_range(0xc0, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xc1, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xd0, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xd1, 0, 5, dq_odt);
	/* Channel B */
	ddr_phyreg_set_range(0xe0, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xe1, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xf0, 0, 5, dq_odt);
	ddr_phyreg_set_range(0xf1, 0, 5, dq_odt);

	/* driver strength config */
	u32 dq_ds = 0x11;
	ddr_phyreg_set_range(0xc2, 0, 5, dq_ds);//pull down
	ddr_phyreg_set_range(0xc3, 0, 5, dq_ds);//pull up
	ddr_phyreg_set_range(0xd2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xd3, 0, 5, dq_ds);

	ddr_phyreg_set_range(0xe2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xe3, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xf2, 0, 5, dq_ds);
	ddr_phyreg_set_range(0xf3, 0, 5, dq_ds);
#elif defined (CONFIG_T40N)
	//driver cmd and ck
	//cmd
	ddr_phyreg_set_range(0xb0, 0, 4, 0x5); //0x4
	ddr_phyreg_set_range(0xb1, 0, 4, 0x5);
	//ck
	ddr_phyreg_set_range(0xb2, 0, 4, 0x5); //pull down,0x5
	ddr_phyreg_set_range(0xb3, 0, 4, 0x5);//pull up
	//dq ODT
	//ch A
	ddr_phyreg_set_range(0xc0, 0, 5, 0x3);
	ddr_phyreg_set_range(0xc1, 0, 5, 0x3);
	ddr_phyreg_set_range(0xd0, 0, 5, 0x3);
	ddr_phyreg_set_range(0xd1, 0, 5, 0x3);
	//chb B
	ddr_phyreg_set_range(0xe0, 0, 4, 0x3);
	ddr_phyreg_set_range(0xe1, 0, 4, 0x3);
	ddr_phyreg_set_range(0xf0, 0, 4, 0x3);
	ddr_phyreg_set_range(0xf1, 0, 4, 0x3);
	// driver strength
	ddr_phyreg_set_range(0xc2, 0, 4, 0x5);//pull down
	ddr_phyreg_set_range(0xc3, 0, 4, 0x5);//pull up
	ddr_phyreg_set_range(0xd2, 0, 4, 0x5);
	ddr_phyreg_set_range(0xd3, 0, 4, 0x5);

	ddr_phyreg_set_range(0xe2, 0, 4, 0x5);
	ddr_phyreg_set_range(0xe3, 0, 4, 0x5);
	ddr_phyreg_set_range(0xf2, 0, 4, 0x5);
	ddr_phyreg_set_range(0xf3, 0, 4, 0x5);
#endif
}

static void ddr_phy_cfg_vref(void)
{

	printf("--------inno A vref reg:0xd7 = 0x%x\n", readl(0xb301135c));
	printf("--------inno A vref reg:0xd8 = 0x%x\n", readl(0xb3011360));
	printf("--------inno B vref reg:0xf7 = 0x%x\n", readl(0xb30113dc));
	printf("--------inno B vref reg:0xf8 = 0x%x\n", readl(0xb30113e0));

	writel(0x85, 0xb301135c);/* A vref */
    	writel(0x85, 0xb3011360);
    	writel(0x7f, 0xb30113dc);/* B vref */
    	writel(0x7f, 0xb30113e0);

	printf("--------inno A vref reg:0xd7 = 0x%x\n", readl(0xb301135c));
	printf("--------inno A vref reg:0xd8 = 0x%x\n", readl(0xb3011360));
	printf("--------inno B vref reg:0xf7 = 0x%x\n", readl(0xb30113dc));
	printf("--------inno B vref reg:0xf8 = 0x%x\n", readl(0xb30113e0));
}

static void ddr_phy_init(void)
{
	u32 idx;
	u32  val;
	/* bit[3]reset digital core; bit[2]reset analog logic; bit[0]Reset Initial status
	 * other reserved */
	val = ddr_readl(DDRP_INNOPHY_INNO_PHY_RST);
	val &= ~(0xff);
	mdelay(2);
	val |= 0x0d;
	ddr_writel(val, DDRP_INNOPHY_INNO_PHY_RST);
	ddr_phy_cfg_driver_odt();

	writel(0xc, 0xb3011000 + (0x15)*4);//default 0x4 cmd
	writel(0x1, 0xb3011000 + (0x16)*4);//default 0x0 ck

	writel(0xc, 0xb3011000 + (0x54)*4);//default 0x4  byte0 dq dll
	writel(0xc, 0xb3011000 + (0x64)*4);//default 0x4  byte1 dq dll
	writel(0xc, 0xb3011000 + (0x84)*4);//default 0x4  byte2 dq dll
	writel(0xc, 0xb3011000 + (0x94)*4);//default 0x4  byte3 dq dll

	writel(0x1, 0xb3011000 + (0x55)*4);//default 0x0  byte0 dqs dll
	writel(0x1, 0xb3011000 + (0x65)*4);//default 0x0  byte1 dqs dll
	writel(0x1, 0xb3011000 + (0x85)*4);//default 0x0  byte2 dqs dll
	writel(0x1, 0xb3011000 + (0x95)*4);//default 0x0  byte3 dqs dll

#ifdef CONFIG_DDR_TYPE_DDR3
	unsigned int i = 0;
	u32 value = 8;
	/* write leveling dq delay time config */
	//cmd
	for (i = 0; i <= 0x1e;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x100+i)*4);///cmd
	}
	//tx DQ
	for (i = 0; i <= 0x8;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	//tx DQ
	for (i = 0xb; i <= 0x13;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	/* rx dqs */
	value = 8;
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x29)*4);//DQS0-A
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x29)*4);//DQS0-B
	ddr_writel(value, DDR_PHY_OFFSET + (0x120+0x34)*4);//DQS1-A
	ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+0x34)*4);//DQS1-B
	//enable bypass write leveling
	//open manual per bit de-skew
	//printf("PHY REG-02 :  0x%x \n",readl(0xb3011008));
	writel((readl(0xb3011008))|(0x8), 0xb3011008);
	//printf("PHY REG-02 :  0x%x \n",readl(0xb3011008));

	dump_ddr_phy_cfg_per_bit_de_skew_register();

#ifdef CONFIG_DDR_DEBUG
	//writel(0x73, 0xb3011000+(0x88*4));
	//writel(0x73, 0xb3011000+(0x98*4));
	printf("--------inno a dll reg:0x58 = 0x%x\n", readl(0xb3011000+(0x58*4)));
	printf("--------inno a dll reg:0x68 = 0x%x\n", readl(0xb3011000+(0x68*4)));
	printf("--------inno b dll reg:0x88 = 0x%x\n", readl(0xb3011000+(0x88*4)));
	printf("--------inno b dll reg:0x98 = 0x%x\n", readl(0xb3011000+(0x98*4)));
#endif
#elif defined(CONFIG_DDR_TYPE_DDR2)
	unsigned int i = 0;
	u32 value = 5;
	/* write leveling dq delay time config */
	//cmd
	for (i = 0; i <= 0x1e;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x100+i)*4);///cmd
	}
	//tx DQ
	for (i = 0; i <= 0x8;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	//tx DQ
	for (i = 0xb; i <= 0x13;i++) {
		ddr_writel(value, DDR_PHY_OFFSET + (0x120+i)*4);//DQ0-DQ15
		ddr_writel(value, DDR_PHY_OFFSET + (0x1a0+i)*4);//DQ15-DQ31
	}
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x9)*4);//DQS0-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x9)*4);//DQS0-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0xa)*4);//DQS0B-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0xa)*4);//DQS0B-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x14)*4);//DQS1-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x14)*4);//DQS1-B
	ddr_writel(2, DDR_PHY_OFFSET + (0x120+0x15)*4);//DQS1B-A
	ddr_writel(2, DDR_PHY_OFFSET + (0x1a0+0x15)*4);//DQS1B-B

	writel((readl(0xb3011008))|(0x8), 0xb3011008);
	//printf("PHY REG-02 :  0x%x \n",readl(0xb3011008));
#endif
	ddrp_pll_init();
}
/*
 * Name     : ddrp_calibration_manual()
 * Function : control the RX DQS window delay to the DQS
 *
 * a_low_8bit_delay	= al8_2x * clk_2x + al8_1x * clk_1x;
 * a_high_8bit_delay	= ah8_2x * clk_2x + ah8_1x * clk_1x;
 *
 * */
static void ddrp_software_calibration(void)
{
	int x, y;
	int w, z;
	int c, o, d;
	unsigned int addr = 0xa1000000, val;
	unsigned int i, n, m = 0;
	unsigned int sel = 0;
	union ddrp_calib calib_val[8 * 2 * 8];
	int ret = -1;
	int j, k, q;

	unsigned int d0 = 0, d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0, d7 = 0;
	unsigned int ddbuf[8] = {0};
	unsigned int calv[128] = {0};
	ddr_phyreg_set_range(0x2, 1, 1, 1);
	printf("BEFORE A CALIB\n");
	printf("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
	printf("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
	printf("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL));
	printf("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH));
#if 1
	for(c = 0; c < 128; c ++) {
		ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_AL);
		ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_AH);
		//ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_BL);
		//ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_BH);
		unsigned int value = 0x12345678;
		for(i = 0; i < 4 * 1024; i += 8) {
			*(volatile unsigned int *)(addr + (i + 0) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 1) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 2) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 3) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 4) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 5) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 6) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 7) * 4) = value;
		}

		for(i = 0; i < 4 * 1024; i += 8) {
			ddbuf[0] = *(volatile unsigned int *)(addr + (i + 0) * 4);
			ddbuf[1] = *(volatile unsigned int *)(addr + (i + 1) * 4);
			ddbuf[2] = *(volatile unsigned int *)(addr + (i + 2) * 4);
			ddbuf[3] = *(volatile unsigned int *)(addr + (i + 3) * 4);
			ddbuf[4] = *(volatile unsigned int *)(addr + (i + 4) * 4);
			ddbuf[5] = *(volatile unsigned int *)(addr + (i + 5) * 4);
			ddbuf[6] = *(volatile unsigned int *)(addr + (i + 6) * 4);
			ddbuf[7] = *(volatile unsigned int *)(addr + (i + 7) * 4);

			for(q = 0; q < 8; q++) {

				if ((ddbuf[q]&0xffff0000) != (value&0xffff0000)) {
					;//printf("#####################################   high error want 0x%x get 0x%x\n", value, ddbuf[q]);
				}
				if ((ddbuf[q]&0xffff) != (value&0xffff)) {
					printf("SET AL,AH %x q[%d] fail want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = -1;
					break;
				} else {
					//printf("SET AL,AH %x q[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					//printf("SET %d  AL[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = 0;
				}
			}
			if (ret) {
				break;
			}
		}

		if(i == 4 * 1024) {
			calv[m] = c;
			m++;
			printf("calib a once idx = %d,  value = %x\n", m, c);
		}
	}

	if(!m) {
		printf("####################### AL a calib bypass fail\n");
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_AL);
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_AH);
//		return;
	} else {
	/* choose the middle parameter */
		sel = m * 1 / 2;
		ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_AL);
		ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_AH);
		//ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_BL);
		//ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_BH);
	}
	printf("calib a done range = %d, value = %x\n", m, calv[sel]);
	printf("AFTER A CALIB\n");
	printf("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
	printf("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
	printf("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL));
	printf("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH));
#endif
#if 1
	m = 0;
	for(c = 0; c < 128; c ++) {
		//ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_AL);
		//ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_AH);
		ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_BL);
		ddr_writel(c, DDRP_INNOPHY_CALIB_DELAY_BH);
		unsigned int value = 0xf0f0f0f0;
		for(i = 0; i < 4 * 1024; i += 8) {
			*(volatile unsigned int *)(addr + (i + 0) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 1) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 2) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 3) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 4) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 5) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 6) * 4) = value;
			*(volatile unsigned int *)(addr + (i + 7) * 4) = value;
			//flush_cache((unsigned int *)(addr + i * 4), 32);
		}

		for(i = 0; i < 4 * 1024; i += 8) {
			ddbuf[0] = *(volatile unsigned int *)(addr + (i + 0) * 4);
			ddbuf[1] = *(volatile unsigned int *)(addr + (i + 1) * 4);
			ddbuf[2] = *(volatile unsigned int *)(addr + (i + 2) * 4);
			ddbuf[3] = *(volatile unsigned int *)(addr + (i + 3) * 4);
			ddbuf[4] = *(volatile unsigned int *)(addr + (i + 4) * 4);
			ddbuf[5] = *(volatile unsigned int *)(addr + (i + 5) * 4);
			ddbuf[6] = *(volatile unsigned int *)(addr + (i + 6) * 4);
			ddbuf[7] = *(volatile unsigned int *)(addr + (i + 7) * 4);

			for(q = 0; q < 8; q++) {

				if ((ddbuf[q]&0xffff0000) != (value&0xffff0000)) {
					printf("SET BL,BH 0x%x q[%d] fail want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = -1;
					break;
				} else {
					//printf("SET BL,BH 0x%x q[%d] pass want 0x%x  get 0x%x \n", c, q, value, ddbuf[q]);
					ret = 0;
				}
			}
			if (ret) {
				break;
			}
		}

		if(i == 4 * 1024) {
			calv[m] = c;
			m++;
			printf("calib b once idx = %d,  value = %x\n", m, c);
		}
	}

	if(!m) {
		printf("####################### AL calib bypass fail\n");
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_BL);
		ddr_writel(0x1c, DDRP_INNOPHY_CALIB_DELAY_BH);

		//return;
	} else {
		/* choose the middle parameter */
		sel = m/2;
		//ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_AL);
		//ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_AH);
		ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_BL);
		ddr_writel(calv[sel], DDRP_INNOPHY_CALIB_DELAY_BH);
	}
	printf("calib b done range = %d, value = %x\n", m, calv[sel]);
	printf("AFTER B CALIB\n");
	printf("CALIB DELAY AL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL));
	printf("CALIB DELAY AH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH));
	printf("CALIB DELAY BL 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BL));
	printf("CALIB DELAY BH 0x%x\n", ddr_readl(DDRP_INNOPHY_CALIB_DELAY_BH));
#endif
}

static void ddrc_dfi_init(enum ddr_type type, int bypass)
{
	FUNC_ENTER();
#ifdef CONFIG_DDR_BUSWIDTH_32
    	ddr_writel((DDRC_DWCFG_DFI_INIT_START | 1), DDRC_DWCFG); // dfi_init_start high   T40XP
    	ddr_writel(1, DDRC_DWCFG); // set buswidth 32bit
#else
    	ddr_writel((DDRC_DWCFG_DFI_INIT_START & 0xfe), DDRC_DWCFG); // dfi_init_start high ddr buswidth 16bit
    	ddr_writel(0, DDRC_DWCFG); // set buswidth 16bit
#endif
	while(!(ddr_readl(DDRC_DWSTATUS) & DDRC_DWSTATUS_DFI_INIT_COMP)); //polling dfi_init_complete

	udelay(50);
	ddr_writel(0, DDRC_CTRL); //set dfi_reset_n high
    	ddr_writel(DDRC_CFG_VALUE, DDRC_CFG);
	udelay(500);
	ddr_writel(DDRC_CTRL_CKE, DDRC_CTRL); // set CKE to high
	udelay(10);

	switch(type) {
		case DDR2:
#define DDRC_LMR_MR(n)												\
			1 << 1 | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
			((DDR_MR##n##_VALUE & 0x1fff) << DDRC_LMR_DDR_ADDR_BIT) |	\
			(((DDR_MR##n##_VALUE >> 13) & 0x3) << DDRC_LMR_BA_BIT)
			while (ddr_readl(DDRC_LMR) & (1 << 0));
			ddr_writel(0x400003, DDRC_LMR);
			udelay(100);
			ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
			udelay(5);
			ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
			udelay(5);
			ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
			udelay(5);
			ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
			udelay(5 * 1000);
			ddr_writel(0x400003, DDRC_LMR);
			udelay(100);
			ddr_writel(0x43, DDRC_LMR);
			udelay(5);
			ddr_writel(0x43, DDRC_LMR);
			udelay(5 * 1000);
#undef DDRC_LMR_MR
			break;
		case LPDDR2:
#define DDRC_LMR_MR(n)													\
		1 << 1| DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |	\
			((DDR_MR##n##_VALUE & 0xff) << 24) |						\
			(((DDR_MR##n##_VALUE >> 8) & 0xff) << (16))
		ddr_writel(DDRC_LMR_MR(63), DDRC_LMR); //set MRS reset
		ddr_writel(DDRC_LMR_MR(10), DDRC_LMR); //set IO calibration
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //set MR1
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //set MR2
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //set MR3
#undef DDRC_LMR_MR
		break;
	case DDR3:
#define DDRC_LMR_MR(n)												\
		DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_LMR |		\
		((DDR_MR##n##_VALUE & 0xffff) << DDRC_LMR_DDR_ADDR_BIT) |	\
			(((DDR_MR##n##_VALUE >> 16) & 0x7) << DDRC_LMR_BA_BIT)

		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(2), DDRC_LMR); //MR2
		//printf("mr2: %x\n", DDRC_LMR_MR(2));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(3), DDRC_LMR); //MR3
		//printf("mr3: %x\n", DDRC_LMR_MR(3));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(1), DDRC_LMR); //MR1
		//printf("mr1: %x\n", DDRC_LMR_MR(1));
		udelay(5);
		ddr_writel(0, DDRC_LMR);udelay(5);
		ddr_writel(DDRC_LMR_MR(0), DDRC_LMR); //MR0
		//printf("mr0: %x\n", DDRC_LMR_MR(0));
		udelay(5);
		//ddr_writel(DDRC_DLMR_VALUE | DDRC_LMR_START | DDRC_LMR_CMD_ZQCL_CS0, DDRC_LMR); //ZQCL
        	udelay(5);

#undef DDRC_LMR_MR
		break;
	default:
		ddr_hang();
	}

	FUNC_EXIT();
}

struct ddr_calib_value {
	unsigned int rate;
	unsigned int refcnt;
	unsigned char bypass_al;
	unsigned char bypass_ah;
};

#define REG32(addr) *(volatile unsigned int *)(addr)
#define CPM_DDRCDR (0xb000002c)

static void ddr_calibration(struct ddr_calib_value *dcv, int div)
{
	unsigned int val;

	// Set change_en
	val = REG32(CPM_DDRCDR);
	val |= ((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
	while((REG32(CPM_DDRCDR) & (1 << 24)))
		;
	/* // Set clock divider */
	val = REG32(CPM_DDRCDR);
	val &= ~(0xf);
	val |= div;
	REG32(CPM_DDRCDR) = val;

	// Polling PHY_FREQ_DONE
	while(((ddr_readl(DDRC_DWSTATUS) & (1 << 3 | 1 << 1)) & 0xf) != 0xa);
	ddrp_hardware_calibration();
	/* ddrp_software_calibration(); */

	dcv->bypass_al = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AL);
	/* printf("auto :CALIB_AL: dcv->bypss_al %x\n", dcv->bypass_al); */
	dcv->bypass_ah = ddr_readl(DDRP_INNOPHY_CALIB_DELAY_AH);
	/* printf("auto:CAHIB_AH: dcv->bypss_ah %x\n", dcv->bypass_ah); */

	// Set Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val |= (1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	// Clear Controller Freq Exit
	val = ddr_readl(DDRC_DWCFG);
	val &= ~(1 << 2);
	ddr_writel(val, DDRC_DWCFG);

	val = REG32(CPM_DDRCDR);
	val &= ~((1 << 29) | (1 << 25));
	REG32(CPM_DDRCDR) = val;
}

static void get_dynamic_calib_value(unsigned int rate)
{
	struct ddr_calib_value *dcv;
	unsigned int drate = 0;
	int div, n, cur_div;
#define CPU_TCSM_BASE (0xb2400000)
	dcv = (struct ddr_calib_value *)(CPU_TCSM_BASE + 2048);
	cur_div = REG32(CPM_DDRCDR) & 0xf;
	div = cur_div + 1;
	do {
		drate = rate / (div + 1);
		if(drate < 100000000) {
			dcv[cur_div].rate = rate;
			dcv[cur_div].refcnt = get_refcnt_value(cur_div);
			ddr_calibration(&dcv[cur_div], cur_div);
			break;
		}
		dcv[div].rate = drate;
		dcv[div].refcnt = get_refcnt_value(div);
		ddr_calibration(&dcv[div], div);
		div ++;
	} while(1);

	/* for(div = 6, n = 0; div > 0; div--, n++) { */
	/* 	dcv[div - 1].rate = rate / div; */
	/* 	if(dcv[div - 1].rate < 100000000) */
	/* 		break; */
	/* 	dcv[div - 1].refcnt = get_refcnt_value(div); */
	/* 	get_calib_value(&dcv[div - 1], div); */
	/* } */
}

static void ddr_phy_cfg_per_bit_de_skew(void)
{
	u32 i,j;
	u32 idx;
	u32 idx_dqs;
	u32 idx_dq;
	u8 bucket_dqs[0x1f];
	u8 bucket_idx;
	//A low
#if 1
	bucket_idx = 0;
	for (i = 0; i <= 0x1f; i++) {
		u32 error = 0;
		for (idx_dqs = 9; idx_dqs <= 0xa; idx_dqs++) {
			writel(i, 0xb3011000+((0x120+idx_dqs)*4));
		}
		for (j = 0; j <= 0x1f; j++) {
			for (idx_dq = 0; idx_dq <= 8; idx_dq++) {
				writel(j, 0xb3011000+((0x120+idx_dq)*4));
			}
			//check
			u32 addr = 0x81000000+(i+j)*4;
			u32 data = 0x5a5a5a5a+(i+j)*4;
			u32 data_get;
			*(volatile unsigned int *)(addr) = data;
	        	flush_dcache_range(addr, addr+4);
	        	flush_l2cache_range(addr, addr+4);
			data_get = *(volatile unsigned int *)(addr);
			if ((data&0xff) != (data_get&0xff)) {
				//printf("tx per bit skew: fail dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
				//	   i, j, data, data_get);
				error++;
			} else {
					printf("tx A low per bit skew: pass dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
					   i, j, data, data_get);
			}
		}
		if (!error) {
			bucket_dqs[bucket_idx] = i;
			bucket_idx++;
		}
	}
	if (bucket_idx) {
		printf("PHY A chn lo tx per-bit-de-skew dqs = %d, dp = %d\n", bucket_dqs[bucket_idx/2], 0x1f/2);
		for (idx = 0; idx <= 0x9; idx++) {
			writel(0x1f/2, 0xb3011000+((0x120+idx)*4));
		}
		for (idx = 0x9; idx <= 0xa; idx++) {
			writel(bucket_dqs[bucket_idx/2], 0xb3011000+((0x120+idx)*4));
		}
	} else {
		printf("PHY A chn lo tx per-bit-de-skew serach error\n");
	}
	//A high
	bucket_idx = 0;
	for (i = 0; i <= 0x1f; i++) {
		u32 error = 0;
		for (idx_dqs = 0x14; idx_dqs <= 0x15; idx_dqs++) {
			writel(i, 0xb3011000+((0x120+idx_dqs)*4));
		}
		for (j = 0; j <= 0x1f; j++) {
			for (idx_dq = 0xb; idx_dq <= 0x13; idx_dq++) {
				writel(j, 0xb3011000+((0x120+idx_dq)*4));
			}
			//check
			u32 addr = 0x81000000+(i+j)*4;
			u32 data = 0x5a5a5a5a+(i+j)*4*0x100;
			u32 data_get;
			*(volatile unsigned int *)(addr) = data;
	        	flush_dcache_range(addr, addr+4);
	        	flush_l2cache_range(addr, addr+4);
			data_get = *(volatile unsigned int *)(addr);
			if ((data&0xff00) != (data_get&0xff00)) {
				//printf("tx per bit skew: fail dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
				//	   i, j, data, data_get);
				error++;
			} else {
					printf("tx A high per bit skew: pass dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
					   i, j, data, data_get);
			}
		}
		if (!error) {
			bucket_dqs[bucket_idx] = i;
			bucket_idx++;
		}
	}
	if (bucket_idx) {
		printf("PHY A chn hi tx per-bit-de-skew dqs = %d, dp = %d\n", bucket_dqs[bucket_idx/2], 0x1f/2);
		for (idx = 0xb; idx <= 0x13; idx++) {
			writel(0x1f/2, 0xb3011000+((0x120+idx)*4));
		}
		for (idx = 0x14; idx <= 0x15; idx++) {
			writel(bucket_dqs[bucket_idx/2], 0xb3011000+((0x120+idx)*4));
		}
	} else {
		printf("PHY A chn hi tx per-bit-de-skew serach error\n");
	}
	//B low
	bucket_idx = 0;
	for (i = 0; i <= 0x1f; i++) {
		u32 error = 0;
		for (idx_dqs = 9; idx_dqs <= 0xa; idx_dqs++) {
			writel(i, 0xb3011000+((0x1a0+idx_dqs)*4));
		}
		for (j = 0; j <= 0x1f; j++) {
			for (idx_dq = 0; idx_dq <= 8; idx_dq++) {
				writel(j, 0xb3011000+((0x1a0+idx_dq)*4));
			}
			//check
			u32 addr = 0x81000000+(i+j)*4;
			u32 data = 0x5a5a5a5a+(i+j)*4*0x10000;
			u32 data_get;
			*(volatile unsigned int *)(addr) = data;
	        	flush_dcache_range(addr, addr+4);
	        	flush_l2cache_range(addr, addr+4);
			data_get = *(volatile unsigned int *)(addr);
			if ((data&0xff0000) != (data_get&0xff0000)) {
				//printf("tx per bit skew: fail dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
				//	   i, j, data, data_get);
				error++;
			} else {
					printf("tx B low per bit skew: pass dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
					   i, j, data, data_get);
			}
		}
		if (!error) {
			bucket_dqs[bucket_idx] = i;
			bucket_idx++;
		}
	}
	if (bucket_idx) {
		printf("PHY B chn lo tx per-bit-de-skew dqs = %d, dp = %d\n", bucket_dqs[bucket_idx/2], 0x1f/2);
		for (idx = 0; idx <= 0x9; idx++) {
			writel(0x1f/2, 0xb3011000+((0x1a0+idx)*4));
		}
		for (idx = 0x9; idx <= 0xa; idx++) {
			writel(bucket_dqs[bucket_idx/2], 0xb3011000+((0x1a0+idx)*4));
		}
	} else {
		printf("PHY B chn lo tx per-bit-de-skew serach error\n");
	}
	//B high
	bucket_idx = 0;
	for (i = 0; i <= 0x1f; i++) {
		u32 error = 0;
		for (idx_dqs = 0x14; idx_dqs <= 0x15; idx_dqs++) {
			writel(i, 0xb3011000+((0x1a0+idx_dqs)*4));
		}
		for (j = 0; j <= 0x1f; j++) {
			for (idx_dq = 0xb; idx_dq <= 0x13; idx_dq++) {
				writel(j, 0xb3011000+((0x1a0+idx_dq)*4));
			}
			//check
			u32 addr = 0x81000000+(i+j)*4;
			u32 data = 0x5a5a5a5a+(i+j)*4*0x1000000;
			u32 data_get;
			*(volatile unsigned int *)(addr) = data;
	        	flush_dcache_range(addr, addr+4);
	        	flush_l2cache_range(addr, addr+4);
			data_get = *(volatile unsigned int *)(addr);
			if ((data&0xff000000) != (data_get&0xff000000)) {
				//printf("tx per bit skew: fail dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
				//	   i, j, data, data_get);
				error++;
			} else {
					printf("tx B high per bit skew: pass dqs = %x, dq = %x, want = 0x%x, get = 0x%x\n",
					   i, j, data, data_get);
			}
		}
		if (!error) {
			bucket_dqs[bucket_idx] = i;
			bucket_idx++;
		}
	}
	if (bucket_idx) {
		printf("PHY B chn hi tx per-bit-de-skew dqs = %d, dp = %d\n", bucket_dqs[bucket_idx/2], 0x1f/2);
		for (idx = 0xb; idx <= 0x13; idx++) {
			writel(0x1f/2, 0xb3011000+((0x1a0+idx)*4));
		}
		for (idx = 0x14; idx <= 0x15; idx++) {
			writel(bucket_dqs[bucket_idx/2], 0xb3011000+((0x1a0+idx)*4));
		}
	} else {
		printf("PHY B chn high tx per-bit-de-skew serach error\n");
	}
#endif

#ifdef CONFIG_DDR_DEBUG
#if 0
    /* TX A Channel low pre bit skew DQS DQ[7:0] DM */
	for (idx = 0; idx <= 0x8; idx++) {
		writel(0xf, 0xb3011000+((0x120+idx)*4));
	}
	for (idx = 0x9; idx <= 0xa; idx++) {
		writel(0xf, 0xb3011000+((0x120+idx)*4));
	}

    /* TX A Channel high pre bit skew DQS DQ[15:8] DM */
	for (idx = 0xb; idx <= 0x13; idx++) {
		writel(0xf, 0xb3011000+((0x120+idx)*4));
	}
	for (idx = 0x14; idx <= 0x15; idx++) {
		writel(0xf, 0xb3011000+((0x120+idx)*4));
	}

    /* TX B Channel low pre bit skew DQS DQ[7:0] DM */
	for (idx = 0; idx <= 0x8; idx++) {
		writel(0xf, 0xb3011000+((0x1a0+idx)*4));
	}
	for (idx = 0x9; idx <= 0xa; idx++) {
		writel(0xf, 0xb3011000+((0x1a0+idx)*4));
	}

    /* TX B Channel high pre bit skew DQS DQ[15:8] DM */
	for (idx = 0xb; idx <= 0x13; idx++) {
		writel(0xf, 0xb3011000+((0x1a0+idx)*4));
	}
	for (idx = 0x14; idx <= 0x15; idx++) {
		writel(0xf, 0xb3011000+((0x1a0+idx)*4));
	}
    /* CK Pre bit skew */
	for (idx = 0x16; idx <= 0x17; idx++) {
		writel(0xf, 0xb3011000+((0x100+idx)*4));
	}
#endif
#endif

    dump_ddr_phy_cfg_per_bit_de_skew_register();
}


/* DDR sdram init */
void sdram_init(void)
{
	enum ddr_type type;
	unsigned int rate;
	int bypass = 0;

	debug("sdram init start\n");

	type = get_ddr_type();
#ifndef CONFIG_FPGA
	clk_set_rate(DDR, gd->arch.gi->ddrfreq);
	/*if(ddr_hook && ddr_hook->prev_ddr_init)*/
		/*ddr_hook->prev_ddr_init(type);*/
	rate = clk_get_rate(DDR);
#else
	rate = gd->arch.gi->ddrfreq;
#endif

#ifndef CONFIG_FAST_BOOT
    printf("DDR clk rate: %d\n\n", rate);
#endif

	ddrc_reset_phy();
	//mdelay(20);
	ddr_phy_init();
#ifdef CONFIG_DDR_DEBUG
	dump_ddrp_register();
#endif
	ddrc_dfi_init(type, bypass);
	ddrc_prev_init();
#ifdef CONFIG_DDR_HARDWARE_TRAINING
	ddrp_hardware_calibration();
#endif
	//ddrp_software_writeleveling_tx();
   	//ddrp_software_writeleveling_rx();
	//ddr_phy_cfg_per_bit_de_skew();

#ifdef	CONFIG_DDR_SOFT_TRAINING
	ddrp_software_calibration();
#endif

#ifdef CONFIG_DDR_DEBUG
    dump_inno_driver_strength_register();
	dump_ddrp_register();

	printf("DDRP_INNOPHY_INNO_PHY 0X1d0    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1d0));
	printf("DDRP_INNOPHY_INNO_PHY 0X1d4    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1d4));
	printf("DDRP_INNOPHY_INNO_PHY 0X290    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x290));
	printf("DDRP_INNOPHY_INNO_PHY 0X294    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x294));

	printf("DDRP_INNOPHY_INNO_PHY 0X1c0    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x1c0));
	printf("DDRP_INNOPHY_INNO_PHY 0X280    0x%x\n", ddr_readl(DDR_PHY_OFFSET + 0x280));
#endif

	ddrc_post_init();

	/*get_dynamic_calib_value(rate);*/

	if(DDRC_AUTOSR_EN_VALUE) {
		ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);
		ddr_writel(1, DDRC_AUTOSR_EN);
	} else {
		ddr_writel(DDRC_AUTOSR_CNT_VALUE, DDRC_AUTOSR_CNT);
		ddr_writel(0, DDRC_AUTOSR_EN);
	}

	ddr_writel(DDRC_HREGPRO_VALUE, DDRC_HREGPRO);
	ddr_writel(DDRC_PREGPRO_VALUE, DDRC_PREGPRO);

#ifdef CONFIG_DDR_DEBUG
	dump_ddrc_register();
#endif

#ifdef CONFIG_DDR_TYPE_DDR3
	//printf("PHY REG-01 :  0x%x \n", ddr_phyreg_get(0x1));
	ddr_phyreg_set_range(0x1, 6, 1, 1);
	//printf("PHY REG-01 :  0x%x \n", ddr_phyreg_get(0x1));
#elif defined(CONFIG_DDR_TYPE_DDR2)
	//printf("PHY REG-01 :  0x%x \n",readl(0xb3011004));
	writel(0x51,0xb3011004);
	//printf("PHY REG-01 :  0x%x \n",readl(0xb3011004));
#endif
	//fifo need set reg 0x01 bit6  to 1
	//printf("PHY REG-0xa :  0x%x \n", ddr_phyreg_get(0xa));
	ddr_phyreg_set_range(0xa, 1, 3, 3);
	//printf("PHY REG-0xa :  0x%x \n", ddr_phyreg_get(0xa));

    	//TX Write Pointer adjust
	//printf("PHY REG-0x8 :  0x%x \n", ddr_phyreg_get(0x8));
	ddr_phyreg_set_range(0x8, 0, 2, 3);
	//printf("PHY REG-0x8 :  0x%x \n", ddr_phyreg_get(0x8));

	//extend rx dqs gateing window
	//ddr_phyreg_set_range(0x9, 7, 1, 1);

    	//ddr_phy_cfg_per_bit_de_skew();
   	//ddrp_software_writeleveling_rx();

	debug("sdram init finished\n");
}

phys_size_t initdram(int board_type)
{
	/* SDRAM size was calculated when compiling. */
#ifndef EMC_LOW_SDRAM_SPACE_SIZE
#ifdef CONFIG_T40A
#define EMC_LOW_SDRAM_SPACE_SIZE 0x80000000 /* 2G */
#else
#define EMC_LOW_SDRAM_SPACE_SIZE 0x20000000 /* 512M */
#endif
#endif /* EMC_LOW_SDRAM_SPACE_SIZE */

	unsigned int ram_size;
	ram_size = (unsigned int)(DDR_CHIP_0_SIZE) + (unsigned int)(DDR_CHIP_1_SIZE);

	if (ram_size > EMC_LOW_SDRAM_SPACE_SIZE)
		ram_size = EMC_LOW_SDRAM_SPACE_SIZE;

	return ram_size;
}
