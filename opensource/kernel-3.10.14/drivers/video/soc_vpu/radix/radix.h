/*************************************************************************************
          JZ-Media RADIX Definition
*************************************************************************************/
#ifndef __RADIX_H__
#define __RADIX_H__

#ifdef JZM_HUNT_SIM
#include "hunt.h"
#else
#ifndef __place_k0_data__
#define __place_k0_data__
#endif
#ifndef __place_k0_text__
#define __place_k0_text__
#endif
#endif

#define RADIX_BASE          0   // force to 0, for we use ralative addr in this driver, this is importent

/*************************************************************************************
          RADIX SLAVE BASE
*************************************************************************************/
#define RADIX_HID_M0		0x0   // CFGC
#define RADIX_HID_M1		0x1   // VDMA
#define RADIX_HID_M2		0x2   // ODMA
#define RADIX_HID_M3		0x3   // TMC
#define RADIX_HID_M4		0x4   // EFE
#define RADIX_HID_M5		0x5
#define RADIX_HID_M6		0x6   // MCE
#define RADIX_HID_M7		0x7   // TFM
#define RADIX_HID_M8		0x8   // MD
#define RADIX_HID_M9		0x9   // DT
#define RADIX_HID_M10		0xA   // DBLK
#define RADIX_HID_M11		0xB   // SAO
#define RADIX_HID_M12		0xC   // BC
#define RADIX_HID_M13		0xD   // SDE
#define RADIX_HID_M14		0xE   // IPRED
#define RADIX_HID_M15		0xF   // STC

#define RADIX_CFGC_BASE	(RADIX_BASE + (RADIX_HID_M0 << 15))
#define RADIX_VDMA_BASE	(RADIX_BASE + (RADIX_HID_M1 << 15))
#define RADIX_ODMA_BASE	(RADIX_BASE + (RADIX_HID_M2 << 15))
#define RADIX_TMC_BASE	(RADIX_BASE + (RADIX_HID_M3 << 15))
#define RADIX_EFE_BASE	(RADIX_BASE + (RADIX_HID_M4 << 15))
#define RADIX_MCE_BASE	(RADIX_BASE + (RADIX_HID_M6 << 15))
#define RADIX_TFM_BASE	(RADIX_BASE + (RADIX_HID_M7 << 15))
#define RADIX_MD_BASE		(RADIX_BASE + (RADIX_HID_M8 << 15))
#define RADIX_DT_BASE		(RADIX_BASE + (RADIX_HID_M9 << 15))
#define RADIX_DBLK_BASE	(RADIX_BASE + (RADIX_HID_M10 << 15))
#define RADIX_SAO_BASE	(RADIX_BASE + (RADIX_HID_M11 << 15))
#define RADIX_BC_BASE		(RADIX_BASE + (RADIX_HID_M12 << 15))
#define RADIX_SDE_BASE	(RADIX_BASE + (RADIX_HID_M13 << 15))
#define RADIX_IPRED_BASE	(RADIX_BASE + (RADIX_HID_M14 << 15))
#define RADIX_STC_BASE	(RADIX_BASE + (RADIX_HID_M15 << 15))

/*************************************************************************************
                       Writing/Reading Registers
**************************************************************************************/
#define RADIX_SET_CFGC_REG(ofst, val)		({write_reg((RADIX_CFGC_BASE + (ofst)), (val));})
#define RADIX_GET_CFGC_REG(ofst)		({read_reg((RADIX_CFGC_BASE + (ofst)), (0));})

#define RADIX_SET_VDMA_REG(ofst, val)		({write_reg((RADIX_VDMA_BASE + (ofst)), (val));})
#define RADIX_GET_VDMA_REG(ofst)		({read_reg((RADIX_VDMA_BASE + (ofst)), (0));})

#define RADIX_SET_ODMA_REG(ofst, val)		({write_reg((RADIX_ODMA_BASE + (ofst)), (val));})
#define RADIX_GET_ODMA_REG(ofst)		({read_reg((RADIX_ODMA_BASE + (ofst)), (0));})

#define RADIX_SET_TMC_REG(ofst, val)		({write_reg((RADIX_TMC_BASE + (ofst)), (val));})
#define RADIX_GET_TMC_REG(ofst)		({read_reg((RADIX_TMC_BASE + (ofst)), (0));})

#define RADIX_SET_EFE_REG(ofst, val)		({write_reg((RADIX_EFE_BASE + (ofst)), (val));})
#define RADIX_GET_EFE_REG(ofst)		({read_reg((RADIX_EFE_BASE + (ofst)), (0));})

#define RADIX_SET_IPRED_REG(ofst, val)	({write_reg((RADIX_IPRED_BASE + (ofst)), (val));})
#define RADIX_GET_IPRED_REG(ofst)		({read_reg((RADIX_IPRED_BASE + (ofst)), (0));})

#define RADIX_SET_MCE_REG(ofst, val)		({write_reg((RADIX_MCE_BASE + (ofst)), (val));})
#define RADIX_GET_MCE_REG(ofst)		({read_reg((RADIX_MCE_BASE + (ofst)), (0));})

#define RADIX_SET_TFM_REG(ofst, val)		({write_reg((RADIX_TFM_BASE + (ofst)), (val));})
#define RADIX_GET_TFM_REG(ofst)		({read_reg((RADIX_TFM_BASE + (ofst)), (0));})

#define RADIX_SET_MD_REG(ofst, val)		({write_reg((RADIX_MD_BASE + (ofst)), (val));})
#define RADIX_GET_MD_REG(ofst)		({read_reg((RADIX_MD_BASE + (ofst)), (0));})

#define RADIX_SET_DT_REG(ofst, val)		({write_reg((RADIX_DT_BASE + (ofst)), (val));})
#define RADIX_GET_DT_REG(ofst)		({read_reg((RADIX_DT_BASE + (ofst)), (0));})

#define RADIX_SET_DBLK_REG(ofst, val)		({write_reg((RADIX_DBLK_BASE + (ofst)), (val));})
#define RADIX_GET_DBLK_REG(ofst)		({read_reg((RADIX_DBLK_BASE + (ofst)), (0));})

#define RADIX_SET_SAO_REG(ofst, val)		({write_reg((RADIX_SAO_BASE + (ofst)), (val));})
#define RADIX_GET_SAO_REG(ofst)		({read_reg((RADIX_SAO_BASE + (ofst)), (0));})

