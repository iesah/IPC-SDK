/*----------------------------------------------+
|
|		ivIvwErrorCode.h - Basic Definitions
|		Ivw3.0 Status return
|		Copyright (c) 1999-2013, iFLYTEK Ltd.
|		All rights reserved.
|
+----------------------------------------------*/

#ifndef IFLYTEK_VOICE__IVWERRORCODE__H
#define IFLYTEK_VOICE__IVWERRORCODE__H

#include "ivIvwDefine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef	ivInt32		IvwErrID;
//typedef ivInt32		ivStatus;

/*
*	ivIVW Status return
*/
#define	IvwErrID_OK						    ivNull
#define	IvwErrID_FAIL					    ((IvwErrID)-1)
#define IvwErr_InvCal					    ((IvwErrID)1)
#define IvwErr_InvArg					    ((IvwErrID)2)
#define IvwErr_TellSize					    ((IvwErrID)3)
#define IvwErr_OutOfMemory				    ((IvwErrID)4)
#define IvwErr_BufferFull				    ((IvwErrID)5)
#define IvwErr_BufferEmpty				    ((IvwErrID)6)
#define IvwErr_InvRes					    ((IvwErrID)7)
#define IvwErr_ReEnter					    ((IvwErrID)8)
#define IvwErr_NotSupport				    ((IvwErrID)9)
#define IvwErr_NotFound				        ((IvwErrID)10)
#define IvwErr_InvSN					    ((IvwErrID)11)
#define IvwErr_Limitted					    ((IvwErrID)12)
#define IvwErr_TimeOut				        ((IvwErrID)13)

#define IvwErr_Enroll1_Success              ((IvwErrID)14)
#define IvwErr_Enroll1_Failed               ((IvwErrID)15)
#define IvwErr_Enroll2_Success              ((IvwErrID)16)
#define IvwErr_Enroll2_Failed               ((IvwErrID)17)
#define IvwErr_Enroll3_Success              ((IvwErrID)18)
#define IvwErr_Enroll3_Failed               ((IvwErrID)19)
#define IvwErr_SpeechTooShort               ((IvwErrID)20)
#define IvwErr_SpeechStop                   ((IvwErrID)21)

#define IvwErr_MdlAdapt_Success             ((IvwErrID)22)
#define IvwErr_MdlAdapt_Failed              ((IvwErrID)23)

#define IvwErr_MergeRes_Failed              ((IvwErrID)24)

#ifdef __cplusplus
}
#endif


#endif /* !IFLYTEK_VOICE__IVWERRORCODE__H */
