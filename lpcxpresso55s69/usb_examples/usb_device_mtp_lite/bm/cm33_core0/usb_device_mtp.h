/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_MTP_H__
#define __USB_DEVICE_MTP_H__

#include "usb_device_mtp_operation.h"
/*!
 * @addtogroup usb_mtp
 * @{
 */

/*! @brief The class code of the MTP class */
#define USB_DEVICE_CONFIG_MTP_CLASS_CODE (0x06U)

/*! @brief Cancel Request (class-specific request) */
#define USB_DEVICE_MTP_CANCEL_REQUEST (0x64U)
/*! @brief Get Extended Event Data (class-specific request) */
#define USB_DEVICE_MTP_GET_EXTENDED_EVENT_DATA (0x65U)
/*! @brief Device Reset Request (class-specific request) */
#define USB_DEVICE_MTP_DEVICE_RESET_REQUEST (0x66U)
/*! @brief Device Reset Request (class-specific request) */
#define USB_DEVICE_MTP_GET_DEVICE_STATUS_REQUEST (0x67U)

/*! @brief Minimum container length*/
#define USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH (12U)
/*! @brief Command container length*/
#define USB_DEVICE_MTP_COMMAND_LENGTH (32U)

/*! @brief Default invalid value, parameter in the operation or maximum container length */
#define USB_DEVICE_MTP_MAX_UINT32_VAL (0xFFFFFFFFU)
/*! @brief Default invalid value is used to support >4GB file transfer */
#define USB_DEVICE_MTP_MAX_UINT64_VAL (0xFFFFFFFFFFFFFFFFU)

/*! @brief MTP current phase */
#define USB_DEVICE_MTP_PHASE_COMMAND      (1U)
#define USB_DEVICE_MTP_PHASE_DATA         (2U)
#define USB_DEVICE_MTP_PHASE_RESPONSE     (3U)
#define USB_DEVICE_MTP_PHASE_CANCELLATION (4U)

/*! @brief MTP container type */
#define USB_DEVICE_MTP_CONTAINER_TYPE_UNDEFINED (0U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_COMMAND   (1U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_DATA      (2U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE  (3U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_EVENT     (4U)

/*! @brief MTP state */
#define USB_DEVICE_MTP_STATE_START                (0U)
#define USB_DEVICE_MTP_STATE_COMMAND              (1U)
#define USB_DEVICE_MTP_STATE_BULK_IN              (2U)
#define USB_DEVICE_MTP_STATE_BULK_OUT             (3U)
#define USB_DEVICE_MTP_STATE_RESPONSE             (4U)
#define USB_DEVICE_MTP_STATE_BULK_IN_STALL        (1U)
#define USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL      (0U)
#define USB_DEVICE_MTP_STATE_BULK_OUT_STALL       (1U)
#define USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL     (0U)
#define USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL   (1U)
#define USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL (0U)

/* Max transfer length */
#define USB_DEVICE_MTP_MAX_RECV_TRANSFER_LENGTH (65536U)
#define USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH (65536U)