#define RADIX_SET_BC_REG(ofst, val)		({write_reg((RADIX_BC_BASE + (ofst)), (val));})
#define RADIX_GET_BC_REG(ofst)		({read_reg((RADIX_BC_BASE + (ofst)), (0));})

#define RADIX_SET_SDE_REG(ofst, val)		({write_reg((RADIX_SDE_BASE + (ofst)), (val));})
#define RADIX_GET_SDE_REG(ofst)		({read_reg((RADIX_SDE_BASE + (ofst)), (0));})

#define RADIX_SET_STC_REG(ofst, val)		({write_reg((RADIX_STC_BASE + (ofst)), (val));})
#define RADIX_GET_STC_REG(ofst)		({read_reg((RADIX_STC_BASE + (ofst)), (0));})

/*************************************************************************************
                  CFGC Module
*************************************************************************************/
#define	RADIX_REG_CFGC_GLB_CTRL	(0x00 << 2)
#define	RADIX_REG_CFGC_ACM_CTRL     	(0x02 << 2)
#define	RADIX_REG_CFGC_TLB_TBA     	(0x04 << 2)
#define	RADIX_REG_CFGC_TLB_CTRL     	(0x05 << 2)
#define RADIX_REG_CFGC_TLBV		(0x06 << 2)
#define	RADIX_REG_CFGC_TLBE		(0x07 << 2)
#define	RADIX_REG_CFGC_STAT		(0x10 << 2)
#define	RADIX_REG_CFGC_BSLEN		(0x11 << 2)
#define	RADIX_REG_CFGC_SDBG_0		(0x12 << 2)
#define	RADIX_REG_CFGC_SDBG_1		(0x13 << 2)
#define	RADIX_REG_CFGC_SSE_0		(0x14 << 2)
#define	RADIX_REG_CFGC_SSE_1		(0x15 << 2)
#define	RADIX_REG_CFGC_SA8D		(0x16 << 2)

#define RADIX_CFGC_STAT_GLB_END		  (0x1 << 2)

#define RADIX_CFGC_TLBE_EFE           (0x1<<16)
#define RADIX_CFGC_TLBE_VDMA          (0x1<<17)
#define RADIX_CFGC_TLBE_ODMA          (0x1<<18)
#define RADIX_CFGC_TLBE_DBLK          (0x1<<19)
#define RADIX_CFGC_TLBE_MCE           (0x1<<20)
#define RADIX_CFGC_TLBE_SDE           (0x1<<21)
#define RADIX_CFGC_INTE_TMOT(a)       (a<<22)  //time out threshold
#define RADIX_CFGC_INTE_ENDF          (0x1<<24)
#define RADIX_CFGC_INTE_BSERR         (0x1<<25)
#define RADIX_CFGC_INTE_AMCERR        (0x1<<26)
#define RADIX_CFGC_INTE_TLBERR        (0x1<<27)
#define RADIX_CFGC_INTE_DIS           (0x1<<28)  //radix disable check
#define RADIX_CFGC_INTE_TOE           (0x1<<29)  //time out intc enable
#define RADIX_CFGC_TOE_WORK           (0x1<<30)  //time out work enable
#define RADIX_CFGC_GLBC_HIMAP         (0x1<<15)
#define RADIX_CFGC_GLBC_EWPRI         (0x0<<2)
#define RADIX_CFGC_GLBC_ERPRI         (0x0<<0)
#define RADIX_CFGC_INTE_MASK          (0x3f<<24)

#define RADIX_CFGC_TLBV_CNM(cnm)      (((cnm) & 0xFFF)<<16)
#define RADIX_CFGC_TLBV_GCN(gcn)      (((gcn) & 0xFFF)<<0)
#define RADIX_CFGC_TLBC_VPN           (0xFFFFF000)
#define RADIX_CFGC_TLBC_RIDX(idx)     (((idx) & 0xFF)<<4)
#define RADIX_CFGC_TLBC_INVLD         (0x1<<1)
#define RADIX_CFGC_TLBC_RETRY         (0x1<<0)

#define RADIX_CFGC_INTST_RADIX_END    (0x1<<8)
#define RADIX_CFGC_INTST_ACM_ERR      (0x1<<1)

/*************************************************************************************
                  EFE Module
*************************************************************************************/
#define RADIX_REG_EFE_GLB_CTRL         	0x0  //global control
#define RADIX_REG_EFE_FRM_SIZE         	0x4  //frame real size
#define RADIX_REG_EFE_TILE_INFO        	0x8  //current tile position
#define RADIX_REG_EFE_RAWY_BA          	0xC  //raw data y base address
#define RADIX_REG_EFE_RAWC_BA          	0x10 //raw data c base address
#define RADIX_REG_EFE_RAW_STR          	0x14 //raw data stride
#define RADIX_REG_EFE_STAT             	0x18 //slice run and status

#define RADIX_REG_EFE_QPG_CTRL         	0x40 //QPG

#define RADIX_REG_EFE_ROI_INFOA        	0x44 //configure roi mode/enable/QP
#define RADIX_REG_EFE_ROI_INFOB        	0x48 //configure roi mode/enable/QP
#define RADIX_REG_EFE_ROI_POS0         	0x4C //roi 0 position
#define RADIX_REG_EFE_ROI_POS1         	0x50 //roi 1 position
#define RADIX_REG_EFE_ROI_POS2         	0x54 //roi 2 position
#define RADIX_REG_EFE_ROI_POS3         	0x58 //roi 3 position
#define RADIX_REG_EFE_ROI_POS4         	0x5C //roi 4 position
#define RADIX_REG_EFE_ROI_POS5         	0x60 //roi 5 position
#define RADIX_REG_EFE_ROI_POS6         	0x64 //roi 6 position
#define RADIX_REG_EFE_ROI_POS7         	0x68 //roi 7 position
#define RADIX_REG_EFE_ROI_INFOC        	0x140 //configure roi mode/enable/QP
#define RADIX_REG_EFE_ROI_INFOD        	0x144 //configure roi mode/enable/QP
#define RADIX_REG_EFE_ROI_POS8         	0x148 //roi 8 position
#define RADIX_REG_EFE_ROI_POS9         	0x14C //roi 9 position
#define RADIX_REG_EFE_ROI_POS10        	0x150 //roi 10 position
#define RADIX_REG_EFE_ROI_POS11        	0x154 //roi 11 position
#define RADIX_REG_EFE_ROI_POS12        	0x158 //roi 12 position
#define RADIX_REG_EFE_ROI_POS13        	0x15C //roi 13 position
#define RADIX_REG_EFE_ROI_POS14        	0x160 //roi 14 position
#define RADIX_REG_EFE_ROI_POS15        	0x164 //roi 15 position
#define RADIX_REG_EFE_TAB_ADDR 	       	0x168 //
#define RADIX_REG_EFE_TAB_DATA	       	0x16C //


