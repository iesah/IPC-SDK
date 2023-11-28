/*----------------------------------------------+
 |
 |	ivDefine.h - Basic Definitions
 |
 |		Copyright (c) 1999-2013, iFLYTEK Ltd.
 |		All rights reserved.
 |
 +----------------------------------------------*/

#ifndef IFLYTEK_VOICE__DEFINE__H
#define IFLYTEK_VOICE__DEFINE__H


#include "ivPlatform.h"

#define IV_CHECK_RES_READ		1			/* �Ƿ����û��ص�������ȡ��Դ�ĳɹ��� */
/*
 *	���η�
 */

#define ivConst		IV_CONST
#define ivStatic	IV_STATIC
#ifdef IV_INLINE
#define ivInline	IV_STATIC IV_INLINE
#else
#define ivInline	IV_STATIC
#endif
#define ivExtern	IV_EXTERN

#define ivPtr		IV_PTR_PREFIX*
#define ivCPtr		ivConst ivPtr
#define ivPtrC		ivPtr ivConst
#define ivCPtrC		ivConst ivPtr ivConst

#define ivCall		IV_CALL_STANDARD	/* �ǵݹ��(���������) */
#define ivReentrant	IV_CALL_REENTRANT /* �ݹ��(�������)*/
#define ivVACall	IV_CALL_VAR_ARG 	/* ֧�ֱ�ε� */

 #define ivProc		ivCall ivPtr


/*
 *	����
 */

#define	ivNull	(0)

/* �������ͼ���ֵ(�����ƽ̨����) */
typedef	int ivBool;

#define	ivTrue	(~0)
#define	ivFalse	(0)

/* �Ա����ͼ���ֵ */
typedef int ivComp;

#define ivGreater	(1)
#define ivEqual		(0)
#define ivLesser	(-1)

#define ivIsGreater(v)	((v)>0)
#define ivIsEqual(v)	(0==(v))
#define ivIsLesser(v)	((v)<0)

/*
 *	��������
 */

/* ����ֵ���� */
typedef	signed IV_TYPE_INT8		ivInt8;	/* 8-bit */
typedef	unsigned IV_TYPE_INT8	ivUInt8;	/* 8-bit */

typedef	signed IV_TYPE_INT16	ivInt16;	/* 16-bit */
typedef	unsigned IV_TYPE_INT16	ivUInt16;	/* 16-bit */

typedef	signed IV_TYPE_INT32	ivInt32;	/* 32-bit */
typedef	unsigned IV_TYPE_INT32	ivUInt32;	/* 32-bit */

typedef ivInt32		ivStatus;

#ifdef IV_TYPE_INT48
typedef	signed IV_TYPE_INT48	ivInt48;	/* 48-bit */
typedef	unsigned IV_TYPE_INT48	ivUInt48;	/* 48-bit */
#endif

#ifdef IV_TYPE_INT64
typedef	signed IV_TYPE_INT64	ivInt64;	/* 64-bit */
typedef	unsigned IV_TYPE_INT64	ivUInt64;	/* 64-bit */
#endif


typedef	signed IV_TYPE_INT24	ivInt24;	/* 24-bit */
typedef	unsigned IV_TYPE_INT24	ivUInt24;	/* 24-bit */

/* ��Ӧ��ָ������ */
typedef	ivInt8		ivPtr ivPInt8;		/* 8-bit */
typedef	ivUInt8		ivPtr ivPUInt8;		/* 8-bit */

typedef	ivInt16		ivPtr ivPInt16;		/* 16-bit */
typedef	ivUInt16	ivPtr ivPUInt16;	/* 16-bit */

typedef	ivInt32		ivPtr ivPInt32;		/* 32-bit */
typedef	ivUInt32	ivPtr ivPUInt32;	/* 32-bit */

#ifdef IV_TYPE_INT48
typedef	ivInt48		ivPtr ivPInt48;		/* 48-bit */
typedef	ivUInt48	ivPtr ivPUInt48;	/* 48-bit */
#endif

#ifdef IV_TYPE_INT64
typedef	ivInt64		ivPtr ivPInt64;		/* 64-bit */
typedef	ivUInt64	ivPtr ivPUInt64;	/* 64-bit */
#endif



/* ����ָ������ */
 typedef	ivInt8		ivCPtr ivPCInt8; 		/* 8-bit */
 typedef	ivUInt8		ivCPtr ivPCUInt8; 	/* 8-bit */

typedef	ivInt16		ivCPtr ivPCInt16;	/* 16-bit */
typedef	ivUInt16	ivCPtr ivPCUInt16;	/* 16-bit */


typedef	ivInt32		ivCPtr ivPCInt32;	/* 32-bit */
typedef	ivUInt32	ivCPtr ivPCUInt32;	/* 32-bit */

