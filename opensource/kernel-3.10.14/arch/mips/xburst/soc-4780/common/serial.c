/*
 * JZ SOC serial routines for early_printk.
 * 
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>

#include <linux/spinlock.h>
#include <asm/io.h>

#include <soc/base.h>

#define UART_BASE	UART0_IOBASE
#define UART_OFF	0x1000

#define OFF_TDR		(0x00)
#define OFF_LCR		(0x0C)
#define OFF_LSR		(0x14)

#define LSR_TDRQ	(1 << 5)
#define LSR_TEMT	(1 << 6)

static void check_uart(char c);

static volatile u8 *uart_base;
typedef void (*putchar_f_t)(char);
static putchar_f_t putchar_f = check_uart;
static void putchar(char ch)
{
	int timeout = 10000;
	volatile u8 *base = uart_base;
	/* Wait for fifo to shift out some bytes */
	while ((base[OFF_LSR] & (LSR_TDRQ | LSR_TEMT))
	       != (LSR_TDRQ | LSR_TEMT) && timeout--)
		;
	base[OFF_TDR] = (u8)ch;
}
void err_putchar(char ch) {
	putchar(ch);
}
static void putchar_dummy(char ch)
{
	return;
}
static void check_uart(char c)
{
	/* We Couldn't use ioremap() here */
	volatile u8 *base = (volatile u8*)CKSEG1ADDR(UART0_IOBASE);
	int i;
	for(i=0; i<5; i++) {
		if(base[OFF_LCR])
			break;
		base += UART_OFF;
	}

	if(i<5) {
		uart_base = base;
		putchar_f = putchar;
		putchar_f(c);
	} else {
		putchar_f = putchar_dummy;
	}
}

/* used by early printk */
void prom_putchar(char c)
{
	putchar_f(c);
}
void prom_putstr(char *s)
{
	while(*s) {
		if(*s == '\n')
			putchar_f('\r');
		putchar_f(*s);
		s++;
	}
}
#ifdef CONFIG_EMERGENCY_MSG
/* for dump msg */
struct emergency_msg
{
	char *msg;
	int len;
	spinlock_t lock;
};
void* init_emergency_msg(void) {
	struct emergency_msg *emsg;
	emsg = (struct emergency_msg *)__get_free_page(GFP_KERNEL);
	emsg->msg = (char *)(emsg + 1);
	spin_lock_init(&emsg->lock);
	emsg->len = PAGE_SIZE - sizeof(struct emergency_msg);
	return (void *)emsg;
}
extern int log_buf_copy(char *dest, int idx, int len);
extern void log_buf_clear(void);
void emergency_msg_outlog(void *handle) {
	struct emergency_msg *emsg = (struct emergency_msg *)handle;
	unsigned long flags;
	int len,idx,i;
	spin_lock_irqsave(&emsg->lock,flags);
	idx = 0;
	while(1) {
		len = log_buf_copy(emsg->msg,idx,emsg->len);
		if(len <= 0)
			break;
		for(i = 0;i < len;i++) {
			if(emsg->msg[i] == '\n')
				putchar('\r');
			putchar(emsg->msg[i]);
		}
		idx += len;
	}
	spin_unlock_irqrestore(&emsg->lock,flags);
}
void deinit_emergency_msg(void *handle) {
	struct emergency_msg *emsg = (struct emergency_msg *)handle;
	free_page((unsigned long)emsg);
}

int is_emergency_msg(void *handle) {
	int lcr;
	char c;
	int r = 0;
	int timeout = 16;
	if(!(*(volatile unsigned int *)0xb0010000 & (1 << 30))) {
		c = 0;
		do {
			lcr = *(volatile unsigned int *)(0xb0033000 + 0x14);
			if(lcr & 1){
				c = *(volatile unsigned int *)0xb0033000;
				if(c == 0x0d) r = 1;
				*(volatile unsigned int *)(0xb0033000 + 0x10) = 0;
			}
		}while((lcr & 1) && timeout--);
		if(r) {
			return 1;
		}
	}
	return 0;
}
void *emergency_msg = NULL;
static int __init init_emergency(void)
{
	emergency_msg = init_emergency_msg();
	return 0;
}
module_init(init_emergency);
#endif

#if 0
static char pbuffer[4096];
void prom_printk(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	vsnprintf(pbuffer, 4096, fmt, args);
	va_end(args);

	prom_putstr(pbuffer);
}
#endif