//CRP
#define RADIX_REG_EFE_CRP_FILT	 	0x70
#define RADIX_REG_EFE_CRP_THR0	 	0x74
#define RADIX_REG_EFE_CRP_THR1	 	0x78
#define RADIX_REG_EFE_CRP_THR2	 	0x7C
#define RADIX_REG_EFE_CRP_THR3	 	0x80
#define RADIX_REG_EFE_CRP_THR4	 	0x84
#define RADIX_REG_EFE_CRP_THR5	 	0x88
#define RADIX_REG_EFE_CRP_THR6	 	0x8C
#define RADIX_REG_EFE_CRP_CU16_OFST0       	0x90
#define RADIX_REG_EFE_CRP_CU16_OFST1       	0x94
#define RADIX_REG_EFE_CRP_CU32_OFST0       	0x98
#define RADIX_REG_EFE_CRP_CU32_OFST1       	0x9C
#define RADIX_REG_EFE_CRP_RGNC_CU16_A	        0xA0
#define RADIX_REG_EFE_CRP_RGNC_CU16_B		0xA4
#define RADIX_REG_EFE_CRP_RGNC_CU16_C		0xA8
#define RADIX_REG_EFE_CRP_RGNC_CU16_D		0xAC
#define RADIX_REG_EFE_CRP_RGNC_CU32_A		0xB0
#define RADIX_REG_EFE_CRP_RGNC_CU32_B		0xB4
#define RADIX_REG_EFE_CRP_RGNC_CU32_C		0xB8
#define RADIX_REG_EFE_CRP_RGNC_CU32_D		0xBC

#define RADIX_REG_EFE_CU32_QP_SUM		0xC0
#define RADIX_REG_EFE_CU16_QP_SUM		0xC4
//QP_TAB
#define RADIX_REG_EFE_QP_TAB_LEN		0xE0
#define RADIX_REG_EFE_QP_TAB_BA		0xE4
//SAS
#define RADIX_REG_EFE_SAS_THDG0_0		0x100
#define RADIX_REG_EFE_SAS_THDG0_1		0x104
#define RADIX_REG_EFE_SAS_THDG0_2		0x108
#define RADIX_REG_EFE_SAS_THDG1_0		0x10C
#define RADIX_REG_EFE_SAS_THDG1_1		0x110
#define RADIX_REG_EFE_SAS_THDG1_2		0x114
#define RADIX_REG_EFE_SAS_CU16_OFSTA		0x118
#define RADIX_REG_EFE_SAS_CU16_OFSTB		0x11C
#define RADIX_REG_EFE_SAS_CU32_OFSTA		0x120
#define RADIX_REG_EFE_SAS_CU32_OFSTB		0x124
#define RADIX_REG_EFE_SAS_CNT			0x128
#define RADIX_REG_EFE_SAS_MCNT0		0x12C
#define RADIX_REG_EFE_SAS_MCNT1		0x130


/*************************************************************************************
                  DBLK Module
*************************************************************************************/
#define RADIX_REG_DBLK_GLB_TRIG         0x0   //software reset
#define RADIX_REG_DBLK_GLB_CTRL         0x4   //global control
#define RADIX_REG_DBLK_MB_SIZE          0x8   //frame size
#define RADIX_REG_DBLK_CHN_BA           0xc   //chain base address, not used now
#define RADIX_REG_DBLK_FLT_PARA         0x10  //filter parameters
#define RADIX_REG_DBLK_TILE_BA          0x14  //backup tile right base address
#define RADIX_REG_DBLK_FRM_SIZE         0x18   //frame size

/*************************************************************************************
                  ODMA Module
*************************************************************************************/
#define RADIX_REG_ODMA_GLB_TRIG         0x0   //software reset
#define RADIX_REG_ODMA_GLB_CTRL         0x4   //global control
#define RADIX_REG_ODMA_PLY_BA           0x8   //plane y base address
#define RADIX_REG_ODMA_PLC_BA           0xc   //plane c base address
#define RADIX_REG_ODMA_PL_STR           0x10  //plane stride
#define RADIX_REG_ODMA_TLY_BA           0x14  //tile y base address
#define RADIX_REG_ODMA_TLC_BA           0x18  //tile c base address
#define RADIX_REG_ODMA_TL_STR           0x1c  //tile stride

/*************************************************************************************
                  MCE Module
*************************************************************************************/
#define RADIX_EXPAND_SIZE     256
#define RADIX_MAX_SECH_STEP   57  //64
#define RADIX_MAX_MVD         2048
#define RADIX_MAX_MVRX        128 //256
#define RADIX_MAX_MVRY        128 //256

//GLB_CTRL
#define RADIX_REG_MCE_GLB_CTRL        0x0000
#define RADIX_MCE_GLB_CTRL_BF         (0x1<<3)
#define RADIX_MCE_GLB_CTRL_CGE        (0x1<<2)
#define RADIX_MCE_GLB_CTRL_WM         (0x1<<1)
#define RADIX_MCE_GLB_CTRL_INIT       (0x1<<0)

