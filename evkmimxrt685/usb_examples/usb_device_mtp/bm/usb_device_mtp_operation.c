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

#if USB_DEVICE_CONFIG_MTP
#include "usb_device_mtp.h"
#include "usb_device_mtp_operation.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern usb_status_t USB_DeviceMtpPrimeResponse(usb_device_mtp_struct_t *mtpHandle,
                                               uint16_t respCode,
                                               uint32_t *respParam,
                                               uint8_t respParamSize);

extern void USB_DevicePrimeDataIn(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo);

extern void USB_DevicePrimeDataOut(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo);

extern void USB_DevicePrimeBulkInAndOutStall(usb_device_mtp_struct_t *mtpHandle, uint16_t code, uint8_t epNeed);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static inline usb_status_t USB_DeviceMtpOpenSession(usb_device_mtp_struct_t *mtpHandle,
                                                    usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

        if (0U == mtpHandle->mtpContainer->param[0])
        {
            dataInfo->code = MTP_RESPONSE_INVALID_PARAMETER;
            return kStatus_USB_Success;
        }

        if (mtpHandle->sessionID == mtpHandle->mtpContainer->param[0])
        {
            dataInfo->code = MTP_RESPONSE_SESSION_ALREADY_OPEN;
            return kStatus_USB_Success;
        }

        mtpHandle->sessionID = mtpHandle->mtpContainer->param[0];
    }

    /* classCallback is initialized in classInit of s_UsbDeviceClassInterfaceMap,
       it is from the second parameter of classInit */
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventOpenSession,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpCloseSession(usb_device_mtp_struct_t *mtpHandle,
                                                     usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

        if (0U == mtpHandle->sessionID)
        {
            dataInfo->code = MTP_RESPONSE_SESSION_NOT_OPEN;
            return kStatus_USB_Success;
        }

        mtpHandle->sessionID     = 0;
        mtpHandle->transactionID = 0xFFFFFFFFU;
    }

    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventCloseSession,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetDeviceInfo(usb_device_mtp_struct_t *mtpHandle,
                                                      usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetDeviceInfo,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetDevicePropDesc(usb_device_mtp_struct_t *mtpHandle,
                                                          usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle,
                                                         kUSB_DeviceMtpEventGetDevicePropDesc, dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjPropsSupported(usb_device_mtp_struct_t *mtpHandle,
                                                             usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle,
                                                         kUSB_DeviceMtpEventGetObjPropsSupported, dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetStorageIDs(usb_device_mtp_struct_t *mtpHandle,
                                                      usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetStorageIDs,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetStorageInfo(usb_device_mtp_struct_t *mtpHandle,
                                                       usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetStorageInfo,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjHandles(usb_device_mtp_struct_t *mtpHandle,
                                                      usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjHandles,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjPropDesc(usb_device_mtp_struct_t *mtpHandle,
                                                       usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjPropDesc,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjPropList(usb_device_mtp_struct_t *mtpHandle,
                                                       usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        /* Object Property Code and Object Property Group Code equals to 0. */
        if ((mtpHandle->mtpContainer->param[2] == 0U) && (mtpHandle->mtpContainer->param[3] == 0U))
        {
            dataInfo->code = MTP_RESPONSE_PARAMETER_NOT_SUPPORTED;
            return kStatus_USB_Success;
        }
    }

    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjPropList,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjInfo(usb_device_mtp_struct_t *mtpHandle,
                                                   usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjInfo,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObj(usb_device_mtp_struct_t *mtpHandle,
                                               usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObj,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpSendObjInfo(usb_device_mtp_struct_t *mtpHandle,
                                                    usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventSendObjInfo,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpSendObj(usb_device_mtp_struct_t *mtpHandle,
                                                usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventSendObj,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpDelObj(usb_device_mtp_struct_t *mtpHandle,
                                               usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventDeleteObj,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetDevicePropVal(usb_device_mtp_struct_t *mtpHandle,
                                                         usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetDevicePropVal,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpSetDevicePropVal(usb_device_mtp_struct_t *mtpHandle,
                                                         usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventSetDevicePropVal,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjPropVal(usb_device_mtp_struct_t *mtpHandle,
                                                      usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjPropVal,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpSetObjPropVal(usb_device_mtp_struct_t *mtpHandle,
                                                      usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventSetObjPropVal,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpGetObjReferences(usb_device_mtp_struct_t *mtpHandle,
                                                         usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventGetObjReferences,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpMoveObj(usb_device_mtp_struct_t *mtpHandle,
                                                usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventMoveObj,
                                                         dataInfo);
}

static inline usb_status_t USB_DeviceMtpCopyObj(usb_device_mtp_struct_t *mtpHandle,
                                                usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

    return mtpHandle->configurationStruct->classCallback((class_handle_t)mtpHandle, kUSB_DeviceMtpEventCopyObj,
                                                         dataInfo);
}

void USB_DeviceMtpProcessCommand(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    usb_status_t status = kStatus_USB_Success;

    do
    {
        /* In the command phase, check transaction and session ID */
        if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
        {
            /* Check transction ID */
            if (mtpHandle->mtpContainer->transactionID == (mtpHandle->transactionID + 1U))
            {
                mtpHandle->transactionID = mtpHandle->mtpContainer->transactionID;

                if (mtpHandle->transactionID == 0xFFFFFFFEU)
                {
                    mtpHandle->transactionID = 0;
                }
            }
            else
            {
                dataInfo->code = MTP_RESPONSE_INVALID_TRANSACTION_ID;
                break;
            }

            /* Check session ID */
            if (mtpHandle->mtpContainer->code != MTP_OPERATION_OPEN_SESSION &&
                mtpHandle->mtpContainer->code != MTP_OPERATION_GET_DEVICE_INFO)
            {
                if (mtpHandle->sessionID == 0U)
                {
                    dataInfo->code = MTP_RESPONSE_SESSION_NOT_OPEN;
                    break;
                }
            }
        }

        /* Transaction and Session ID is correct, continue to proccess command. */
        switch (mtpHandle->mtpContainer->code)
        {
            case MTP_OPERATION_OPEN_SESSION:
                status = USB_DeviceMtpOpenSession(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_CLOSE_SESSION:
                status = USB_DeviceMtpCloseSession(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_DEVICE_INFO:
                status = USB_DeviceMtpGetDeviceInfo(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_DEVICE_PROP_DESC:
                status = USB_DeviceMtpGetDevicePropDesc(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
                status = USB_DeviceMtpGetObjPropsSupported(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_STORAGE_IDS:
                status = USB_DeviceMtpGetStorageIDs(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_STORAGE_INFO:
                status = USB_DeviceMtpGetStorageInfo(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_HANDLES:
                status = USB_DeviceMtpGetObjHandles(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_DESC:
                status = USB_DeviceMtpGetObjPropDesc(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_LIST:
                status = USB_DeviceMtpGetObjPropList(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_INFO:
                status = USB_DeviceMtpGetObjInfo(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT:
                status = USB_DeviceMtpGetObj(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_SEND_OBJECT_INFO:
                status = USB_DeviceMtpSendObjInfo(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_SEND_OBJECT:
                status = USB_DeviceMtpSendObj(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_DELETE_OBJECT:
                status = USB_DeviceMtpDelObj(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
                status = USB_DeviceMtpGetDevicePropVal(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
                status = USB_DeviceMtpSetDevicePropVal(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
                status = USB_DeviceMtpGetObjPropVal(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
                status = USB_DeviceMtpSetObjPropVal(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_REFERENCES:
                status = USB_DeviceMtpGetObjReferences(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_MOVE_OBJECT:
                status = USB_DeviceMtpMoveObj(mtpHandle, dataInfo);
                break;

            case MTP_OPERATION_COPY_OBJECT:
                status = USB_DeviceMtpCopyObj(mtpHandle, dataInfo);
                break;

            default:
                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    /* Invalid command, need to stall Bulk-in & Bulk-out endpoints */
                    dataInfo->code = MTP_RESPONSE_OPERATION_NOT_SUPPORTED;
                }
                break;
        }

        /* Prime first data block */
        if ((dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND) && (dataInfo->code == MTP_RESPONSE_UNDEFINED) &&
            (status == kStatus_USB_Success))
        {
            switch (mtpHandle->mtpContainer->code)
            {
                case MTP_OPERATION_GET_DEVICE_INFO:
                case MTP_OPERATION_GET_DEVICE_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
                case MTP_OPERATION_GET_STORAGE_IDS:
                case MTP_OPERATION_GET_STORAGE_INFO:
                case MTP_OPERATION_GET_OBJECT_HANDLES:
                case MTP_OPERATION_GET_OBJECT_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROP_LIST:
                case MTP_OPERATION_GET_OBJECT_INFO:
                case MTP_OPERATION_GET_OBJECT:
                case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_REFERENCES:
                    USB_DevicePrimeDataIn(mtpHandle, dataInfo); /* C */
                    break;

                case MTP_OPERATION_SEND_OBJECT_INFO:
                case MTP_OPERATION_SEND_OBJECT:
                case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
                    USB_DevicePrimeDataOut(mtpHandle, dataInfo); /* C */
                    break;

                default:
                    /* no action */
                    break;
            }
        }
    } while (0);

    if (MTP_RESPONSE_UNDEFINED != dataInfo->code)
    {
        if (USB_DEVICE_MTP_STATE_RESPONSE == mtpHandle->mtpState) /* C */
        {
            /* Command-Response or Command-Data-Response transaction, prime response block here. */
            status = USB_DeviceMtpPrimeResponse(mtpHandle, dataInfo->code, (uint32_t *)&dataInfo->param[0],
                                                dataInfo->curSize);
        }
        else
        {
            /* Command-Data-Response transaction, error, stall Bulk-in & Bulk-out endpoints. */
            USB_DevicePrimeBulkInAndOutStall(mtpHandle, dataInfo->code, 0);
        }
    }
}

#endif /* USB_DEVICE_CONFIG_MTP */