#ifdef IV_TYPE_INT48
typedef	ivInt48		ivCPtr ivPCInt48;	/* 48-bit */
typedef	ivUInt48	ivCPtr ivPCUInt48;	/* 48-bit */
#endif

#ifdef IV_TYPE_INT64
typedef	ivInt64		ivCPtr ivPCInt64;	/* 64-bit */
typedef	ivUInt64	ivCPtr ivPCUInt64;	/* 64-bit */
#endif

/* �߽�ֵ���� */
#define IV_SBYTE_MAX	(+127)
#define IV_MAX_INT16	(+32767)
#define IV_MAX_UINT16	(+65535)
#define IV_INT_MAX		(+8388607L)
#define IV_MAX_INT32	(+2147483647L)

#define IV_SBYTE_MIN	(-IV_SBYTE_MAX - 1)
#define IV_MIN_INT16	(-IV_MAX_INT16 - 1)
#define IV_INT_MIN		(-IV_INT_MAX - 1)
#define IV_MIN_INT32	(-IV_MAX_INT32 - 1)


#define IV_BYTE_MAX		(0xffU)
#define IV_USHORT_MAX	(0xffffU)
#define IV_UINT_MAX		(0xffffffUL)
#define IV_ULONG_MAX	(0xffffffffUL)
/*
*	�ַ���ض���
*/

/* �ַ����Ͷ��� */
typedef ivInt8		ivCharA;
typedef ivUInt16	ivCharW;

#if IV_UNICODE
typedef ivCharW		ivChar;
#else
typedef ivCharA		ivChar;
#endif

/* �ַ������Ͷ��� */
typedef ivCharA ivPtr	ivStrA;
typedef ivCharA ivCPtr	ivCStrA;

typedef ivCharW ivPtr	ivStrW;
typedef ivCharW ivCPtr	ivCStrW;

typedef ivChar ivPtr	ivStr;
typedef ivChar ivCPtr	ivCStr;

/* �ڴ������Ԫ */
 typedef	ivUInt8		ivByte;
 typedef	ivPUInt8	ivPByte;
 typedef	ivPCUInt8	ivPCByte;

typedef	signed			ivInt;
typedef	signed ivPtr	ivPInt;
typedef	signed ivCPtr	ivPCInt;

typedef	unsigned		ivUInt;
typedef	unsigned ivPtr	ivPUInt;
typedef	unsigned ivCPtr	ivPCUInt;

typedef float ivFloat;
typedef float ivPtr ivPFloat;

/* ָ�� */
typedef	void ivPtr	ivPointer;
typedef	void ivCPtr	ivCPointer;

typedef void ivVoid;

/* ��� */
typedef ivPointer ivHandle;

/* ��ַ���ߴ����� */
typedef	IV_TYPE_SIZE	ivSize;
typedef	IV_TYPE_ADDRESS	ivAddress;

#define ivProc		ivCall ivPtr

/*
*	��Դ��ַ���ߴ����ͼ��������ʹ�С����
*/

typedef ivUInt32 ivResAddress;
typedef ivUInt32 ivResSize;

#define ivAssert(f)			IV_ASSERT(f)
#define	Assert_Int16(x)		ivAssert( ( (x) >= -32768 ) && ( (x) <= 32767 ) )

/* �ı������� */
#define ivTextA(s)	((ivCStrA)s)
#define ivTextW(s)	((ivCStrW)L##s)

#define ivResSize_Int8		1
#define ivResSize_Int16		2
#define ivResSize_Int32		4

/* read resource callback type */
#if  IV_UNIT_BITS == 16

/* #if IV_CHECK_RES_READ */
typedef ivBool (ivProc ivCBReadResExt)(
								  ivPointer		pParameter,			/* [in] user callback parameter */
								  ivPointer		pBuffer,			/* [out] read resource buffer */
								  ivResAddress	iPos,				/* [in] read start position */
								  ivResSize		nSize,				/* [in] read size */
	ivSize			nCount );			/* [in] read count */
/* #else */
typedef void (ivProc ivCBReadRes)(
								  ivPointer		pParameter,			/* [in] user callback parameter */
								  ivPointer		pBuffer,			/* [out] read resource buffer */
								  ivResAddress	iPos,				/* [in] read start position */
								  ivResSize		nSize,				/* [in] read size */
	ivSize			nCount );			/* [in] read count */
/* #endif */

#else

/* #if IV_CHECK_RES_READ */
typedef ivBool (ivProc ivCBReadResExt)(
								  ivPointer		pParameter,			/* [in] user callback parameter */
								  ivPointer		pBuffer,			/* [out] read resource buffer */
								  ivResAddress	iPos,				/* [in] read start position */
	ivResSize		nSize );			/* [in] read size */
