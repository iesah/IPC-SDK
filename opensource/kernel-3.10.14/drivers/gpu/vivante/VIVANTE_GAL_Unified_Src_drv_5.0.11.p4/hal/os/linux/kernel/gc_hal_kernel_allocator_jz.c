/****************************************************************************
*
*    Copyright (C) 2005 - 2014 by Vivante Corp.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the license, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*****************************************************************************/


#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel_allocator.h"
#include <linux/pagemap.h>
#include <linux/seq_file.h>
#include <linux/mman.h>
#include <asm/atomic.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>

#include "gc_hal_kernel_platform.h"

#define _GC_OBJ_ZONE    gcvZONE_OS
#define GPU_RESERVE_SIZE   (8 * 1024 * 1024)

#define M_ALLOC(size) kmalloc((size),GFP_KERNEL)
#define M_FREE(p)     kfree(p)
struct _mm_cell
{
	struct list_head head;
	unsigned long start;
};
struct _mmclass
{
	struct list_head free;
	int size;
	int order;
};
struct _continue_mm
{
	struct _mmclass *array;
	int used_size;
	int free_size;
	void* start;
	unsigned int size;
	struct mutex mutex;
};
static long mmclass_alloc(struct _mmclass *mm,int size)
{
	struct _mm_cell *mc,*next_mc;
	struct _mmclass *next_mm;
	unsigned int start;
	unsigned int msize;
	if(list_empty(&mm->free))
	{
		if(mm->size == -1)
			return -1;
		else {
			long addr;
			addr = mmclass_alloc((mm + 1),size);
			if(addr == -1)
				return -1;
			mc = (struct _mm_cell *)addr;
		}
	}else{
		mc = list_entry(mm->free.next,struct _mm_cell,head);
		list_del(&mc->head);
	}
	start = mc->start;
	msize = mm->size > 0 ? mm->size : PAGE_SIZE << mm->order;

	if(msize / 2 < size || mm->order == 0)
	{
		M_FREE(mc);
		return start;
	}
	if(msize / 2 >= size)
	{
		next_mm = (mm - 1);
		next_mc = M_ALLOC(sizeof(struct _mm_cell));
		next_mc->start = start + msize / 2; //split
		list_add_tail(&next_mc->head,&next_mm->free);
		return (unsigned long)mc;
	}
	return -1;

}
static void mmclass_free(struct _mmclass *mm,struct _mm_cell *mc,int size)
{
	struct _mm_cell *next,*merge;
	unsigned long mask;
	if(mm->size == -1)
	{
		list_add_tail(&mc->head,&mm->free);
		return;
	}
	/* next = list_entry(mm->free.next,(struct _mm_cell),head); */
	/* list_del(&next->head); */
	mask = ~((2 << (mm->order + PAGE_SHIFT)) - 1);
	//printk("order = %d mask = %lx\n",mm->order,mask);
	merge = NULL;
	list_for_each_entry(next,&mm->free,head)
	{
		if((next->start & mask) == (mc->start & mask))
		{
			merge = next;
			break;
		}
	}
	if(merge == NULL) {
		list_add_tail(&mc->head,&mm->free);
	}else
	{
		merge->start = mc->start & mask;
		M_FREE(mc);
		list_del(&merge->head);
		mmclass_free(mm + 1,merge,mm->size * 2);
	}
}
void continue_mm_free(void *handle,void *p,int size)
{
	struct _continue_mm *mm = handle;
	int order = get_order(size);
	struct _mm_cell *mc;
	mc = M_ALLOC(sizeof(struct _mm_cell));
	mc->start = (unsigned long)(p - mm->start);
	mutex_lock(&mm->mutex);
	mm->free_size += PAGE_SIZE << order;
	mm->used_size -= PAGE_SIZE << order;
	//printk("free = %lx %d size = %lx\n",mc->start,order,PAGE_SIZE << order);
	mmclass_free(&mm->array[order],mc,size);
	mutex_unlock(&mm->mutex);
}
void dump_continue_mm(void *handle)
{
	struct _continue_mm *mm = handle;
	int i;
	struct _mm_cell *mc;
	mutex_lock(&mm->mutex);
	for(i = 0;i < get_order(mm->size) + 1;i++)
	{
		printk("%02d (%08d):",i,mm->array[i].size);
		list_for_each_entry(mc,&mm->array[i].free,head)
		{
			printk("\t .");
		}
		printk("\n");
	}
	mutex_unlock(&mm->mutex);
}
void *continue_mm_alloc(void *handle,int size)
{
	struct _continue_mm *mm = handle;
	int order = get_order(size);
	long start;
	mutex_lock(&mm->mutex);
	mm->free_size -= PAGE_SIZE << order;
	mm->used_size += PAGE_SIZE << order;
	start = mmclass_alloc(&mm->array[order],size);
	//printk("alloc = %lx %d size = %lx %p\n",start,order,PAGE_SIZE << order,mm->start + start);
	mutex_unlock(&mm->mutex);
	if(start < 0){
		//printk("return NULL\n");
		return NULL;
	}
	//dma_cache_sync(NULL,mm->start + start, size,DMA_TO_DEVICE);
	return (mm->start + start);
}

