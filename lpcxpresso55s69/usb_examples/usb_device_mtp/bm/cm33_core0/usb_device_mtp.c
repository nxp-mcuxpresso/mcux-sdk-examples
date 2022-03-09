/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"

#if ((defined(USB_DEVICE_CONFIG_MTP)) && (USB_DEVICE_CONFIG_MTP > 0U))
#include "usb_device_mtp.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void USB_DeviceMtpProcessCommand(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo);

usb_status_t USB_DeviceMtpRecv(usb_device_mtp_struct_t *mscHandle);
usb_status_t USB_DeviceMtpSend(usb_device_mtp_struct_t *mscHandle);
usb_status_t USB_DeviceMtpEndpointsInit(usb_device_mtp_struct_t *mscHandle);
usb_status_t USB_DeviceMtpEndpointsDeinit(usb_device_mtp_struct_t *mscHandle);

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if (USB_DEVICE_CONFIG_MTP == 1U)

USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer1;
#define MTP_CONTAINER_ARRAY \
    {                       \
        &s_MtpContainer1    \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent1;
#define MTP_EVENT_ARRAY \
    {                   \
        &s_MtpEvent1    \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus1;
#define MTP_DEVICE_STATUS_ARRAY \
    {                           \
        &s_MtpDeviceStatus1     \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest1;
#define MTP_CANCEL_REQUEST_ARRAY \
    {                            \
        &s_MtpCancelRequest1     \
    }

#elif (USB_DEVICE_CONFIG_MTP == 2U)
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer2;
#define MTP_CONTAINER_ARRAY                \
    {                                      \
        &s_MtpContainer1, &s_MtpContainer2 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent2;
#define MTP_EVENT_ARRAY            \
    {                              \
        &s_MtpEvent1, &s_MtpEvent2 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus2;
#define MTP_DEVICE_STATUS_ARRAY                 \
    {                                           \
        &s_MtpDeviceStatus1, s_MtpDeviceStatus2 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest2;
#define MTP_CANCEL_REQUEST_ARRAY                  \
    {                                             \
        &s_MtpCancelRequest1, s_MtpCancelRequest2 \
    }

#elif (USB_DEVICE_CONFIG_MTP == 3U)
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer2;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_container_t s_MtpContainer3;
#define MTP_CONTAINER_ARRAY                                  \
    {                                                        \
        &s_MtpContainer1, &s_MtpContainer2, &s_MtpContainer3 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent2;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_event_container_t s_MtpEvent3;
#define MTP_EVENT_ARRAY                          \
    {                                            \
        &s_MtpEvent1, &s_MtpEvent2, &s_MtpEvent3 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus2;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_device_status_t s_MtpDeviceStatus3;
#define MTP_DEVICE_STATUS_ARRAY                                     \
    {                                                               \
        &s_MtpDeviceStatus1, s_MtpDeviceStatus2, s_MtpDeviceStatus3 \
    }
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest1;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest2;
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_cancel_request_t s_MtpCancelRequest3;
#define MTP_CANCEL_REQUEST_ARRAY                                       \
    {                                                                  \
        &s_MtpCancelRequest1, s_MtpCancelRequest2, s_MtpCancelRequest3 \
    }

#else
#error "the max support USB_DEVICE_CONFIG_MTP is 3"

#endif /* USB_DEVICE_CONFIG_MTP */

#if (USB_DATA_ALIGN_SIZE > 8U)
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(USB_DATA_ALIGN_SIZE) static usb_device_mtp_struct_t
    g_mtp_handle[USB_DEVICE_CONFIG_MTP];
#else
USB_GLOBAL USB_RAM_ADDRESS_ALIGNMENT(8U) static usb_device_mtp_struct_t g_mtp_handle[USB_DEVICE_CONFIG_MTP];
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Allocate a device mtp class handle.
 *
 * This function allocates a device mtp class handle.
 *
 * @param handle          It is out parameter, is used to return pointer of the device mtp class handle to the caller.
 *
 * @retval kStatus_USB_Success              Get a device mtp class handle successfully.
 * @retval kStatus_USB_Busy                 Cannot allocate a device mtp class handle.
 */
static usb_status_t USB_DeviceMtpAllocateHandle(usb_device_mtp_struct_t **handle)
{
    uint32_t count;
    usb_device_mtp_container_t *mtpContainerArray[]          = MTP_CONTAINER_ARRAY;
    usb_device_mtp_event_container_t *mtpEventArray[]        = MTP_EVENT_ARRAY;
    usb_device_mtp_device_status_t *mtpDeviceStatusArray[]   = MTP_DEVICE_STATUS_ARRAY;
    usb_device_mtp_cancel_request_t *mtpCancelRequestArray[] = MTP_CANCEL_REQUEST_ARRAY;

    for (count = 0; count < USB_DEVICE_CONFIG_MTP; count++)
    {
        if (NULL == g_mtp_handle[count].handle)
        {
            g_mtp_handle[count].mtpContainer   = mtpContainerArray[count];
            g_mtp_handle[count].eventContainer = mtpEventArray[count];
            g_mtp_handle[count].deviceStatus   = mtpDeviceStatusArray[count];
            g_mtp_handle[count].cancelRequest  = mtpCancelRequestArray[count];
            *handle                            = &g_mtp_handle[count];
            return kStatus_USB_Success;
        }
    }

    return kStatus_USB_Busy;
}

/*!
 * @brief Free a device mtp class handle.
 *
 * This function frees a device mtp class handle.
 *
 * @param handle          The device mtp class handle.
 *
 * @retval kStatus_USB_Success              Free device mtp class handle successfully.
 */
static usb_status_t USB_DeviceMtpFreeHandle(usb_device_mtp_struct_t *handle)
{
    handle->handle              = NULL;
    handle->configurationStruct = (usb_device_class_config_struct_t *)NULL;
    handle->configuration       = 0;
    handle->alternate           = 0;
    return kStatus_USB_Success;
}

static void USB_DeviceMtpPrimeCommand(usb_device_mtp_struct_t *mtpHandle)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_COMMAND;
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
    usb_echo("start transfer error\r\n");
#endif
    USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, (uint8_t *)mtpHandle->mtpContainer,
                          USB_DEVICE_MTP_COMMAND_LENGTH);
}

usb_status_t USB_DeviceMtpPrimeResponse(usb_device_mtp_struct_t *mtpHandle,
                                        uint16_t respCode,
                                        uint32_t *respParam,
                                        uint8_t respParamSize)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

    mtpHandle->mtpContainer->containerLength = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH + respParamSize * 4U;
    mtpHandle->mtpContainer->containerType   = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
    mtpHandle->mtpContainer->code            = respCode;

    while (respParamSize--)
    {
        mtpHandle->mtpContainer->param[respParamSize] = respParam[respParamSize];
    }

    return USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkInEndpoint, (uint8_t *)mtpHandle->mtpContainer,
                                 mtpHandle->mtpContainer->containerLength);
}

void USB_DevicePrimeDataIn(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    usb_device_mtp_container_t *container;

    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_BULK_IN;

    mtpHandle->transferTotal  = dataInfo->totalSize;
    mtpHandle->transferLength = dataInfo->curSize;
    mtpHandle->transferBuffer = dataInfo->buffer;
    mtpHandle->transferDone   = 0;
    mtpHandle->transferOffset = 0;

    container                  = (usb_device_mtp_container_t *)&mtpHandle->transferBuffer[0];
    container->containerLength = (mtpHandle->transferTotal > USB_DEVICE_MTP_MAX_UINT32_VAL) ?
                                     USB_DEVICE_MTP_MAX_UINT32_VAL :
                                     mtpHandle->transferTotal;
    container->containerType = USB_DEVICE_MTP_CONTAINER_TYPE_DATA;
    container->code          = mtpHandle->mtpContainer->code;
    container->transactionID = mtpHandle->transactionID;

    USB_DeviceMtpSend(mtpHandle);
}

void USB_DevicePrimeDataOut(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_BULK_OUT;

    mtpHandle->transferTotal  = 0;
    mtpHandle->transferLength = dataInfo->curSize;
    mtpHandle->transferBuffer = dataInfo->buffer;
    mtpHandle->transferDone   = 0;
    mtpHandle->transferOffset = 0;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceMtpRecv(mtpHandle))
    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
        usb_echo("mtp receive error\r\n");
#endif
    }
#else
    (void)USB_DeviceMtpRecv(mtpHandle);
#endif
}

void USB_DevicePrimeBulkInAndOutStall(usb_device_mtp_struct_t *mtpHandle, uint16_t code, uint8_t epNeed)
{
    mtpHandle->bulkInStallFlag  = USB_DEVICE_MTP_STATE_BULK_IN_STALL;
    mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_STALL;

    /* prepare data for getting device status */
    if (epNeed != 0U)
    {
        mtpHandle->deviceStatus->wLength   = 6U;
        mtpHandle->deviceStatus->epNumber1 = mtpHandle->bulkInEndpoint;
        mtpHandle->deviceStatus->epNumber2 = mtpHandle->bulkOutEndpoint;
    }
    else
    {
        mtpHandle->deviceStatus->wLength = 4U;
    }
    mtpHandle->deviceStatus->code = code;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if ((kStatus_USB_Success != USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkInEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkOutEndpoint)))
    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
        usb_echo("stall endpoint error\r\n");
#endif
    }
#else
    (void)USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkInEndpoint);
    (void)USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkOutEndpoint);
