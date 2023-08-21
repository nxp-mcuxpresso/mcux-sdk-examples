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
 * @addtogroup usb_device_mtp_drv
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

/*! @brief Minimum container length */
#define USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH (12U)
/*! @brief Command container length */
#define USB_DEVICE_MTP_COMMAND_LENGTH (32U)

/*! @brief Default invalid value, parameter in the operation or maximum container length */
#define USB_DEVICE_MTP_MAX_UINT32_VAL (0xFFFFFFFFU)
/*! @brief Default invalid value is used to support >4GB file transfer */
#define USB_DEVICE_MTP_MAX_UINT64_VAL (0xFFFFFFFFFFFFFFFFU)

/*!
 * @name USB MTP current phase
 * @{
 */

#define USB_DEVICE_MTP_PHASE_COMMAND      (1U)
#define USB_DEVICE_MTP_PHASE_DATA         (2U)
#define USB_DEVICE_MTP_PHASE_RESPONSE     (3U)
#define USB_DEVICE_MTP_PHASE_CANCELLATION (4U)

/*! @}*/

/*!
 * @name USB MTP container type
 * @{
 */

#define USB_DEVICE_MTP_CONTAINER_TYPE_UNDEFINED (0U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_COMMAND   (1U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_DATA      (2U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE  (3U)
#define USB_DEVICE_MTP_CONTAINER_TYPE_EVENT     (4U)

/*! @}*/

/* MTP state */
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
    kUSB_DeviceMtpEventInvalid = 0U,         /*!< Invalid value */
    kUSB_DeviceMtpEventOpenSession,          /*!< OpenSession command */
    kUSB_DeviceMtpEventCloseSession,         /*!< CloseSession command */
    kUSB_DeviceMtpEventGetDeviceInfo,        /*!< GetDeviceInfo command */
    kUSB_DeviceMtpEventGetDevicePropDesc,    /*!< GetDevicePropDesc command */
    kUSB_DeviceMtpEventGetObjPropsSupported, /*!< GetObjectPropsSupported command */
    kUSB_DeviceMtpEventGetStorageIDs,        /*!< GetStorageIDs command */
    kUSB_DeviceMtpEventGetStorageInfo,       /*!< GetStorageInfo command */
    kUSB_DeviceMtpEventGetObjHandles,        /*!< GetObjectHandles command */
    kUSB_DeviceMtpEventGetObjPropDesc,       /*!< GetObjectPropDesc command */
    kUSB_DeviceMtpEventGetObjPropList,       /*!< GetObjectPropList command */
    kUSB_DeviceMtpEventGetObjInfo,           /*!< GetObjectInfo command */
    kUSB_DeviceMtpEventGetObj,               /*!< GetObject command */
    kUSB_DeviceMtpEventSendObjInfo,          /*!< SendObjectInfo command */
    kUSB_DeviceMtpEventSendObj,              /*!< SendObject command */
    kUSB_DeviceMtpEventDeleteObj,            /*!< DeleteObject command */
    kUSB_DeviceMtpEventGetDevicePropVal,     /*!< GetDevicePropVal command */
    kUSB_DeviceMtpEventSetDevicePropVal,     /*!< SetDevicePropVal command */
    kUSB_DeviceMtpEventGetObjPropVal,        /*!< GetObjectPropVal command */
    kUSB_DeviceMtpEventSetObjPropVal,        /*!< SetObjectPropVal command */
    kUSB_DeviceMtpEventGetObjReferences,     /*!< GetObjectReferences command */
    kUSB_DeviceMtpEventMoveObj,              /*!< MoveObject command */
    kUSB_DeviceMtpEventCopyObj,              /*!< CopyObject command */
    kUSB_DeviceMtpEventSendResponseError,    /*!< The result of asynchronous event notification */
    kUSB_DeviceMtpEventSendResponseSuccess,  /*!< The result of asynchronous event notification */
    kUSB_DeviceMtpEventDeviceResetRequest,   /*!< Class specific request callback */
    kUSB_DeviceMtpEventGetExtendedEventData, /*!< Class specific request callback */
} usb_device_mtp_callback_event_t;

/*! @brief MTP generic container structure.
 * The structure is used as a header to transfer data in the bulk pipe, and only used by the class driver.
 */
typedef struct _usb_device_mtp_container
{
    uint32_t containerLength;
    uint16_t containerType;
    uint16_t code;
    uint32_t transactionID;
    uint32_t param[5];
} usb_device_mtp_container_t;

/*! @brief MTP asynchronous event interrupt data format.
 * The structure is used by the class driver to notify the host of occurrence of certain events.
 */
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

/*! @brief MTP format of get device status request data.
 * The structure is used by the class driver to transfer the status and protocol state of device to the host.
 */
typedef struct _usb_device_mtp_device_status
{
    uint16_t wLength;
    uint16_t code;
    uint8_t epNumber1;
    uint8_t epNumber2;
} usb_device_mtp_device_status_t;

/*! @brief MTP format of cancel request data.
 * The structure is used by the class driver to receive the cancel request data from the host.
 */
