/*
 * /proc/jz/ procfs for jz on-chip modules.
 *
 * Copyright (C) 2006 Ingenic Semiconductor Inc.
 * Author: <jlwei@ingenic.cn>
 *
 *  This program is free software; you can distribute it and/or modify it
 *  under the terms of the GNU General Public License (Version 2) as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/page-flags.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <jz_proc.h>
//#define DEBUG 1
#undef DEBUG

/* CP0 hazard avoidance. */
static void ipu_del_wired_entry( void )
{
	unsigned long flags;
	unsigned long wired;

	local_irq_save(flags);
	wired = read_c0_wired();
	if ( wired > 0 ) {
		write_c0_wired(wired - 1);
	}
	local_irq_restore(flags);
}

/***********************************************************************
 * IPU memory management (used by mplayer and other apps)
 *
 * We reserved 16MB memory for IPU
 * The memory base address is jz_ipu_framebuf
 */

/* Usage:
 *
 * echo n  > /proc/jz/imem 		// n = [0,...,10], allocate memory, 2^n pages
 * echo xxxxxxxx > /proc/jz/imem	// free buffer which addr is xxxxxxxx
 * echo FF > /proc/jz/ipu 		// FF, free all buffers
 * od -X /proc/jz/imem 			// return the allocated buffer address and the max order of free buffer
 */

//#define DEBUG_IMEM 1

/*
 * Allocated buffer list
 */
typedef struct imem_list {
	unsigned int phys_start;	/* physical start addr */
	unsigned int phys_end;		/* physical end addr */
	struct imem_list *next;
} imem_list_t;

#define IMEM_MAX_ORDER 13		/* max 2^12 * 4096 = 16MB */
static unsigned int jz_imem_base;	/* physical base address of ipu memory */
static unsigned int allocated_phys_addr = 0;
static struct imem_list *imem_list_head = NULL; /* up sorted by phys_start */

#define IMEM1_MAX_ORDER 12              /* max 2^11 * 4096 = 8MB */
static unsigned int jz_imem1_base;      /* physical base address of ipu memory */
static unsigned int allocated_phys_addr1 = 0;
static struct imem_list *imem1_list_head = NULL; /* up sorted by phys_start */


#ifdef DEBUG_IMEM
static void dump_imem_list(void)
{
	struct imem_list *imem;

	printk("*** dump_imem_list 0x%x ***\n", (u32)imem_list_head);
	imem = imem_list_head;
	while (imem) {
		printk("imem=0x%x phys_start=0x%x phys_end=0x%x next=0x%x\n", (u32)imem, imem->phys_start, imem->phys_end, (u32)imem->next);
		imem = imem->next;
	}

        printk("*** dump_imem_list 0x%x ***\n", (u32)imem1_list_head);
        imem = imem1_list_head;
        while (imem) {
                printk("imem=0x%x phys_start=0x%x phys_end=0x%x next=0x%x\n", (u32)imem, imem->phys_start, imem->phys_end, (u32)imem->next);
                imem = imem->next;
        }
}
#endif

/* allocate 2^order pages inside the 16MB memory */
static int imem_alloc(unsigned int order)
{
	int alloc_ok = 0;
	unsigned int start, end;
	unsigned int size = (1 << order) * PAGE_SIZE;
	struct imem_list *imem, *imemn, *imemp;

	allocated_phys_addr = 0;

	start = jz_imem_base;
	end = start + (1 << IMEM_MAX_ORDER) * PAGE_SIZE;

	imem = imem_list_head;
	while (imem) {
		if ((imem->phys_start - start) >= size) {
			/* we got a valid address range */
			alloc_ok = 1;
			break;
		}

		start = imem->phys_end + 1;
		imem = imem->next;
	}

	if (!alloc_ok) {
		if ((end - start) >= size)
			alloc_ok = 1;
	}

	if (alloc_ok) {
		end = start + size - 1;
		allocated_phys_addr = start;

		/* add to imem_list, up sorted by phys_start */
		imemn = kmalloc(sizeof(struct imem_list), GFP_KERNEL);
		if (!imemn) {
			printk("-->%s, kmalloc failed1\n", __FUNCTION__);
			return -ENOMEM;
		}
		imemn->phys_start = start;
		imemn->phys_end = end;
		imemn->next = NULL;

		if (!imem_list_head)
			imem_list_head = imemn;
		else {
			imem = imemp = imem_list_head;
			while (imem) {
				if (start < imem->phys_start) {
					break;
				}

				imemp = imem;
				imem = imem->next;
			}

			if (imem == imem_list_head) {
				imem_list_head = imemn;
				imemn->next = imem;
			}
			else {
				imemn->next = imemp->next;
				imemp->next = imemn;
			}
		}
	}

#ifdef DEBUG_IMEM
	dump_imem_list();
#endif
	return 0;
}

