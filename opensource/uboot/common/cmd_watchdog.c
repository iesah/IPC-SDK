/*
 *watchdog support
 *The unit of timeout is ms.
 *The max value of timeout is 128000 ms.
 */

#include <common.h>
#include <environment.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <asm/arch/wdt.h>
#include <linux/ctype.h>

#ifdef CONFIG_CMD_WATCHDOG

int do_watchdog (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int timeout;
	int time,i;
	char *type = argv[1];

	/*timeout is a number*/
	for(i=0;type[i] != '\0';i++)
	{
		if(!isdigit(type[i]))
		{
			printf("unsupport the timeout!\n");
			return -1;
		}
	}

	timeout = simple_strtol(argv[1], NULL, 10);
	time = RTC_FREQ / WDT_DIV * timeout / 1000;
	if(time > 65535)
		time = 65535;
	if(timeout < 0)
	{
		printf("unsupport the timeout!\n");
		return -1;
	}
	else if(timeout == 0)
	{
		writel(0,WDT_BASE + WDT_TCER);
		writel(1<<10, WDT_BASE + WDT_TCSR);
		printf("watchdog close!\n");
	}else{
		writel(0,WDT_BASE + WDT_TCER);

		writel(1<<10, WDT_BASE + WDT_TCSR);
		writel(time, WDT_BASE + WDT_TDR);
		writel(TCSR_PRESCALE | TCSR_RTC_EN, WDT_BASE + WDT_TCSR);

		writel(TCER_TCEN,WDT_BASE + WDT_TCER);
		printf("watchdog open!\n");
	}
	return 0;
}

U_BOOT_CMD(
	watchdog,   2,  1,  do_watchdog,
	"open or colse the watchdog",
	"<interface> <timeout>\n"
	"the unit of timeout is ms,reset after timeout\n"
	"the max value of timeout is 128000 ms\n"
	"when timeout is greater than 128000ms, timeout is equal to 128000ms\n"
	"timeout = 0 --> close\n"
	"timeout > 0 --> open\n"
	"timeout < 0 or not numbers --> unsupport"
);

#endif
