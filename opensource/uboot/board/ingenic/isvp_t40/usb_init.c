

#include <common.h>
#include <usb.h>


#define USBRDT_VBFIL_LD_EN		25
#define USBRDT_UTMI_RST		27
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
#define USBRDT_IDDIG_EN			24
#define USBRDT_IDDIG_REG		23

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

unsigned int usbpcr1,tmp;
#if 0
	/*set pins*/
	reg_set_bit(17,0x10010260);
	reg_set_bit(17,0x10010264);
#endif
	/*feed clock to otg*/
        reg_clr_bit(3,CPM_CLKGR0);
      /*  reg_wr(0x0bcf5780,CPM_CLKGR0); */
      /* reg_wr(0x0470890d,CPM_CPAPCR); */
	mdelay(100);
        /* softreset otg */
	reg_set_bit(SRBC_USB_SR, CPM_SRBC);
	udelay(40);
	reg_clr_bit(SRBC_USB_SR, CPM_SRBC);

	reg_set_bit(8, CPM_USBPCR1);
	reg_set_bit(9, CPM_USBPCR1);
	reg_set_bit(28, CPM_USBPCR1);
	reg_set_bit(29, CPM_USBPCR1);
	reg_set_bit(30, CPM_USBPCR1);

	reg_clr_bit(USBPCR1_WORD_IF0, CPM_USBPCR1);
	usbpcr1 = reg_rd(CPM_USBPCR1);
	usbpcr1 &= ~(0x7 << 23);
	usbpcr1 |= (5 << 23);
	reg_wr(usbpcr1, CPM_USBPCR1);

	reg_wr(0, CPM_USBVBFIL);

	reg_wr(0x96, CPM_USBRDT);

	reg_set_bit(USBRDT_VBFIL_LD_EN, CPM_USBRDT);

	reg_wr(0x8380385a, CPM_USBPCR);

		tmp = reg_rd(CPM_USBPCR);
		tmp |= 1 << USBPCR_USB_MODE | 1 << USBPCR_COMMONONN;
		tmp &= ~(1 << USBPCR_OTG_DISABLE | 1 << USBPCR_SIDDQ |
				0x03 << USBPCR_IDPULLUP_MASK | 1 << USBPCR_VBUSVLDEXT |
				1 << USBPCR_VBUSVLDEXTSEL);
	reg_wr(tmp, CPM_USBPCR);

	reg_set_bit(USBPCR_POR, CPM_USBPCR);
	reg_clr_bit(USBRDT_UTMI_RST, CPM_USBRDT);
	reg_set_bit(SRBC_USB_SR, CPM_SRBC);
	udelay(10);
	reg_clr_bit(USBPCR_POR, CPM_USBPCR);

	udelay(20);
	reg_set_bit(OPCR_SPENDN0, CPM_OPCR);
	mdelay(50);

	udelay(950);
	reg_set_bit(USBRDT_UTMI_RST, CPM_USBRDT);

	udelay(20);
	reg_clr_bit(SRBC_USB_SR, CPM_SRBC);
	mdelay(10);


     return 0;
}
