/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm-generic/errno-base.h>
#include <jz_proc.h>

#define read_perf_cnter_jz(source, sel)				\
({ int __res;								\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mfc0\t%0, " #source ", " #sel "\n\t"		\
			".set\tmips0\n\t"				\
			: "=r" (__res));				\
	__res;								\
})


#define write_perf_cnter_jz(register, sel, value)			\
({ do {									\
		__asm__ __volatile__(					\
			".set\tmips32\n\t"				\
			"mtc0\t%0, " #register ", " #sel "\n\t"	\
			".set\tmips0"					\
			: : "r" ((unsigned int)(value)));		\
	} while (0);									\
})

struct proc_dir_entry *proc_entry = NULL;

/* NOTE: FSE_INDEX is the index of perf48  +1 in array performance_command. */
#define FSE_INDEX	12

static const char *abbr_perf_name[] = {
				"CC", "PSCC", "IMN", "DMN", "IDN", "IIN",
				"ITE", "DTE", "IMCC", "DMCC", "PSFCC",
				"FIOCC", "FISCC", "FILCC", "FIMCC", "FIXLCC",
				"FIIUCC", "FIBRCC", "FIRBCC", "FSLDCC", "FSLCCC",
				"FSSDCC", "FSSCCC", "****"};
#if 1
static const char *performance_command[] = {
                "perf0","perf1","perf2","perf3","perf4","perf5",
                "perf6","perf7", "perf8", "perf9", "perf10",
				"perf48", "perf49", "perf50", "perf51", "perf52",
				"perf53", "perf54", "perf55", "perf56", "perf57",
				"perf58", "perf59", "perfstop"};
#endif

static int perform_show(struct seq_file *m, void *v)
{
	int len = 0;
	u64 cnt0 = 0;
	u64 cnt1 = 0;

	cnt0 = read_perf_cnter_jz($25, 0);
	cnt0 &= 0x7e0;
	cnt0 = cnt0 >> 5;

	if (((cnt0 > 0xa) && (cnt0 < 0x30)) || (cnt0 > 0x3b)) {
		cnt0 = ARRAY_SIZE(abbr_perf_name) - 1;
	} else if (cnt0 >= 0x30) {
		cnt0 = cnt0 - 0x30 + FSE_INDEX - 1;
	}

	len = seq_printf(m, " PERF-%s: ", abbr_perf_name[cnt0]);
	cnt0 = read_perf_cnter_jz($25, 1);
	cnt0 += current->thread.pfc[0].perfcnt;
	len += seq_printf(m, "%llu\n", cnt0);

	cnt1 = read_perf_cnter_jz($25, 2);
	cnt1 &= 0x7e0;
	cnt1 = cnt1 >> 5;
	if (((cnt1 > 0xa) && (cnt1 < 0x30)) || (cnt1 > 0x3b)) {
		cnt1 = ARRAY_SIZE(abbr_perf_name) - 1;
	} else if (cnt1 >= 0x30) {
		cnt1 = cnt1 - 0x30 + FSE_INDEX - 1;
	}

	len += seq_printf(m, "PERF-%s: ", abbr_perf_name[cnt1]);
	cnt1 = read_perf_cnter_jz($25, 3);
	cnt1 += current->thread.pfc[1].perfcnt;
	len += seq_printf(m, "%llu\n\n", cnt1);

	return len;
}

static int perform_write(struct file *file, const char __user *buffer,size_t count, loff_t *data)
{
	int command = 0;
	int i, offset;
	int counter_sel = 0;
	char cmd_array[64] = {0};

//	printk("Input buffer cmd is: %s\n",buffer); //Debug echo cmd
	if ((count < 5) || (count > 64))
		return -EINVAL;
	offset = count < strlen(buffer) ? count : strlen(buffer);
	copy_from_user(cmd_array, buffer, offset);
	//printk("Read cmd is: %s\n", cmd_array); //Debug echo cmd
	for (i = 0; i < ARRAY_SIZE(performance_command); i++) {
		if (!strncmp(cmd_array, performance_command[i], strlen(performance_command[i]))) {
			command = i + 1;
			break;
		}
	}
	if (command == 0) {
		return -EINVAL;
	} else {
		i = command - 1;
		if (strlen(cmd_array) == strlen(performance_command[i]))
			counter_sel = 0;
		else if (strlen(cmd_array) > strlen(performance_command[i]) + 2) {
			i = strlen(performance_command[i]);
			if (cmd_array[i] != ' ' && cmd_array[i] != '\t')
				return -EINVAL;
			while (i + 2 < strlen(cmd_array)) {
				if (cmd_array[i + 1] == 'c' && (cmd_array[i + 2] == '0' || cmd_array[i + 2] == '1')) {
					counter_sel = cmd_array[i + 2] - '0';
					break;
				} else if (cmd_array[i + 1] == ' ' || cmd_array[i + 1] == '\t') {
					i += 1;
					continue;
				} else
					return -EINVAL;
			}
			if (i + 2 >= strlen(cmd_array))
				return -EINVAL;
			while (cmd_array[i + 3]) {
				if (cmd_array[i + 3] != ' ' && cmd_array[i + 3] != '\t')
					return -EINVAL;
				else
					i += 1;
			}
		} else
			return -EINVAL;
	}

	switch (command) {
		int t;
	case 1:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0xFFFFF810) | 0x08;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x08;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 2:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x28;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x28;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 3:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x48;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x48;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 4:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x68;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x68;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 5:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x88;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x88;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 6:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0xa8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0xa8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 7:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0xc8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0xc8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 8:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0xe8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0xe8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 9:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x108;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x108;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 10:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x128;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x128;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case 11:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x148;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x148;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case FSE_INDEX:
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x608;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x608;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 1):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x628;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x628;
			write_perf_cnter_jz($25, 3, 0);
			write_perf_cnter_jz($25, 2, t);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 2):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x648;
			write_perf_cnter_jz($25, 1, 0);
			write_perf_cnter_jz($25, 0, t);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x648;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 3):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x668;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x668;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 4):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x688;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x688;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 5):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x6a8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x6a8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 6):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x6c8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x6c8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 7):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x6e8;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x6e8;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 8):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x708;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x708;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 9):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x728;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x728;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 10):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x748;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x748;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 11):
		t = 0;
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = (t & 0XFFFFF810) | 0x768;
			write_perf_cnter_jz($25, 0, t);
			write_perf_cnter_jz($25, 1, 0);
			current->thread.pfc[0].perfcnt = 0;
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = (t & 0XFFFFF810) | 0x768;
			write_perf_cnter_jz($25, 2, t);
			write_perf_cnter_jz($25, 3, 0);
			current->thread.pfc[1].perfcnt = 0;
		}
		break;

	case (FSE_INDEX + 12):
		if (!counter_sel) {
			t = read_perf_cnter_jz($25, 0);
			t = t & 0xFFFF8000;
			write_perf_cnter_jz($25, 0, t);
		} else {
			t = read_perf_cnter_jz($25, 2);
			t = t & 0xFFFF8000;
			write_perf_cnter_jz($25, 2, t);
		}
		break;

	}
	return offset;
}

static int perform_open(struct inode *inode, struct file *file)
{
	return single_open(file, perform_show, PDE_DATA(inode));
}

static const struct file_operations perform_fops ={
	.read = seq_read,
	.open = perform_open,
	.write = perform_write,
	.llseek = seq_lseek,
	.release = single_release,
};

int __init init_performance_cnter(void)
{
	struct proc_dir_entry *p;

	p = jz_proc_mkdir("pmon");
	if (!p) {
		pr_warning("create_proc_entry for common clock failed.\n");
		return -ENODEV;
	}

	proc_create_data("perform", 0600,p,&perform_fops,0);

	return 0;
}


module_init(init_performance_cnter);
