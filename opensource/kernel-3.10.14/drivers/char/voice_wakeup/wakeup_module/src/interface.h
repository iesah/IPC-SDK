#ifndef __INTERFACE_H__
#define __INTERFACE_H__


extern int (*h_handler)(const char *fmt, ...);
/*#define CONFIG_SLEEP_DEBUG*/

#ifdef CONFIG_SLEEP_DEBUG
#define printk	h_handler
#define printf printk
#else
#define debug_print(fmt, args...) do{} while(0)
#define printk debug_print
#define printf printk
#endif

enum open_mode {
	EARLY_SLEEP = 1,
	DEEP_SLEEP,
	NORMAL_RECORD,
	NORMAL_WAKEUP
};


#define DMIC_IOCTL_SET_SAMPLERATE	0x200


/* same define as kernel */
#define		SLEEP_BUFFER_SIZE	(32 * 1024)
#define		NR_BUFFERS			(8)

struct sleep_buffer {
	unsigned char *buffer[NR_BUFFERS];
	unsigned int nr_buffers;
	unsigned long total_len;
};

#define TCSM_DATA_BUFFER_ADDR	(0xb3422000) /* bank0 */
#define TCSM_DATA_BUFFER_SIZE	(4096)

#define TCSM_DESC_ADDR			(0xb3424000) /* bank2 start */

#define TCSM_SP_ADDR			(0xb3425fff) /* bank3 end */

#define LOAD_ADDR	0x8ff00000
#define LOAD_SIZE	(256 * 1024)

#define OFF_TDR         (0x00)
#define OFF_LCR         (0x0C)
#define OFF_LSR         (0x14)

#define LSR_TDRQ        (1 << 5)
#define LSR_TEMT        (1 << 6)

#define UART1_IOBASE    0x10031000
#define U1_IOBASE (UART1_IOBASE + 0xa0000000)
#ifdef CONFIG_SLEEP_DEBUG
#define TCSM_PCHAR(x)                           \
	*((volatile unsigned int*)(U1_IOBASE+OFF_TDR)) = x;     \
while ((*((volatile unsigned int*)(U1_IOBASE + OFF_LSR)) & (LSR_TDRQ | LSR_TEMT)) != (LSR_TDRQ | LSR_TEMT))

static inline void serial_put_hex(unsigned int x) {
	int i;
	unsigned int d;
	for(i = 7;i >= 0;i--) {
		d = (x  >> (i * 4)) & 0xf;
		if(d < 10) d += '0';
		else d += 'A' - 10;
		TCSM_PCHAR(d);
	}
}
#else
#define TCSM_PCHAR(x)	do {} while(0)
#define serial_put_hex(x) do {} while(0)
#endif



#endif