void *init_continue_mm(void *p,int size)
{
	struct _continue_mm *mm;
	struct _mm_cell *mc;
	int count;
	int i;
	mm = M_ALLOC(sizeof(struct _continue_mm));
	count = get_order(size);
	mm->array = (struct _mmclass *)M_ALLOC((count + 1)* sizeof(struct _mmclass));
	for(i = 0; i < count;i++){
		INIT_LIST_HEAD(&mm->array[i].free);
		mm->array[i].size = PAGE_SIZE << i;
		mm->array[i].order = i;
	}
	mm->free_size = size;
	mm->used_size = 0;
	INIT_LIST_HEAD(&mm->array[i].free);

	mc = M_ALLOC(sizeof(struct _mm_cell));
	mc->start = 0;
	list_add_tail(&mc->head,&mm->array[i].free);
	mm->array[i].size = -1;
	mm->array[i].order = i;
	mm->start = p;
	mm->size = size;
	mutex_init(&mm->mutex);
	return (void*)mm;
}
/**
 * record alloc page address, and manager it;
 */
struct alloc_addr{
	struct list_head head;
	unsigned int start;
	unsigned int size;
	unsigned int bitmap;
	unsigned int bitmask;
};
typedef struct _WriteAccel_Priv* WriteAccel_Priv;
typedef struct _WriteAccel_Priv {
	struct list_head alloc_addr_top;
	struct list_head map_addr_top;
	int max_reserve_size;
	int current_size;
	void *continue_mem;

}g_WriteAccel_Priv;

/******************************************************************************\
************************** Default Allocator Debugfs ***************************
\******************************************************************************/

static int gc_usage_show(struct seq_file* m, void* data)
{
    gcsINFO_NODE *node = m->private;
    gckALLOCATOR Allocator = node->device;
    WriteAccel_Priv priv = Allocator->privateData;
    priv = priv;
    seq_printf(m,"max_reserve_size:%d\n",priv->max_reserve_size);
    seq_printf(m,"current_size:%d\n",priv->current_size);
    return 0;
}

static gcsINFO InfoList[] =
{
    {"Usage", gc_usage_show},
};

static void
_WriteAccel_AllocatorDebugfsInit(
    IN gckALLOCATOR Allocator,
    IN gckDEBUGFS_DIR Root
    )
{
    gcmkVERIFY_OK(
        gckDEBUGFS_DIR_Init(&Allocator->debugfsDir, Root->root, "writeaccelate"));

    gcmkVERIFY_OK(gckDEBUGFS_DIR_CreateFiles(
        &Allocator->debugfsDir,
        InfoList,
        gcmCOUNTOF(InfoList),
        Allocator
        ));
}