/*! @brief MTP callback event */
typedef enum
{
    kUSB_DeviceMtpEventInvalid = 0U,
    kUSB_DeviceMtpEventOpenSession,
    kUSB_DeviceMtpEventCloseSession,
    kUSB_DeviceMtpEventGetDeviceInfo,
    kUSB_DeviceMtpEventGetDevicePropDesc,
    kUSB_DeviceMtpEventGetObjPropsSupported,
    kUSB_DeviceMtpEventGetStorageIDs,
    kUSB_DeviceMtpEventGetStorageInfo,
    kUSB_DeviceMtpEventGetObjHandles,
    kUSB_DeviceMtpEventGetObjPropDesc,
    kUSB_DeviceMtpEventGetObjPropList,
    kUSB_DeviceMtpEventGetObjInfo,
    kUSB_DeviceMtpEventGetObj,
    kUSB_DeviceMtpEventSendObjInfo,
    kUSB_DeviceMtpEventSendObj,
    kUSB_DeviceMtpEventDeleteObj,
    kUSB_DeviceMtpEventGetDevicePropVal,
    kUSB_DeviceMtpEventSetDevicePropVal,
    kUSB_DeviceMtpEventGetObjPropVal,
    kUSB_DeviceMtpEventSetObjPropVal,
    kUSB_DeviceMtpEventGetObjReferences,
    kUSB_DeviceMtpEventMoveObj,
    kUSB_DeviceMtpEventCopyObj,
    kUSB_DeviceMtpEventSendResponseError,    /*!< the result of asynchronous event notification */
    kUSB_DeviceMtpEventSendResponseSuccess,  /*!< the result of asynchronous event notification */
    kUSB_DeviceMtpEventDeviceResetRequest,   /*!< class specific request callback */
    kUSB_DeviceMtpEventGetExtendedEventData, /*!< class specific request callback */
} usb_device_mtp_callback_event_t;

/*! @brief MTP generic container structure */
typedef struct _usb_device_mtp_container
{
    uint32_t containerLength;
    uint16_t containerType;
    uint16_t code;
    uint32_t transactionID;
    uint32_t param[5];
} usb_device_mtp_container_t;

/*! @brief MTP asynchronous event interrupt data format */
typedef struct _usb_device_mtp_event_container
{
    uint32_t containerLength;
    uint16_t containerType;
    uint16_t code;
    uint32_t transactionID;
    uint32_t param1;
    uint32_t param2;
    uint32_t param3;
} usb_device_mtp_event_container_t;

/*! @brief MTP format of get device status request data */
typedef struct _usb_device_mtp_device_status
{
    uint16_t wLength;
    uint16_t code;
    uint8_t epNumber1;
    uint8_t epNumber2;
} usb_device_mtp_device_status_t;

/*! @brief MTP format of cancel request data */
STRUCT_PACKED
struct _usb_device_mtp_cancel_request
{
    uint16_t code;
    uint32_t transactionID;
} STRUCT_UNPACKED;
typedef struct _usb_device_mtp_cancel_request usb_device_mtp_cancel_request_t;

/*! @brief MTP format of get extended event data request */
typedef struct _usb_device_mtp_extended_event_data
{
    uint16_t code;
    uint16_t paramNumber;
    struct
    {
        uint16_t paramSize;
        uint8_t *param;
    } * paramArray;
} usb_device_mtp_extended_event_data_t;

/*! @brief MTP command callback structure */
typedef struct _usb_device_mtp_cmd_data_struct
{
    uint8_t *buffer;    /*!< [IN] The memory address to hold the data need to be transferred. */
    uint32_t curSize;   /*!< [IN] In the command or data phase, used to save currently how many bytes need to be
                           transferred. */
                        /*!< [IN] In the response phase, used to save the number of response parameter */
    uint64_t totalSize; /*!< [IN] In total how many bytes need to be sent. */
                        /*!< [OUT] In total how many bytes need to be received. */
    uint64_t curPos;    /*!< [OUT] the number of bytes has been transferred. */
    uint32_t param[5];  /*!< [OUT] In the command phase, used to save command parameter. */
                        /*!< [IN] In the repsonse phase, used to save response parameter. */
    uint16_t code;      /*!< [IN] response code */
    uint16_t curPhase;  /*!< [OUT] In the command phase, equals to USB_DEVICE_MTP_PHASE_COMMAND. */
                        /*!< [OUT] In the data phase, equals to USB_DEVICE_MTP_PHASE_DATA. */
                        /*!< [OUT] In the response phase, equals to USB_DEVICE_MTP_PHASE_RESPONSE. */
                        /*!< [OUT] When host or device cancels the current transaction,
                              equals to USB_DEVICE_MTP_PHASE_CANCELLATION. */
} usb_device_mtp_cmd_data_struct_t;