//COMP_CTRL
#define RADIX_REG_MCE_COMP_CTRL       0x0010
#define RADIX_MCE_COMP_CTRL_CCE       (0x1<<31)
#define RADIX_MCE_COMP_CTRL_CWT(a)    (((a) & 0x3)<<26)
#define RADIX_MCE_COMP_CTRL_CRR       (0x1<<25)
#define RADIX_MCE_COMP_CTRL_CIC       (0x1<<24)
#define RADIX_MCE_COMP_CTRL_CAT       (0x1<<23)
#define RADIX_MCE_COMP_CTRL_CTAP(a)   (((a) & 0x3)<<20)
#define RADIX_MCE_COMP_CTRL_CSPT(a)   (((a) & 0x3)<<18)
#define RADIX_MCE_COMP_CTRL_CSPP(a)   (((a) & 0x3)<<16)
#define RADIX_MCE_COMP_CTRL_YCE       (0x1<<15)
#define RADIX_MCE_COMP_CTRL_YWT(a)    (((a) & 0x3)<<10)
#define RADIX_MCE_COMP_CTRL_YRR       (0x1<<9)
#define RADIX_MCE_COMP_CTRL_YIC       (0x1<<8)
#define RADIX_MCE_COMP_CTRL_YAT       (0x1<<7)
#define RADIX_MCE_COMP_CTRL_YTAP(a)   (((a) & 0x3)<<4)
#define RADIX_MCE_COMP_CTRL_YSPT(a)   (((a) & 0x3)<<2)
#define RADIX_MCE_COMP_CTRL_YSPP(a)   (((a) & 0x3)<<0)

#define RADIX_MCE_WT_BIAVG            0
#define RADIX_MCE_WT_UNIWT            1
#define RADIX_MCE_WT_BIWT             2
#define RADIX_MCE_WT_IMWT             3

#define RADIX_MCE_TAP_TAP2            0
#define RADIX_MCE_TAP_TAP4            1
#define RADIX_MCE_TAP_TAP6            2
#define RADIX_MCE_TAP_TAP8            3

#define RADIX_MCE_SPT_AUTO            0
#define RADIX_MCE_SPT_SPEC            1
#define RADIX_MCE_SPT_BILI            2
#define RADIX_MCE_SPT_SYMM            3

#define RADIX_MCE_SPP_HPEL            0
#define RADIX_MCE_SPP_QPEL            1
#define RADIX_MCE_SPP_EPEL            2

//ESTI_CTRL
#define RADIX_REG_MCE_ESTI_CTRL       0x0040
#define RADIX_MCE_ESTI_CTRL_FBG(a)    (((a) & 0x1)<<28)
#define RADIX_MCE_ESTI_CTRL_BDIR(a)   (((a) & 0x1)<<27)
#define RADIX_MCE_ESTI_CTRL_CLMV      (0x1<<26)
#define RADIX_MCE_ESTI_CTRL_SCL(a)    (((a) & 0x3)<<24)
#define RADIX_MCE_ESTI_CTRL_MSS(a)    (((a) & 0xFF)<<16)
#define RADIX_MCE_ESTI_CTRL_QRL(a)    (((a) & 0x3)<<14)
#define RADIX_MCE_ESTI_CTRL_HRL(a)    (((a) & 0x3)<<12)
#define RADIX_MCE_ESTI_CTRL_RF8(a)    (((a) & 0x1)<<29)
#define RADIX_MCE_ESTI_CTRL_RF16(a)   (((a) & 0x1)<<30)
#define RADIX_MCE_ESTI_CTRL_RF32(a)   (((a) & 0x1)<<31)
#define RADIX_MCE_ESTI_CTRL_PUE_64X64 (0x1<<9)
#define RADIX_MCE_ESTI_CTRL_PUE_32X32 (0x1<<6)
#define RADIX_MCE_ESTI_CTRL_PUE_32X16 (0x1<<5)
#define RADIX_MCE_ESTI_CTRL_PUE_16X32 (0x1<<4)
#define RADIX_MCE_ESTI_CTRL_PUE_16X16 (0x1<<3)
#define RADIX_MCE_ESTI_CTRL_PUE_16X8  (0x1<<2)
#define RADIX_MCE_ESTI_CTRL_PUE_8X16  (0x1<<1)
#define RADIX_MCE_ESTI_CTRL_PUE_8X8   (0x1<<0)

//MRGI
#define RADIX_REG_MCE_MRGI            0x0044
#define RADIX_MCE_MRGI_MRGE_64X64     (0x1<<9)
#define RADIX_MCE_MRGI_MRGE_32X32     (0x1<<6)
#define RADIX_MCE_MRGI_MRGE_32X16     (0x1<<5)
#define RADIX_MCE_MRGI_MRGE_16X32     (0x1<<4)
#define RADIX_MCE_MRGI_MRGE_16X16     (0x1<<3)
#define RADIX_MCE_MRGI_MRGE_16X8      (0x1<<2)
#define RADIX_MCE_MRGI_MRGE_8X16      (0x1<<1)
#define RADIX_MCE_MRGI_MRGE_8X8       (0x1<<0)

//MVR
#define RADIX_REG_MCE_MVR             0x0048
#define RADIX_MCE_MVR_MVRY(a)         (((a) & 0xFFFF)<<16)
#define RADIX_MCE_MVR_MVRX(a)         (((a) & 0xFFFF)<<0)

//FRM_SIZE
#define RADIX_REG_MCE_FRM_SIZE        0x0060
#define RADIX_MCE_FRM_SIZE_FH(a)      (((a) & 0xFFFF)<<16)
#define RADIX_MCE_FRM_SIZE_FW(a)      (((a) & 0xFFFF)<<0)

//FRM_STRD
#define RADIX_REG_MCE_FRM_STRD        0x0064
#define RADIX_MCE_FRM_STRD_STRDC(a)   (((a) & 0xFFFF)<<16)
#define RADIX_MCE_FRM_STRD_STRDY(a)   (((a) & 0xFFFF)<<0)

//SLC_SPOS
#define RADIX_REG_MCE_SLC_SPOS        0x0068
#define RADIX_MCE_SLC_SPOS_CU64Y(a)   (((a) & 0xFF)<<8)
#define RADIX_MCE_SLC_SPOS_CU64X(a)   (((a) & 0xFF)<<0)

//RLUT
#define RADIX_SLUT_MCE_RLUT(l, i)     (0x0800 + (i)*8 + (l)*0x80)

//ILUT
#define RADIX_SLUT_MCE_ILUT_Y         0x0900
#define RADIX_SLUT_MCE_ILUT_C         0x0980
#define RADIX_MCE_ILUT_INFO(fir, clip, idgl, edgl, dir,		\
			    rnd, sft, savg, srnd, sbias)	\
  ( ((fir) & 0x1)<<31 |						\
    ((clip) & 0x1)<<27 |					\
    ((idgl) & 0x1)<<26 |					\
    ((edgl) & 0x1)<<25 |					\
    ((dir) & 0x1)<<24 |						\
    ((rnd) & 0xFF)<<16 |					\
    ((sft) & 0xF)<<8 |						\
    ((savg) & 0x1)<<2 |						\
    ((srnd) & 0x1)<<1 |						\
    ((sbias) & 0x1)<<0						\
    )