static void
_WriteAccel_AllocatorDebugfsCleanup(
    IN gckALLOCATOR Allocator
    )
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(
        &Allocator->debugfsDir,
        InfoList,
        gcmCOUNTOF(InfoList)
        ));

    gckDEBUGFS_DIR_Deinit(&Allocator->debugfsDir);
}


static void _NonContiguousFree(struct page ** Pages,int startpos,unsigned int NumPages)
{
    gctINT i;

    gcmkHEADER_ARG("Pages=0x%X, NumPages=%d", Pages, NumPages);

    gcmkASSERT(Pages != gcvNULL);

    for (i = startpos; i < NumPages; i++)
    {
        __free_page(Pages[i]);
    }

    if (is_vmalloc_addr(Pages))
    {
        vfree(Pages);
    }
    else
    {
        kfree(Pages);
    }

    gcmkFOOTER_NO();
}
static int add_alloc_list(WriteAccel_Priv p,unsigned int start,unsigned int end)
{
	struct alloc_addr *alloc;
	unsigned int i = 0;
	alloc = (struct alloc_addr *)vmalloc(sizeof(struct alloc_addr));
	if(!alloc)
		return -1;
	alloc->bitmap = 0;
	for(i = 0;i < (end - start) / PAGE_SIZE; i++)
		alloc->bitmap |= 1 << i;
	alloc->start = start;
	alloc->size = end - start;
	alloc->bitmask = alloc->bitmap;
	p->max_reserve_size += (end - start);
	list_add_tail(&alloc->head,&p->alloc_addr_top);
	return 0;
}
static struct alloc_addr *find_free_list(struct list_head *p)
{
	struct alloc_addr *alloc;
	struct alloc_addr *alloc_next;
	list_for_each_entry_safe(alloc,alloc_next,p,head){
		if(alloc->bitmap ^ alloc->bitmask){
			return alloc;
		}
	}
	return NULL;
}
static unsigned int find_free_bitmap(struct alloc_addr *alloc,int *start)
{
	int n;
	for(n = *start;n < 32;n++)
	{
		if((alloc->bitmask & (1 << n)) == 0)
			break;
		if((alloc->bitmap & (1 << n)) == 0){
			*start = n + 1;
			alloc->bitmap |= 1 << n;
			return alloc->start + n * PAGE_SIZE;
		}
	}
	*start = 0;
	return -1;
}
#define phys_to_page(phys) pfn_to_page((phys) >> PAGE_SHIFT)
static int alloc_reserve_mem(WriteAccel_Priv p,unsigned NumPages,struct page ** pages)
{
    unsigned int phys_addr;
    int i = 0,pos;
    struct alloc_addr *alloc;
    alloc = find_free_list(&p->alloc_addr_top);
    if(alloc)
    {
	    pos = 0;
	    for(i = 0;i < NumPages;i++)
	    {
		    do {
			    phys_addr = find_free_bitmap(alloc,&pos);
			    if(phys_addr == -1)
			    {
				    alloc = find_free_list(&alloc->head);
				    if(!alloc)
					    break;
			    }
		    }while(phys_addr == -1);
		    if(phys_addr != -1)
			    pages[i] = phys_to_page(phys_addr);
		    else
			    break;
	    }
    }
    return i;
}
static struct page **_NonContiguousAlloc(gckALLOCATOR Allocator,unsigned int NumPages)
{
    struct page ** pages;
    struct page *page;
    int i, size,j;
    unsigned int start_addr = -1;
    unsigned int end_addr = -1;
    WriteAccel_Priv p = Allocator->privateData;
    gcmkHEADER_ARG("NumPages=%lu", NumPages);
    if(p->max_reserve_size > GPU_RESERVE_SIZE)
    {
	    return gcvNULL;
    }
    if (NumPages > num_physpages)
    {
        gcmkFOOTER_NO();
        return gcvNULL;
    }
    size = NumPages * sizeof(struct page *);

    pages = kmalloc(size, GFP_KERNEL | gcdNOWARN);
    if (!pages)
    {
	    pages = vmalloc(size);
        if (!pages)
        {
            gcmkFOOTER_NO();
            return gcvNULL;
        }
    }