STRUCT_PACKED
struct _usb_device_mtp_cancel_request
{
    uint16_t code;
    uint32_t transactionID;
} STRUCT_UNPACKED;
typedef struct _usb_device_mtp_cancel_request usb_device_mtp_cancel_request_t;

/*! @brief MTP format of get extended event data request.
 * The structure is used by the class driver to transfer the extended information regarding
 * an asynchronous event or vendor condition to the host.
 */
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
    uint64_t curPos;    /*!< [OUT] The number of bytes has been transferred. */
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
    usb_device_handle handle;                              /*!< The device handle */
    usb_device_class_config_struct_t *configurationStruct; /*!< The configuration of the class*/
    usb_device_interface_struct_t *interfaceHandle;        /*!< Current interface handle */

    uint32_t sessionID;     /*!< MTP session ID */
    uint32_t transactionID; /*!< MTP transaction ID */

    uint8_t *transferBuffer; /*!< Data buffer */
    uint64_t transferTotal;  /*!< The total length of data to be transferred */
    uint64_t transferDone;   /*!< The length of data transferred */
    uint32_t transferOffset; /*!< Transfer backward offset */
    uint32_t transferLength; /*!< Transfer forward offset */

    usb_device_mtp_container_t *mtpContainer;         /*!< Command or Response structure */
    usb_device_mtp_event_container_t *eventContainer; /*!< Event structure */
    usb_device_mtp_device_status_t *deviceStatus;     /*!< Device status request */
    usb_device_mtp_cancel_request_t *cancelRequest;   /*!< Cancel request */
    uint16_t bulkInMaxPacketSize;                     /*!< Max packet size in bulk in endpoint */
    uint16_t bulkOutMaxPacketSize;                    /*!< Max packet size in bulk out endpoint */
    uint8_t mtpState;                                 /*!< Internal referenced MTP state */
    uint8_t bulkInStallFlag;                          /*!< Bulk IN endpoint stall flag */
    uint8_t bulkOutStallFlag;                         /*!< Bulk OUT endpoint stall flag */
    uint8_t interruptInStallFlag;                     /*!< Interrupt IN endpoint stall flag*/
    uint8_t interruptInBusy;                          /*!< Interrupt IN endpoint busy flag */

    uint8_t isHostCancel; /*!< Host cancels current transaction */

    uint8_t bulkInEndpoint;      /*!< Bulk IN endpoint number*/
    uint8_t bulkOutEndpoint;     /*!< Bulk OUT endpoint number*/
    uint8_t interruptInEndpoint; /*!< Interrupt IN endpoint number*/
    uint8_t alternate;           /*!< Current alternate setting of the interface */
    uint8_t configuration;       /*!< Current configuration */
    uint8_t interfaceNumber;     /*!< The interface number of the class */
} usb_device_mtp_struct_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @brief Initializes the MTP class.
 *
 * This function is used to initialize the MTP class.
 *
 * @param controllerId   The controller ID of the USB IP. See the enumeration usb_controller_index_t.
 * @param config          The class configuration information.
 * @param handle          A parameter used to return pointer of the MTP class handle to the caller.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMtpInit(uint8_t controllerId,
                                      usb_device_class_config_struct_t *config,
                                      class_handle_t *handle);
/*!
 * @brief Deinitializes the device MTP class.
 *
 * The function deinitializes the device MTP class.
 *
 * @param handle The MTP class handle received from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMtpDeinit(class_handle_t handle);

/*!
 * @brief Handles the event passed to the MTP class.
 *
 * This function handles the event passed to the MTP class. This function only can be called by #USB_DeviceClassEvent.
 *
 * @param[in] handle          The MTP class handle received from the usb_device_class_config_struct_t::classHandle.
 * @param[in] event           The event codes. See the enumeration usb_device_class_event_t.
 * @param[in,out] param           The parameter type is determined by the event code.
 *
 * @return A USB error code or kStatus_USB_Success.
 * @retval kStatus_USB_Success              Free device handle successfully.
 * @retval kStatus_USB_InvalidParameter     The device handle not be found.
 * @retval kStatus_USB_InvalidRequest       The request is invalid, and the control pipe is stalled by the caller.
 */
extern usb_status_t USB_DeviceMtpEvent(void *handle, uint32_t event, void *param);

/*!
 * @name USB device MTP class APIs
 * @{
 */

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
extern usb_status_t USB_DeviceMtpEventSend(class_handle_t handle, usb_device_mtp_event_struct_t *event);

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
extern usb_status_t USB_DeviceMtpResponseSend(class_handle_t handle, usb_device_mtp_response_struct_t *response);

/*!
 * @brief Cancel current transacion.
 *
 * The function is used to cancel current transaction in the bulk in, bulk out and interrupt in endpoints.
 * The function calls USB_DeviceCancel internally.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 */
extern usb_status_t USB_DeviceMtpCancelCurrentTransaction(class_handle_t handle);

/*! @}*/

#ifdef __cplusplus
}
#endif

/*! @}*/

#endif /* __USB_DEVICE_MTP_H__ */
