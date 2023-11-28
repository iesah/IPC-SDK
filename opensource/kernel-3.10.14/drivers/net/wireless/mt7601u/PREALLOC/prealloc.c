
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include "rt_config.h"

enum BLK_TYPE {
	BLK_TX0,
	BLK_TX1,
	BLK_TX2,
	BLK_TX3,
	BLK_TX4,
	BLK_NULL,
	BLK_PSPOLL,
	BLK_RX0,
	BLK_RX1,
	BLK_RX2,
	BLK_RX3,
	BLK_RX4,
	BLK_RX5,
	BLK_RX6,
	BLK_RX7,
	BLK_CMD,
};

#define NUM_OF_TOTAL_BLK	5+2+8+1 //TX+Null+PsPoll+RX+CMD

ULONG PreAllocBuffer[NUM_OF_TOTAL_BLK];  
ULONG PreAllocDmaAddr[NUM_OF_TOTAL_BLK];

int __init prealloc_init(void)
{
	int result = 0, tx = 0, rx = 0, k = 0;
	
	memset(PreAllocBuffer,0, NUM_OF_TOTAL_BLK);
	
	//TX
	for(tx=0; tx<5; tx++)
	{
		PreAllocBuffer[tx+BLK_TX0] = kmalloc(sizeof(HTTX_BUFFER), GFP_ATOMIC | GFP_DMA);	
		if (!PreAllocBuffer[tx+BLK_TX0])
			goto fail_malloc1;
		
		PreAllocDmaAddr[tx+BLK_TX0] = virt_to_phys(PreAllocBuffer[tx+BLK_TX0]);
	}
	
	//Null
	PreAllocBuffer[BLK_NULL] = kmalloc(sizeof(TX_BUFFER), GFP_ATOMIC | GFP_DMA);	
	if (!PreAllocBuffer[BLK_NULL])
		goto fail_malloc1;
			
	PreAllocDmaAddr[BLK_NULL] = virt_to_phys(PreAllocBuffer[BLK_NULL]);
	
	//PsPoll
	PreAllocBuffer[BLK_PSPOLL] = kmalloc(sizeof(TX_BUFFER), GFP_ATOMIC | GFP_DMA);	
	if (!PreAllocBuffer[BLK_PSPOLL])
		goto fail_malloc2;
		
	PreAllocDmaAddr[BLK_PSPOLL] = virt_to_phys(PreAllocBuffer[BLK_PSPOLL]);
	
	//RX
	for (rx = 0; rx < 8; rx++)
	{
		PreAllocBuffer[rx+BLK_RX0] = kmalloc(MAX_RXBULK_SIZE, GFP_ATOMIC | GFP_DMA);	
		if (!PreAllocBuffer[rx+BLK_RX0])
			goto fail_malloc3;
		
		PreAllocDmaAddr[rx+BLK_RX0] = virt_to_phys(PreAllocBuffer[rx+BLK_RX0]);
	}
	
	//CMD
	PreAllocBuffer[BLK_CMD] = kmalloc(CMD_RSP_BULK_SIZE, GFP_ATOMIC | GFP_DMA);	
	if (!PreAllocBuffer[BLK_CMD])
			goto fail_malloc4;
			
	PreAllocDmaAddr[BLK_CMD] = virt_to_phys(PreAllocBuffer[BLK_CMD]);
	
	for (k=0;k<NUM_OF_TOTAL_BLK;k++)
		printk("==>[%d]:PreBuff:0x%x, DmaAddr:0x%x\n", k, PreAllocBuffer[k], PreAllocDmaAddr[k]);
		
	printk("install prealloc ok\n");
	return result; /* succeed */

fail_malloc4:
	kfree(PreAllocBuffer[BLK_CMD]);
	
fail_malloc3:
	for (k=0;k<rx;k++)
		kfree(PreAllocBuffer[k+BLK_RX0]);
	kfree(PreAllocBuffer[BLK_PSPOLL]);	
	
fail_malloc2:
	kfree(PreAllocBuffer[BLK_NULL]);
	
fail_malloc1:
	for (k=0;k<tx;k++)
		kfree(PreAllocBuffer[k]);	
		
	result = -ENOMEM;
	return result;
}



void prealloc_cleanup(void)
{
	int i;

	for (i=0;i<NUM_OF_TOTAL_BLK;i++)
		kfree(PreAllocBuffer[i]); 
	printk("remove prealloc ok\n");
}

void *RTMPQMemAddr(int size, dma_addr_t *pDmaAddr, int type)
{
	switch(type)
	{
		case BLK_TX0:
		case BLK_TX1:
		case BLK_TX2:
		case BLK_TX3:
		case BLK_TX4:				
			if (size > sizeof(HTTX_BUFFER))
				return NULL;
			break;
		case BLK_NULL:
		case BLK_PSPOLL:		
			if (size > sizeof(TX_BUFFER))
				return NULL;
			break;
		case BLK_RX0:
		case BLK_RX1:
		case BLK_RX2:
		case BLK_RX3:	
		case BLK_RX4:
		case BLK_RX5:
		case BLK_RX6:
		case BLK_RX7:
			if (size > MAX_RXBULK_SIZE)
				return NULL;
			break;	
		case BLK_CMD:
			if (size > CMD_RSP_BULK_SIZE)
				return NULL;				
			break;
		default:
			printk("Non-support memory type!!!!\n");
			return NULL;		
	}
	
	*pDmaAddr = PreAllocDmaAddr[type];
	return 	PreAllocBuffer[type];
}

EXPORT_SYMBOL(RTMPQMemAddr);

module_init(prealloc_init);
module_exit(prealloc_cleanup);
