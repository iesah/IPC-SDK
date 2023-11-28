/*
 * JZSOC CPM register definition.
 *
 * CPM (Clock reset and Power control Management)
 *
 * Copyright (C) 2010 Ingenic Semiconductor Co., Ltd.
 */

#ifndef __JZSOC_CPM_H__
#define __JZSOC_CPM_H__

#define CPM_CPCCR	(0x00)
#define CPM_CPCSR	(0xd4)

#define CPM_DDRCDR	(0x2c)
#define CPM_VPUCDR	(0x30)
#define CPM_I2SCDR	(0x60)
#define CPM_LPCDR0	(0x54)
#define CPM_LPCDR1	(0x64)
#define CPM_MSC0CDR	(0x68)
#define CPM_MSC1CDR	(0xa4)
#define CPM_MSC2CDR	(0xa8)
#define CPM_UHCCDR	(0x6c)
#define CPM_SSICDR	(0x74)
#define CPM_CIMCDR	(0x7c)
#define CPM_PCMCDR	(0x84)
#define CPM_GPUCDR	(0x88)
#define CPM_HDMICDR	(0x8c)
#define CPM_BCHCDR	(0xac)

#define CPM_INTR	(0xb0)
#define CPM_INTRE	(0xb4)
#define CPM_CPSPPR	(0x38)
#define CPM_CPPSR	(0x34)

#define CPM_USBPCR	(0x3c)
#define CPM_USBRDT	(0x40)
#define CPM_USBVBFIL	(0x44)
#define CPM_USBPCR1	(0x48)

#define CPM_CPPCR	(0x0c)
#define CPM_CPAPCR	(0x10)
#define CPM_CPMPCR	(0x14)
#define CPM_CPEPCR	(0x18)
#define CPM_CPVPCR	(0x1c)

#define CPM_LCR		(0x04)
#define CPM_SPCR0	(0xb8)
#define CPM_SPCR1	(0xbc)
#define CPM_SPPC	(0xc0)
#define CPM_PSWC0ST     (0x90)
#define CPM_PSWC1ST     (0x94)
#define CPM_PSWC2ST     (0x98)
#define CPM_PSWC3ST     (0x9c)
#define CPM_CLKGR0	(0x20)
#define CPM_CLKGR1	(0x28)
#define CPM_SRBC	(0xc4)
#define CPM_SLBC	(0xc8)
#define CPM_SLPC	(0xcc)
#define CPM_OPCR	(0x24)
#define CPM_RSR		(0x08)

#define LCR_LPM_MASK		(0x3)
#define LCR_LPM_SLEEP		(0x1)

#define CPM_LCR_PD_SCPU		(0x1<<31)
#define CPM_LCR_PD_VPU		(0x1<<30)
#define CPM_LCR_PD_GPU		(0x1<<29)
#define CPM_LCR_PD_GPS		(0x1<<28)
#define CPM_LCR_PD_MASK		(0x7<<28)
#define CPM_LCR_SCPUS 		(0x1<<27)
#define CPM_LCR_VPUS		(0x1<<26)
#define CPM_LCR_GPUS		(0x1<<25) 
#define CPM_LCR_GPSS 		(0x1<<25)
#define CPM_LCR_STATUS_MASK 	(0xf<<24)
#define CPM_LCR_GPU_IDLE 	(0x1<<24)

#define OPCR_ERCS		(0x1<<2)
#define OPCR_PD			(0x1<<3)
#define OPCR_IDLE		(0x1<<31)

#define CLKGR1_VPU              (0x1<<2)


#ifndef CONFIG_FPGA_TEST 
#define cpm_inl(off)		inl(CPM_IOBASE + (off))
#define cpm_outl(val,off)	outl(val,CPM_IOBASE + (off))
#define cpm_clear_bit(val,off)	do{cpm_outl((cpm_inl(off) & ~(1<<(val))),off);}while(0)
#define cpm_set_bit(val,off)	do{cpm_outl((cpm_inl(off) |  (1<<val)),off);}while(0)
#define cpm_test_bit(val,off)	(cpm_inl(off) & (0x1<<val))
#else
#define cpm_inl(x)		0x3
#define cpm_outl(val,off)	do{}while(0)
//#define cpm_outl(v,x)		do{}while(0)
#define cpm_clear_bit(off,x)	do{}while(0)
#define cpm_set_bit(off,x)	do{}while(0)
#define cpm_test_bit(val,off)	do{}while(0)
#endif

int cpm_start_ehci(void);
int cpm_stop_ehci(void);
int cpm_start_ohci(void);
int cpm_stop_ohci(void);
#endif