    j = alloc_reserve_mem(p,NumPages,pages);
    for (i = j; i < NumPages; i++)
    {
	    page = alloc_page(GFP_KERNEL | gcdNOWARN);
	    if (!page)
	    {
		    _NonContiguousFree(pages, j,i);
		    gcmkFOOTER_NO();
		    return gcvNULL;
	    }
	    SetPageReserved(page);
	    pages[i] = page;
	    if(start_addr == -1) {
		    start_addr = (unsigned int)page_to_phys(page);
		    end_addr = start_addr + PAGE_SIZE;
	    }
	    else if((unsigned int)page_to_phys(page) == end_addr)
	    {
		    end_addr = (unsigned int)page_to_phys(page) + PAGE_SIZE;
		    if(end_addr - start_addr >= 32 * PAGE_SIZE)
		    {
			    gcmkVERIFY_OK(gckOS_CacheFlush(Allocator->os, _GetProcessID(), gcvNULL,
							   start_addr,
							   phys_to_virt(start_addr),
							   end_addr - start_addr));
			    if(add_alloc_list(p,start_addr,end_addr)) {
				    _NonContiguousFree(pages, j, i);
				    gcmkFOOTER_NO();
				    return gcvNULL;
			    }
			    start_addr = -1;
		    }
	    }else{
		    gcmkVERIFY_OK(gckOS_CacheFlush(Allocator->os, _GetProcessID(), gcvNULL,
						   start_addr,
						   phys_to_virt(start_addr),
						   end_addr - start_addr));
		    if(add_alloc_list(p,start_addr,end_addr)) {
			    _NonContiguousFree(pages, j, i);
			    gcmkFOOTER_NO();
			    return gcvNULL;
		    }
		    start_addr = (unsigned int)page_to_phys(page);
		    end_addr = start_addr + PAGE_SIZE;
	    }
    }
    if(start_addr != -1)
    {
	    end_addr = (unsigned int)page_to_phys(page) + PAGE_SIZE;
	    gcmkVERIFY_OK(gckOS_CacheFlush(Allocator->os, _GetProcessID(), gcvNULL,
					   start_addr,
					   phys_to_virt(start_addr),
					   end_addr - start_addr));
	    if(add_alloc_list(p,start_addr,end_addr)) {
		    _NonContiguousFree(pages, j, i);
		    gcmkFOOTER_NO();
		    return gcvNULL;
	    }
    }
    p->current_size += NumPages * PAGE_SIZE;

    gcmkFOOTER_ARG("pages=0x%X", pages);
    return pages;
}

static gctSTRING
_KCreateKernelVirtualMapping(
    IN PLINUX_MDL Mdl
    )
{
    gctSTRING addr = 0;
    gctINT numPages = Mdl->numPages;

#if gcdNONPAGED_MEMORY_CACHEABLE
    if (Mdl->contiguous)
    {
        addr = page_address(Mdl->u.contiguousPages);
    }
    else
    {
        addr = vmap(Mdl->u.nonContiguousPages,
                    numPages,
                    0,
                    PAGE_KERNEL);

        /* Trigger a page fault. */
        memset(addr, 0, numPages * PAGE_SIZE);
    }
#else
    struct page ** pages;
    gctBOOL free = gcvFALSE;
    gctINT i;

    if (Mdl->contiguous)
    {
        pages = kmalloc(sizeof(struct page *) * numPages, GFP_KERNEL | gcdNOWARN);

        if (!pages)
        {
            return gcvNULL;
        }

        for (i = 0; i < numPages; i++)
        {
            pages[i] = nth_page(Mdl->u.contiguousPages, i);
        }

        free = gcvTRUE;
    }
    else
    {
        pages = Mdl->u.nonContiguousPages;
    }

    /* ioremap() can't work on system memory since 2.6.38. */
//    printk("\nmap: %lx\n",gcmkNONPAGED_MEMROY_PROT(PAGE_KERNEL));
    addr = vmap(pages, numPages, 0, gcmkNONPAGED_MEMROY_PROT(PAGE_KERNEL));

    if (free)
    {
        kfree(pages);
    }

#endif
    return addr;
}