#endif
}

static void USB_DeviceMtpStateMachine(usb_device_mtp_struct_t *mtpHandle,
                                      usb_device_endpoint_callback_message_struct_t *message)
{
    usb_device_mtp_cmd_data_struct_t dataInfo;

    /* Step B: state is not changed */
    /* Step C: change to new state */

    if (message->length == USB_UNINITIALIZED_VAL_32)
    {
        /* callback to cancel current transaction */
        dataInfo.curSize  = 0U;
        dataInfo.code     = MTP_RESPONSE_UNDEFINED;
        dataInfo.curPhase = USB_DEVICE_MTP_PHASE_CANCELLATION;
        memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
        USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
        return;
    }

    switch (mtpHandle->mtpState)
    {
        case USB_DEVICE_MTP_STATE_COMMAND: /* B */
        {
            if (USB_DEVICE_MTP_STATE_BULK_OUT_STALL == mtpHandle->bulkOutStallFlag)
            {
                break;
            }

            if ((message->length >= USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH) &&
                (message->length == mtpHandle->mtpContainer->containerLength) &&
                (mtpHandle->mtpContainer->containerType == USB_DEVICE_MTP_CONTAINER_TYPE_COMMAND))
            {
                dataInfo.curSize  = 0U;
                dataInfo.code     = MTP_RESPONSE_UNDEFINED;
                dataInfo.curPhase = USB_DEVICE_MTP_PHASE_COMMAND;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                /* Invalid command, stall bulk pipe. */
                USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
            }
            break;
        }

        case USB_DEVICE_MTP_STATE_BULK_IN: /* B */
        {
            mtpHandle->transferDone += message->length;
            mtpHandle->transferOffset += message->length;
            mtpHandle->transferLength -= message->length;

            if (USB_DEVICE_MTP_STATE_BULK_IN_STALL == mtpHandle->bulkInStallFlag)
            {
                break;
            }

            if (mtpHandle->transferDone == mtpHandle->transferTotal)
            {
                /* If the number of bytes specified in the first four bytes of the Data Block are an integral multiple
                   of the wMaxPacketSize field of the Endpoint Descriptor the Data phase, will end in a NULL packet,
                   so prime zero packet here. */
                if (((mtpHandle->transferDone % mtpHandle->bulkInMaxPacketSize) == 0U) &&
                    (mtpHandle->transferLength == 0U) && (message->length != 0U))
                {
                    /* Prime NULL packet */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    if (kStatus_USB_Success !=
                        USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0))
                    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                        usb_echo("send error\r\n");
#endif
                    }
#else
                    (void)USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0);
#endif
                    break;
                }

                /* Transfer complete */
                mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                dataInfo.curPos   = mtpHandle->transferDone;
                dataInfo.curSize  = 0;
                dataInfo.code     = MTP_RESPONSE_OK;
                dataInfo.curPhase = USB_DEVICE_MTP_PHASE_RESPONSE;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                /* Prime response block here. */
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                if (((message->length % mtpHandle->bulkInMaxPacketSize) != 0U) || (message->length == 0U))
                {
                    /* A short packet indicates the end of a Data phase.
                      If the number of bytes transferred in the Data phase is less than that specified in the first
                      four bytes of the Data Block and the data receiver detects this condition before the
                      initiation of the Response phase the data transfer may be cancelled. */
                    USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
                    break;
                }

                /* Transfer not complete */
                if (mtpHandle->transferLength != 0U)
                {
                    /* continue to transfer */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    if (kStatus_USB_Success != USB_DeviceMtpSend(mtpHandle))
                    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                        usb_echo("mtp send error\r\n");
#endif
                    }
#else
                    (void)USB_DeviceMtpSend(mtpHandle);
#endif
                }
                else
                {
                    /* callback to request data buffer. */
                    dataInfo.curPos   = mtpHandle->transferDone;
                    dataInfo.curSize  = 0;
                    dataInfo.code     = MTP_RESPONSE_UNDEFINED;
                    dataInfo.curPhase = USB_DEVICE_MTP_PHASE_DATA;
                    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

                    if (MTP_RESPONSE_UNDEFINED != dataInfo.code)
                    {
                        /* error, stall Bulk-in & Bulk-out endpoints. */
                    }
                    else
                    {
                        mtpHandle->transferLength = dataInfo.curSize;
                        mtpHandle->transferBuffer = dataInfo.buffer;
                        mtpHandle->transferOffset = 0U;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                        if (kStatus_USB_Success != USB_DeviceMtpSend(mtpHandle))
                        {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                            usb_echo("mtp send error\r\n");
#endif
                        }
#else
                        (void)USB_DeviceMtpSend(mtpHandle);
#endif
                    }
                }
            }
            break;
        }

        case USB_DEVICE_MTP_STATE_BULK_OUT: /* B */
        {
            mtpHandle->transferDone += message->length;
            mtpHandle->transferOffset += message->length;
            mtpHandle->transferLength -= message->length;

            if (USB_DEVICE_MTP_STATE_BULK_OUT_STALL == mtpHandle->bulkOutStallFlag)
            {
                break;
            }

            if (mtpHandle->transferTotal == 0U)
            {
                /* Is the first packet. */
                mtpHandle->transferTotal = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(message->buffer);

                /* Special process for receiving >4GB file. */
                if (mtpHandle->transferTotal == USB_DEVICE_MTP_MAX_UINT32_VAL)
                {
                    mtpHandle->transferTotal = USB_DEVICE_MTP_MAX_UINT64_VAL;
                }
            }

            if (mtpHandle->transferDone == mtpHandle->transferTotal)
            {
                /* If the number of bytes specified in the first four bytes of the Data Block are an integral multiple
                   of the wMaxPacketSize field of the Endpoint Descriptor the Data phase, will end in a NULL packet,
                   so prime zero packet here. */
                if (((mtpHandle->transferDone % mtpHandle->bulkOutMaxPacketSize) == 0U) &&
                    (mtpHandle->transferLength == 0U) && (message->length != 0U))
                {
                    /* Prime NULL packet */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    if (kStatus_USB_Success !=
                        USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0))
                    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                        usb_echo("mtp receive error\r\n");
#endif
                    }
#else
                    (void)USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0);
