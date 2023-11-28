#include "ivDefine.h"
#include "ivIvwDefine.h"
#include "ivIvwErrorCode.h"
#include "ivIVW.h"
#include "ivPlatform.h"

#include "jz_dma.h"
#include "circ_buf.h"
#include "dmic_config.h"
#include "interface.h"
#include "voice_wakeup.h"
#include <common.h>


unsigned int __res_mem wakeup_res[] = {
	//#include "ivModel_v21.data"
};

//#define MAX_PROCESS_LEN		(512 * 1024)
#define MAX_PROCESS_LEN		(128 * 1024)
//#define MAX_PROCESS_LEN		(64 * 1024)
//#define MAX_PROCESS_LEN		(32 * 1024)
//#define MAX_PROCESS_LEN		(16 * 1024)

//#define PROCESS_BYTES_PER_TIME	(2048)
#define PROCESS_BYTES_PER_TIME	(1024)
//#define PROCESS_BYTES_PER_TIME	(512)
//#define PROCESS_BYTES_PER_TIME	(128)

int total_process_len = 0;

unsigned char __attribute__((aligned(32))) pIvwObjBuf[20*1024];
unsigned char __attribute__((aligned(32))) nReisdentBuf[38];

ivPointer __attribute__((aligned(32))) pIvwObj;
ivPointer __attribute__((aligned(32))) pResidentRAM;

int send_data_to_process(ivPointer pIvwObj, unsigned char *pdata, unsigned int size)
{
	unsigned int nSize_PCM = size;
	unsigned int tmp;
	int i;
	ivInt16 CMScore  =  0;
	ivInt16 ResID = 0;
	ivInt16 KeywordID = 0;
	ivUInt32 StartMS = 0;
	ivUInt32 EndMS = 0;
	unsigned int Samples;
	unsigned int BytesPerSample;
	unsigned char *pData = pdata;
	unsigned int nSamples_count;
	ivStatus iStatus1;
	ivStatus iStatus2;
	Samples = 110;

	//serial_put_hex(size);
	BytesPerSample = 2;
	nSamples_count = nSize_PCM / (Samples * BytesPerSample);
	/*process 16k, 16bit samples*/
	for(i = 0; i <= nSamples_count; i++ )
	{
		if(i == nSamples_count) {
			tmp = nSize_PCM % (Samples*BytesPerSample);
			if(!tmp) {
				break;
			}
			Samples = tmp / BytesPerSample;
		}
		iStatus1 = IvwAppendAudioData(pIvwObj,(ivCPointer)pData, Samples);
		pData = pData + Samples * BytesPerSample;
		if( iStatus1 != IvwErrID_OK )
		{
			TCSM_PCHAR('E');
			TCSM_PCHAR('r');
			TCSM_PCHAR('r');
			if( iStatus1 == IvwErr_BufferFull ){
				TCSM_PCHAR('1');
				//serial_put_hex(Samples);
				//printf("IvwAppendAudioData Error:IvwErr_BufferFull\n");
			}else if( iStatus1 ==  IvwErr_InvArg ){
				TCSM_PCHAR('2');
				//printf("IvwAppendAudioData Error:IvwErr_InvArg\n");
			}else if( iStatus1 ==  IvwErr_InvCal ){
				TCSM_PCHAR('3');
				//printf("IvwAppendAudioData Error:IvwErr_InvCal\n");
			}
			//printf("IvwAppendAudioData Error: %d\n", iStatus1);
		} else {
			//printf("IvwAppendAudioData Ok!!!!!!!!\n");
		}
		iStatus2 = IvwRunStepEx( pIvwObj, &CMScore,  &ResID, &KeywordID,  &StartMS, &EndMS );
		if( IvwErr_WakeUp == iStatus2 ){
			return IvwErr_WakeUp;
		}
		else if( iStatus2 == IvwErrID_OK ) {

		}
		else if( iStatus2 == IvwErr_InvArg ) {
			TCSM_PCHAR('E');
			TCSM_PCHAR('r');
			TCSM_PCHAR('4');
			//printf("%d: iStatus2 == IvwErr_InvArg\n", i);
		}
		else if( iStatus2 == IvwErr_ReEnter ) {
			//printf("%d: iStatus2 == IvwErr_ReEnter\n", i);
			TCSM_PCHAR('E');
			TCSM_PCHAR('r');
			TCSM_PCHAR('5');
		}
		else if( iStatus2 == IvwErr_InvCal ) {
			//printf("%d: iStatus2 == IvwErr_InvCal\n", i);
			TCSM_PCHAR('E');
			TCSM_PCHAR('r');
			TCSM_PCHAR('6');
		}
		else if( iStatus2 == IvwErr_BufferEmpty ) {
			//printf("%d: iStatus2 == IvwErr_BufferEmpty\n", i);
			//TCSM_PCHAR('E');
			//TCSM_PCHAR('r');
			//TCSM_PCHAR('7');
		}
	}
	return 0;
}



