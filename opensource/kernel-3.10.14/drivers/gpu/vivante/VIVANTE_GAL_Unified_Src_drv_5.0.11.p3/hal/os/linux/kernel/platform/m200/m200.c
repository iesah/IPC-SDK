/****************************************************************************
*
*    Copyright (C) 2005 - 2014 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/
#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel_platform.h"

extern unsigned long coreClock;
#define DEBUG
#ifdef DEBUG
static inline int dump_m200_gpu_clock(void)
{
#define REG_CPM_LPG *((volatile unsigned long *)0xb0000004)
#define REG_CPM_GT1 *((volatile unsigned long *)0xb0000028)
#define REG_CPM_GPU *((volatile unsigned long *)0xb0000088)

	printk("==============================\n");
	printk("REG_CPM_LPG=%08x\n",(unsigned int)REG_CPM_LPG);
	printk("REG_CPM_GT1=%08x\n",(unsigned int)REG_CPM_GT1);
	printk("REG_CPM_GPU=%08x\n",(unsigned int)REG_CPM_GPU);


	return 0;
}
#endif

gctBOOL
_NeedAddDevice(
		IN gckPLATFORM Platform
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [addDevice] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvFALSE;
	}
	return gcvTRUE;
}

gceSTATUS
_getPower(
		IN gckPLATFORM Platform
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [getPower] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvSTATUS_INVALID_ARGUMENT;
	}

	{
#if ENABLE_GPU_CLOCK_BY_DRIVER && defined(CONFIG_GPU_DYNAMIC_CLOCK_POWER)
		Platform->clk_pwc_gpu = clk_get(NULL, "pwc_gpu");
		if (IS_ERR(Platform->clk_pwc_gpu)) {
			printk("gpu clock get error: platform.clk_pwc_gpu\n");
		}
		Platform->Power_ON = 0;

		Platform->clk_gpu = clk_get(NULL,"gpu");
		if (IS_ERR(Platform->clk_gpu)) {
			printk("gpu clock get error: platform.clk_gpu\n");
		}

		Platform->clk_cgu_gpu = clk_get(NULL, "cgu_gpu");
		if (IS_ERR(Platform->clk_cgu_gpu)) {
			printk("gpu clock get error: platform.clk_cgu_gpu\n");
		}

		if (clk_set_rate(Platform->clk_cgu_gpu, coreClock * 2))
		{
			gcmkTRACE_ZONE(
					gcvLEVEL_ERROR, gcvZONE_DRIVER,
					"%s(%d): Failed to set core clock.\n",
					__FUNCTION__, __LINE__
					);

			//gcmONERROR(gcvSTATUS_GENERIC_IO);
		}
		Platform->Clock_ON = 0;
#else
		struct clk * clk_gpu;
		struct clk * clk_cgu_gpu;

                /* Note: Must set M200 cgu_gpu before enable gpu gate clock! */
		clk_cgu_gpu = clk_get(NULL, "cgu_gpu");

		if (IS_ERR(clk_cgu_gpu))
		{
			gcmkTRACE_ZONE(
					gcvLEVEL_ERROR, gcvZONE_DRIVER,
					"%s(%d): clk get error: %d\n",
					__FUNCTION__, __LINE__,
					PTR_ERR(clk_cgu_gpu)
					);

			//gcmkONERROR(gcvSTATUS_GENERIC_IO);
		}

		/*
		 * APMU_GC_156M, APMU_GC_312M, APMU_GC_PLL2, APMU_GC_PLL2_DIV2 currently.
		 * Use the 2X clock.
		 */
		if (clk_set_rate(clk_cgu_gpu, coreClock * 2))
		{
			gcmkTRACE_ZONE(
					gcvLEVEL_ERROR, gcvZONE_DRIVER,
					"%s(%d): Failed to set core clock.\n",
					__FUNCTION__, __LINE__
					);

		//	gcmkONERROR(gcvSTATUS_GENERIC_IO);
		}
		clk_enable(clk_cgu_gpu);
		//clk_put(clk);

                /* Note: enable gpu gate clock after set cgu_gpu. */
		clk_gpu = clk_get(NULL, "gpu");
		clk_enable(clk_gpu);

#endif//ENABLE_GPU_CLOCK_BY_DRIVER
	}

	return gcvSTATUS_OK;
}

