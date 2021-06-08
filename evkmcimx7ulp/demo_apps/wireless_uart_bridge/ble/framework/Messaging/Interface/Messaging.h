/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MESSAGING_H
#define _MESSAGING_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "GenericList.h"
#include "MemManager.h"

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
#define anchor_t   list_t
#define msgQueue_t list_t

/************************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
************************************************************************************/
/* Put a message in a queue. */
#define MSG_Queue(anchor, element)     ListAddTailMsg((anchor), (element))
#define MSG_QueueHead(anchor, element) ListAddHeadMsg((anchor), (element))

/* Get a message from a queue. Returns NULL if no messages in queue. */
#define MSG_DeQueue(anchor) ListRemoveHeadMsg(anchor)

/* Check if a message is pending in a queue. Returns */
/* TRUE if any pending messages, and FALSE otherwise. */
#define MSG_Pending(anchor) ((anchor)->head != 0)

#define MSG_InitQueue(anchor)    ListInitMsg(anchor)
#define List_ClearAnchor(anchor) ListInitMsg(anchor)

#define MSG_Alloc(element)  MEM_BufferAlloc(element)
#define MSG_AllocType(type) MEM_BufferAlloc(sizeof(type))
#define MM_Free             MEM_BufferFree
#define MSG_Free(element)   MEM_BufferFree(element)
#define MSG_FreeQueue(anchor)          \
    while (MSG_Pending(anchor))        \
    {                                  \
        MSG_Free(MSG_DeQueue(anchor)); \
    }

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/
#define ListInitMsg(listPtr) ListInit((listPtr), 0)
listStatus_t ListAddTailMsg(listHandle_t list, void *pMsg);
listStatus_t ListAddHeadMsg(listHandle_t list, void *pMsg);
listStatus_t ListAddPrevMsg(void *pMsg, void *pNewMsg);
listStatus_t ListRemoveMsg(void *pMsg);
void *ListRemoveHeadMsg(listHandle_t list);
void *ListGetHeadMsg(listHandle_t list);
void *ListGetNextMsg(void *pMsg);

/*================================================================================================*/

#endif /* _MESSAGING_H */