//CLUT
#define RADIX_SLUT_MCE_CLUT_Y         0x0A00
#define RADIX_SLUT_MCE_CLUT_C         0x0B00
#define RADIX_MCE_CLUT_INFO(c4, c3, c2, c1)	\
  ( ((c4) & 0xFF)<<24 |				\
    ((c3) & 0xFF)<<16 |				\
    ((c2) & 0xFF)<<8 |				\
    ((c1) & 0xFF)<<0				\
    )

/*************************************************************************************
                  MD Module
*************************************************************************************/
#define RADIX_REG_MD_INIT         0x0
#define RADIX_REG_MD_CFG0         0x4
#define RADIX_REG_MD_CFG1         0x8
#define RADIX_REG_MD_CFG2         0xc
#define RADIX_REG_MD_CFG3         0x10
#define RADIX_REG_MD_CFG4         0x14
#define RADIX_REG_MD_CFG5         0x18
#define RADIX_REG_MD_SCFG0        0xC0
#define RADIX_REG_MD_SCFG1        0xC4
#define RADIX_REG_MD_SCFG2        0xC8
#define RADIX_REG_MD_SCFG3        0xCC
#define RADIX_REG_MD_RCFG0        0x100
#define RADIX_REG_MD_RCFG1        0x104
#define RADIX_REG_MD_RCFG2        0x108
#define RADIX_REG_MD_RCFG3        0x10C
/*************************************************************************************
                  BC Module
*************************************************************************************/
#define RADIX_REG_BC_CFG0         0x0
#define RADIX_REG_BC_CFG1         0x4
#define RADIX_REG_BC_STATE_BASE   0x1000//bc state start addr
/*************************************************************************************
                  SDE Module
*************************************************************************************/
#define RADIX_REG_SDE_CFG0         0x0
#define RADIX_REG_SDE_CFG1         0x4
#define RADIX_REG_SDE_BS_ADDR      0x8
#define RADIX_REG_SDE_BS_LENG      0xc
#define RADIX_REG_SDE_INIT         0x10
#define RADIX_REG_SDE_SLBL         0x24
#define RADIX_REG_SDE_STATE_BASE   0x1000//sde state start addr
/*************************************************************************************
                  IPRED Module
*************************************************************************************/
#define RADIX_REG_IPRED_CTRL       0x0
#define RADIX_REG_IPRED_BC_ST      0x4
#define RADIX_REG_IPRED_BC_INIT    0x8
#define RADIX_REG_IPRED_FRM_SIZE   0xc
#define RADIX_REG_IPRED_SLV_MODE   0x14
#define RADIX_REG_IPRED_MOD_CTRL   0x18
#define RADIX_REG_IPRED_MODE0      0x1c
#define RADIX_REG_IPRED_MODE1      0x20
#define RADIX_REG_IPRED_MODE2      0x24
/*************************************************************************************
                  TFM Module
*************************************************************************************/
#define RADIX_TFM_REG_SCTRL		(4*0x00) //0
#define RADIX_TFM_REG_SINIT		(4*0x01) //1
#define RADIX_TFM_REG_STATUS0	        (4*2)
#define RADIX_TFM_REG_STATUS1	        (4*3)
#define RADIX_TFM_REG_STATUS2	        (4*4)
#define RADIX_TFM_REG_STATUS3	        (4*5)
#define RADIX_TFM_REG_STATUS4	        (4*6)
#define RADIX_TFM_REG_STATUS5	        (4*7)
#define RADIX_TFM_REG_STATUS6	        (4*8)
#define RADIX_TFM_REG_STATUS7	        (4*9)
#define RADIX_TFM_REG_STATUS8	        (4*10)
#define RADIX_TFM_REG_STATUS9	        (4*11)
#define RADIX_TFM_REG_STATUS10	(4*12)
#define RADIX_TFM_RAM_ADDR		(4*13)
#define RADIX_TFM_RAM_EN		(4*14)
#define RADIX_TFM_MD0_CRC		(4*26)
#define RADIX_TFM_MD1_CRC		(4*27)
#define RADIX_TFM_MD2_CRC		(4*28)
#define RADIX_TFM_MD3_CRC		(4*29)
#define RADIX_TFM_MD4_CRC		(4*30)
#define RADIX_TFM_FRM_SIZE		(4*31)
/*************************************************************************************
                  STC Module
*************************************************************************************/
#define RADIX_REG_STC_CFG0         0x0
#define RADIX_REG_STC_CFG1         0x4
#define RADIX_REG_STC_SAO_MG       0x1000
#define RADIX_REG_STC_SAO_TP       0x1004
/*************************************************************************************
                  SAO Module
*************************************************************************************/
#define RADIX_REG_SAO_GLB_INFO         0x20
#define RADIX_REG_SAO_PIC_SIZE         0x28
#define RADIX_REG_SAO_SLICE_XY         0x2c
#define RADIX_REG_SAO_SLICE_INIT       0x34
/*************************************************************************************
                  DT Module
*************************************************************************************/
#define RADIX_REG_DT_INIT         0x0
#define RADIX_REG_DT_CFG0         0x4
#define RADIX_REG_DT_CFG1         0x8
/*************************************************************************************
                  VDMA Module
*************************************************************************************/
#define RADIX_VDMA_ACFG_RUN        (0x1)
#define RADIX_VDMA_ACFG_DHA(a)     (((uint32_t)(a)) & 0xFFFFFF00)

#define RADIX_VDMA_ACFG_VLD        (0x1<<31)
#define RADIX_VDMA_ACFG_TERM       (0x1<<30)
#define RADIX_VDMA_ACFG_IDX(a)     (((uint32_t)(a)) & 0xFFFFC)

#define RADIX_GEN_VDMA_ACFG(chn, reg, term, val)		\
({*chn++ = val;                                            \
*chn++ = (RADIX_VDMA_ACFG_VLD | (term) | RADIX_VDMA_ACFG_IDX(reg));  \
})

/********************************************
  Motion interpolation programable table
*********************************************/
#define RADIX_IS_SKIRT  0
#define RADIX_IS_MIRROR 1

