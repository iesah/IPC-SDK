/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/syscore_ops.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>

#include <soc/ahbm.h>
#include <soc/base.h>
#include <soc/cpm.h>

static int period = 2; //nr
static int sum = 500; //ms
static int rate = 20; //ms


struct saved_value {
	unsigned int ddr[5];
	unsigned int cim[11];
	unsigned int ahb0[11];
	unsigned int gpu[16];
	unsigned int lcd[16];
	unsigned int xxx[16];
	unsigned int ahb2[11];
};
static int __print_value(struct seq_file *m,unsigned int *buf,int chan,
		unsigned int base,int count)
{
	int i=0;
	for(i=0;i<count;i++) {
		seq_printf(m,"CH%d(%x):%x\n",chan,(base+i*0x4),buf[i]);
	}
	return 0;
}
static void __save_value(unsigned int *buf,
		unsigned int base,int count)
{
	int i=0;
	for(i=0;i<count;i++) {
		buf[i] = inl(base+i*0x4);
	}
}
static int print_value(struct seq_file *m,struct saved_value *buf)
{
	int len = 0;
	seq_printf(m,"DDR(%x):%x\n",DDR_MC,buf->ddr[0]);
	seq_printf(m,"DDR(%x):%x\n",DDR_RESULT_1,buf->ddr[1]);
	seq_printf(m,"DDR(%x):%x\n",DDR_RESULT_2,buf->ddr[2]);
	seq_printf(m,"DDR(%x):%x\n",DDR_RESULT_3,buf->ddr[3]);
	seq_printf(m,"DDR(%x):%x\n",DDR_RESULT_4,buf->ddr[4]);
#if 1
	__print_value(m,buf->cim,0,AHBM_CIM_IOB,11);
	__print_value(m,buf->ahb0,1,AHBM_AHB0_IOB,11);
	__print_value(m,buf->gpu,2,AHBM_GPU_IOB,16);
	__print_value(m,buf->lcd,3,AHBM_LCD_IOB,16);
	__print_value(m,buf->xxx,4,AHBM_XXX_IOB,16);
	__print_value(m,buf->ahb2,5,AHBM_AHB2_IOB,11);
#endif
	return len;
}
static void save_value(struct saved_value *buf)
{
	buf->ddr[0] = inl(DDR_MC);
	buf->ddr[1] = inl(DDR_RESULT_1);
	buf->ddr[2] = inl(DDR_RESULT_2);
	buf->ddr[3] = inl(DDR_RESULT_3);
	buf->ddr[4] = inl(DDR_RESULT_4);

	__save_value(buf->cim,AHBM_CIM_IOB,11);
	__save_value(buf->ahb0,AHBM_AHB0_IOB,11);
	__save_value(buf->gpu,AHBM_GPU_IOB,16);
	__save_value(buf->lcd,AHBM_LCD_IOB,16);
	__save_value(buf->xxx,AHBM_XXX_IOB,16);
	__save_value(buf->ahb2,AHBM_AHB2_IOB,11);
}

//static int ahbm_read_proc(char *page, char **start, off_t off,
//		int count, int *eof, void *data)
static int ahbm_show(struct seq_file *m, void *v)
{
	int i = 0, len = 0;
	int ignore_time = (sum/period)-rate;
	struct saved_value *vbuf;

	vbuf = vmalloc(sizeof(struct saved_value) * period);
	if(!vbuf) {
		seq_printf(m,"//period = %d\nsum = %dms\nrate = %dms",period,sum,rate);
		seq_printf(m,"malloc err.\n");
		return len;
	}

	//cpm_clear_bit(11,CPM_CLKGR);
	cpm_clear_bit(4,CPM_CLKGR);
	while(i<period) {
		outl(0x0,DDR_MC);
		outl(0x0,DDR_RESULT_1);
		outl(0x0,DDR_RESULT_2);
		outl(0x0,DDR_RESULT_3);
		outl(0x0,DDR_RESULT_4);
		outl(0x1,DDR_MC);

		ahbm_restart(CIM);
		ahbm_restart(AHB0);
		ahbm_restart(GPU);
		ahbm_restart(LCD);
		ahbm_restart(XXX);
		ahbm_restart(AHB2);

		msleep(rate);

		outl(0x0,DDR_MC);

		ahbm_stop(CIM);
		ahbm_stop(AHB0);
		ahbm_stop(GPU);
		ahbm_stop(LCD);
		ahbm_stop(XXX);
		ahbm_stop(AHB2);

		save_value(&vbuf[i]);

		msleep(ignore_time);
		i++;
	}
	cpm_set_bit(4,CPM_CLKGR);

	seq_printf(m,"//period=%d sum=%dms rate = %dms\n",period,sum,rate);

	i=0;
	while(i<period) {
		print_value(m,&vbuf[i]);
		seq_printf(m,"SAMPLE_FINISH:%d\n",i);
		i++;
	}

	vfree(vbuf);
	return 0;
}

static int ahbm_write(struct file *file, const char __user *buffer,
		size_t count, loff_t *data)
{
	int ret;
	char buf[32];

	if (count > 32)
		count = 32;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	ret = sscanf(buf,"sum=%dms;period=%d;rate=%dms\n",&sum,&period,&rate);

	return count;
}

static int ahbm_open(struct inode *inode, struct file *file)
{
	return single_open(file, ahbm_show, PDE_DATA(inode));
}

static const struct file_operations ahbm_proc_fops ={
	.read = seq_read,
	.open = ahbm_open,
	.write = ahbm_write,
	.llseek = seq_lseek,
	.release = single_release,
};


static int __init init_ahbm_proc(void)
{
	proc_create("ahbm", 0444, NULL,&ahbm_proc_fops);
	return 0;
}

module_init(init_ahbm_proc);

