/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DFU_H__
#define __USB_DFU_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief DFU status definition */
#define USB_DFU_STATUS_OK (0x00U)         /*!< No error condition is present */
#define USB_DFU_STATUS_ERR_TARGET (0x01U) /*!< File is not targeted for use by this device */
#define USB_DFU_STATUS_ERR_FILE   (0x02U) /*!< File is for this device but fails some vendor-specific verification test */
#define USB_DFU_STATUS_ERR_WRITE (0x03U)        /*!< Device is unable to write memory */
#define USB_DFU_STATUS_ERR_ERASE (0x04U)        /*!< Memory erase function failed */
#define USB_DFU_STATUS_ERR_CHECK_ERASED (0x05U) /*!< Memory erase check failed */
#define USB_DFU_STATUS_ERR_PROG (0x06U)         /*!< Program memory function failed */
#define USB_DFU_STATUS_ERR_VERIFY (0x07U)       /*!< Programed memory failed verification */
#define USB_DFU_STATUS_ERR_ADDRESS (0x08U)      /*!< Received address is out of range */
#define USB_DFU_STATUS_ERR_NOT_DONE (0x09U) /*!< Received DFU DNLOAD request with length = 0 but device does not think it has all of the data yet */
#define USB_DFU_STATUS_ERR_FIRMWARE (0x0AU)   /*!< Firmware is corrupt */
#define USB_DFU_STATUS_ERR_VENDOR (0x0BU)     /*!< Vendor specific error */
#define USB_DFU_STATUS_ERR_USBR (0x0CU)       /*!< Detect unexpected USB reset signaling */
#define USB_DFU_STATUS_ERR_POR (0x0DU)        /*!< Detect unexpected power on reset */
#define USB_DFU_STATUS_ERR_UNKNOWN (0x0EU)    /*!< Unknown error */
#define USB_DFU_STATUS_ERR_STALLEDPKT (0x0FU) /*!< Device stalled an unexpected request */

/*! @brief DFU Block status */
#define USB_DFU_BLOCK_TRANSFER_COMPLETE (0x00U)    /*!< Block transfer complete */
#define USB_DFU_BLOCK_TRANSFER_IN_PROGRESS (0x01U) /*!< Block transfer is still in progress */
#define USB_DFU_BLOCK_TRANSFER_UNDEFINED (0xFFU)   /*!< Block transfer status undefined */

/*! @brief DFU manifestation phase status */
#define USB_DFU_MANIFEST_COMPLETE (0x00U)    /*!< Manifestation phase complete */
#define USB_DFU_MANIFEST_IN_PROGRESS (0x01U) /*!< Manifestation phase in progress */
#define USB_DFU_MANIFEST_UNDEFINED (0xFFU)   /*!< Manifestation phase undefined */

#define DFU_EVENT_QUEUE_MAX (16)

/*! @brief DFU state definition. */
typedef enum _usb_dfu_state_struct
{
    kState_AppIdle = 0U,         /* App idle */
    kState_AppDetach,            /* App detach */
    kState_DfuIdle,              /* DFU idle */
    kState_DfuDnLoadSync,        /* DFU dnload sync */
    kState_DfuDnBusy,            /* DFU dnload busy */
    kState_DfuDnLoadIdle,        /* DFU dnload idle */
    kState_DfuManifestSync,      /* DFU manifest sync */
    kState_DfuManifest,          /* DFU manifest */
    kState_DfuManifestWaitReset, /* DFU manifest wait reset */
    kState_DfuUpLoadIdle,        /* DFU upload idle */
    kState_DfuError,
} usb_dfu_state_struct_t;

/*! @brief DFU file suffix definition. */
typedef struct _usb_dfu_suffix_struct
{
    uint8_t bcdDevice[2];      /* release number of the device associated with firmware file */
    uint8_t idProduct[2];      /* product ID */
    uint8_t idVendor[2];       /* Vendor ID */
    uint8_t bcdDFU[2];         /* DFU specification number */
    uint8_t ucDfuSignature[3]; /* DFU signature */
    uint8_t bLength;           /* Length of DFU suffix */
    uint8_t dwCRC[4];          /* The CRC of entire file */
} usb_dfu_suffix_struct_t;

/*! @brief DFU status definition. */
typedef struct _usb_dfu_status_struct
{
    uint8_t bStatus;           /* status result */
    uint8_t bwPollTimeout[3U]; /* The minimum time host should wait before sending
                                  a subsequent DFU GETSTATUS request */
    uint8_t bState;            /* dfu state */
    uint8_t iString;           /* Index of status description in string table */
} usb_dfu_status_struct_t;

/*! @brief DFU device definition. */
typedef struct _usb_dfu_struct
{
    usb_dfu_status_struct_t *dfuStatus;
    uint32_t dfuFirmwareBlockLength;
    uint32_t dfuIsTheFirstBlock;
    uint32_t dfuCRC;
    uint8_t *dfuFirmwareBlock;
    uint8_t dfuFirmwareBlockStatus;
    uint8_t dfuIsDownloadingFinished;
    uint8_t dfuManifestationPhaseStatus;
    uint32_t dfuFirmwareAddress;
    uint32_t dfuFirmwareSize;
    uint32_t dfuCurrentUploadLenght;
    uint8_t dfuSuffix[0xFF];
    uint8_t dfuReboot;
    uint8_t crcCheck;
    uint8_t dfuTimerId;
} usb_dfu_struct_t;

/*! @brief DFU device event definition. */
typedef enum _usb_device_dfu_state_event
{
    kUSB_DeviceDfuEventDetachReq,
    kUSB_DeviceDfuEventGetStatusReq,
    kUSB_DeviceDfuEventClearStatusReq,
    kUSB_DeviceDfuEventGetStateReq,
    kUSB_DeviceDfuEventDnloadReq,
    kUSB_DeviceDfuEventAbortReq,
    kUSB_DeviceDfuEventUploadReq,
    kUSB_DeviceDfuEventDetachTimeout,
    kUSB_DeviceDfuEventPollTimeout,
} usb_device_dfu_state_event_t;


/* Define DFU event struct */
typedef struct _usb_device_dfu_event_struct
{
    usb_device_dfu_state_event_t name;
    uint16_t wValue;
    uint16_t wLength;
} usb_device_dfu_event_struct_t;

/* Define DFU_ENET queue struct */
typedef struct _dfu_queue
{
    uint32_t head;
    uint32_t tail;
    uint32_t maxSize;
    uint32_t curSize;
    osa_mutex_handle_t mutex;
    uint32_t mutexBuffer[(OSA_MUTEX_HANDLE_SIZE + 3)/4]; /*!< The mutex buffer. */
    usb_device_dfu_event_struct_t qArray[DFU_EVENT_QUEUE_MAX];
} dfu_queue_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif

extern usb_status_t USB_DeviceDfuDemoCallback(class_handle_t handle, uint32_t event, void *param);
extern usb_status_t USB_DeviceDfuDemoVendorCallback(usb_device_handle handle, void *param);
extern void USB_DeviceDfuBusReset(void);
extern void USB_DeviceDfuSwitchMode(void);
extern void USB_DeviceDfuDemoInit(void);
extern void USB_DeviceDfuTask(void);

#if defined(__cplusplus)
}
#endif
#endif /* __USB_DFU_H__ */