#endif
                    break;
                }

                /* Transfer complete */
                mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                dataInfo.curPos   = mtpHandle->transferDone;
                dataInfo.curSize  = 0;
                dataInfo.code     = MTP_RESPONSE_OK;
                dataInfo.curPhase = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                /* Prime response block here. */
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                if (((message->length % mtpHandle->bulkOutMaxPacketSize) != 0U) || (message->length == 0U))
                {
                    /* Special process for receiving >4GB file. */
                    if ((mtpHandle->transferTotal != USB_DEVICE_MTP_MAX_UINT64_VAL) ||
                        ((mtpHandle->transferTotal == USB_DEVICE_MTP_MAX_UINT64_VAL) &&
                         (mtpHandle->transferDone < USB_DEVICE_MTP_MAX_UINT32_VAL)))
                    {
                        /* A short packet or NULL packet indicates the end of a Data phase.
                          If the number of bytes transferred in the Data phase is less than that specified in the first
                          four bytes of the Data Block and the data receiver detects this condition before the
                          initiation of the Response phase the data transfer may be cancelled. */
                        USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
                        break;
                    }
                    else
                    {
                        /* Transfer complete */
                        mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                        dataInfo.curPos   = mtpHandle->transferDone;
                        dataInfo.curSize  = 0;
                        dataInfo.code     = MTP_RESPONSE_OK;
                        dataInfo.curPhase = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
                        memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                        /* Prime response block here. */
                        USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
                        break;
                    }
                }

                /* Transfer not complete */
                if (mtpHandle->transferLength != 0U)
                {
                    /* continue to transfer */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    if (kStatus_USB_Success != USB_DeviceMtpRecv(mtpHandle))
                    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                        usb_echo("mtp receive error\r\n");
#endif
                    }
