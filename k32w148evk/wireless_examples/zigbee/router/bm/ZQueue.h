/****************************************************************************
 *
 * Copyright 2020, 2023-2024 NXP
 *
 * NXP Confidential.
 *
 * This software is owned or controlled by NXP and may only be used strictly
 * in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing, activating
 * and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.
 * If you do not agree to be bound by the applicable license terms,
 * then you may not retain, install, activate or otherwise use the software.
 *
 *
 ****************************************************************************/


/****************************************************************************
 *
 * MODULE:ZQueue.h
 *
 * DESCRIPTION:
 *
 ****************************************************************************/

#ifndef ZQUEUE_H_
#define ZQUEUE_H_

#include "jendefs.h"
#include "string.h"

#ifndef ZIGBEE_USE_FRAMEWORK
#define ZIGBEE_USE_FRAMEWORK 0
#endif

#if ZIGBEE_USE_FRAMEWORK
#if defined(K32W1480_SERIES) || defined(K32W1) || defined(MCXW716A_SERIES) || defined(MCXW716C_SERIES) || defined(RW612_SERIES)
#include "fsl_component_generic_list.h"
#else
#include "GenericList.h"
#endif

typedef struct
{
#if defined(K32W1480_SERIES) || defined(K32W1) || defined(MCXW716A_SERIES) || defined(MCXW716C_SERIES) || defined(RW612_SERIES)
    list_label_t list;
#else
    list_t list;
#endif
    uint32 u32ItemSize;
}tszQueue;
#else
typedef struct
{
    uint32 u32Length;                    /*< The length of the queue defined as the number of items it will hold, not the number of bytes. */
    uint32 u32ItemSize;                /*< The size of each items that the queue will hold. */
    uint32 u32MessageWaiting;
    void  *pvHead;                    /*< Points to the beginning of the queue storage area. */
    void  *pvWriteTo;                    /*< Points to the free next place in the storage area. */
    void  *pvReadFrom;                /*< Points to the free next place in the storage area. */
}tszQueue;
#endif

PUBLIC void ZQ_vQueueCreate (tszQueue *psQueueHandle, const uint32 uiQueueLength, const uint32 uiItemSize, uint8* pu8StartQueue);
PUBLIC bool_t ZQ_bQueueSend(void *pvQueueHandle, const void *pvItemToQueue);
PUBLIC bool_t ZQ_bQueueReceive(void *pvQueueHandle, void *pvItemFromQueue);
PUBLIC bool_t ZQ_bQueueIsEmpty(void *pvQueueHandle);
PUBLIC uint32 ZQ_u32QueueGetQueueSize(void *pvQueueHandle);
PUBLIC uint32 ZQ_u32QueueGetQueueMessageWaiting ( void*    pu8QueueHandle );
PUBLIC void* ZQ_pvGetFirstElementOnQueue ( void* pvQueueHandle );
PUBLIC void* ZQ_pvGetNextElementOnQueue ( void* pvQueueHandle, void* pvMsg );
PUBLIC void ZQ_bQueueFlush(void *pvQueueHandle);
#endif /*ZQUEUE_H_*/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