struct fifo {
	struct circ_buf xfer;
	u32	n_size;
};
struct fifo rx_fifo[0];

void wakeup_reset_fifo()
{
	struct circ_buf * xfer = &rx_fifo->xfer;

	xfer->head = 0;
	xfer->tail = 0;
	total_process_len = 0;
}

int wakeup_open(void)
{
	ivSize nIvwObjSize = 20 * 1024;
	pIvwObj = (ivPointer)pIvwObjBuf;
	ivUInt16 nResidentRAMSize = 38;
	pResidentRAM = (ivPointer)nReisdentBuf;
	unsigned char __attribute__((aligned(32))) *pResKey = (unsigned char *)wakeup_res;
	unsigned int dma_addr;

	ivUInt16 nWakeupNetworkID = 0;

	ivStatus iStatus;
	//printf("wakeu module init#####\n");
	//printf("pIvwObj:%x, pResidentRAM:%x, pResKey:%x\n", pIvwObj, pResidentRAM, pResKey);
	iStatus = IvwCreate(pIvwObj, &nIvwObjSize, pResidentRAM, &nResidentRAMSize, pResKey, nWakeupNetworkID);
	if( IvwErrID_OK != iStatus ){
		//printf("IvwVreate Error: %d\n", iStatus);
		return ivFalse;
	}
	//printf("[voice wakeup] OBJECT create ok\n");
	IvwSetParam( pIvwObj, IVW_CM_THRESHOLD, 10, 0 ,0);
	IvwSetParam( pIvwObj, IVW_CM_THRESHOLD, 20, 1 ,0);
	IvwSetParam( pIvwObj, IVW_CM_THRESHOLD, 15, 2 ,0);
	/* code that need rewrite */
	struct circ_buf *xfer = &rx_fifo->xfer;
	rx_fifo->n_size	= BUF_SIZE; /*tcsm 4kBytes*/
	xfer->buf = (char *)TCSM_DATA_BUFFER_ADDR;
	dma_addr = pdma_trans_addr(DMA_CHANNEL, 2);
	if((dma_addr >= (TCSM_DATA_BUFFER_ADDR & 0x1fffffff)) && (dma_addr <= ((TCSM_DATA_BUFFER_ADDR + BUF_SIZE)& 0x1fffffff))) {
		xfer->head = (char *)(dma_addr | 0xA0000000) - xfer->buf;
	} else {
		xfer->head = 0;
	}
	xfer->tail = xfer->head;

	printf("pdma_trans_addr:%x, xfer->buf:%x, xfer->head:%x, xfer->tail:%x\n",pdma_trans_addr(DMA_CHANNEL, 2), xfer->buf, xfer->head, xfer->tail);
	return 0;
}

int wakeup_close(void)
{

	return 0;
}

#ifdef CONFIG_CPU_SWITCH_FREQUENCY
int get_valid_bytes()
{
	unsigned int dma_addr;
	dma_addr = pdma_trans_addr(DMA_CHANNEL, DMA_DEV_TO_MEM);
	int nbytes;
	struct circ_buf *xfer;
	xfer = &rx_fifo->xfer;
	xfer->head = (char *)(dma_addr | 0xA0000000) - xfer->buf;

	nbytes = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);

	return nbytes;
}
int process_nbytes(int nbytes)
{
	struct circ_buf * xfer;
	int ret;
	xfer = &rx_fifo->xfer;

	while(1) {
		int nread;
		nread = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);
		if(nread > CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size)) {
			nread = CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size);
		} else if(nread == 0) {
			break;
		}
		ret = send_data_to_process(pIvwObj, (unsigned char *)xfer->buf + xfer->tail, nread);
		if(ret == IvwErr_WakeUp) {
			return SYS_WAKEUP_OK;
		}

		xfer->tail += nread;
		xfer->tail %= rx_fifo->n_size;
	}

	total_process_len += nbytes;
	if(total_process_len >= MAX_PROCESS_LEN) {
		TCSM_PCHAR('F');
		serial_put_hex(total_process_len);
		total_process_len = 0;
		return SYS_WAKEUP_FAILED;
	} else {
		return SYS_NEED_DATA;
	}

	return SYS_WAKEUP_FAILED;
}