#else
                    (void)USB_DeviceMtpRecv(mtpHandle);
#endif
                }
                else
                {
                    /* callback to request data buffer. */
                    dataInfo.totalSize = mtpHandle->transferTotal;
                    dataInfo.curPos    = mtpHandle->transferDone;
                    dataInfo.curSize   = 0;
                    dataInfo.code      = MTP_RESPONSE_UNDEFINED;
                    dataInfo.curPhase  = USB_DEVICE_MTP_PHASE_DATA;
                    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

                    if (MTP_RESPONSE_UNDEFINED != dataInfo.code)
                    {
                        /* error, stall Bulk-in & Bulk-out endpoints. */
                    }
                    else
                    {
                        mtpHandle->transferLength = dataInfo.curSize;
                        mtpHandle->transferBuffer = dataInfo.buffer;
                        mtpHandle->transferOffset = 0U;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                        if (kStatus_USB_Success != USB_DeviceMtpRecv(mtpHandle))
                        {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                            usb_echo("mtp receive error\r\n");
#endif
                        }
#else
                        (void)USB_DeviceMtpRecv(mtpHandle);
#endif
                    }
                }
            }

            break;
        }

        case USB_DEVICE_MTP_STATE_RESPONSE: /* B */
        {
            if (USB_DEVICE_MTP_STATE_BULK_IN_STALL == mtpHandle->bulkInStallFlag)
            {
                break;
            }

            if ((message->length >= USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH) &&
                (message->length == mtpHandle->mtpContainer->containerLength))
            {
                USB_DeviceMtpPrimeCommand(mtpHandle); /* C */
            }
            else
            {
                USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_GENERAL_ERROR, 0);
            }

            break;
        }

        default:
            /* No action here */
            break;
    }
}

/*!
 * @brief Bulk IN endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Bulk IN pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Bulk IN pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceMtpBulkIn(usb_device_handle handle,
                                        usb_device_endpoint_callback_message_struct_t *message,
                                        void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    USB_DeviceMtpStateMachine(mtpHandle, message);

    return error;
}

/*!
 * @brief Bulk OUT endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Bulk OUT pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Bulk OUT pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceMtpBulkOut(usb_device_handle handle,
                                         usb_device_endpoint_callback_message_struct_t *message,
                                         void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    USB_DeviceMtpStateMachine(mtpHandle, message);

    return error;
}

/*!
 * @brief Interrupt IN endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Interrupt IN pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Interrupt IN pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
static usb_status_t USB_DeviceMtpInterruptIn(usb_device_handle handle,
                                             usb_device_endpoint_callback_message_struct_t *message,
                                             void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;
    usb_device_mtp_callback_event_t event;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }
    mtpHandle->interruptInBusy = 0U;

    if (message->length == mtpHandle->eventContainer->containerLength)
    {
        event = kUSB_DeviceMtpEventSendResponseSuccess;
    }
    else
    {
        event = kUSB_DeviceMtpEventSendResponseError;
    }

    if ((NULL != mtpHandle->configurationStruct) && (NULL != mtpHandle->configurationStruct->classCallback))
    {
        /* Notify the application data sent by calling the mtp class callback. classCallback is initialized
           in classInit of s_UsbDeviceClassInterfaceMap,it is from the second parameter of classInit */
        error = mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, event, NULL);
    }

    return error;
}

