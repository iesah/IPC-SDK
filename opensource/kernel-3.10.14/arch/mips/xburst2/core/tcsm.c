#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#define CPU_TCSM_BASE 0xf4000000
#define CPU_TCSM_SIZE (1024*16)

#define MAX_COUNT 20
struct tcsm_mem {
	unsigned int start;
	unsigned short size;
	unsigned short cpu_alloc;
	char name[20];
};

static DEFINE_SPINLOCK(tcsm_lock);
static struct tcsm_mem tcsm_cell[MAX_COUNT];
static int tcsm_cur_pos = 0;
static int tcsm_cur_offset = 0;


void tcsm_init(void)
{
	int i;
	for (i = 0; i < (CPU_TCSM_SIZE / 4); i++) {
		unsigned int *p = (unsigned int *)(CPU_TCSM_BASE + i * 4);
		*p = 0;
	}
}

unsigned int get_cpu_tcsm(int cpu,int len,char *name) {
	struct tcsm_mem *p;
	int i;
	unsigned int mem = 0;
	len = (len + 3) & ~(0x3);
	spin_lock(&tcsm_lock);
	for(i = 0;i < tcsm_cur_pos;i++) {
		if(strcmp(name,tcsm_cell[i].name) == 0) {
			p = &tcsm_cell[i];
			mem = p->start + CPU_TCSM_BASE;
			p->cpu_alloc |= (1 << cpu);
			break;
		}
	}

	if(!mem) {
		p = &tcsm_cell[tcsm_cur_pos];
		p->start = tcsm_cur_offset;
		p->cpu_alloc |= (1 << cpu);
		p->size = len;
		strcpy(p->name,name);
		mem = p->start + CPU_TCSM_BASE;
		tcsm_cur_offset += len;
		tcsm_cur_pos++;

	}
	spin_unlock(&tcsm_lock);
	if(mem) {
    		return mem;
	}
	return 0;
}

#ifdef CONFIG_TRAPS_USE_TCSM_CHECK
static int error = 0;
unsigned long save_buf[ 32 * 1024 ] __attribute__ ((aligned (32)));
unsigned long check_buf[ 32 * 1024 ] __attribute__ ((aligned (32)));
void tcsm_check(int *a,int *b,int size)
{
	int i;
	for(i=0;i<size/4;i++) {
		if(a[i] != b[i]) {
			printk("not sync.\nsave buf:");
			for(i=0;i<size/4;i++) {
				if(i%8 == 0) printk("\n");
				printk("%08x ",a[i]);
			}
			printk("\ncheck buf:");

			for(i=0;i<size/4;i++) {
				if(i%8 == 0) printk("\n");
				printk("%08x ",b[i]);
			}
			printk("\n");

			error = 1;
			break;
		}
	}
}
#endif

extern unsigned long reserved_for_alloccache[]; //from c-jz.c
void cpu0_save_tscm(void) {
	struct tcsm_mem *pos;
	int i;
#ifdef CONFIG_TRAPS_USE_TCSM_CHECK
	if(error) while(1);
#endif
	spin_lock(&tcsm_lock);
	for(i = 0;i < tcsm_cur_pos;i++) {
		pos = &tcsm_cell[i];
		if(pos->cpu_alloc){
			memcpy((void*)((unsigned int)reserved_for_alloccache + pos->start),
			       (void*)(pos->start + CPU_TCSM_BASE),
			       pos->size);
#ifdef CONFIG_TRAPS_USE_TCSM_CHECK
			memcpy((void*)((unsigned int)save_buf + pos->start),
				(void*)(pos->start + CPU_TCSM_BASE),
				pos->size);
#endif
		}
	}
	spin_unlock(&tcsm_lock);
}
void cpu0_restore_tscm(void) {
	struct tcsm_mem *pos;
	int i;
	spin_lock(&tcsm_lock);
	for(i = 0;i < tcsm_cur_pos;i++) {
		pos = &tcsm_cell[i];
		if(pos->cpu_alloc){
			memcpy((void*)(pos->start + CPU_TCSM_BASE),
			       (void*)((unsigned int)reserved_for_alloccache + pos->start),
			       pos->size);
#ifdef CONFIG_TRAPS_USE_TCSM_CHECK
			memcpy((void*)((unsigned int)check_buf + pos->start),
				(void*)(pos->start + CPU_TCSM_BASE),
				pos->size);
			tcsm_check((int *)save_buf,(int *)check_buf,pos->size);
#endif
		}
	}
	spin_unlock(&tcsm_lock);
	dma_cache_wback((long)reserved_for_alloccache,CPU_TCSM_SIZE);
}