gceSTATUS
_adjustParam(
		IN gckPLATFORM Platform,
		OUT gcsMODULE_PARAMETERS *Args
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [adjustParam] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvSTATUS_INVALID_ARGUMENT;
	}
	Args->irqLine         = 71;
	Args->registerMemBase = 0x13040000;
	Args->contiguousSize  = (CONFIG_GPU_CONTIGUOUS_SIZE_MB) << 20;
	Args->physSize        = 0x80000000;
	Args->compression     = 0;

	return gcvSTATUS_OK;
}
gceSTATUS
_setPower(
		IN gckPLATFORM Platform,
		IN gceCORE GPU,
		IN gctBOOL Enable
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [setpower] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvSTATUS_INVALID_ARGUMENT;
	}
#if 0
	printk("Platform->Power_ON = %d ,Enable = %d\n",Platform->Power_ON,Enable);
#endif

#ifdef CONFIG_GPU_DYNAMIC_CLOCK_POWER
	if(!Platform->Power_ON && Enable){
		clk_enable(Platform->clk_pwc_gpu);
		Platform->Power_ON = 1;
	}
	if(Platform->Power_ON && !Platform->Clock_ON && !Enable){
		clk_disable(Platform->clk_pwc_gpu);
		Platform->Power_ON = 0;
	}
	if(Platform->Power_ON && Platform->Clock_ON && !Enable){
		Platform->Power_ON = 0;
	}
#endif
#if 0
	dump_m200_gpu_clock();
#endif
	return gcvSTATUS_OK;
}

gceSTATUS
_setClock(
		IN gckPLATFORM Platform,
		IN gceCORE GPU,
		IN gctBOOL Enable
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [setclock] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvSTATUS_INVALID_ARGUMENT;
	}
#if 0
	printk("Platform->Clock_ON = %d ,Enable = %d\n",Platform->Clock_ON,Enable);
#endif

#ifdef CONFIG_GPU_DYNAMIC_CLOCK_POWER
	if(!Platform->Clock_ON && Enable) {
		clk_enable(Platform->clk_gpu);
		clk_enable(Platform->clk_cgu_gpu);
		Platform->Clock_ON = 1;
	}
	if(Platform->Clock_ON && !Enable) {
		clk_disable(Platform->clk_cgu_gpu);
		clk_disable(Platform->clk_gpu);
		Platform->Clock_ON = 0;
	}
	if(!Platform->Power_ON && !Platform->Clock_ON && !Enable) {
		clk_disable(Platform->clk_pwc_gpu);
	}

#endif
#if 0
	dump_m200_gpu_clock();
#endif
	return gcvSTATUS_OK;
}

gceSTATUS
_putPower(
		IN gckPLATFORM Platform
		)
{
	if(!Platform){
		printk("Error! No platform! Can't [putpower] \nIN %s:%d \n",__FILE__,__LINE__);
		return gcvSTATUS_INVALID_ARGUMENT;
	}
	{
#ifdef CONFIG_GPU_DYNAMIC_CLOCK_POWER
		if(Platform ->Clock_ON) {
			clk_disable(Platform->clk_cgu_gpu);
			clk_disable(Platform->clk_gpu);
			clk_disable(Platform->clk_pwc_gpu);
		}
		clk_put(Platform->clk_cgu_gpu);
		clk_put(Platform->clk_gpu);
		clk_put(Platform->clk_pwc_gpu);
		Platform->Power_ON = 0;
#else
		struct clk * clk = NULL;
		clk = clk_get(NULL, "cgu_gpu");
		clk_disable(clk);
		clk_put(clk);
		clk = clk_get(NULL, "gpu");
		clk_disable(clk);
		clk_put(clk);
#endif
	}
	return gcvSTATUS_OK;
}

gcsPLATFORM_OPERATIONS platformOperations =
{
	.needAddDevice = _NeedAddDevice,
	.adjustParam   = _adjustParam,
	.getPower      = _getPower,
#ifdef CONFIG_GPU_DYNAMIC_CLOCK_POWER
	.setPower      = _setPower,
	.setClock      = _setClock,
#else
	.setPower      = NULL,
	.setClock      = NULL,
#endif
	.putPower      = _putPower,
};

void
gckPLATFORM_QueryOperations(
		IN gcsPLATFORM_OPERATIONS ** Operations
		)
{
	*Operations = &platformOperations;
}
