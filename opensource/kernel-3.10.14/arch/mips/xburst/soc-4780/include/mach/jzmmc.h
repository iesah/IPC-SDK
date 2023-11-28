#ifndef __JZ_MMC_H__
#define __JZ_MMC_H__

#include <linux/regulator/consumer.h>
#include <linux/wakelock.h>

#define MMC_BOOT_AREA_PROTECTED	(0x1234)	/* Can not modified the area protected */
#define MMC_BOOT_AREA_OPENED	(0x4321)	/* Can modified the area protected */

enum {
	DONTCARE = 0,
	NONREMOVABLE,
	REMOVABLE,
	MANUAL,
};

struct mmc_partition_info {
	char				name[32];
	unsigned int			saddr;
	unsigned int			len;
	int				type;
};

struct mmc_recovery_info {
	struct mmc_partition_info	*partition_info;
	unsigned int			partition_num;
	unsigned int			permission;
	unsigned int			protect_boundary;
};

struct jzmmc_pin {
	short				num;
#define LOW_ENABLE			0
#define HIGH_ENABLE			1
	short 				enable_level;
};

struct card_gpio {
	struct jzmmc_pin 		wp;
	struct jzmmc_pin 		cd;
	struct jzmmc_pin 		pwr;
};

struct wifi_data {
	struct wake_lock		wifi_wake_lock;
	struct regulator		*wifi_power;
	int				wifi_reset;
};

/**
 * struct jzmmc_platform_data is a struct which defines board MSC informations
 * @removal: This shows the card slot's type:
 *	REMOVABLE/IRREMOVABLE/MANUAL (Tablet card/Phone card/build-in SDIO).
 * @sdio_clk: SDIO device's clock can't use Low-Power-Mode.
 * @ocr_mask: This one shows the voltage that host provide.
 * @capacity: Shows the host's speed capacity and bus width.
 * @max_freq: The max freqency of mmc host.
 *
 * @recovery_info: Informations that Android recovery mode uses.
 * @gpio: Slot's gpio information including pins of write-protect, card-detect and power.
 * @pio_mode: Indicate that whether the MSC host use PIO mode.
 * @private_init: Board private initial function, mostly for SDIO devices.
 */
struct jzmmc_platform_data {
	unsigned short			removal;
	unsigned short			sdio_clk;
	unsigned int			ocr_avail;
	unsigned int			capacity;
	unsigned int			pm_flags;
	unsigned int			max_freq;
	struct mmc_recovery_info	*recovery_info;
	struct card_gpio		*gpio;
	unsigned int			pio_mode;
	int				(*private_init)(void);
};

#define ENABLE_CLK32K     0x00000006
#define DISABLE_CLK32K    0x00000010 
#define RTC_IOBASE        0x10003000
#define RTC_RTCCR        (0x00)  
#define RTC_WENR         (0x3c)  
#define RTC_CKPCR        (0x40) 
#define RTCCR_WRDY        BIT(7)
#define WENR_WEN          BIT(31)
#define ENABLE_CLK32K     0x00000006
#define DISABLE_CLK32K    0x00000010 
static void inline rtc_write_reg(int reg,int value)
{
        while(!(inl(RTC_IOBASE + RTC_RTCCR) & RTCCR_WRDY));
        outl(0xa55a,(RTC_IOBASE + RTC_WENR));
        while(!(inl(RTC_IOBASE + RTC_RTCCR) & RTCCR_WRDY));
        while(!(inl(RTC_IOBASE + RTC_WENR) & WENR_WEN));
        while(!(inl(RTC_IOBASE + RTC_RTCCR) & RTCCR_WRDY));
        outl(value,(RTC_IOBASE + reg));
        while(!(inl(RTC_IOBASE + RTC_RTCCR) & RTCCR_WRDY));
}
#define jzrtc_enable_clk32k()  rtc_write_reg(RTC_CKPCR,ENABLE_CLK32K)
#define jzrtc_disable_clk32k() rtc_write_reg(RTC_CKPCR,DISABLE_CLK32K)

extern int jzmmc_manual_detect(int index, int on);
extern int jzmmc_clk_ctrl(int index, int on);

#endif
