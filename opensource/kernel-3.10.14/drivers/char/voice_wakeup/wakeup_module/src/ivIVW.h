/*----------------------------------------------+
 |
 |		ivIVW.h - LowEnd API
 |
 |		Copyright (c) 1999-2018, iFLYTEK Ltd.
 |		All rights reserved.
 |
 +----------------------------------------------*/

#if !defined(IVW_TEAM__2014_03_04__IVW__H)
#define IVW_TEAM__2014_03_04__IVW__H

#include "ivIvwDefine.h"
#include "ivPlatform.h"
#include "ivIvwErrorCode.h"
#include "ivwConfig.h"

/* Definition of IVW parameters and parameter value */
/* Parameter ID for Ivw WakeUp Threshold*/
#define IVW_CM_THRESHOLD            (2011)
	#define CM_THRESH_DEFALT	    ((ivUInt32)0)

#define IVW_SNR_ON                  (2012)
    #define CM_SNR_ON_DEFALT	    ((ivUInt32)1)

/* --------------����ȼ����ز���.Begin----------------------------- */
#define IVW_SNR_THRESH              (2013) /* ��������޷�ֵ���� */
    #define IVW_SNR_THRESH_DEFAULT      ((ivUInt32)105) /* �����Ĭ������ */

#define IVW_SNR_SKIPNOISE_MS        (2014) /* �����������ĺ����� */
    #define IVW_SNR_SKIPNOISE_MS_DEFAULT    ((ivUInt32)0)

#define IVW_SNR_NOISE_MS            (2015) /* �����κ����� */
    #define IVW_SNR_NOISE_MS_DEFAULT        ((ivUInt32)400)
    #define IVW_SNR_NOISE_MS_MIN            ((ivUInt32)100) /* ���ڼ�����������С�����������С�ڸ�ֵ���򲻽��м�� */

#define IVW_SNR_CMTHR_DIF           (2016) /*  */
    #define IVW_SNR_CMTHR_DIF_DEFAULT       ((ivUInt32)100)  /* �����Ѻ�CM�÷ֺ��û��趨��CM��ֵ�����20����,���������ȼ�� */

#define IVW_SNR_ENDSIL_MS           (2017) /*  */
    #define IVW_SNR_ENDSIL_MS_DEFAULT       ((ivUInt32)30) /* ���û��Ѵʺ����ӵľ����ε�ʱ��.��λ:���� */
/* --------------����ȼ����ز���.End----------------------------- */

#define IVW_KEYWORD_MAX_MS          (2018)  /* ����֧�ֵĻ��Ѵʵ����ʱ��.��λ:���� */
    #define IVW_KEYWORD_MAX_MS_DEFAULT      ((ivUInt32)4000)

#define IVW_KEYWORD_MIN_MS          (2019)  /* ����֧�ֵĻ��Ѵʵ����ʱ��.��λ:���� */
    #define IVW_KEYWORD_MIN_MS_DEFAULT      ((ivUInt32)100)

#define IVW_DOUBLE_THRESH_INTER_MS      ((ivUInt16)5000)      /* ����˫���޵�ʱ����.��λ:���� */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Interface */
/* Create Ivw object */
ivStatus
ivCall
IvwCreate(
		  ivPointer		pObj,			    /* IVW Object */
		  ivSize ivPtr	pnObjSize,		    /* [In/Out] Size of IVW object */
		  ivPointer		pResidentRAM,		/* Resident RAM */
		  ivPUInt16		pnResidentRAMSize,	/* [In/Out] Size of Resident RAM */
		  ivCPointer	pWakeupRes,			/* [In]  key Resource */
		  ivUInt16		nWakeupNetworkID	/* [In] resource file network ID */
		  );

ivStatus
ivCall
IvwCreateEx(
          ivPointer		pObj,			/* IVW Object */
          ivSize ivPtr	pnObjSize,		/* [In/Out] Size of IVW object */
          ivPointer		pResidentRAM,		/* Resident RAM */
          ivPUInt16		pnResidentRAMSize,	/* [In/Out] Size of Resident RAM */
          ivCPointer ivPtr	ppWakeupRes,	/* [In] wakeup Resource.support for multiple */
          ivUInt16      nWakeupResNum       /* [In] wakeup resource number */
          );

ivStatus
ivCall
IvwSetParam(
			ivPointer	pObj,           /* IVW Object */
			ivUInt32	nParamID,		/* Parameter ID */
			ivInt32		nParamValue,	/* Parameter Value */
			ivUInt32	iKeyWordID,		/* valid when nParamID=IVW_CM_THRESHOLD. Keyword ID. start with 0. */
            ivUInt32    iResID          /* valid when nParamID=IVW_CM_THRESHOLD. Res ID. start with 0. */
			);

