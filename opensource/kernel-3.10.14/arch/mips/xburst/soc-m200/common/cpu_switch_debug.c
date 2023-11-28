
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <asm/page.h>
#include <asm/r4kcache.h>
#include <soc/base.h>
#define REG32(x) *(volatile unsigned int *)(x)

#define OFF_TDR         (0x00)
#define OFF_LCR         (0x0C)
#define OFF_LSR         (0x14)

#define LSR_TDRQ        (1 << 5)
#define LSR_TEMT        (1 << 6)

#define U1_IOBASE (UART1_IOBASE + 0xa0000000)
#define TCSM_PCHAR(x)							\
	*((volatile unsigned int*)(U1_IOBASE+OFF_TDR)) = x;		\
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
extern unsigned int _tlb_entry_regs[];
extern unsigned int _ready_flag;
extern unsigned int _wait_flag;
void flush_cache_all_2(void)
{
	blast_dcache32();
	blast_scache32();
}
void flush_cache_all_1(void)
{
	unsigned int *d = _tlb_entry_regs;
	unsigned int *d1 = &_ready_flag;
	unsigned int *d2 = &_wait_flag;
	int i;
	d = (unsigned int *)((unsigned int)d + 0x20000000);
	d1 = (unsigned int *)((unsigned int)d1 + 0x20000000);
	d2 = (unsigned int *)((unsigned int)d2 + 0x20000000);
	printk("add = %p\n",d);
	udelay(1000000);
	printk("_ready_flag = %d\n",*d1);
	printk("_wait_flag = %d\n",*d2);

	mb();
	blast_dcache32();
	blast_scache32();
	mb();
	for(i = 0;i < 32;i++)
		printk("0x%08x 0x%08x 0x%08x\n",d[i * 3],d[i *3 + 1],d[i*3 +2]);
	udelay(1000000);
	printk("_ready_flag = %d\n",*d1);
	printk("_wait_flag = %d\n",*d2);

	udelay(1000000);

	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');
}
void show_tlb_reg(void)
{
	unsigned int *d = _tlb_entry_regs;
	int i;
	for(i = 0;i < 32;i++)
		printk("0x%08x 0x%08x 0x%08x\n",d[i * 3],d[i *3 + 1],d[i*3 +2]);
}
void show_message(unsigned int a)
{
	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');
	serial_put_hex(a);
	TCSM_PCHAR('\r');
	TCSM_PCHAR('\n');

}
