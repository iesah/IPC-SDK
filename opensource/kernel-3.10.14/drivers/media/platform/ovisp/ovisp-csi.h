#ifndef __OVISP_CSI_H__
#define __OVISP_CSI_H__

/* csi host regs, base addr should be defined in board cfg */
#define DWC_CSI_BASE		0xB3013000

#define VERSION				0x00
#define N_LANES				0x04
#define PHY_SHUTDOWNZ		0x08
#define DPHY_RSTZ			0x0C
#define CSI2_RESETN			0x10
#define PHY_STATE			0x14
#define DATA_IDS_1			0x18
#define DATA_IDS_2			0x1C
#define ERR1				0x20
#define ERR2				0x24
#define MASK1				0x28
#define MASK2				0x2C
#define PHY_TST_CTRL0       0x30
#define PHY_TST_CTRL1       0x34


typedef enum
{
	ERR_NOT_INIT = 0xFE,
	ERR_ALREADY_INIT = 0xFD,
	ERR_NOT_COMPATIBLE = 0xFC,
	ERR_UNDEFINED = 0xFB,
	ERR_OUT_OF_BOUND = 0xFA,
	SUCCESS = 0
} csi_error_t;


#define dwc_csi_readl(reg)              \
	__raw_readl((unsigned int *)(DWC_CSI_BASE + reg))
#define dwc_csi_writel(reg, value)          \
	__raw_writel((value), (unsigned int *)(DWC_CSI_BASE + reg))

#define csi_core_write(addr, value) dwc_csi_writel(addr, value)
#define csi_core_read(addr) dwc_csi_readl(addr)

/* function */
extern int csi_phy_init(void);
extern int csi_phy_release(void);
extern int csi_phy_start(unsigned int id, unsigned int freq, unsigned int lans);
extern int csi_phy_stop(unsigned int id);

extern void dump_csi_reg(void);
extern void check_csi_error(void);
extern unsigned char csi_set_on_lanes(unsigned char lanes);

#endif/*__OVISP_CSI_H__*/