/*!
 * @brief Initialize the endpoints of the mtp class.
 *
 * This callback function is used to initialize the endpoints of the mtp class.
 *
 * @param mtpHandle          The device mtp class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpEndpointsInit(usb_device_mtp_struct_t *mtpHandle)
{
    usb_device_interface_list_t *interfaceList;
    usb_device_interface_struct_t *interface = (usb_device_interface_struct_t *)NULL;
    usb_status_t error                       = kStatus_USB_Error;
    uint32_t count;
    uint32_t index;

    /* Check the configuration is valid or not. */
    if ((0U == mtpHandle->configuration) ||
        (mtpHandle->configuration > mtpHandle->configurationStruct->classInfomation->configurations))
    {
        return error;
    }

    /* Get the interface list of the new configuration. */
    /* Check the interface list is valid or not. */
    if (NULL == mtpHandle->configurationStruct->classInfomation->interfaceList)
    {
        return error;
    }
    interfaceList = &mtpHandle->configurationStruct->classInfomation->interfaceList[mtpHandle->configuration - 1U];

    /* Find interface by using the alternate setting of the interface. */
    for (count = 0; count < interfaceList->count; count++)
    {
        if (USB_DEVICE_CONFIG_MTP_CLASS_CODE == interfaceList->interfaces[count].classCode)
        {
            for (index = 0; index < interfaceList->interfaces[count].count; index++)
            {
                if (interfaceList->interfaces[count].interface[index].alternateSetting == mtpHandle->alternate)
                {
                    interface = &interfaceList->interfaces[count].interface[index];
                    break;
                }
            }
            mtpHandle->interfaceNumber = interfaceList->interfaces[count].interfaceNumber;
            break;
        }
    }
    if (NULL == interface)
    {
        /* Return error if the interface is not found. */
        return error;
    }

    /* Keep new interface handle. */
    mtpHandle->interfaceHandle = interface;
    /* Initialize the endpoints of the new interface. */
    for (count = 0; count < interface->endpointList.count; count++)
    {
        usb_device_endpoint_init_struct_t epInitStruct;
        usb_device_endpoint_callback_struct_t epCallback;
        epInitStruct.zlt             = 0U;
        epInitStruct.interval        = interface->endpointList.endpoint[count].interval;
        epInitStruct.endpointAddress = interface->endpointList.endpoint[count].endpointAddress;
        epInitStruct.maxPacketSize   = interface->endpointList.endpoint[count].maxPacketSize;
        epInitStruct.transferType    = interface->endpointList.endpoint[count].transferType;

        if (USB_ENDPOINT_BULK == epInitStruct.transferType)
        {
            if (USB_REQUEST_TYPE_DIR_MASK ==
                (epInitStruct.endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
            {
                mtpHandle->bulkInEndpoint      = epInitStruct.endpointAddress;
                mtpHandle->bulkInMaxPacketSize = epInitStruct.maxPacketSize;
                epCallback.callbackFn          = USB_DeviceMtpBulkIn;
            }
            else
            {
                mtpHandle->bulkOutEndpoint      = epInitStruct.endpointAddress;
                mtpHandle->bulkOutMaxPacketSize = epInitStruct.maxPacketSize;
                epCallback.callbackFn           = USB_DeviceMtpBulkOut;
            }
        }
        else if (USB_ENDPOINT_INTERRUPT == epInitStruct.transferType)
        {
            mtpHandle->interruptInEndpoint = epInitStruct.endpointAddress;
            epCallback.callbackFn          = USB_DeviceMtpInterruptIn;
        }
        else
        {
            return error;
        }
        epCallback.callbackParam = mtpHandle;

        error = USB_DeviceInitEndpoint(mtpHandle->handle, &epInitStruct, &epCallback);
    }

    mtpHandle->mtpState             = USB_DEVICE_MTP_STATE_START;
    mtpHandle->bulkInStallFlag      = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
    mtpHandle->bulkOutStallFlag     = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
    mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL;
    mtpHandle->interruptInBusy      = 0U;

    return error;
}

/*!
 * @brief De-initialize the endpoints of the mtp class.
 *
 * This callback function is used to de-initialize the endpoints of the mtp class.
 *
 * @param mtpHandle          The device mtp class handle. It equals the value returned from
 * usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpEndpointsDeinit(usb_device_mtp_struct_t *mtpHandle)
{
    usb_status_t status = kStatus_USB_Error;
    uint32_t count;

    if (NULL == mtpHandle->interfaceHandle)
    {
        return status;
    }
    /* De-initialize all endpoints of the interface */
    for (count = 0; count < mtpHandle->interfaceHandle->endpointList.count; count++)
    {
        status = USB_DeviceDeinitEndpoint(mtpHandle->handle,
                                          mtpHandle->interfaceHandle->endpointList.endpoint[count].endpointAddress);
    }
    mtpHandle->interfaceHandle = NULL;
    return status;
}

/*!
 * @brief Initialize the mtp class.
 *
 * This function is used to initialize the mtp class.
 *
 * @param controllerId   The controller id of the USB IP. Please refer to the enumeration usb_controller_index_t.
 * @param config          The class configuration information.
 * @param handle          It is out parameter, is used to return pointer of the mtp class handle to the caller.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpInit(uint8_t controllerId, usb_device_class_config_struct_t *config, class_handle_t *handle)
{
    usb_device_mtp_struct_t *mtpHandle;
    usb_status_t error;

    /* Allocate a mtp class handle. */
    error = USB_DeviceMtpAllocateHandle(&mtpHandle);

    if (kStatus_USB_Success != error)
    {
        return error;
    }

    /* Get the device handle according to the controller id. */
    error = USB_DeviceClassGetDeviceHandle(controllerId, &mtpHandle->handle);

    if ((kStatus_USB_Success != error) || (NULL == mtpHandle->handle))
    {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if (kStatus_USB_Success != USB_DeviceMtpFreeHandle(mtpHandle))
        {
            return kStatus_USB_Error;
        }
#else
        (void)USB_DeviceMtpFreeHandle(mtpHandle);
#endif
        return error;
    }

    /* Save the configuration of the class. */
    mtpHandle->configurationStruct = config;
    /* Clear the configuration value. */
    mtpHandle->configuration = 0U;
    mtpHandle->alternate     = 0xFFU;
    mtpHandle->transactionID = 0xFFFFFFFFU;
    mtpHandle->sessionID     = 0U;

    *handle = (class_handle_t)mtpHandle;

    return error;
}

/*!
 * @brief De-initialize the device mtp class.
 *
 * The function de-initializes the device mtp class.
 *
 * @param handle The mtp class handle got from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpDeinit(class_handle_t handle)
{
    usb_device_mtp_struct_t *mtpHandle;
    usb_status_t status = kStatus_USB_Error;

    mtpHandle = (usb_device_mtp_struct_t *)handle;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }
    status = USB_DeviceMtpEndpointsDeinit(mtpHandle);
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceMtpFreeHandle(mtpHandle))
    {
        kStatus_USB_Error;
    }
#else
    (void)USB_DeviceMtpFreeHandle(mtpHandle);
#endif

    return status;
}

/*!
 * @brief Handle the event passed to the mtp class.
 *
 * This function handles the event passed to the mtp class.
 *
 * @param handle          The mtp class handle, got from the usb_device_class_config_struct_t::classHandle.
 * @param event           The event codes. Please refer to the enumeration usb_device_class_event_t.
 * @param param           The param type is determined by the event code.
 *
 * @return A USB error code or kStatus_USB_Success.
 * @retval kStatus_USB_Success              Free device handle successfully.
 * @retval kStatus_USB_InvalidParameter     The device handle not be found.
 * @retval kStatus_USB_InvalidRequest       The request is invalid, and the control pipe will be stalled by the caller.
 */
