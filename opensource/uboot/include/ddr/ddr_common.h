#ifndef __DDR_COMMON_H__
#define __DDR_COMMON_H__

#ifdef CONFIG_CPU_XBURST2
#ifdef CONFIG_FPGA
#include <ddr/ddrcp_fpga/ddr_chips.h>
#include <ddr/ddrcp_fpga/ddr_params.h>
#include <ddr/ddrcp_fpga/ddrc.h>
#include <ddr/ddrcp_fpga/ddrp_inno.h>
#include <asm/ddr_innophy_fpga.h>
#else
#include <ddr/ddr_chips_v2.h>
#include <ddr/ddrcp_chip/ddr_params.h>
#include <ddr/ddrcp_chip/ddrc.h>
#include <ddr/ddrcp_chip/ddrp_inno.h>
#include <asm/ddr_innophy.h>
#endif/*CONFIG_FPGA*/
#endif

#endif /* __DDR_COMMON_H__ */