#define RADIX_IS_BIAVG  0
#define RADIX_IS_WT1    1
#define RADIX_IS_WT2    2
#define RADIX_IS_FIXWT  3

#define RADIX_IS_ILUT0  0
#define RADIX_IS_ILUT1  2
#define RADIX_IS_EC     1

#define RADIX_IS_TCS     1
#define RADIX_NOT_TCS    0
#define RADIX_IS_SCS     1
#define RADIX_NOT_SCS    0
#define RADIX_IS_HLDGL   1
#define RADIX_NOT_HLDGL  0
#define RADIX_IS_AVSDGL  1
#define RADIX_NOT_AVSDGL 0

#define RADIX_INTP_HDIR  0
#define RADIX_INTP_VDIR  1

#define CPM_RADIX_SR(ID)		(28-3*(ID))
#define CPM_RADIX_STP(ID)		(27-3*(ID))
#define CPM_RADIX_ACK(ID)		(26-3*(ID))

  enum radix_IntpID {
    RADIX_MPEG_HPEL = 0,
    RADIX_MPEG_QPEL,
    RADIX_H264_QPEL,
    RADIX_H264_EPEL,
    RADIX_RV8_TPEL,
    RADIX_RV9_QPEL,
    RADIX_RV9_CPEL,
    RADIX_WMV2_QPEL,
    RADIX_VC1_QPEL,
    RADIX_AVS_QPEL,
    RADIX_VP6_QPEL,
    RADIX_VP8_QPEL,
    RADIX_VP8_EPEL,
    RADIX_VP8_BIL,
    RADIX_VP8_FPEL, /*full-pixel for chroma*/
    RADIX_HEVC_QPEL,
    RADIX_HEVC_EPEL,
  };

enum radix_PosID {
  RADIX_H0V0 = 0,
  RADIX_H1V0,
  RADIX_H2V0,
  RADIX_H3V0,
  RADIX_H0V1,
  RADIX_H1V1,
  RADIX_H2V1,
  RADIX_H3V1,
  RADIX_H0V2,
  RADIX_H1V2,
  RADIX_H2V2,
  RADIX_H3V2,
  RADIX_H0V3,
  RADIX_H1V3,
  RADIX_H2V3,
  RADIX_H3V3,
};

enum radix_TapTYP {
  RADIX_TAP2 = 0,
  RADIX_TAP4,
  RADIX_TAP6,
  RADIX_TAP8,
};

enum radix_SPelSFT {
  RADIX_HPEL = 0,
  RADIX_QPEL,
  RADIX_EPEL,
};

typedef struct radix_IntpFMT_t{
  char tap;
  char intp_pkg[2];
  char hldgl;
  char avsdgl;
  char intp[2];
  char intp_dir[2];
  char intp_coef[2][8];
  char intp_rnd[2];
  char intp_sft[2];
  char intp_sintp[2];
  char intp_srnd[2];
  char intp_sbias[2];
}radix_IntpFMT_t;

#if 0
const static char radix_AryFMT[] = {
  RADIX_IS_SKIRT, RADIX_IS_MIRROR, RADIX_IS_SKIRT, RADIX_IS_SKIRT,
  RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT,
  RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT,
  RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT, RADIX_IS_SKIRT,
};

const static char radix_SubPel[] = {
  RADIX_HPEL, RADIX_QPEL, RADIX_QPEL, RADIX_EPEL,
  RADIX_QPEL, RADIX_QPEL, RADIX_QPEL, RADIX_QPEL,
  RADIX_QPEL, RADIX_QPEL, RADIX_QPEL, RADIX_QPEL,
  RADIX_EPEL, RADIX_HPEL, RADIX_QPEL, RADIX_QPEL, RADIX_EPEL
};