static void
_KDestoryKernelVirtualMapping(
    IN gctSTRING Addr
    )
{
#if !gcdNONPAGED_MEMORY_CACHEABLE
    vunmap(Addr);
#endif
}

static void
_KUnmapUserLogical(
    IN gctPOINTER Logical,
    IN gctUINT32  Size
)
{
    if (unlikely(current->mm == gcvNULL))
    {
        /* Do nothing if process is exiting. */
        return;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,5,0)
    if (vm_munmap((unsigned long)Logical, Size) < 0)
    {
        gcmkTRACE_ZONE(
                gcvLEVEL_WARNING, gcvZONE_OS,
                "%s(%d): vm_munmap failed",
                __FUNCTION__, __LINE__
                );
    }
#else
    down_write(&current->mm->mmap_sem);
    if (do_munmap(current->mm, (unsigned long)Logical, Size) < 0)
    {
        gcmkTRACE_ZONE(
                gcvLEVEL_WARNING, gcvZONE_OS,
                "%s(%d): do_munmap failed",
                __FUNCTION__, __LINE__
                );
    }
    up_write(&current->mm->mmap_sem);
#endif
}

/***************************************************************************\
************************ Default Allocator **********************************
\***************************************************************************/
#define C_MAX_PAGENUM  (75*1024)
static gceSTATUS
_WriteAccelate_Alloc(gckALLOCATOR Allocator,PLINUX_MDL Mdl,gctSIZE_T NumPages,gctUINT32 Flags)
{
    gceSTATUS status;
//    gctUINT32 order;
    gctSIZE_T bytes;
    gctUINT32 numPages;
    gctBOOL contiguous = Flags & gcvALLOC_FLAG_CONTIGUOUS;

    //struct sysinfo temsysinfo;
    gcmkHEADER_ARG("Mdl=%p NumPages=%d", Mdl, NumPages);

    numPages = NumPages;
    bytes = NumPages * PAGE_SIZE;
    if (contiguous)
    {
	    WriteAccel_Priv p = Allocator->privateData;
	    void *addr;
	    addr = continue_mm_alloc(p->continue_mem,bytes);
	    //dump_continue_mm(p->continue_mem);
	    Mdl->u.contiguousPages = addr ? virt_to_page(addr) : gcvNULL;
	    Mdl->exact = gcvFALSE;
    }
    else
    {
	    Mdl->u.nonContiguousPages = NULL;
	    if(0)
		    _NonContiguousAlloc(Allocator,numPages);
    }

    if (Mdl->u.contiguousPages == gcvNULL && Mdl->u.nonContiguousPages == gcvNULL)
    {
	    gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}
static struct alloc_addr *find_match_alloc_addr(struct list_head* top,unsigned int startaddr)
{
	struct alloc_addr *alloc = NULL;
	struct alloc_addr *alloc_next = NULL;
	list_for_each_entry_safe(alloc,alloc_next,top,head)
	{
		if((startaddr >= alloc->start) && (startaddr < alloc->start + alloc->size))
			return alloc;
	}
	return NULL;
}
static int alloc_list_mark_free(struct alloc_addr *alloc,unsigned int addr)
{
	int start = alloc->start;
	int bit;
	if((addr < start) || addr >= (start + alloc->size))
		return -1;
	bit = (addr - start) / PAGE_SIZE;
	alloc->bitmap &= ~ (1 << bit);
	return 0;
}
static void _WriteAccelate_Free(gckALLOCATOR Allocator,PLINUX_MDL Mdl)
{
    gctINT i = 0;
    struct page * page;
    unsigned int phys_addr;
    struct alloc_addr *alloc = NULL;
    WriteAccel_Priv priv = (WriteAccel_Priv)Allocator->privateData;
    int mark;
    if(Mdl->contiguous)
    {
	    continue_mm_free(priv->continue_mem,page_address(Mdl->u.contiguousPages),Mdl->numPages * PAGE_SIZE);
    }else {
	    while(i < Mdl->numPages){
		    page = _NonContiguousToPage(Mdl->u.nonContiguousPages, i);
		    phys_addr = page_to_phys(page);
		    do{
			    if(alloc == NULL){
				    alloc = find_match_alloc_addr(&priv->alloc_addr_top,phys_addr);
				    if(alloc == NULL){
					    BUG();
					    break;
				    }
			    }
			    mark = alloc_list_mark_free(alloc,phys_addr);
			    if(mark == -1)
				    alloc = NULL;

		    }while(mark == -1);
		    i++;
	    }
	    if (is_vmalloc_addr(Mdl->u.nonContiguousPages))
	    {
		    vfree(Mdl->u.nonContiguousPages);
	    }
	    else
	    {
		    kfree(Mdl->u.nonContiguousPages);
	    }
	    priv->current_size -=  Mdl->numPages * PAGE_SIZE;
    }
}

static int _WriteAccelate_MapUser(gckALLOCATOR Allocator,PLINUX_MDL Mdl,PLINUX_MDL_MAP MdlMap,gctBOOL Cacheable)
{

    gctSTRING       addr;
    unsigned long   start;
    unsigned long   pfn;
    gctINT i;
    gckOS           os = Allocator->os;
    gcsPLATFORM *   platform = os->device->platform;

    PLINUX_MDL      mdl = Mdl;
    PLINUX_MDL_MAP  mdlMap = MdlMap;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p MdlMap=%p gctBOOL=%d", Allocator, Mdl, MdlMap, Cacheable);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
    mdlMap->vmaAddr = (gctSTRING)vm_mmap(gcvNULL,
                    0L,
                    mdl->numPages * PAGE_SIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    0);
#else
    down_write(&current->mm->mmap_sem);

    mdlMap->vmaAddr = (gctSTRING)do_mmap_pgoff(gcvNULL,
                    0L,
                    mdl->numPages * PAGE_SIZE,
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED,
                    0);

    up_write(&current->mm->mmap_sem);
#endif

    gcmkTRACE_ZONE(
        gcvLEVEL_INFO, gcvZONE_OS,
        "%s(%d): vmaAddr->0x%X for phys_addr->0x%X",
        __FUNCTION__, __LINE__,
        (gctUINT32)(gctUINTPTR_T)mdlMap->vmaAddr,
        (gctUINT32)(gctUINTPTR_T)mdl
        );

    if (IS_ERR(mdlMap->vmaAddr))
    {
        gcmkTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_OS,
            "%s(%d): do_mmap_pgoff error",
            __FUNCTION__, __LINE__
            );

        mdlMap->vmaAddr = gcvNULL;

        gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_MEMORY);
        return gcvSTATUS_OUT_OF_MEMORY;
    }

    down_write(&current->mm->mmap_sem);

    mdlMap->vma = find_vma(current->mm, (unsigned long)mdlMap->vmaAddr);

    if (mdlMap->vma == gcvNULL)
    {
        up_write(&current->mm->mmap_sem);

        gcmkTRACE_ZONE(
            gcvLEVEL_INFO, gcvZONE_OS,
            "%s(%d): find_vma error",
            __FUNCTION__, __LINE__
            );

        mdlMap->vmaAddr = gcvNULL;

        gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_RESOURCES);
        return gcvSTATUS_OUT_OF_RESOURCES;
    }

    mdlMap->vma->vm_flags |= gcdVM_FLAGS;

    if (Cacheable == gcvFALSE)
    {
        /* Make this mapping non-cached. */
        mdlMap->vma->vm_page_prot = gcmkPAGED_MEMROY_PROT(mdlMap->vma->vm_page_prot);
    }

    if (platform && platform->ops->adjustProt)
    {
        platform->ops->adjustProt(mdlMap->vma);
    }

    addr = mdl->addr;

    /* Now map all the vmalloc pages to this user address. */
    if (mdl->contiguous)
    {
/* map kernel memory to user space.. */
	    if (remap_pfn_range(mdlMap->vma,
				mdlMap->vma->vm_start,
				page_to_pfn(mdl->u.contiguousPages),
				mdlMap->vma->vm_end - mdlMap->vma->vm_start,
				mdlMap->vma->vm_page_prot) < 0)
	    {
		    up_write(&current->mm->mmap_sem);

		    gcmkTRACE_ZONE(
			    gcvLEVEL_INFO, gcvZONE_OS,
			    "%s(%d): unable to mmap ret",
			    __FUNCTION__, __LINE__
			    );

		    mdlMap->vmaAddr = gcvNULL;

		    gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_MEMORY);
		    return gcvSTATUS_OUT_OF_MEMORY;
	    }
    }
    else
    {
        start = mdlMap->vma->vm_start;

        for (i = 0; i < mdl->numPages; i++)
        {
            pfn = _NonContiguousToPfn(mdl->u.nonContiguousPages, i);
            if (remap_pfn_range(mdlMap->vma,start,
                                pfn,
                                PAGE_SIZE,
                                mdlMap->vma->vm_page_prot) < 0)
            {
                up_write(&current->mm->mmap_sem);

                mdlMap->vmaAddr = gcvNULL;

                gcmkFOOTER_ARG("*status=%d", gcvSTATUS_OUT_OF_MEMORY);
                return gcvSTATUS_OUT_OF_MEMORY;
            }

            start += PAGE_SIZE;
            addr += PAGE_SIZE;
        }
    }

    up_write(&current->mm->mmap_sem);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

