#ifndef __DMA_MSG_HANDLER_H_
#define __DMA_MSG_HANDLER_H_

#include "nand_ops.h"
#include "ndmessage.h"
#include "nddata.h"
#include "lib_nandopsinterface.h"

#define MSG_NUMBER          16

/* TCSM info ,same with firmware */
#define PDMA_FW_TCSM_PA		0x13422000
#define PDMA_MSG_CHANNEL	3
#define PDMA_BANK3_PA		0x13423800
#define PDMA_BANK4_PA		0x13424000
#define PDMA_BANK5_PA		0x13424800
#define PDMA_BANK6_PA		0x13425000
#define PDMA_MSG_TCSMPA     	(PDMA_BANK4_PA - 0xc0)

#define PDMA_FW_TCSM_VA		0xB3422000
#define PDMA_BANK3_VA		0xB3423800
#define PDMA_BANK4_VA		0xB3424000
#define PDMA_BANK5_VA		0xB3424800
#define PDMA_BANK6_VA		0xB3425000
#define PDMA_BANK7_VA		0xB3425800
#define PDMA_MSG_TCSMVA    (PDMA_BANK4_VA - 0xc0)

int dma_msg_handle_suspend(int msghandler);
int dma_msg_handle_resume(int msghandler, Nand_Task *nandtask, int id);
int dma_msg_handle_init(nand_data *data, Nand_Task *nandtask, int id);
void dma_msg_handle_deinit(int msghandler);
#endif
