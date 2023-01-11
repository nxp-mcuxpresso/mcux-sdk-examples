/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_cdc_rndis.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "virtual_nic_enetif.h"
#include "virtual_nic_enet_adapter.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#if (!FSL_FEATURE_SOC_ENET_COUNT)
#error This application requires FSL_FEATURE_SOC_ENET_COUNT defined non-zero.
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ENET_RXRTCSBUFF_NUM (6)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
enet_err_t ENETIF_Init(void);
enet_err_t ENETIF_Output(pbuf_t *packetBuffer);
uint32_t ENETIF_GetSpeed(void);
bool ENETIF_GetLinkStatus(void);
void VNIC_EnetRxBufFree(pbuf_t *pbuf);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_cdc_vnic_t g_cdcVnic;
/* Ethernet rx queue for nic device. */
queue_t g_enetRxServiceQueue;
/* Counter for enter into or exit from critical section. */
volatile uint32_t g_vnicEnterCriticalCnt = 0;
/* Buffer queue of the ethernet rx queue. */
static vnic_enet_transfer_t s_enetRxQueueBuf[ENET_QUEUE_MAX];
/* Enet rx data bufffer queue. */
static uint8_t *s_dataRxBuffQue;
/* Enet rx data bufffer. */
USB_DMA_DATA_NONCACHEABLE static uint8_t s_dataRxBuffer[ENET_RXRTCSBUFF_NUM]
                                                       [ENET_FRAME_MAX_FRAMELEN + sizeof(enet_header_t)];
/* Enet available rx data buffer count . */
static uint32_t s_dataRxBufferFreeCnt = 0;

/* Ethernet tx queue for nic device. */
queue_t g_enetTxServiceQueue;
/* Buffer queue of the ethernet tx queue. */
static vnic_enet_transfer_t s_enetTxQueueBuf[ENET_QUEUE_MAX];
/* Enet tx data bufffer queue. */
static uint8_t *s_dataTxBuffQue;
/* Enet tx data bufffer. */
USB_DMA_DATA_NONCACHEABLE static uint8_t s_dataTxBuffer[ENET_RXRTCSBUFF_NUM]
                                                       [ENET_FRAME_MAX_FRAMELEN + +sizeof(enet_header_t)];
/* Enet available tx data buffer count . */
static uint32_t s_dataTxBufferFreeCnt = 0;

