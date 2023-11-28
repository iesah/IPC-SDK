#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <mach/jzcpm_pwc.h>
#include <soc/base.h>
#include <soc/cpm.h>
struct cpm_pwc_desc
{
	const char *name;
	int ctrl_bit;
	int state_bit;
	int refcount;
};

struct cpm_pwc_desc cpm_pwc[] = {
	{PWC_X2D,31,27,0},{PWC_VPU,30,26,0}
};

static DEFINE_SPINLOCK(cpm_pwc_lock);
static int find_cpm_pwc(char *s) {
	int i;
	for(i = 0;i < ARRAY_SIZE(cpm_pwc);i++) {
		if(strcmp(cpm_pwc[i].name,s) == 0)
			break;
	}
	if(i >= ARRAY_SIZE(cpm_pwc)) return -1;
	return i;
}
int cpm_pwc_enable(void *handle)
{
	struct cpm_pwc_desc *pwc = (struct cpm_pwc_desc *)handle;
	unsigned long flags;
	unsigned int count = 100000;
	spin_lock_irqsave(&cpm_pwc_lock,flags);
	cpm_clear_bit(pwc->ctrl_bit,CPM_LCR);
	while(cpm_test_bit(pwc->state_bit,CPM_LCR) && count--);
	spin_unlock_irqrestore(&cpm_pwc_lock,flags);
	if(count <= 0) {
		printk("wait %s power off timeout!\n",pwc->name);
	}
	return 0;
}

int cpm_pwc_disable(void *handle)
{
	struct cpm_pwc_desc *pwc = (struct cpm_pwc_desc *)handle;
	unsigned long flags;
	unsigned int count = 100000;
	spin_lock_irqsave(&cpm_pwc_lock,flags);
	cpm_set_bit(pwc->ctrl_bit,CPM_LCR);
	while((!cpm_test_bit(pwc->state_bit,CPM_LCR)) && count--);
	spin_unlock_irqrestore(&cpm_pwc_lock,flags);
	if(count <= 0) {
		printk("wait %s power off timeout!\n",pwc->name);
	}
	return 0;
}

int cpm_pwc_is_enabled(void *handle)
{
	struct cpm_pwc_desc *pwc = (struct cpm_pwc_desc *)handle;
	return !cpm_test_bit(pwc->state_bit,CPM_LCR);
}
void *cpm_pwc_get(char *name){
	int index = find_cpm_pwc(name);
	if(index == -1) {
		printk("%s not finded!\n",name);
		dump_stack();
		return NULL;
	}
	if(cpm_pwc[index].refcount > 1) {
		printk("%s refcount > 1!\n",name);
		dump_stack();
	}
	cpm_pwc[index].refcount++;
	return (void*)&cpm_pwc[index];
}
void cpm_pwc_put(void *handle) {
	struct cpm_pwc_desc *pwc = (struct cpm_pwc_desc *)handle;
	pwc->refcount--;
	if(pwc->refcount < 0) {
		printk("%s refcount < 0!\n",pwc->name);
		dump_stack();
	}
}
void cpm_pwc_init(void) {
#ifndef CONFIG_FPGA_TEST
	unsigned long lcr = cpm_inl(CPM_LCR);

	cpm_outl(lcr | CPM_LCR_PD_MASK | 0x8f<<8,CPM_LCR);
	while((cpm_inl(CPM_LCR) & (0x3<<26)) != (0x3<<26));

	cpm_outl(0,CPM_PSWC0ST);
	cpm_outl(16,CPM_PSWC1ST);
	cpm_outl(24,CPM_PSWC2ST);
	cpm_outl(8,CPM_PSWC3ST);
#endif
}

