#include <common.h>
#include "interface.h"
#include "xxx_ops.h"


int (*h_handler)(const char *fmt, ...);
#define printk  h_handler


unsigned int dst[4*1024];
unsigned int src[4*1024];
//unsigned int _data_2[4*1024];

int open(void)
{
	printk("[######## second refresh]: open\n");
	return 0;
}

int handler(void)
{
	static volatile int i;
	i++;
	TCSM_PCHAR('h');
	TCSM_PCHAR('a');
	TCSM_PCHAR('n');
	TCSM_PCHAR('d');
	TCSM_PCHAR('l');
	TCSM_PCHAR('e');
	serial_put_hex(i);
	xxx_memcopy(dst, src, 4*1024);
	return 0;
}
int close(void)
{
	printk("[######## second refresh]: close\n");
	return 0;
}

int cache_prefetch(void)
{
	volatile unsigned int a;
	volatile unsigned int *addr = LOAD_ADDR;
	int i;
	printk("#######################3 caceh prefetch\n");
	for(i=0; i<LOAD_SIZE/32; i++) {
		a = *(addr + i * 8);
	}

	return 0;
}

int set_handler(void *handler)
{
	h_handler = (int(*)(const char *fmt, ...)) handler;
	return 0;
}
