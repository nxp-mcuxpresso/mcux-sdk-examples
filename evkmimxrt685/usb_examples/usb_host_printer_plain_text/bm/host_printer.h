/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HOST_PRINTER_H__
#define __HOST_PRINTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USB_HOST_PRINTER_APP_RECEIVE_TRY_DELAY (500)
#define USB_HOST_PRINTER_APP_ONEMS_COUNT (200)
#define USB_HOST_PRINTER_APP_BUFFER_SIZE (300)

/*! @brief host app run status */
typedef enum _usb_host_printer_run_state
{
    kUSB_HostPrinterRunIdle = 0,            /*!< idle */
    kUSB_HostPrinterRunSetInterface,        /*!< execute set interface code */
    kUSB_HostPrinterRunWaitSetInterface,    /*!< wait set interface done */
    kUSB_HostPrinterRunGetDeviceId,         /*!< get device id, get all the string */
    kUSB_HostPrinterRunWaitGetDeviceId,     /*!< wait get device id callback */
    kUSB_HostPrinterRunGetDeviceIdDone,     /*!< get device id success */
    kUSB_HostPrinterRunWaitGetDeviceIdAll,  /*!< get whole device id */
    kUSB_HostPrinterRunGetDeviceIdAllDone,  /*!< get whole device id done */
    kUSB_HostPrinterRunGetDeviceIdAllError, /*!< get whole device id error */
    kUSB_HostPrinterRunPrinterTest,         /*!< test the device printer */
    kUSB_HostPrinterRunPrimeReceive,        /*!< prime receive */
    kUSB_HostPrinterRunDataReceived,        /*!< receive data done */
    kUSB_HostPrinterRunParseDeviceId,       /*!< parse device id */
} usb_host_printer_run_state_t;

typedef enum _usb_host_printer_device_type
{
    kPrinter_NXPVirtual = 0u,
    kPrinter_PJLPostscriptor,
} usb_host_printer_device_type_t;

typedef struct _usb_host_printer_app
{
    usb_host_configuration_handle configHandle; /*!< the printer's configuration handle */
    usb_device_handle deviceHandle;             /*!< the printer's device handle */
    usb_host_class_handle classHandle;          /*!< the printer's class handle */
    usb_host_interface_handle interfaceHandle;  /*!< the printer's interface handle */
    usb_status_t callbackStatus;                /*!< keep the callback status */
    uint8_t *deviceIdBuffer;                    /*!< get device id */
    uint32_t receiveLength;                     /*!< received data length */
    uint32_t receiveDelay;                      /*!< receive periodical delay */
    uint8_t *printerAppBuffer;                  /*!< get device id and receive buffer, increasing 1 for \0 character */
    uint8_t deviceState;                        /*!< device attach/detach status */
    uint8_t prevState;                          /*!< device attach/detach previous status */
    uint8_t runState;                           /*!< printer application run status */
    uint8_t runWaitState; /*!< printer application wait status, go to next run status when the wait status success */
    uint8_t selectAlternateSetting; /*!< the supported alternate setting interface */
    uint8_t waitCallback;           /*!< wait callback label */
    uint8_t deviceLanguageType;     /*!< reference to #usb_host_printer_device_type_t */
} usb_host_printer_app_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief host printer task function.
 *
 * This function implements the host printer action, it is used to create task.
 *
 * @param param   the host printer instance pointer.
 */
extern void USB_HostPrinterAppTask(void *param);

/*!
 * @brief host printer callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain printer interface.
 */
extern usb_status_t USB_HostPrinterAppEvent(usb_device_handle deviceHandle,
                                            usb_host_configuration_handle configurationHandle,
                                            uint32_t eventCode);

#endif
