#ifndef __VOICE_WAKEUP_H__
#define __VOICE_WAKEUP_H__



#include "ivDefine.h"
#include "ivIvwDefine.h"
#include "ivIvwErrorCode.h"
#include "ivIVW.h"
#include "ivPlatform.h"


int send_data_to_process(ivPointer pIvwObj, unsigned char *pdata, unsigned int size);

int wakeup_open(void);
int wakeup_close(void);

int process_dma_data(void);
int process_dma_data_2(void);
int process_buffer_data(unsigned char *buffer, unsigned long len);

int process_dma_data_3(void);

void wakeup_reset_fifo(void);

#define SYS_WAKEUP_OK	0x1
#define SYS_WAKEUP_FAILED 0x2
#define SYS_NEED_DATA	0x3


extern unsigned int wakeup_res[];

#endif