void _WriteAccelate_UnmapUser(
    IN gckALLOCATOR Allocator,
    IN gctPOINTER Logical,
    IN gctUINT32 Size
    )
{
    _KUnmapUserLogical(Logical, Size);
}

static gceSTATUS _WriteAccelate_MapKernel(
    IN gckALLOCATOR Allocator,
    IN PLINUX_MDL Mdl,
    OUT gctPOINTER *Logical
    )
{

    *Logical = _KCreateKernelVirtualMapping(Mdl);
    return gcvSTATUS_OK;
}

static gceSTATUS
_WriteAccelate_UnmapKernel(
    IN gckALLOCATOR Allocator,
    IN PLINUX_MDL Mdl,
    IN gctPOINTER Logical
    )
{
    _KDestoryKernelVirtualMapping(Logical);
    return gcvSTATUS_OK;
}

gceSTATUS
_WriteAccelate_LogicalToPhysical(
    IN gckALLOCATOR Allocator,
    IN PLINUX_MDL Mdl,
    IN gctPOINTER Logical,
    IN gctUINT32 ProcessID,
    OUT gctUINT32_PTR Physical
    )
{
    return _ConvertLogical2Physical(
                Allocator->os, Logical, ProcessID, Mdl, Physical);
}

gceSTATUS
_WriteAccelate_Cache(
    IN gckALLOCATOR Allocator,
    IN PLINUX_MDL Mdl,
    IN gctPOINTER Logical,
    IN gctUINT32 Physical,
    IN gctUINT32 Bytes,
    IN gceCACHEOPERATION Operation
    )
{
    return gcvSTATUS_OK;
}

