#ifndef __DMIC_CONFIG_H__
#define __DMIC_CONFIG_H__

#include "dmic_ops.h"
#include "interface.h"


#define DMA_CHANNEL     (5)
#define DMIC_REQ_TYPE	(5)


#define NR_DESC         (4)
#define DMIC_RX_FIFO    (DMIC_BASE_ADDR + DMICDR)

#define DMA_DESC_ADDR		(TCSM_DESC_ADDR)
#define DMA_DESC_SIZE	(NR_DESC * 8)

#define BUF_SIZE    TCSM_DATA_BUFFER_SIZE



extern void dma_init_for_dmic(void);
#endif