int process_dma_data_3(void)
{
	int ret;
	int nbytes;

	nbytes = get_valid_bytes();
	if(nbytes >= PROCESS_BYTES_PER_TIME) {
		_cpu_switch_restore(); /*switch to the sleep freq setted in kernel to process data.120MHZ*/
		ret = process_nbytes(nbytes);
		_cpu_switch_24MHZ();	/*switch to 24MHZ. */
	} else {
		ret = SYS_NEED_DATA;
	}

	return ret;
}
#endif
int process_dma_data_2(void)
{

	unsigned int dma_addr;
	int nbytes;
	volatile int ret;
	struct circ_buf *xfer;

	dma_addr = pdma_trans_addr(DMA_CHANNEL, DMA_DEV_TO_MEM);
	xfer = &rx_fifo->xfer;
	xfer->head = (char *)(dma_addr | 0xA0000000) - xfer->buf;

	nbytes = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);

	while(1) {
		int nread;
		nread = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);
		if(nread > CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size)) {
			nread = CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size);
		} else if(nread == 0) {
			break;
		}
		ret = send_data_to_process(pIvwObj, (unsigned char *)xfer->buf + xfer->tail, nread);
		if(ret == IvwErr_WakeUp) {
			return SYS_WAKEUP_OK;
		}

		xfer->tail += nread;
		xfer->tail %= rx_fifo->n_size;
	}

	total_process_len += nbytes;
	if(total_process_len >= MAX_PROCESS_LEN) {
		TCSM_PCHAR('F');
		serial_put_hex(total_process_len);
		total_process_len = 0;
		return SYS_WAKEUP_FAILED;
	} else {
		return SYS_NEED_DATA;
	}

	return SYS_WAKEUP_FAILED;
}

int process_dma_data(void)
{

	unsigned int dma_addr;
	int nbytes;
	int ret;
	struct circ_buf *xfer;

	dma_addr = pdma_trans_addr(DMA_CHANNEL, DMA_DEV_TO_MEM);
	xfer = &rx_fifo->xfer;
	xfer->head = (char *)(dma_addr | 0xA0000000) - xfer->buf;

	nbytes = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);

	//printk("xfer->head:%d, xfer->tail:%d, nbyts:%d\n", xfer->head, xfer->tail, nbytes);
	if(nbytes > 220) {
		while(1) {
			int nread;
			nread = CIRC_CNT(xfer->head, xfer->tail, rx_fifo->n_size);
			if(nread > CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size)) {
				nread = CIRC_CNT_TO_END(xfer->head, xfer->tail, rx_fifo->n_size);
			} else if(nread == 0) {
				break;
			}
			ret = send_data_to_process(pIvwObj, (unsigned char *)xfer->buf + xfer->tail, nread);
			if(ret == IvwErr_WakeUp) {
				printf("####[%s]system wakeup ok!!!\n", __func__);
				return SYS_WAKEUP_OK;
			}

			xfer->tail += nread;
			xfer->tail %= rx_fifo->n_size;
		}
	} else {
		/*need more data*/
	}
	return SYS_WAKEUP_FAILED;
}

int process_buffer_data(unsigned char *buf, unsigned long len)
{
	int ret;
	unsigned char *a_buf = (unsigned char *)((unsigned int)buf | 0xA0000000);
	ret = send_data_to_process(pIvwObj, a_buf, len);
	if(ret == IvwErr_WakeUp) {
		return SYS_WAKEUP_OK;
	}
	return SYS_WAKEUP_FAILED;
}