ivStatus
ivCall
IvwSetThresh(
             ivPointer  pObj,           /* IVW Object */
             ivInt16    nLThresh,       /* Param: lower cm thresh */
             ivInt16    nHThresh,       /* Param: higher cm thresh */
             ivUInt16   nIntervalMS,    /* Param: Double thresh interval time(millisecond)*/
             ivUInt32   iKeyWordID,     /* Keyword ID */
             ivUInt32   iResID          /* Res ID. start with 0. */
             );


ivStatus
ivCall
IvwRunStep(
		   ivPointer	pObj,	    /* IVW Object */
		   ivPInt16		pnCMScore ,  /* IVW CMScore */
		   ivPInt16		pnKeywordID /* IVW multi wakeup keyword ID*/
		   );

ivStatus
ivCall
IvwRunStepEx(
           ivPointer	pObj,           /* IVW Object */
           ivPInt16		pnCMScore ,     /* [out].IVW CMScore */
           ivPInt16     pnResID,        /* [out] */
           ivPInt16		pnKeywordID,    /* []IVW multi wakeup keyword ID*/
           ivPUInt32    pnStartMS,
           ivPUInt32    pnEndMS
           );

/* Append Audio data to the Ivw object,In general, Call this function in record thread */
ivStatus
ivCall
IvwAppendAudioData(
			  ivPointer			pObj,	/* IVW Object */
			  ivCPointer		pData,		/* [In] Pointer to the address of PCM data buffer */
			  ivUInt16			nSamples	/* [In] Specifies the length, in samples, of PCM data */
			  );

/* Reset object */
ivStatus
ivCall
IvwReset(
		   ivPointer	pObj		/* IVW Object */
		   );

/* Get SDK Version */
ivStatus
ivCall
IvwGetVersion(
              ivPUInt16		piMajor,
              ivPUInt16		piMinor,
              ivPUInt16		piRevision
              );

#if IVW_SUPPORT_ENROLLVP
/* Create VP object */
ivStatus
ivCall
IvwCreateVPObj(
               ivPointer    pObj,
               ivPUInt32    pnObjSize,
               ivCPointer   pUBMMdl
               );

/* Start one process of enroll */
ivStatus
ivCall
IvwStartEnroll(
               ivPointer   pObj
               );

ivStatus
ivCall
IvwEnrollStep(
              ivPointer	    pObj,
              ivPInt16		pnCMScore,
              ivPointer*    ppWakeupRes,
              ivPUInt32     pnWakeupResSize,
              ivPUInt32     pnStartMS,
              ivPUInt32     pnEndMS,
              ivPInt16		pnVpThresh
              );

ivStatus
ivCall
IvwMergeRes(
            ivCPointer	pRes1,		    /* I: ���ϲ�������Դ1 */
            ivCPointer  pRes2,			/* I: ���ϲ�������Դ2 */
            ivPointer	pMergeRes,		/* O: �ϲ���Ļ�����Դ */
            ivPUInt32   pnMergeResSize	/* I/O: ����pMergeRes��С�����ʵ��ʹ�õ�pMergeRes��С.(��λ���ֽ�) */
            );

#endif /* #if IVW_SUPPORT_ENROLLVP */

#if IVW_SUPPORT_VPMODEL_ADAPT
ivStatus
ivCall
IvwCreateVPMdlAdaptObj(
               ivPointer    pObj,
               ivPUInt32    pnObjSize,
               ivCPointer   pUBMMdl,
               ivCPointer   pWakeupRes,
               ivInt16      nKeywordID,
               ivBool       bEnableVAD
               );

ivStatus
ivCall
IvwVPMdlAdaptRunStep(
              ivPointer	    pObj,
              ivPInt16      pnCMScore,
              ivPUInt32     pnStartMS,
              ivPUInt32     pnEndMS,
              ivPointer *   ppAdaptRes,
              ivPUInt32     pnAdaptResSize
              );
#endif /* #if IVW_SUPPORT_VPMODEL_ADAPT */


/* ǿ���ƻ��Ѵ�3���Զ����²��� */
#if IVW_SUPPORT_MODEL_ADAPT
ivStatus
ivCall
IvwCreateMdlAdaptObj(
                       ivPointer    pObj,
                       ivPUInt32    pnObjSize,
                       ivCPointer   pWakeupRes,
                       ivInt16      nKeywordID, /* Begin index:0 */
                       ivUInt16     nAdaptTimes,
                       ivBool       bEnableVAD
                       );

ivStatus
ivCall
IvwStartAdapt(
               ivPointer   pObj
               );

ivStatus
ivCall
IvwMdlAdaptRunStep(
                   ivPointer                pObj,
                   PMdlAdaptResult ivPtr    ppResult		/* [Out] To Receive model adapt result */
                   );
#endif /* #if IVW_SUPPORT_MODEL_ADAPT */

#if IVW_SUPPORT_ENROLLVP || IVW_SUPPORT_MODEL_ADAPT
ivStatus
ivCall
IvwEndData(
           ivPointer	pObj		/* IVW Object */
           );
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !defined(IVW_TEAM__2014_03_04__IVW__H) */