/* allocate 2^order pages inside the 8MB memory */
static int imem1_alloc(unsigned int order)
{
	int alloc_ok = 0;
	unsigned int start, end;
	unsigned int size = (1 << order) * PAGE_SIZE;
	struct imem_list *imem, *imemn, *imemp;

	allocated_phys_addr1 = 0;

	start = jz_imem1_base;
	end = start + (1 << IMEM1_MAX_ORDER) * PAGE_SIZE;

	imem = imem1_list_head;
	while (imem) {
		if ((imem->phys_start - start) >= size) {
			/* we got a valid address range */
			alloc_ok = 1;
			break;
		}

		start = imem->phys_end + 1;
		imem = imem->next;
	}

	if (!alloc_ok) {
		if ((end - start) >= size)
			alloc_ok = 1;
	}

	if (alloc_ok) {
		end = start + size - 1;
		allocated_phys_addr1 = start;

		/* add to imem_list, up sorted by phys_start */
		imemn = kmalloc(sizeof(struct imem_list), GFP_KERNEL);
		if (!imemn) {
			printk("-->%s, kmalloc failed2\n", __FUNCTION__);
			return -ENOMEM;
		}
		imemn->phys_start = start;
		imemn->phys_end = end;
		imemn->next = NULL;

		if (!imem1_list_head)
			imem1_list_head = imemn;
		else {
			imem = imemp = imem1_list_head;
			while (imem) {
				if (start < imem->phys_start) {
					break;
				}

				imemp = imem;
				imem = imem->next;
			}

			if (imem == imem1_list_head) {
				imem1_list_head = imemn;
				imemn->next = imem;
			}
			else {
				imemn->next = imemp->next;
				imemp->next = imemn;
			}
		}
	}

#ifdef DEBUG_IMEM
	dump_imem_list();
#endif
	return 0;
}

static void imem_free(unsigned int phys_addr)
{
	struct imem_list *imem, *imemp;

	imem = imemp = imem_list_head;
	while (imem) {
		if (phys_addr == imem->phys_start) {
			if (imem == imem_list_head) {
				imem_list_head = imem->next;
			}
			else {
				imemp->next = imem->next;
			}

			kfree(imem);
			break;
		}

		imemp = imem;
		imem = imem->next;
	}

        imem = imemp = imem1_list_head;
        while (imem) {
                if (phys_addr == imem->phys_start) {
                        if (imem == imem1_list_head) {
                                imem1_list_head = imem->next;
                        }
                        else {
                                imemp->next = imem->next;
                        }

                        kfree(imem);
                        break;
                }

                imemp = imem;
                imem = imem->next;
        }

#ifdef DEBUG_IMEM
	dump_imem_list();
#endif
}

static void imem_free_all(void)
{
	struct imem_list *imem, *tmp;

	imem = imem_list_head;
	while (imem) {
		tmp = imem;
		imem = imem->next;
		kfree(tmp);
	}

	imem_list_head = NULL;

	allocated_phys_addr = 0;

        imem = imem1_list_head;
        while (imem) {
		tmp = imem;
                imem = imem->next;
		kfree(tmp);
        }

        imem1_list_head = NULL;

        allocated_phys_addr1 = 0;

#ifdef DEBUG_IMEM
	dump_imem_list();
#endif
}

/*
 * Return the allocated buffer address and the max order of free buffer
 */
static int imem_read_proc(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	unsigned int start_addr, end_addr, max_order, max_size;
	struct imem_list *imem;

	unsigned int *tmp = (unsigned int *)(page + len);

	start_addr = jz_imem_base;
	end_addr = start_addr + (1 << IMEM_MAX_ORDER) * PAGE_SIZE;

	if (!imem_list_head)
		max_size = end_addr - start_addr;
	else {
		max_size = 0;
		imem = imem_list_head;
		while (imem) {
			if (max_size < (imem->phys_start - start_addr))
				max_size = imem->phys_start - start_addr;

			start_addr = imem->phys_end + 1;
			imem = imem->next;
		}

		if (max_size < (end_addr - start_addr))
			max_size = end_addr - start_addr;
	}

	if (max_size > 0) {
		max_order = get_order(max_size);
		if (((1 << max_order) * PAGE_SIZE) > max_size)
		    max_order--;
	}
	else {
		max_order = 0xffffffff;	/* No any free buffer */
	}

	*tmp++ = allocated_phys_addr;	/* address allocated by 'echo n > /proc/jz/imem' */
	*tmp = max_order;		/* max order of current free buffers */

	len += 2 * sizeof(unsigned int);

	return len;
}