static gceSTATUS _WriteAccelate_Physical(
    IN gckALLOCATOR Allocator,
    IN PLINUX_MDL Mdl,
    IN gctUINT32 Offset,
    OUT gctUINT32_PTR Physical
    )
{
    gcmkASSERT(Mdl->pagedMem && !Mdl->contiguous);
    *Physical = _NonContiguousToPhys(Mdl->u.nonContiguousPages, Offset);

    return gcvSTATUS_OK;
}

void _WriteAccelate_AllocatorDestructor(
    IN void* PrivateData
    )
{
    kfree(PrivateData);
}

/* Default allocator operations. */
gcsALLOCATOR_OPERATIONS WriteAccelate_AllocatorOperations = {
    .Alloc              = _WriteAccelate_Alloc,
    .Free               = _WriteAccelate_Free,
    .MapUser            = _WriteAccelate_MapUser,
    .UnmapUser          = _WriteAccelate_UnmapUser,
    .MapKernel          = _WriteAccelate_MapKernel,
    .UnmapKernel        = _WriteAccelate_UnmapKernel,
    .LogicalToPhysical  = _WriteAccelate_LogicalToPhysical,
    .Cache              = _WriteAccelate_Cache,
    .Physical           = _WriteAccelate_Physical,
};

/* Default allocator entry. */
gceSTATUS _WriteAccelate_AlloctorInit(
    IN gckOS Os,
    OUT gckALLOCATOR * Allocator
    )
{
    gceSTATUS status;
    gckALLOCATOR allocator;
    WriteAccel_Priv priv = gcvNULL;

    gcmkONERROR(
	    gckALLOCATOR_Construct(Os, &WriteAccelate_AllocatorOperations, &allocator));

    priv = kzalloc(gcmSIZEOF(struct _WriteAccel_Priv), GFP_KERNEL | gcdNOWARN);
    // 255-32 = 223m size:32M
    priv->continue_mem = init_continue_mm((void*)0x80000000 + 1024 * 1024 * 191,64*1024*1024);

    INIT_LIST_HEAD(&priv->alloc_addr_top);
    INIT_LIST_HEAD(&priv->map_addr_top);
    if (!priv)
    {
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    /* Register private data. */
    allocator->privateData = priv;
    allocator->privateDataDestructor = _WriteAccelate_AllocatorDestructor;

    allocator->debugfsInit = _WriteAccel_AllocatorDebugfsInit;
    allocator->debugfsCleanup = _WriteAccel_AllocatorDebugfsCleanup;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    return status;
}

#if 0
/***************************************************************************\
************************ Allocator helper ***********************************
\***************************************************************************/

gceSTATUS
gckALLOCATOR_Construct(
    IN gckOS Os,
    IN gcsALLOCATOR_OPERATIONS * Operations,
    OUT gckALLOCATOR * Allocator
    )
{
    gceSTATUS status;
    gckALLOCATOR allocator;

    gcmkHEADER_ARG("Os=%p, Operations=%p, Allocator=%p",
                   Os, Operations, Allocator);

    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Allocator != gcvNULL);
    gcmkVERIFY_ARGUMENT
        (  Operations
        && Operations->Alloc
        && Operations->Free
        && Operations->MapUser
        && Operations->UnmapUser
        && Operations->MapKernel
        && Operations->UnmapKernel
        && Operations->LogicalToPhysical
        && Operations->Cache
        && Operations->Physical
        );

    gcmkONERROR(
        gckOS_Allocate(Os, gcmSIZEOF(gcsALLOCATOR), (gctPOINTER *)&allocator));

    gckOS_ZeroMemory(allocator, gcmSIZEOF(gcsALLOCATOR));

    /* Record os. */
    allocator->os = Os;

    /* Set operations. */
    allocator->ops = Operations;
//gcvALLOC_FLAG_CONTIGUOUS |
    allocator->capability = gcvALLOC_FLAG_NON_CONTIGUOUS
                          | gcvALLOC_FLAG_CACHEABLE
                          | gcvALLOC_FLAG_MEMLIMIT;
                          ;

    *Allocator = allocator;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

/******************************************************************************\
******************************** Debugfs Support *******************************
\******************************************************************************/

static gceSTATUS
_AllocatorDebugfsInit(
    IN gckOS Os
    )
{
    gceSTATUS status;
    gckGALDEVICE device = Os->device;

    gckDEBUGFS_DIR dir = &Os->allocatorDebugfsDir;

    gcmkONERROR(gckDEBUGFS_DIR_Init(dir, device->debugfsDir.root, "allocators"));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static void
_AllocatorDebugfsCleanup(
    IN gckOS Os
    )
{
    gckDEBUGFS_DIR dir = &Os->allocatorDebugfsDir;

    gckDEBUGFS_DIR_Deinit(dir);
}
#endif
