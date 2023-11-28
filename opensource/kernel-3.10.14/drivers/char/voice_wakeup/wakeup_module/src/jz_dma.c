#include "interface.h"
#include "jz_dma.h"

void build_one_desc(struct dma_config* dma, struct dma_desc* _desc, struct dma_desc* next_desc){
	struct dma_desc *desc = (struct dma_desc *)((unsigned int)_desc | 0xA0000000);
	desc->dsa = dma->src;
	desc->dta = dma->dst;
	desc->dtc = (dma->count&0xffffff) | ( ((unsigned long)(next_desc) >> 4) << 24);
	desc->drt = dma->type;
	desc->sd = dma->sd;
	desc->dcm = dma->increment<<22 | dma->tsz<<8 | dma->sp_dp<<12 | dma->rdil<<16 |dma->stde<<2| dma->tie<<1 | dma->link;
}


void pdma_config(struct dma_config* dma){
	if(!dma->descriptor){
		REG_DMADSA(dma->channel) = dma->src;
		REG_DMADTA(dma->channel) = dma->dst;
		REG_DMADTC(dma->channel) = dma->count;
		REG_DMADRT(dma->channel) = dma->type;
		REG_DMADCS(dma->channel) = 0x80000000;
		REG_DMADCM(dma->channel) = 0;
		REG_DMADCM(dma->channel) = dma->increment<<22 | dma->tsz<<8 | dma->rdil<<16 | dma->sp_dp<<12|dma->stde<<2| dma->tie<<1 | dma->link ;
	}else{
		REG_DMADCS(dma->channel) = 0;
		REG_DMADDA(dma->channel) = dma->desc;
		if(dma->des8){
			REG_DMADCS(dma->channel) |= dma->des8<<30;
		}else{
			REG_DMADRT(dma->channel) = dma->type;
			REG_DMADSD(dma->channel) = dma->sd;
		}

		REG_DMADDB |= 1 << dma->channel;
		REG_DMADDS |= 1 << dma->channel;
	}
}
void pdma_start(int channel){
	REG_DMADMAC |= 1;
	REG_DMADCS(channel) |= 1;
}

void pdma_wait(channel){
	while(!(REG_DMADCS(channel)&1<<3)){
		printf("dma channel %d wait TT\n",channel);
	}
	printf("channel %d Transfer done.\n",channel);
}

void pdma_end(int channel){
	REG_DMADCS(channel) &= ~1;
}

unsigned int pdma_trans_addr(int channel, int direction)
{
	if(direction == DMA_MEM_TO_DEV) { /*ddr to device*/
		return REG_DMADSA(channel);
	} else if (direction == DMA_DEV_TO_MEM){/*device to dma*/
		return REG_DMADTA(channel);
	} else if(direction == DMA_MEM_TO_MEM) {
		printf("src:%08x , dst:%08x\n", REG_DMADSA(channel), REG_DMADTA(channel));
	}
	return 0;
}