const static radix_IntpFMT_t radix_IntpFMT[][16] = {
  {
    /************* MPEG_RADIX_HPEL ***************/
    {/*H0V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0} },
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
  },

  {
    /************* MPEG_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 1},
      {/*intp_srnd*/1, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 1},
      {/*intp_srnd*/1, 1},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1},{0},},
      {/*intp_rnd*/15, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H1V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 1},
      {/*intp_srnd*/1, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H2V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H3V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 3, -6, 20, 20, -6, 3, -1}, {-1, 3, -6, 20, 20, -6, 3, -1},},
      {/*intp_rnd*/15, 15},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/1, 1},
      {/*intp_srnd*/1, 1},
      {/*intp_sbias*/1, 1},
    },
  },

  {
    /************* H264_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_IS_SCS}, RADIX_IS_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 16},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 10},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_IS_SCS}, RADIX_IS_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 16},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 10},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 10},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 10},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H0V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H1V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_IS_SCS}, RADIX_IS_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 16},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H2V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 10},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H3V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_IS_SCS}, RADIX_IS_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 16},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/1, 1},
    },
  },

  {
    /************* H264_RADIX_EPEL ***************/
    {/*H0V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/0},
    {/*H2V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V0*/0},
    {/*H0V1*/0},
    {/*H1V1*/0},
    {/*H2V1*/0},
    {/*H3V1*/0},
    {/*H0V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V2*/0},
    {/*H2V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V2*/0},
    {/*H0V3*/0},
    {/*H1V3*/0},
    {/*H2V3*/0},
    {/*H3V3*/0},
  },

  {
    /************* RV8_TPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 12, 6, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 6, 12, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/0},
    {/*H0V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 12, 6, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 12, 6, -1, 0, 0, 0, 0}, {-1, 12, 6, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 0}, //{0,128}
      {/*intp_sft*/0, 8},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 6, 12, -1, 0, 0, 0, 0}, {-1, 12, 6, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 0}, //{0,128}
      {/*intp_sft*/0, 8},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/0},
    {/*H0V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 6, 12, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 12, 6, -1, 0, 0, 0, 0}, {-1, 6, 12, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 0}, //{0,128}
      {/*intp_sft*/0, 8},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 6, 12, -1, 0, 0, 0, 0}, {-1, 6, 12, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 0}, //{0,128}
      {/*intp_sft*/0, 8},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/0},
    {/*H0V3*/0},
    {/*H1V3*/0},
    {/*H2V3*/0},
    {/*H3V3*/0},
  },

  {
    /************* RV9_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 52, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -5, 20, 52, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 52, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 52, 20, -5, 1, 0, 0}, {1, -5, 52, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/32, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 52, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 32},
      {/*intp_sft*/5, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 52, -5, 1, 0, 0}, {1, -5, 52, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/32, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/16, 0},
      {/*intp_sft*/5, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 52, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/32, 16},
      {/*intp_sft*/6, 5},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 16},
      {/*intp_sft*/5, 5},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 52, -5, 1, 0, 0}, {1, -5, 20, 20, -5, 1, 0, 0},},
      {/*intp_rnd*/32, 16},
      {/*intp_sft*/6, 5},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -5, 20, 52, -5, 1, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 52, 20, -5, 1, 0, 0}, {1, -5, 20, 52, -5, 1, 0, 0},},
      {/*intp_rnd*/32, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -5, 20, 20, -5, 1, 0, 0}, {1, -5, 20, 52, -5, 1, 0, 0},},
      {/*intp_rnd*/16, 32},
      {/*intp_sft*/5, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 2},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* RV9_CPEL ***************/
    {/*H0V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0}, {3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 7},
      {/*intp_sft*/0, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 4},
      {/*intp_sft*/0, 3},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0}, {3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 7},
      {/*intp_sft*/0, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 4},
      {/*intp_sft*/0, 3},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 1},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 4},
      {/*intp_sft*/0, 3},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0}, {1, 3, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 7},
      {/*intp_sft*/0, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 3, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 4},
      {/*intp_sft*/0, 3},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 1},
      {/*intp_sft*/0, 2},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* WMV2_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/1, 0},
      {/*intp_srnd*/1, 0},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V1*/0},
    {/*H1V1*/0},
    {/*H2V1*/0},
    {/*H3V1*/0},
    {/*H0V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 8},
      {/*intp_sft*/4, 4},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 8},
      {/*intp_sft*/4, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/8, 8},
      {/*intp_sft*/4, 4},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H0V3*/0},
    {/*H1V3*/0},
    {/*H2V3*/0},
    {/*H3V3*/0},
  },

  {
    /************* VC1_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-4, 53, 18, -3, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/8, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-3, 18, 53, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-4, 53, 18, -3, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/31, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-4, 53, 18, -3, 0, 0, 0, 0}, {-4, 53, 18, -3, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-4, 53, 18, -3, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 8},
      {/*intp_sft*/6, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-4, 53, 18, -3, 0, 0, 0, 0}, {-3, 18, 53, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/7, 0},
      {/*intp_sft*/4, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-4, 53, 18, -3, 0, 0, 0, 0},},
      {/*intp_rnd*/7, 32},
      {/*intp_sft*/4, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/7, 8},
      {/*intp_sft*/4, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-1, 9, 9, -1, 0, 0, 0, 0}, {-3, 18, 53, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/7, 32},
      {/*intp_sft*/4, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-3, 18, 53, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/31, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-3, 18, 53, -4, 0, 0, 0, 0}, {-4, 53, 18, -3, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-3, 18, 53, -4, 0, 0, 0, 0}, {-1, 9, 9, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 8},
      {/*intp_sft*/6, 4},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_IS_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_VDIR, RADIX_INTP_HDIR},
      {/*intp_coef*/{-3, 18, 53, -4, 0, 0, 0, 0}, {-3, 18, 53, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/31, 32},
      {/*intp_sft*/6, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* AVS_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, -2, 96, 42, -7, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{0, -7, 42, 96, -2, -1, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, -2, 96, 42, -7, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_IS_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0}, {-1, 5, -5, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, -1, 5, 5, -1, 0, 0, 0}, {-1, -2, 96, 42, -7, 0, 0, 0},},
      {/*intp_rnd*/64, 0}, //{0,512}
      {/*intp_sft*/0, 10},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_IS_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0}, {-1, 5, -5, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/1, 0},
    },
    {/*H0V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, -2, 96, 42, -7, 0, 0, 0}, {0, -1, 5, 5, -1, 0, 0, 0},},
      {/*intp_rnd*/64, 0}, //{0,512}
      {/*intp_sft*/0, 10},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0}, {-1, 5, 5, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, -7, 42, 96, -2, -1, 0, 0}, {0, -1, 5, 5, -1, 0, 0, 0},},
      {/*intp_rnd*/64, 0}, //{0,512}
      {/*intp_sft*/0, 10},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{0, -7, 42, 96, -2, -1, 0, 0}, {0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_IS_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0}, {-1, 5, -5, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/0, 1},
    },
    {/*H2V3*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, -1, 5, 5, -1, 0, 0, 0}, {0, -7, 42, 96, -2, -1, 0, 0},},
      {/*intp_rnd*/64, 0}, //{0,512}
      {/*intp_sft*/0, 10},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_IS_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 5, 5, -1, 0, 0, 0, 0}, {-1, 5, -5, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/0, 32},
      {/*intp_sft*/0, 6},
      {/*intp_sintp*/0, 1},
      {/*intp_srnd*/0, 1},
      {/*intp_sbias*/1, 1},
    },
  },

  {
    /************* VP6_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-4, 109, 24, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-4, 68, 68, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 24, 109, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-4, 109, 24, -1, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 109, 24, -1, 0, 0, 0, 0}, {-4, 109, 24, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 68, 68, -4, 0, 0, 0, 0}, {-4, 109, 24, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 24, 109, -4, 0, 0, 0, 0}, {-4, 109, 24, -1, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-4, 68, 68, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 109, 24, -1, 0, 0, 0, 0}, {-4, 68, 68, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 68, 68, -4, 0, 0, 0, 0}, {-4, 68, 68, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 24, 109, -4, 0, 0, 0, 0}, {-4, 68, 68, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 24, 109, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 109, 24, -1, 0, 0, 0, 0}, {-1, 24, 109, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 68, 68, -4, 0, 0, 0, 0}, {-1, 24, 109, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP4, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 24, 109, -4, 0, 0, 0, 0}, {-1, 24, 109, -4, 0, 0, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* VP8_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP6, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{2, -11, 108, 36, -8, 1, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{3, -16, 77, 77, -16, 3, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, -8, 36, 108, -11, 2, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{2, -11, 108, 36, -8, 1, 0, 0}, {0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{2, -11, 108, 36, -8, 1, 0, 0}, {2, -11, 108, 36, -8, 1, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, -16, 77, 77, -16, 3, 0, 0}, {2, -11, 108, 36, -8, 1, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -8, 36, 108, -11, 2, 0, 0}, {2, -11, 108, 36, -8, 1, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{3, -16, 77, 77, -16, 3, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{2, -11, 108, 36, -8, 1, 0, 0}, {3, -16, 77, 77, -16, 3, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, -16, 77, 77, -16, 3, 0, 0}, {3, -16, 77, 77, -16, 3, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -8, 36, 108, -11, 2, 0, 0}, {3, -16, 77, 77, -16, 3, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, -8, 36, 108, -11, 2, 0, 0},{0},},
      {/*intp_rnd*/64, 0},
      {/*intp_sft*/7, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{2, -11, 108, 36, -8, 1, 0, 0}, {1, -8, 36, 108, -11, 2, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, -16, 77, 77, -16, 3, 0, 0}, {1, -8, 36, 108, -11, 2, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP6, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, -8, 36, 108, -11, 2, 0, 0}, {1, -8, 36, 108, -11, 2, 0, 0},},
      {/*intp_rnd*/64, 64},
      {/*intp_sft*/7, 7},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* VP8_RADIX_EPEL ***************/
    {/*H0V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/0},
    {/*H2V0*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V0*/0},
    {/*H0V1*/0},
    {/*H1V1*/0},
    {/*H2V1*/0},
    {/*H3V1*/0},
    {/*H0V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/4, 0},
      {/*intp_sft*/3, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V2*/0},
    {/*H2V2*/
      RADIX_TAP2, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/4, 4},
      {/*intp_sft*/3, 3},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V2*/0},
    {/*H0V3*/0},
    {/*H1V3*/0},
    {/*H2V3*/0},
    {/*H3V3*/0},
  },

  {
    /************* VP8_BIL ***************/
    {/*H0V0*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V0*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V0*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V1*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V1*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 2},
      {/*intp_sft*/2, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V1*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/1, 2},
      {/*intp_sft*/1, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V1*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{3, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 2},
      {/*intp_sft*/2, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V2*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/1, 0},
      {/*intp_sft*/1, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V2*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 1},
      {/*intp_sft*/2, 1},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V2*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/1, 1},
      {/*intp_sft*/1, 1},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V2*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{1, 1, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 1},
      {/*intp_sft*/2, 1},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H0V3*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/2, 0},
      {/*intp_sft*/2, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V3*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{3, 1, 0, 0, 0, 0, 0, 0},{1, 3, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 2},
      {/*intp_sft*/2, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H2V3*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 1, 0, 0, 0, 0, 0, 0},{1, 3, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/1, 2},
      {/*intp_sft*/1, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H3V3*/
      RADIX_TAP2, {RADIX_IS_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{1, 3, 0, 0, 0, 0, 0, 0},{1, 3, 0, 0, 0, 0, 0, 0},},
      {/*intp_rnd*/2, 2},
      {/*intp_sft*/2, 2},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
  },

  {
    /************* VP8_FPEL ***************/
    {/*H0V0*/0},
    {/*H1V0*/0},
    {/*H2V0*/0},
    {/*H3V0*/0},
    {/*H0V1*/0},
    {/*H1V1*/0},
    {/*H2V1*/0},
    {/*H3V1*/0},
    {/*H0V2*/0},
    {/*H1V2*/0},
    {/*H2V2*/0},
    {/*H3V2*/0},
    {/*H0V3*/0},
    {/*H1V3*/0},
    {/*H2V3*/0},
    {/*H3V3*/0},
  },

  {
    /************* HEVC_RADIX_QPEL ***************/
    {/*H0V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 4, -10, 58, 17, -5, 1, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{-1, 4, -11, 40, 40, -11, 4, -1},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, 0},
      {/*intp_coef*/{0, 1, -5, 17, 58, -10, 4, -1},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 4, -10, 58, 17, -5, 1, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -10, 58, 17, -5, 1, 0},{-1, 4, -10, 58, 17, -5, 1, 0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -11, 40, 40, -11, 4, -1},{-1, 4, -10, 58, 17, -5, 1, 0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V1*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, 1, -5, 17, 58, -10, 4, -1},{-1, 4, -10, 58, 17, -5, 1, 0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{-1, 4, -11, 40, 40, -11, 4, -1},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -10, 58, 17, -5, 1, 0},{-1, 4, -11, 40, 40, -11, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -11, 40, 40, -11, 4, -1},{-1, 4, -11, 40, 40, -11, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, 1, -5, 17, 58, -10, 4, -1},{-1, 4, -11, 40, 40, -11, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_VDIR, 0},
      {/*intp_coef*/{0, 1, -5, 17, 58, -10, 4, -1},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -10, 58, 17, -5, 1, 0},{0, 1, -5, 17, 58, -10, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-1, 4, -11, 40, 40, -11, 4, -1},{0, 1, -5, 17, 58, -10, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP8, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 1}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{0, 1, -5, 17, 58, -10, 4, -1},{0, 1, -5, 17, 58, -10, 4, -1},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/0, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },

  {
    /************* HEVC_RADIX_EPEL ***************/
    {/*H0V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0},
      {/*intp_coef*/{1, 0, 0, 0, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0},
      {/*intp_srnd*/0},
      {/*intp_sbias*/0},
    },
    {/*H1V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-2, 58, 10, -2, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 54, 16, -2, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-6, 46, 28, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H4V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 36, 36, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H5V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-4, 28, 46, -6, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H6V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-2, 16, 54, -4, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H7V0*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/1, 0}, {RADIX_INTP_HDIR, RADIX_INTP_VDIR},
      {/*intp_coef*/{-2, 10, 58, -2, 0, 0, 0, 0},{0},},
      {/*intp_rnd*/32, 0},
      {/*intp_sft*/6, 12},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V2*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H0V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H1V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H2V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
    {/*H3V3*/
      RADIX_TAP4, {RADIX_NOT_TCS, RADIX_NOT_SCS}, RADIX_NOT_HLDGL, RADIX_NOT_AVSDGL,
      {/*intp*/0, 0}, {0, 0},
      {/*intp_coef*/{0},{0},},
      {/*intp_rnd*/0, 0},
      {/*intp_sft*/0, 0},
      {/*intp_sintp*/0, 0},
      {/*intp_srnd*/0, 0},
      {/*intp_sbias*/0, 0},
    },
  },
};
#endif

#endif // __RADIX_H__