static int imem_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned int val;

	val = simple_strtoul(buffer, 0, 16);

	if (val == 0xff) {
		/* free all memory */
		imem_free_all();
	}
	else if ((val >= 0) && (val <= IMEM_MAX_ORDER)) {
		/* allocate 2^val pages */
		imem_alloc(val);
	}
	else {
		/* free buffer which phys_addr is val */
		imem_free(val);
	}

	return count;
}

/*
 * Return the allocated buffer address and the max order of free buffer
 */
static int imem1_read_proc(char *page, char **start, off_t off,
			  int count, int *eof, void *data)
{
	int len = 0;
	unsigned int start_addr, end_addr, max_order, max_size;
	struct imem_list *imem;

	unsigned int *tmp = (unsigned int *)(page + len);

	start_addr = jz_imem1_base;
	end_addr = start_addr + (1 << IMEM1_MAX_ORDER) * PAGE_SIZE;

	if (!imem1_list_head)
		max_size = end_addr - start_addr;
	else {
		max_size = 0;
		imem = imem1_list_head;
		while (imem) {
			if (max_size < (imem->phys_start - start_addr))
				max_size = imem->phys_start - start_addr;

			start_addr = imem->phys_end + 1;
			imem = imem->next;
		}

		if (max_size < (end_addr - start_addr))
			max_size = end_addr - start_addr;
	}

	if (max_size > 0) {
		max_order = get_order(max_size);
		if (((1 << max_order) * PAGE_SIZE) > max_size)
		    max_order--;
	}
	else {
		max_order = 0xffffffff;	/* No any free buffer */
	}

	*tmp++ = allocated_phys_addr1;	/* address allocated by 'echo n > /proc/jz/imem' */
	*tmp = max_order;		/* max order of current free buffers */

	len += 2 * sizeof(unsigned int);

	return len;
}

static int imem1_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned int val;

	val = simple_strtoul(buffer, 0, 16);

	if (val == 0xff) {
		/* free all memory */
		imem_free_all();
		ipu_del_wired_entry();
	} else if ((val >= 0) && (val <= IMEM1_MAX_ORDER)) {
		/* allocate 2^val pages */
		imem1_alloc(val);
	} else {
		/* free buffer which phys_addr is val */
		imem_free(val);
	}

	return count;
}

/*
 * /proc/jz/xxx entry
 *
 */
static int __init jz_proc_init(void)
{
	struct proc_dir_entry *res;
	struct proc_dir_entry *jz_proc;
	unsigned int virt_addr, i;
#ifndef CONFIG_USE_JZ_ROOT_DIR
	jz_proc = jz_proc_mkdir("mem");
#else
	jz_proc = get_jz_proc_root();
#endif
	/*
	 * Reserve a 16MB memory for IPU on JZ.
	 */
	jz_imem_base = (unsigned int)__get_free_pages(GFP_KERNEL, IMEM_MAX_ORDER);
	if (jz_imem_base) {
		/* imem (IPU memory management) */
		res = create_proc_entry("imem", 0644, jz_proc);
		if (res) {
			res->read_proc = imem_read_proc;
			res->write_proc = imem_write_proc;
			res->data = NULL;
		}

		/* Set page reserved */
		virt_addr = jz_imem_base;
		for (i = 0; i < (1 << IMEM_MAX_ORDER); i++) {
			SetPageReserved(virt_to_page((void *)virt_addr));
			virt_addr += PAGE_SIZE;
		}

		/* Convert to physical address */
		jz_imem_base = virt_to_phys((void *)jz_imem_base);

		printk("Total %dMB memory at 0x%x was reserved for IPU\n",
		       (unsigned int)((1 << IMEM_MAX_ORDER) * PAGE_SIZE)/1000000, jz_imem_base);
	}
        else
           printk("NOT enough memory for imem\n");

        jz_imem1_base = (unsigned int)__get_free_pages(GFP_KERNEL, IMEM1_MAX_ORDER);
        if (jz_imem1_base) {
                /* imem (IPU memory management) */
                res = create_proc_entry("imem1", 0644, jz_proc);
                if (res) {
                        res->read_proc = imem1_read_proc;
                        res->write_proc = imem1_write_proc;
                        res->data = NULL;
                }

                /* Set page reserved */
                virt_addr = jz_imem1_base;
                for (i = 0; i < (1 << IMEM1_MAX_ORDER); i++) {
                        SetPageReserved(virt_to_page((void *)virt_addr));
                        virt_addr += PAGE_SIZE;
                }

                /* Convert to physical address */
                jz_imem1_base = virt_to_phys((void *)jz_imem1_base);

                printk("Total %dMB memory1 at 0x%x was reserved for IPU\n",
                       (unsigned int)((1 << IMEM1_MAX_ORDER) * PAGE_SIZE)/1000000, jz_imem1_base);
        }
        else
           printk("NOT enough memory for imem1\n");

	return 0;
}

__initcall(jz_proc_init);
