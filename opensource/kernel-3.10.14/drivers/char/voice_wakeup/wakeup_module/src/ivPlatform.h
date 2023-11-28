/*----------------------------------------------+
 |
 |	ivPlatform.h - Platform Config
 |
 |		Copyright (c) 1999-2013, iFLYTEK Ltd.
 |		All rights reserved.
 |
 +----------------------------------------------*/


/*
 *	TODO: ���������Ŀ��ƽ̨������Ҫ�Ĺ���ͷ�ļ�
 */


#ifdef _WIN32
#include <stdio.h>
#include <crtdbg.h>
#endif


/*
 *	TODO: ����Ŀ��ƽ̨�����޸����������ѡ��
 */

#define IV_PTR_GRID				4			/* ���ָ�����ֵ */
#define IV_UNIT_BITS            8			/* �ڴ������Ԫλ�� */
//#define IV_BIG_ENDIAN			0			/* �Ƿ��� Big-Endian �ֽ��� */

#define IV_CSI                  (0)			/* ��ʤѶ16λƽ̨ */

#define IV_PTR_PREFIX						/* ָ�����ιؼ���(����ȡֵ�� near | far, ����Ϊ��) */
#define IV_CONST				const		/* �����ؼ���(����Ϊ��) */
#define IV_EXTERN				extern		/* �ⲿ�ؼ��� */
#define IV_STATIC				static		/* ��̬�����ؼ���(����Ϊ��) */
#define IV_INLINE				//__inline	/* �����ؼ���(����ȡֵ�� inline, ����Ϊ��) */
#define IV_CALL_STANDARD		//__stdcall	/* ��ͨ�������ιؼ���(����ȡֵ�� stdcall | fastcall | pascal, ����Ϊ��) */

#define IV_CALL_REENTRANT					/* �ݹ麯�����ιؼ���(����ȡֵ�� stdcall | reentrant, ����Ϊ��) */
#define IV_CALL_VAR_ARG						/* ��κ������ιؼ���(����ȡֵ�� cdecl, ����Ϊ��) */

#if IV_CSI  /* ��Ϊ��ʤѶƽ̨����. ��ԴҲ��Ҫ���´����ƥ��!!!! */
#define IV_TYPE_INT8			short		/* 8λ�������� */
#else
#define IV_TYPE_INT8			char		/* 8λ�������� */
#endif

#define IV_TYPE_INT16			short		/* 16λ�������� */
#define IV_TYPE_INT24			int			/* 24λ�������� */
#define IV_TYPE_INT32			long		/* 32λ�������� */

#define IV_SIZEOF(exp)			(sizeof(exp))

#define IV_TYPE_ADDRESS			unsigned long		/* ��ַ�������� */
#define IV_TYPE_SIZE			unsigned long		/* ��С�������� */

#define IV_VOLATILE				volatile

#define IV_ANSI_MEMORY			0			/* �Ƿ�ʹ�� ANSI �ڴ������ */
#define	IV_ANSI_STRING			0			/* �Ƿ�ʹ�� ANSI �ַ��������� */

#if defined __GNUC__
#define IV_GNUC_COMPILER        1
#endif

#define IV_ARM_COMPILER			0

#if defined _MSC_VER
#define IV_WIN32_COMPILER       1
#endif

#ifdef _WIN32
#define IV_ASSERT(exp)			_ASSERT(exp)/* ���Բ���(����Ϊ��) */
#else
#define IV_ASSERT(exp)			/* ���Բ���(����Ϊ��) */
#endif

#define IV_YIELD				/* ���в���(��Э��ʽ����ϵͳ��Ӧ����Ϊ�����л�����, ����Ϊ��) */

/* ����ƽ̨����ѡ������Ƿ�֧�ֵ��� */
#if defined(DEBUG) || defined(_DEBUG)
	#define IV_DEBUG			1			/* �Ƿ�֧�ֵ��� */
#else
	#define IV_DEBUG			0			/* �Ƿ�֧�ֵ��� */
#endif

/* ����ƽ̨����ѡ������Ƿ��� Unicode ��ʽ���� */
#if defined(UNICODE) || defined(_UNICODE)
	#define IV_UNICODE			1			/* �Ƿ��� Unicode ��ʽ���� */
#else
	#define IV_UNICODE			0			/* �Ƿ��� Unicode ��ʽ���� */
#endif
