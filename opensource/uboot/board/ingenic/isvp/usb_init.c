

#include <common.h>
#include <usb.h>


#define USBRDT_VBFIL_LD_EN		25
#define USBRDT_IDDIG_EN			24
#define USBRDT_IDDIG_REG		23
#define USBPCR_TXPREEMPHTUNE		6
#define USBPCR_POR			22
#define USBPCR_USB_MODE			31
#define USBPCR_COMMONONN		25
#define USBPCR_VBUSVLDEXT		24
#define USBPCR_VBUSVLDEXTSEL		23
#define USBPCR_OTG_DISABLE		20
#define USBPCR_SIDDQ			21
#define USBPCR_IDPULLUP_MASK		28
#define OPCR_SPENDN0			7
#define USBPCR1_USB_SEL			28
#define USBPCR1_WORD_IF0		19
#define USBPCR1_WORD_IF1		18
#define SRBC_USB_SR			12

#define CPM_SRBC		0x100000c4
#define CPM_USBPCR1		0x10000048
#define CPM_USBVBFIL		0x10000044
#define CPM_USBRDT		0x10000040
#define CPM_USBPCR              0x1000003c
#define CPM_CLKGR0              0x10000020
#define CPM_CLKGR1              0x10000028
#define CPM_OPCR		0x10000024
#define CPM_CPAPCR              0x10000010



#define reg_set_bit(bit,reg)           *(volatile unsigned int *)(reg) |= 1UL<<(bit)
#define reg_clr_bit(bit,reg)	       *(volatile unsigned int *)(reg) &= ~(1UL<<(bit))
#define reg_wr(val,reg)       	       *(volatile unsigned int *)(reg) = (val)
#define reg_rd(reg)		       *(volatile unsigned int *)(reg)

int board_usb_init(void)
{

	unsigned int usbpcr1, usbrdt;

	/* select dwc otg */
	reg_set_bit(28, CPM_USBPCR1);
	reg_set_bit(29, CPM_USBPCR1);

	/* select utmi data bus width of port0 to 16bit/30M */
	reg_set_bit(USBPCR1_WORD_IF0, CPM_USBPCR1);

	usbpcr1 = reg_rd(CPM_USBPCR1);
	usbpcr1 &= ~(0x7 << 23);
	usbpcr1 |= (5 << 23);
	reg_wr(usbpcr1, CPM_USBPCR1);

	/*unsuspend*/
	reg_set_bit(7, CPM_OPCR);
	udelay(45);
	reg_clr_bit(USBPCR_SIDDQ, CPM_USBPCR);

	/* fil */
	reg_wr(0, CPM_USBVBFIL);

	/* rdt */
	usbrdt = reg_rd(CPM_USBRDT);
	usbrdt &= ~(USBRDT_VBFIL_LD_EN | ((1 << 23) - 1));
	usbrdt |= 0x96;
	reg_wr(usbrdt, CPM_USBRDT);

	/* rdt - filload_en */
	reg_set_bit(USBRDT_VBFIL_LD_EN, CPM_USBRDT);

	reg_wr(0x83803857, CPM_USBPCR);

	unsigned int tmp;
	tmp = reg_rd(CPM_USBPCR);
	tmp |= 1 << USBPCR_USB_MODE | 1 << USBPCR_COMMONONN;
	tmp &= ~(1 << USBPCR_OTG_DISABLE | 1 << USBPCR_SIDDQ |
			0x03 << USBPCR_IDPULLUP_MASK | 1 << USBPCR_VBUSVLDEXT |
			1 << USBPCR_VBUSVLDEXTSEL);
	reg_wr(tmp, CPM_USBPCR);

	reg_set_bit(USBPCR_POR, CPM_USBPCR);
	mdelay(1);
	reg_clr_bit(USBPCR_POR, CPM_USBPCR);
	mdelay(1);

     return 0;
}