usb_status_t USB_DeviceMtpEvent(void *handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;
    usb_device_mtp_struct_t *mtpHandle;
    uint16_t interfaceAlternate;
    uint8_t *temp8;
    uint8_t alternate;
    usb_device_class_event_t eventCode = (usb_device_class_event_t)event;

    if ((NULL == param) || (NULL == handle))
    {
        return kStatus_USB_InvalidHandle;
    }

    /* Get the mtp class handle. */
    mtpHandle = (usb_device_mtp_struct_t *)handle;
    switch (eventCode)
    {
        case kUSB_DeviceClassEventDeviceReset:
            /* Bus reset, clear the configuration. */
            mtpHandle->configuration = 0U;
            mtpHandle->transactionID = 0xFFFFFFFFU;
            mtpHandle->sessionID     = 0U;
            error                    = kStatus_USB_Success;
            break;
        case kUSB_DeviceClassEventSetConfiguration:
            /* Get the new configuration. */
            temp8 = ((uint8_t *)param);
            if (NULL == mtpHandle->configurationStruct)
            {
                break;
            }
            if (*temp8 == mtpHandle->configuration)
            {
                error = kStatus_USB_Success;
                break;
            }

            if (0U != mtpHandle->configuration)
            {
                /* De-initialize the endpoints when current configuration is none zero. */
                error = USB_DeviceMtpEndpointsDeinit(mtpHandle);
            }
            /* Save new configuration. */
            mtpHandle->configuration = *temp8;
            /* Clear the alternate setting value. */
            mtpHandle->alternate = 0;
            /* Initialize the endpoints of the new current configuration by using the alternate setting 0. */
            error = USB_DeviceMtpEndpointsInit(mtpHandle);
            if (kStatus_USB_Success == error)
            {
                USB_DeviceMtpPrimeCommand(mtpHandle);
            }
            break;
        case kUSB_DeviceClassEventSetInterface:

            if (NULL == mtpHandle->configurationStruct)
            {
                break;
            }
            /* Get the new alternate setting of the interface */
            interfaceAlternate = *((uint16_t *)param);
            /* Get the alternate setting value */
            alternate = (uint8_t)(interfaceAlternate & 0xFFU);

            /* Whether the interface belongs to the class. */
            if (mtpHandle->interfaceNumber != ((uint8_t)(interfaceAlternate >> 8)))
            {
                break;
            }
            /* Only handle new alternate setting. */
            if (alternate == mtpHandle->alternate)
            {
                error = kStatus_USB_Success;
                break;
            }
            error = USB_DeviceMtpEndpointsDeinit(mtpHandle);
            /* Initialize new endpoints */
            error = USB_DeviceMtpEndpointsInit(mtpHandle);
            if (kStatus_USB_Success == error)
            {
                USB_DeviceMtpPrimeCommand(mtpHandle);
            }
            mtpHandle->alternate = alternate;

            break;
        case kUSB_DeviceClassEventSetEndpointHalt:
            if ((NULL == mtpHandle->configurationStruct) || (NULL == mtpHandle->interfaceHandle))
            {
                break;
            }
            /* Get the endpoint address */
            temp8 = ((uint8_t *)param);

            if ((mtpHandle->bulkInStallFlag != USB_DEVICE_MTP_STATE_BULK_IN_STALL) &&
                (*temp8 == mtpHandle->bulkInEndpoint))
            {
                /* Only stall the endpoint belongs to the class */
                mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_STALL;
                error                      = USB_DeviceStallEndpoint(mtpHandle->handle, *temp8);
            }
            if ((mtpHandle->bulkOutStallFlag != USB_DEVICE_MTP_STATE_BULK_OUT_STALL) &&
                (*temp8 == mtpHandle->bulkOutEndpoint))
            {
                mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_STALL;
                error                       = USB_DeviceStallEndpoint(mtpHandle->handle, *temp8);
            }

            if ((mtpHandle->interruptInStallFlag != USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL) &&
                (*temp8 == mtpHandle->interruptInEndpoint))
            {
                mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL;
                error                           = USB_DeviceStallEndpoint(mtpHandle->handle, *temp8);
            }

            break;
        case kUSB_DeviceClassEventClearEndpointHalt:
            if ((NULL == mtpHandle->configurationStruct) || (NULL == mtpHandle->interfaceHandle))
            {
                break;
            }

            /* Get the endpoint address */
            temp8 = ((uint8_t *)param);
            /* Only un-stall the endpoint belongs to the class , If the endpoint is in stall status ,then
             * un-stall it*/
            if ((mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL) &&
                (*temp8 == mtpHandle->bulkInEndpoint))
            {
                mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
                error                      = USB_DeviceUnstallEndpoint(mtpHandle->handle, *temp8);
                if (mtpHandle->bulkOutStallFlag != USB_DEVICE_MTP_STATE_BULK_OUT_STALL)
                {
                    USB_DeviceMtpPrimeCommand(mtpHandle);
                }
            }
            if ((mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL) &&
                (*temp8 == mtpHandle->bulkOutEndpoint))
            {
                mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
                error                       = USB_DeviceUnstallEndpoint(mtpHandle->handle, *temp8);
                if (mtpHandle->bulkInEndpoint != USB_DEVICE_MTP_STATE_BULK_IN_STALL)
                {
                    USB_DeviceMtpPrimeCommand(mtpHandle);
                }
            }
            if ((mtpHandle->interruptInStallFlag == USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL) &&
                (*temp8 == mtpHandle->interruptInEndpoint))
            {
                mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL;
                error                           = USB_DeviceUnstallEndpoint(mtpHandle->handle, *temp8);
            }
            break;
        case kUSB_DeviceClassEventClassRequest:
        {
            /* Handle the mtp class specific request. */
            usb_device_control_request_struct_t *control_request = (usb_device_control_request_struct_t *)param;

            if ((control_request->setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) !=
                USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
            {
                break;
            }

            if ((control_request->setup->wIndex & 0xFFU) != mtpHandle->interfaceNumber)
            {
                break;
            }

            error = kStatus_USB_InvalidRequest;
            switch (control_request->setup->bRequest)
            {
                case USB_DEVICE_MTP_CANCEL_REQUEST:
                    if ((0U == control_request->setup->wValue) && (0x0006U == control_request->setup->wLength) &&
                        ((control_request->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) ==
                         USB_REQUEST_TYPE_DIR_OUT))
                    {
                        if (control_request->isSetup != 0U)
                        {
                            if (0U == mtpHandle->isHostCancel)
                            {
                                /* assign buffer to receive data from host */
                                control_request->buffer = (uint8_t *)mtpHandle->cancelRequest;
                                control_request->length = 0x0006U;
                                mtpHandle->isHostCancel = 1;
                                error                   = kStatus_USB_Success;
                            }
                        }
                        else
                        {
                            if (0U != mtpHandle->isHostCancel)
                            {
                                if ((mtpHandle->cancelRequest->code == MTP_EVENT_CANCEL_TRANSACTION) &&
                                    (USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
                                         ((uint8_t *)&mtpHandle->cancelRequest->transactionID)) ==
                                     mtpHandle->transactionID))
                                {
                                    /* Host cancels current transaction */
                                    error = USB_DeviceMtpCancelCurrentTransaction(mtpHandle);
                                }
                            }
                        }
                    }
                    break;

                case USB_DEVICE_MTP_GET_EXTENDED_EVENT_DATA:
                    if ((0U == control_request->setup->wValue) && (0U != control_request->setup->wLength) &&
                        ((control_request->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) ==
                         USB_REQUEST_TYPE_DIR_IN))
                    {
                        usb_device_mtp_extended_event_struct_t extendedEvent;
                        extendedEvent.buffer = NULL;
                        extendedEvent.length = 0U;

                        error = mtpHandle->configurationStruct->classCallback(
                            (class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetExtendedEventData, &extendedEvent);

                        if ((kStatus_USB_Success == error) && (NULL != extendedEvent.buffer))
                        {
                            USB_LONG_TO_LITTLE_ENDIAN_ADDRESS(mtpHandle->mtpContainer->transactionID,
                                                              (extendedEvent.buffer + 2U));
                        }

                        control_request->buffer = extendedEvent.buffer;
                        control_request->length = extendedEvent.length;
                    }
                    break;

                case USB_DEVICE_MTP_DEVICE_RESET_REQUEST:
                    if ((0U == control_request->setup->wValue) && (0U == control_request->setup->wLength) &&
                        ((control_request->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) ==
                         USB_REQUEST_TYPE_DIR_OUT))
                    {
                        /* The DEVICE clears its command buffer, closes all open sessions, and returns to the
                           Configured State. */
                        mtpHandle->transactionID = 0xFFFFFFFFU;
                        mtpHandle->sessionID     = 0U;

                        if (mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL)
                        {
                            mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
                            error = USB_DeviceUnstallEndpoint(mtpHandle->handle, mtpHandle->bulkInEndpoint);
                            if (kStatus_USB_Success != error)
                            {
                                break;
                            }
                        }
                        if (mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL)
                        {
                            mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
                            error = USB_DeviceUnstallEndpoint(mtpHandle->handle, mtpHandle->bulkOutEndpoint);
                            if (kStatus_USB_Success != error)
                            {
                                break;
                            }
                        }
                        if (mtpHandle->interruptInStallFlag == USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL)
                        {
                            mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL;
                            error = USB_DeviceUnstallEndpoint(mtpHandle->handle, mtpHandle->interruptInEndpoint);
                            if (kStatus_USB_Success != error)
                            {
                                break;
                            }
                        }

                        error = mtpHandle->configurationStruct->classCallback(
                            (class_handle_t)mtpHandle, kUSB_DeviceMtpEventDeviceResetRequest, NULL);

                        USB_DeviceMtpPrimeCommand(mtpHandle);
                    }
                    break;

                case USB_DEVICE_MTP_GET_DEVICE_STATUS_REQUEST:
                    if ((0U == control_request->setup->wValue) && (4U <= control_request->setup->wLength) &&
                        ((control_request->setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) ==
                         USB_REQUEST_TYPE_DIR_IN))
                    {
                        if ((mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL) ||
                            (mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL))
                        {
                            /* do nothing, device status has been built before. */
                        }
                        else if (0U != mtpHandle->isHostCancel)
                        {
                            mtpHandle->deviceStatus->wLength = 4U;
                            mtpHandle->deviceStatus->code    = MTP_RESPONSE_DEVICE_BUSY;

                            mtpHandle->isHostCancel = 0;
                            USB_DeviceMtpPrimeCommand(mtpHandle);
                        }
                        else
                        {
                            mtpHandle->deviceStatus->wLength = 4U;
                            mtpHandle->deviceStatus->code    = MTP_RESPONSE_OK;
                        }

                        control_request->buffer = (uint8_t *)mtpHandle->deviceStatus;
                        control_request->length = mtpHandle->deviceStatus->wLength;

                        error = kStatus_USB_Success;
                    }
                    break;

                default:
                    /*no action*/
                    break;
            }
        }
        break;

        default:
            /*no action*/
            break;
    }

    return error;
}

usb_status_t USB_DeviceMtpRecv(usb_device_mtp_struct_t *mtpHandle)
{
    uint32_t size;

    /* which one is smaller? */
    /* size =
       (mtpHandle->transferTotal > mtpHandle->transferLength) ? mtpHandle->transferLength : mtpHandle->transferTotal; */
    size = (USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH > mtpHandle->transferLength) ?
               mtpHandle->transferLength :
               USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH;

    return USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint,
                                 &mtpHandle->transferBuffer[mtpHandle->transferOffset], size);
}