/* #else */
typedef void (ivProc ivCBReadRes)(
	ivPointer		pParameter,			/* [in] user callback parameter */
	ivPointer		pBuffer,			/* [out] read resource buffer */
	ivResAddress	iPos,				/* [in] read start position */
	ivResSize		nSize );			/* [in] read size */
/* #endif */

#endif

/* map resource callback type */
typedef ivCPointer (ivProc ivCBMapRes)(
	ivPointer		pParameter,			/* [in] user callback parameter */
	ivResAddress	iPos,				/* [in] map start position */
	ivResSize		nSize );			/* [in] map size */

/* log callback type */
typedef ivUInt16 (ivProc ivCBLogExt)
(
 ivPointer	pParameter,			/* [out] user callback parameter */
 ivCPointer	pcData,				/* [in] output data buffer */
 ivSize		nSize				/* [in] output data size */
 );


/* resource pack description */
typedef struct tagResPackDescExt ivTResPackDescExt, ivPtr ivPResPackDescExt;

struct tagResPackDescExt
{
	ivPointer		pCBParam;			/* resource callback parameter */
	ivCBReadResExt	pfnRead;			/* read resource callback entry */
	ivCBMapRes		pfnMap;				/* map resource callback entry (optional) */
	ivResSize		nSize;				/* resource pack size (optional, 0 for null) */

	ivPUInt8		pCacheBlockIndex;	/* cache block index (optional, size = dwCacheBlockCount) */
	ivPointer		pCacheBuffer;		/* cache buffer (optional, size = dwCacheBlockSize * dwCacheBlockCount) */
	ivSize			nCacheBlockSize;	/* cache block size (optional, must be 2^N) */
	ivSize			nCacheBlockCount;	/* cache block count (optional, must be 2^N) */
	ivSize			nCacheBlockExt;		/* cache block ext (optional) */
};


/* ESR resource pack description */
typedef struct tagResPackDesc ivTResPackDesc, ivPtr ivPResPackDesc;

struct tagResPackDesc
{
	ivPointer		pCBParam;			/* resource callback parameter */
	ivCBReadRes		pfnRead;			/* read resource callback entry */
	ivCBMapRes		pfnMap;				/* map resource callback entry (optional) */
	ivResSize		nSize;				/* resource pack size (optional, 0 for null) */

	ivPUInt8		pCacheBlockIndex;	/* cache block index (optional, size = dwCacheBlockCount) */
	ivPointer		pCacheBuffer;		/* cache buffer (optional, size = dwCacheBlockSize * dwCacheBlockCount) */
	ivSize			nCacheBlockSize;	/* cache block size (optional, must be 2^N) */
	ivSize			nCacheBlockCount;	/* cache block count (optional, must be 2^N) */
	ivSize			nCacheBlockExt;		/* cache block ext (optional) */
};







/* Save data callback type */
typedef ivBool (ivProc ivCBSaveData)(
	ivPointer		pUserParam,
	ivPointer		pSrcBuffer,
	ivSize			nBufferBytes
	);

/* Load data callback type */
typedef ivBool (ivProc ivCBLoadData)(
	ivPointer		pUserParam,
	ivPointer		pDstBuffer,
	ivSize ivPtr	pnBufferBytes
	);

/* LOG output callback type */
typedef void (ivProc ivCBLog)(
	ivPointer		pUserParam,
	ivCPointer		pLogData,
	ivSize			nBytes
	);


typedef ivPointer (ivProc ivCBRealloc)(
	ivPointer	pUserParam,
	ivPointer	pMemblock,
	ivSize		nBytes
	);

typedef void (ivProc ivCBFree)(
	ivPointer	pUserParam,
	ivPointer	pBuffer
	);

typedef ivBool (ivProc ivCBStartRecord)(
	ivPointer	pUserParam
	);

typedef ivBool (ivProc ivCBStopRecord)(
	ivPointer	pUserParam
	);

typedef struct tagUserSys
{
	ivPointer	pWorkBuffer;
	ivSize		nWorkBufferBytes;

	ivPointer	pResidentBuffer;
	ivSize		nResidentBufferBytes;

	ivBool		bCheckResource;

	ivPointer		pCBParam;

	ivCBSaveData	pfnSaveData;
	ivCBLoadData	pfnLoadData;

	ivCBRealloc		pfnRealloc;
	ivCBFree		pfnFree;

	ivCBStartRecord pfnStartRecord;
	ivCBStopRecord	pfnStopRecord;

	ivCBLog			pfnLog;
}TUserSys, ivPtr PUserSys, ivPtr ivPUserSys;



#endif /* !IFLYTEK_VOICE__DEFINE__H */
