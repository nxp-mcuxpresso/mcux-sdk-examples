/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VIRTUAL_NIC_ENET_ADAPTER_H_
#define _VIRTUAL_NIC_ENET_ADAPTER_H_ 1

#include "virtual_nic.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* USB header size of RNDIS packet in bytes. */
#define RNDIS_USB_OVERHEAD_SIZE (44)
/* Specifies the offset in bytes from the start of the DataOffset field of rndis_packet_msg_struct_t to the start of the
 * data. */
#define RNDIS_DATA_OFFSET (36)

/* ENET queue size. */
#define ENET_QUEUE_MAX (16)

/* Define VNIC_ENET transfer struct */
typedef struct _vnic_enet_transfer
{
    uint8_t *buffer;
    uint32_t length;
} vnic_enet_transfer_t;

/* Define VNIC_ENET queue struct */
typedef struct _queue
{
    uint32_t head;
    uint32_t tail;
    uint32_t maxSize;
    uint32_t curSize;
    osa_mutex_handle_t mutex;
    uint32_t mutexBuffer[(OSA_MUTEX_HANDLE_SIZE + 3) / 4];
    vnic_enet_transfer_t *qArray;
} queue_t;

/* Alloc variable for primask. */
#define USB_DEVICE_VNIC_PRIMASK_ALLOC() uint32_t prmsk

/* Alloc variable for primask. */
#define USB_DEVICE_VNIC_CRITICAL_ALLOC() USB_DEVICE_VNIC_PRIMASK_ALLOC()

/* Eneter critical section. */
#define USB_DEVICE_VNIC_ENTER_CRITICAL() prmsk = VNIC_EnetEnterCritical()

/* Exit critical section. */
#define USB_DEVICE_VNIC_EXIT_CRITICAL() VNIC_EnetExitCritical(prmsk)

/*******************************************************************************
 * API
 ******************************************************************************/
extern volatile uint32_t g_vnicEnterCriticalCnt;
/*!
 * @brief Disable interrupt to enter critical section.
 *
 * @return The value of PRIMASK register before disable irq.
 */
static inline uint32_t VNIC_EnetEnterCritical()
{
    uint32_t primask;
#if defined(CPSR_I_Msk)
    primask = __get_CPSR() & CPSR_I_Msk;
#else
    primask = __get_PRIMASK();
#endif
    if (0 == g_vnicEnterCriticalCnt)
    {
        primask = DisableGlobalIRQ();
    }
    g_vnicEnterCriticalCnt++;
    return primask;
}

/*!
 * @brief Enable interrupt to exit critical section.
 *
 * @return None.
 */
static inline void VNIC_EnetExitCritical(uint32_t primask)
{
    if (g_vnicEnterCriticalCnt > 0)
    {
        g_vnicEnterCriticalCnt--;
        if (0 == g_vnicEnterCriticalCnt)
        {
            EnableGlobalIRQ(primask);
        }
    }
}

/*!
 * @brief Initialize the queue.
 *
 * @return Error code.
 */
static inline usb_status_t VNIC_EnetQueueInit(queue_t *q, uint32_t maxSize)
{
    usb_status_t error = kStatus_USB_Error;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    (q)->head    = 0;
    (q)->tail    = 0;
    (q)->maxSize = maxSize;
    (q)->curSize = 0;
    (q)->mutex   = (osa_mutex_handle_t)(&(q)->mutexBuffer[0]);
    if (KOSA_StatusSuccess != OSA_MutexCreate(((q)->mutex)))
    {
        usb_echo("queue mutex create error!");
    }
    error = kStatus_USB_Success;
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return error;
}

/*!
 * @brief Put element into the queue.
 *
 * @return Error code.
 */
static inline usb_status_t VNIC_EnetQueuePut(queue_t *q, vnic_enet_transfer_t *e)
{
    usb_status_t error = kStatus_USB_Error;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    if ((q)->curSize < (q)->maxSize)
    {
        (q)->qArray[(q)->head++] = *(e);
        if ((q)->head == (q)->maxSize)
        {
            (q)->head = 0;
        }
        (q)->curSize++;
        error = kStatus_USB_Success;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return error;
}

/*!
 * @brief Get element from the queue.
 *
 * @return Error code.
 */
static inline usb_status_t VNIC_EnetQueueGet(queue_t *q, vnic_enet_transfer_t *e)
{
    usb_status_t error = kStatus_USB_Error;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    if ((q)->curSize)
    {
        *(e) = (q)->qArray[(q)->tail++];
        if ((q)->tail == (q)->maxSize)
        {
            (q)->tail = 0;
        }
        (q)->curSize--;
        error = kStatus_USB_Success;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return error;
}

/*!
 * @brief Delete the queue.
 *
 * @return Error code.
 */
static inline usb_status_t VNIC_EnetQueueDelete(queue_t *q)
{
    usb_status_t error = kStatus_USB_Error;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    (q)->head    = 0;
    (q)->tail    = 0;
    (q)->maxSize = 0;
    (q)->curSize = 0;
    error        = kStatus_USB_Success;
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return error;
}

/*!
 * @brief Check if the queue is empty.
 *
 * @return 1: queue is empty, 0: not empty.
 */
static inline uint8_t VNIC_EnetQueueIsEmpty(queue_t *q)
{
    return ((q)->curSize == 0) ? 1 : 0;
}

/*!
 * @brief Check if the queue is full.
 *
 * @return 1: queue is full, 0: not full.
 */
static inline uint8_t VNIC_EnetQueueIsFull(queue_t *q)
{
    return ((q)->curSize >= (q)->maxSize) ? 1 : 0;
}

/*!
 * @brief Get the size of the queue.
 *
 * @return Size of the quue.
 */
static inline uint32_t VNIC_EnetQueueSize(queue_t *q)
{
    return (q)->curSize;
}

/*!
 * @brief Initialize the ethernet module.
 *
 * @return Error code.
 */
extern uint32_t VNIC_EnetInit(void);

/*!
 * @brief Clear the transfer requests in enet queue.
 *
 * @return Error code.
 */
extern usb_status_t VNIC_EnetClearEnetQueue(void);

/*!
 * @brief Send packets to Ethernet module.
 *
 * @return Error code.
 */
#endif /* _VIRTUAL_NIC_ENET_ADAPTER_H_ */