usb_status_t USB_DeviceMtpSend(usb_device_mtp_struct_t *mtpHandle)
{
    uint32_t size;

    /* which one is smaller? */
    size =
        (mtpHandle->transferTotal > mtpHandle->transferLength) ? mtpHandle->transferLength : mtpHandle->transferTotal;
    size = (USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH > size) ? size : USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH;

    return USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkInEndpoint,
                                 &mtpHandle->transferBuffer[mtpHandle->transferOffset], size);
}

/*!
 * @brief Send event through interrupt in endpoint.
 *
 * The function is used to send event through interrupt in endpoint.
 * The function calls USB_DeviceSendRequest internally.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 * @param event Please refer to the structure usb_device_mtp_event_struct_t.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the sending request is successful or not; the transfer done is notified by
 * USB_DeviceMtpInterruptIn.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceMtpEventSend(class_handle_t handle, usb_device_mtp_event_struct_t *event)
{
    usb_device_mtp_struct_t *mtpHandle;
    usb_status_t error = kStatus_USB_Error;

    if (NULL == handle)
    {
        return kStatus_USB_InvalidHandle;
    }

    mtpHandle = (usb_device_mtp_struct_t *)handle;

    if (0U != mtpHandle->interruptInBusy)
    {
        return kStatus_USB_Busy;
    }

    mtpHandle->interruptInBusy = 1U;

    /* build event structure. */
    mtpHandle->eventContainer->containerLength = (event->paramNumber * 4U) + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    mtpHandle->eventContainer->containerType   = USB_DEVICE_MTP_CONTAINER_TYPE_EVENT;
    mtpHandle->eventContainer->code            = event->code;
    mtpHandle->eventContainer->transactionID   = mtpHandle->mtpContainer->transactionID;
    mtpHandle->eventContainer->param1          = event->param1;
    mtpHandle->eventContainer->param2          = event->param2;
    mtpHandle->eventContainer->param3          = event->param3;

    error = USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->interruptInEndpoint,
                                  (uint8_t *)mtpHandle->eventContainer, mtpHandle->eventContainer->containerLength);

    if (kStatus_USB_Success != error)
    {
        mtpHandle->interruptInBusy = 0U;
    }
    return error;
}