/* Mac address of device. */
uint8_t g_hwaddr[ENET_MAC_ADDR_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Enqueue the rx packet buffer.
 *
 * @return none.
 */
static inline void VNIC_EnetEnqueueRxBuffer(void **queue, void *buffer)
{
    bool isFree = false;
    void *temp;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    temp = *queue;
    /* If the packet buffer already in free list, then ignore. */
    while (NULL != temp)
    {
        if (temp == buffer)
        {
            isFree = true;
            break;
        }
        temp = *(void **)temp;
    }
    if (false == isFree)
    {
        *((void **)buffer) = *queue;
        *queue             = buffer;
        s_dataRxBufferFreeCnt++;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
}

/*!
 * @brief Dequeue the rx packet buffer.
 *
 * @return The address of the available packet buffer.
 */
static inline void *VNIC_EnetDequeueRxBuffer(void **queue)
{
    void *buffer;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    buffer = *queue;

    if (buffer)
    {
        *queue = *((void **)buffer);
    }
    if (NULL != buffer)
    {
        s_dataRxBufferFreeCnt--;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return buffer;
}

/*!
 * @brief Allocate the rx packet buffer.
 *
 * @return The address of the available packet buffer.
 */
uint8_t *VNIC_EnetRxBufAlloc(void)
{
    uint8_t *ret = NULL;
    ret          = VNIC_EnetDequeueRxBuffer((void **)&s_dataRxBuffQue);
    return (uint8_t *)ret;
}

/*!
 * @brief Recycle the rx packet buffer.
 *
 * @return none.
 */
void VNIC_EnetRxBufFree(pbuf_t *pbuf)
{
    VNIC_EnetEnqueueRxBuffer((void **)&s_dataRxBuffQue, pbuf->payload);
}

/*!
 * @brief Enqueue the tx packet buffer.
 *
 * @return none.
 */
static inline void VNIC_EnetEnqueueTxBuffer(void **queue, void *buffer)
{
    bool isFree = false;
    void *temp;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    temp = *queue;
    /* If the packet buffer already in free list, then ignore. */
    while (NULL != temp)
    {
        if (temp == buffer)
        {
            isFree = true;
            break;
        }
        temp = *(void **)temp;
    }
    if (false == isFree)
    {
        *((void **)buffer) = *queue;
        *queue             = buffer;
        s_dataTxBufferFreeCnt++;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
}

/*!
 * @brief Dequeue the tx packet buffer.
 *
 * @return The address of the available packet buffer.
 */
static inline void *VNIC_EnetDequeueTxBuffer(void **queue)
{
    void *buffer;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    buffer = *queue;

    if (buffer)
    {
        *queue = *((void **)buffer);
    }
    if (NULL != buffer)
    {
        s_dataTxBufferFreeCnt--;
    }
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return buffer;
}

/*!
 * @brief Allocate the tx packet buffer.
 *
 * @return The address of the available packet buffer.
 */
uint8_t *VNIC_EnetTxBufAlloc(void)
{
    uint8_t *ret = NULL;
    ret          = VNIC_EnetDequeueTxBuffer((void **)&s_dataTxBuffQue);
    return (uint8_t *)ret;
}

/*!
 * @brief Recycle the tx packet buffer.
 *
 * @return none.
 */
void VNIC_EnetTxBufFree(pbuf_t *pbuf)
{
    VNIC_EnetEnqueueTxBuffer((void **)&s_dataTxBuffQue, pbuf->payload);
}

/*!
 * @brief Services an IP packet.
 *
 * @return ENET or error code.
 */
void VNIC_EnetCallback(pbuf_t *pbuffer)
{
    usb_status_t error = kStatus_USB_Error;
    vnic_enet_transfer_t cdcAcmTransfer;
    cdcAcmTransfer.buffer = pbuffer->payload;
    cdcAcmTransfer.length = pbuffer->length;

    error = VNIC_EnetQueuePut(&g_enetRxServiceQueue, &cdcAcmTransfer);
    if (kStatus_USB_Success != error)
    {
        error = kStatus_USB_Busy;
        VNIC_EnetRxBufFree(pbuffer);
    }
    else
    {
        g_cdcVnic.nicTrafficInfo.enetRxEnet2usb++;
    }
    return;
}

/*!
 * @brief Recycle the buffer when Enet Tx complete.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t VNIC_EnetTxDone(void)
{
    usb_status_t error = kStatus_USB_Error;
    pbuf_t packetBuffer;
    vnic_enet_transfer_t cdcAcmTransfer;
    error = VNIC_EnetQueueGet(&g_enetTxServiceQueue, &cdcAcmTransfer);
    if (kStatus_USB_Success == error)
    {
        packetBuffer.payload = cdcAcmTransfer.buffer;
        packetBuffer.length  = cdcAcmTransfer.length;

        g_cdcVnic.nicTrafficInfo.enetTxUsb2enetSent++;
        VNIC_EnetTxBufFree(&packetBuffer);
    }

    return error;
}
/*!
 * @brief Initialize the ethernet module.
 *
 * @return Error code.
 */
uint32_t VNIC_EnetInit(void)
{
    uint32_t error;
    uint32_t count;
    g_enetRxServiceQueue.qArray = s_enetRxQueueBuf;
    g_enetTxServiceQueue.qArray = s_enetTxQueueBuf;

    VNIC_EnetQueueInit(&g_enetRxServiceQueue, ENET_QUEUE_MAX);
    VNIC_EnetQueueInit(&g_enetTxServiceQueue, ENET_QUEUE_MAX);

    /* Initialize the rx packet buffer */
    for (count = 0; count < ENET_RXRTCSBUFF_NUM; count++)
    {
        VNIC_EnetEnqueueRxBuffer((void **)&s_dataRxBuffQue, &s_dataRxBuffer[count]);
    }
    /* Initialize the tx packet buffer */
    for (count = 0; count < ENET_RXRTCSBUFF_NUM; count++)
    {
        VNIC_EnetEnqueueTxBuffer((void **)&s_dataTxBuffQue, &s_dataTxBuffer[count]);
    }

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    error = ENETIF_Init();
    if (error != ENET_OK)
    {
        usb_echo("ENET_Initialization Failed\n");
        return ENET_ERROR;
    }
    return error;
}

/*!
 * @brief Send packets to Ethernet module.
 *
 * @return Error code.
 */
usb_status_t VNIC_EnetSend(uint8_t *buf, uint32_t len)
{
    enet_err_t error = ENET_ERROR;
    usb_status_t sts;
    pbuf_t packetBuffer;
    vnic_enet_transfer_t cdcAcmTransfer;
    g_cdcVnic.nicTrafficInfo.enetTxHost2usb++;
    packetBuffer.payload = VNIC_EnetTxBufAlloc();
    if (NULL == packetBuffer.payload)
    {
        sts = kStatus_USB_Busy;
        g_cdcVnic.nicTrafficInfo.enetTxHost2usbDiscard++;
        usb_echo("VNIC_EnetTxBufAlloc failed\n");
        return sts;
    }
    else
    {
        packetBuffer.length   = len;
        cdcAcmTransfer.buffer = packetBuffer.payload;
        cdcAcmTransfer.length = packetBuffer.length;
        memcpy(cdcAcmTransfer.buffer, buf, cdcAcmTransfer.length);
        do
        {
            error = ENETIF_Output(&packetBuffer);
            /* wait for a while */
        } while (ENET_BUSY == error);

        if (ENET_OK != error)
        {
            g_cdcVnic.nicTrafficInfo.enetTxUsb2enetFail++;
            VNIC_EnetTxBufFree(&packetBuffer);
            sts = kStatus_USB_Error;
            usb_echo("VNIC_EnetSend failed\n");
        }
        else
        {
            sts = VNIC_EnetQueuePut(&g_enetTxServiceQueue, &cdcAcmTransfer);
            if (kStatus_USB_Success != sts)
            {
                sts = kStatus_USB_Busy;
                VNIC_EnetTxBufFree(&packetBuffer);

                usb_echo("VNIC_EnetQueuePut failed\n");
                return sts;
            }
            g_cdcVnic.nicTrafficInfo.enetTxUsb2enet++;
        }
    }

    return sts;
}

/*!
 * @brief Get the ethernet speed.
 *
 * @return Value of ethernet speed.
 */
uint32_t VNIC_EnetGetSpeed(void)
{
    return ENETIF_GetSpeed();
}

/*!
 * @brief Get the ethernet link status.
 *
 * @return Ture if linked, otherwise false.
 */
bool VNIC_EnetGetLinkStatus(void)
{
    return ENETIF_GetLinkStatus();
}
/*!
 * @brief Clear the transfer requests in enet queue.
 *
 * @return Error code.
 */
usb_status_t VNIC_EnetClearEnetQueue(void)
{
    usb_status_t error = kStatus_USB_Success;
    vnic_enet_transfer_t cdcAcmTransfer;
    pbuf_t enetPbuf;
    /* Clear Rx queue */
    while (kStatus_USB_Success == error)
    {
        error = VNIC_EnetQueueGet(&g_enetRxServiceQueue, &cdcAcmTransfer);
        if (kStatus_USB_Success == error)
        {
            enetPbuf.payload = cdcAcmTransfer.buffer;
            enetPbuf.length  = cdcAcmTransfer.length;
            VNIC_EnetRxBufFree(&enetPbuf);
            g_cdcVnic.nicTrafficInfo.enetRxUsb2hostCleared++;
        }
    }
    /* Clear Tx queue */
    error = kStatus_USB_Success;
    while (kStatus_USB_Success == error)
    {
        error = VNIC_EnetQueueGet(&g_enetTxServiceQueue, &cdcAcmTransfer);
        if (kStatus_USB_Success == error)
        {
            enetPbuf.payload = cdcAcmTransfer.buffer;
            enetPbuf.length  = cdcAcmTransfer.length;
            VNIC_EnetTxBufFree(&enetPbuf);
            g_cdcVnic.nicTrafficInfo.enetTxUsb2hostCleared++;
        }
    }
    return error;
}
