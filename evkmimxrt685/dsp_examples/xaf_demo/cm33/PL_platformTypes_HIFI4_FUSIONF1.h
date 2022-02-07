/*
* Copyright 2020 NXP
*
* NXP Confidential. This software is owned or controlled by NXP and may only
* be used strictly in accordance with the applicable license terms found in
* file LICENSE.txt
*/


#ifndef _PLATFORM_HIFI4_H_
#define _PLATFORM_HIFI4_H_

#ifdef __cplusplus
extern "C" {
#endif



typedef     signed char            PL_INT8;  
typedef     short                  PL_INT16; 
typedef     int                    PL_INT32;
typedef     long long              PL_INT64;
typedef     unsigned char          PL_UINT8; 
typedef     unsigned short         PL_UINT16;
typedef     unsigned int           PL_UINT32;
typedef     float                  PL_FLOAT;
typedef     double                 PL_DOUBLE;
typedef     unsigned short         PL_BOOL;
typedef     PL_UINT32              PL_UINTPTR;

#define     PL_NULL                NULL                ///< NULL pointer
#define     PL_MAXENUM             2147483647        ///< Maximum value for enumerator

//PL_BOOL
enum { PL_FALSE, PL_TRUE };


//Memory alignment
#define     PL_MEM_ALIGN(var, alignbytes)      var __attribute__((aligned(alignbytes)))

//Alignment required by VIT model
#define     VIT_MODEL_ALIGN_BYTES  64


#include "PL_memoryRegion.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_PLATFORM_PLATFORM_HIFI4_H_*/