/*!
 * @brief Send response through bulk in endpoint.
 *
 * The function is used to send response through bulk in endpoint.
 * The function calls USB_DeviceSendRequest internally.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 * @param response Please refer to the structure usb_device_mtp_response_struct_t.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The function is used to asynchronously send response to the host. Some operations may consume a lot of time to
 * handle current transaction, such as CopyObject or DeleteObject, which causes the subsequent USB event cannot be
 * responded in time. In this case, a separated task is needed to handle these operations. When the process is complete,
 * a response needs to be sent to the host by calling this function.
 */
usb_status_t USB_DeviceMtpResponseSend(class_handle_t handle, usb_device_mtp_response_struct_t *response)
{
    usb_device_mtp_struct_t *mtpHandle;

    if (NULL == handle)
    {
        return kStatus_USB_InvalidHandle;
    }

    mtpHandle = (usb_device_mtp_struct_t *)handle;

    return USB_DeviceMtpPrimeResponse(mtpHandle, response->code, &response->param1, response->paramNumber);
}

/*!
 * @brief Cancel current transacion.
 *
 * The function is used to cancel current transaction in the bulk in, bulk out and interrupt in endpoints.
 * The function calls USB_DeviceCancel internally. If there is a transaction ongoing, the function will call
 * callback function to inform application that the transaction is cancelled.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpCancelCurrentTransaction(class_handle_t handle)
{
    usb_device_mtp_struct_t *mtpHandle;
    usb_device_mtp_cmd_data_struct_t dataInfo;

    if (NULL == handle)
    {
        return kStatus_USB_InvalidHandle;
    }

    mtpHandle = (usb_device_mtp_struct_t *)handle;

#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if ((kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkInEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkOutEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->interruptInEndpoint)))
    {
        kStatus_USB_Error;
    }
#else
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkInEndpoint);
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkOutEndpoint);
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->interruptInEndpoint);
#endif

    /* callback to cancel current transaction */
    dataInfo.curSize  = 0;
    dataInfo.code     = MTP_RESPONSE_UNDEFINED;
    dataInfo.curPhase = USB_DEVICE_MTP_PHASE_CANCELLATION;
    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

    return kStatus_USB_Success;
}

#endif /* USB_DEVICE_CONFIG_MTP */
