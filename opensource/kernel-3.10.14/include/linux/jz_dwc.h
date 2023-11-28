#ifndef __JZ_DWC_H__
#define __JZ_DWC_H__

#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>

struct jzdwc_pin {
	short				num;
#define LOW_ENABLE			0
#define HIGH_ENABLE			1
	short 				enable_level;
};

struct dwc_jz_pri {
	struct clk		*clk;

	int 			irq;
	struct jzdwc_pin 	*dete;
	struct delayed_work	work;
	struct delayed_work	charger_delay_work;

	struct regulator 	*vbus;
	struct regulator 	*ucharger;

	spinlock_t		lock;
	struct mutex		mutex;
	void			*core_if;
	int			pullup_on;

	void			(*start)(struct dwc_jz_pri *jz_pri);
	void			(*callback)(struct dwc_jz_pri *jz_pri);
};

typedef enum otg_mode {
	DEVICE_ONLY = 1,
	HOST_ONLY = 2,
	DUAL_MODE = 3,
} otg_mode_t;

static otg_mode_t inline dwc2_usb_mode(void) {
	if (IS_ENABLED(CONFIG_USB_DWC2_HOST_ONLY))
		return HOST_ONLY;
	else if (IS_ENABLED(CONFIG_USB_DWC2_DEVICE_ONLY))
		return DEVICE_ONLY;
	else
		return DUAL_MODE;
}

void jz_otg_phy_init(otg_mode_t mode);
void jz_otg_phy_suspend(int suspend);
int  jz_otg_phy_is_suspend(void);
void jz_otg_phy_powerdown(void);
void jz_otg_ctr_reset(void);
#endif
