/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HOST_MOUSE_H_
#define _HOST_MOUSE_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief buffer for receiving report descriptor and data */
#define HID_BUFFER_SIZE 200U
/*! @brief define the hird value in lpm token, it should be 0~15 */
#define LPM_HIRD_VALUE 6U
#define LPM_ENABLE_REMOTEWAKEUP 1U

 /*!
 * @brief  There is an update for the specification 'Errata for USB 2.0 ECN: Link Power Management (LPM) - 7/2007' in 2011 year.
 *         In this update, it requires USB host controller must use zero(0) to be the value of ENDP(Endpoint) field of LPM EXT token.
 *         However, IP3516 USB host IP and IP3511HS USB device IP do not consider this requirement because this IP is designed before
 *         the errata is released. IP3516 will use the endpoint number of the last transaction for LPM token no matter the response of
 *         this transaction is ACK or NAK. If IP3516 host use endpoint 1 to issue LPM token, then the devices that supports checking 
 *         ENDP 0 of LPM EXT token will have no response for this LPM token so that L1 sleep will fail. It does not matter for IP3511HS
 *         USB device IP because it can response for LPM token using non-zero endpoint.
 *
 *         To make this case is more robust, we use this software workaround to make sure the last transaction of this case is sent to
 *         endpoint 0 before sending LPM token so that IP3516 will use endpoint 0 to send LPM token. This workaround will force the
 *         application layer of this case to cancel transfer of interrupt IN pipe then use endpoint 0 to send GetBOSDescriptor request,
 *         which it will make sure the last transaction is sent to endpoint 0. In user's application, user needs to do the similar change
 *         to make sure the last transaction is sent to endpoint 0. (1) cancel transfers for all of endpoints except the endpoint 0;
 *         (2) prime one transaction to endpoint 0 before sending the LPM token.
 *
 *         Users can disable this macro if the USB IP of the attached device does not care about the ENDP field of the LPM EXT token.
 *        
 */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS))
#define APP_IP3516HS_LPM_ERRATA_WORKAROUND (1U)
#endif

/*! @brief host app run status */
typedef enum _usb_host_hid_mouse_run_state
{
    kUSB_HostHidRunIdle = 0,                /*!< idle */
    kUSB_HostHidRunSetInterface,            /*!< execute set interface code */
    kUSB_HostHidRunWaitSetInterface,        /*!< wait set interface done */
    kUSB_HostHidRunSetInterfaceDone,        /*!< set interface is done, execute next step */
    kUSB_HostHidRunWaitSetIdle,             /*!< wait set idle done */
    kUSB_HostHidRunSetIdleDone,             /*!< set idle is done, execute next step */
    kUSB_HostHidRunWaitGetReportDescriptor, /*!< wait get report descriptor done */
    kUSB_HostHidRunGetReportDescriptorDone, /*!< get report descriptor is done, execute next step */
    kUSB_HostHidRunWaitSetProtocol,         /*!< wait set protocol done */
    kUSB_HostHidRunSetProtocolDone,         /*!< set protocol is done, execute next step */
    kUSB_HostHidRunWaitDataReceived,        /*!< wait interrupt in data */
    kUSB_HostHidRunDataReceived,            /*!< interrupt in data received */
    kUSB_HostHidRunPrimeDataReceive,        /*!< prime interrupt in receive */
} usb_host_hid_mouse_run_state_t;

/*! @brief host suspend/resume device status */
typedef enum _usb_host_suspend_resume_device_state
{
    kStatus_Idle = 0,                     /*!< idle */
    kStatus_SartSuspend,                  /*!< Start suspend process */
    kStatus_SuspendSetRemoteWakeup,       /*!< Prepare to set/clear remote wakeup feature */
    kStatus_SuspendWaitSetRemoteWakeup,   /*!< Wait for set remote wakeup finished */
    kStatus_SuspendWaitClearRemoteWakeup, /*!< Wait for clear remote wakeup finished */
    kStatus_SuspendFailRemoteWakeup,      /*!< Set/Clear remote wakeup request is failed*/
    kStatus_Suspending,                   /*!< Prepare to send suspend request */
    kStatus_SuspendRequest,               /*!< Wait suspend request finished */
    kStatus_Suspended,                    /*!< The device/bus has been suspended */
    kStatus_WaitResume,                   /*!< Wait for enter resume flow */
    kStatus_ResumeRequest,                /*!< Wait resume request finished */
    kStatus_Resumed,                      /*!< idle */

    kUSB_HostRunStartGetBOSDescriptor5, /*!< enter get bos descriptor state*/
    kUSB_HostRunGetBOSDescriptor5,      /*!< start get bos descriptor done */
    kUSB_HostRunWaitGetBOSDescriptor5,  /*!< wait get bos descriptor done */
    kUSB_HostRunGetBOSDescriptor5Done,  /*!< get bos descriptor is done, execute next step */
    kUSB_HostRunWaitGetBOSDescriptor,   /*!< wait get bos descriptor done */
    kUSB_HostRunGetBOSDescriptorDone,   /*!< get bos descriptor is done, execute next step */
    kUSB_HostRunParseBOSDescriptor,     /*!< parse bos descriptor to judge device support LPM */
    kStatus_L1StartSleep,               /*!< Start suspend process */
    kStatus_L1Sleepding,                /*!< Prepare to send suspend request */
    kStatus_L1SleepRequest,             /*!< Wait suspend request finished */
    kStatus_L1Sleeped,                  /*!< The device/bus has been suspended */
    kStatus_L1WaitResume,               /*!< Wait for enter resume flow */
    kStatus_L1ResumeRequest,            /*!< Wait resume request finished */
    kStatus_L1Resumed,                  /*!< idle */
} usb_host_suspend_resume_device_state_t;

/*! @brief USB host mouse instance structure */
typedef struct _usb_host_mouse_instance
{
    usb_host_configuration_handle configHandle; /*!< the mouse's configuration handle */
    usb_device_handle deviceHandle;             /*!< the mouse's device handle */
    usb_host_class_handle classHandle;          /*!< the mouse's class handle */
    usb_host_interface_handle interfaceHandle;  /*!< the mouse's interface handle */
    uint8_t *device_bos_descriptor;             /*!< the mouse's bos descriptor handle */
    volatile uint64_t hwTick;
    uint8_t deviceState;    /*!< device attach/detach status */
    uint8_t prevState;      /*!< device attach/detach previous status */
    uint8_t runState;       /*!< mouse application run status */
    uint8_t runWaitState;   /*!< mouse application wait status, go to next run status when the wait status success */
    uint16_t maxPacketSize; /*!< Interrupt in max packet size */
    uint8_t *mouseBuffer;   /*!< use to receive report descriptor and data */
    uint8_t deviceSupportRemoteWakeup;
    uint8_t L1SetRemoteWakeup;
    uint8_t L1sleepResumeState;
    uint8_t L1sleepBus;
    uint8_t *mouseBosHeadBuffer; /*!< use to receive bos descriptor head */
    uint8_t getBosretryDone;
    uint8_t supportLPM;
    volatile uint8_t selfWakeup;
} usb_host_mouse_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief host mouse task function.
 *
 * This function implements the host mouse action, it is used to create task.
 *
 * @param param   the host mouse instance pointer.
 */
extern void USB_HostHidMouseTask(void *param);

/*!
 * @brief host mouse callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
extern usb_status_t USB_HostHidMouseEvent(usb_device_handle deviceHandle,
                                          usb_host_configuration_handle configurationHandle,
                                          uint32_t eventCode);

#endif /* _HOST_MOUSE_H_ */
