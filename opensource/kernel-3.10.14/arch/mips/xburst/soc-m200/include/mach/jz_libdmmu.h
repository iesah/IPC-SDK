#ifndef _JZ_LIB_DMMU_H
#define _JZ_LIB_DMMU_H
#include <asm/pgtable-bits.h>

/* Bit 0~11 is secondary tlb check code in our TLB design */
//#define DMMU_PTE_CHECK_PAGE_VALID		_PAGE_VALID
#define DMMU_PTE_CHECK_PAGE_VALID		_PAGE_PRESENT
/* no use now */
#define DMMU_PTE_CHECK_PAGE_READ		(_PAGE_VALID | _PAGE_READ)
#define DMMU_PTE_CHECK_PAGE_WRITE		(_PAGE_VALID | _PAGE_WRITE)
#define DMMU_PTE_CHECK_PAGE_RW			(_PAGE_VALID | _PAGE_READ | _PAGE_WRITE)

/* TLB FLAGS */
/* Bit 0 is the valid bit in our TLB design */
#define DMMU_PGD_VLD		(1 << 0)
#define DMMU_PTE_VLD		DMMU_PTE_CHECK_PAGE_VALID


/* user must use interface */
int dmmu_init(void);
int dmmu_get_page_table_base_phys(unsigned int *phys_addr);
int dmmu_match_user_mem_tlb(void * vaddr, int size);
int dmmu_map_user_mem(void * vaddr, int size);
int dmmu_unmap_user_mem(void * vaddr, int size);
int dmmu_deinit(void);

#endif
