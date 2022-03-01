/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HOST_KEYPAD_H__
#define __HOST_KEYPAD_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HID_BUFFER_SIZE 100

/*! @brief host app run status */
typedef enum _host_keypad_run_state
{
    kUSB_HostHidRunIdle = 0,         /*!< idle */
    kUSB_HostHidRunSetInterface,     /*!< execute set interface code */
    kUSB_HostHidRunWaitSetInterface, /*!< wait set interface done */
    kUSB_HostHidRunSetInterfaceDone, /*!< set interface is done, execute next step */
    kUSB_HostHidRunWaitDataReceived, /*!< wait interrupt in data */
    kUSB_HostHidRunDataReceived,     /*!< interrupt in data received */
    kUSB_HostHidRunPrimeDataReceive, /*!< prime interrupt in receive */
} host_keypad_run_state_t;

/*! @brief USB host keypad instance structure */
typedef struct _host_keypad_instance
{
    usb_device_handle deviceHandle;            /*!< the keypad's device handle */
    usb_host_class_handle classHandle;         /*!< the keypad's class handle */
    usb_host_interface_handle interfaceHandle; /*!< the keypad's interface handle */
    uint8_t devState;                          /*!< device attach/detach status */
    uint8_t prevState;                         /*!< device attach/detach previous status */
    uint8_t runState;                          /*!< keypad application run status */
    uint8_t runWaitState;   /*!< keypad application wait status, go to next run status when the wait status success */
    uint16_t maxPacketSize; /*!< Interrupt in max packet size */
    uint8_t *keypadBuffer;  /*!< use to receive report descriptor and data */
} host_keypad_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/
extern void USB_KeypadTask(void *arg);

extern usb_status_t USB_HostKeypadEvent(usb_device_handle deviceHandle,
                                        usb_host_configuration_handle configurationHandle,
                                        uint32_t eventCode);

#endif /* __HOST_KEYPAD_H__ */