/*! @brief MTP response callback structure */
typedef struct _usb_device_mtp_response_struct
{
    uint16_t code;        /*!< MTP response code, such as MTP_RESPONSE_OK, MTP_RESPONSE_SESSION_NOT_OPEN, etc. For more
                               response codes, please refer to Media Transfer Protocol Rev 1.1, Appendix F - Responses. */
    uint16_t paramNumber; /*!< The number of parameter. */
    uint32_t param1;      /*!< This field contains the 1st parameter associated with the event if needed.*/
    uint32_t param2;      /*!< This field contains the 2nd parameter associated with the event if needed.*/
    uint32_t param3;      /*!< This field contains the 3rd parameter associated with the event if needed.*/
    uint32_t param4;      /*!< This field contains the 4th parameter associated with the event if needed.*/
    uint32_t param5;      /*!< This field contains the 5th parameter associated with the event if needed.*/
} usb_device_mtp_response_struct_t;

/*! @brief MTP event callback structure */
typedef struct _usb_device_mtp_event_struct
{
    uint16_t code;        /*!< MTP event code, such as MTP_EVENT_OBJECT_ADDED, MTP_EVENT_OBJECT_REMOVED, etc. For more
                               event codes, please refer to Media Transfer Protocol Rev 1.1, Appendix G - Events. */
    uint16_t paramNumber; /*!< The number of parameter. */
    uint32_t param1;      /*!< This field contains the 1st parameter associated with the event if needed.*/
    uint32_t param2;      /*!< This field contains the 2nd parameter associated with the event if needed.*/
    uint32_t param3;      /*!< This field contains the 3rd parameter associated with the event if needed.*/
} usb_device_mtp_event_struct_t;

/*! @brief MTP get extended event callback structure */
typedef struct _usb_device_mtp_extended_event_struct
{
    uint8_t *buffer; /*!< The memory address to hold the data need to be sent. User needs to organize the buffer
                          according to USB Still Image Capture Device Definition Rev 1.0, Table 5.2-4. */
    uint32_t length; /*!< The data length need to be sent.*/
} usb_device_mtp_extended_event_struct_t;

/*! @brief The MTP device structure */
typedef struct _usb_device_mtp_struct
{
    usb_device_handle handle; /*!< The device handle */

    uint32_t sessionID;     /*!< MTP session ID */
    uint32_t transactionID; /*!< MTP transaction ID */

    uint8_t *transferBuffer;
    uint64_t transferTotal;  /*!< Total length of data to be transferred */
    uint64_t transferDone;   /*!< Length of data transferred */
    uint32_t transferOffset; /*!< Transfer backward offset */
    uint32_t transferLength; /*!< Transfer forward offset */

    usb_device_mtp_container_t *mtpContainer;         /*!< Command or Response structure */
    usb_device_mtp_event_container_t *eventContainer; /*!< Event structure */
    usb_device_mtp_device_status_t *deviceStatus;     /*!< Device status request */
    uint16_t bulkInMaxPacketSize;                     /*!< Max packet size in bulk in endpoint */
    uint16_t bulkOutMaxPacketSize;                    /*!< Max packet size in bulk out endpoint */
    uint8_t mtpState;
    uint8_t bulkInStallFlag;
    uint8_t bulkOutStallFlag;
    uint8_t interruptInStallFlag;
    uint8_t interruptInBusy;

    uint8_t isHostCancel; /*!< Host cancels current transaction */

    uint8_t bulkInEndpoint;      /*!< Bulk in endpoint number*/
    uint8_t bulkOutEndpoint;     /*!< Bulk out endpoint number*/
    uint8_t interruptInEndpoint; /*!< Interrupt in endpoint number*/
    uint8_t alternate;           /*!< Current alternate setting of the interface */
    uint8_t configuration;       /*!< Current configuration */
    uint8_t interfaceNumber;     /*!< The interface number of the class */
} usb_device_mtp_struct_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

/*! @}*/

#endif
