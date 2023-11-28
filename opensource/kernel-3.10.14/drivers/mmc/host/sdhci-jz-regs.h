#ifndef __SDHCI_JZ_REGS_H__
#define __SDHCI_JZ_REGS_H__

/* Software redefinition caps */
#define CAPABILITIES1_SW	0x276dc898
#define CAPABILITIES2_SW	0

#define SDHCI_EMMC_CTRL_HS_SDR200 0x0003

#ifdef CONFIG_FPGA_TEST
#define CPM_MSC0_CLK_R		(0xB0000068)
#define CPM_MSC1_CLK_R		(0xB00000a4)
#define MSC_CLK_H_FREQ		(0x1 << 20)
#endif //CONFIG_FPGA_TEST


#endif //__SDHCI_JZ_REGS_H__
