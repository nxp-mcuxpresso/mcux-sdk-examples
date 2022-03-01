/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HOST_CDC_H__
#define __HOST_CDC_H__
#define USB_HOST_CDC_UART_RX_MAX_LEN 1U

#define USB_HOST_CDC_SERIAL_CR '\r'
#define USB_HOST_CDC_SERIAL_LF '\n'
/*! @brief buffer for size for send and receive data */
/* host example will send the buffer's data to device cdc if the data numbers in the buffer reach
 * USB_HOST_SEND_RECV_PER_TIME or the time reach USB_HOST_UART_RECV_TIMEOUT_THRSHOLD*/
/*the data in the buffer will format a data packet and be transfed on the usb bus, the bigger the
 * USB_HOST_SEND_RECV_PER_TIME value is ,the bigger the packet on the */
/*usb bus will be */
/*the deafule value is 32 ,becuse this could save ram,  but the data transfer efficiency will be lower compared with 64
 * byte size buffer. */
#define USB_HOST_SEND_RECV_PER_TIME 32U
/*! @brief whether device support hardware flow control */
#define USB_HOST_UART_SUPPORT_HW_FLOW 1U
/*! @brief buffer number used to data transfer  */
#define USB_HOST_CDC_BUFFER_NUM 4U
/*! @brief if the data number is not multiple of USB_HOST_SEND_RECV_PER_TIME, the remained data will store in data
 * buffer*/
/*! if the g_UartActive number is bigger than this thrshold , task will output the remained data in buffer*/
#define USB_HOST_UART_RECV_TIMEOUT_THRSHOLD ((5 * SystemCoreClock) / 12000U)

typedef struct _usb_uart_buffer_struct
{
    uint8_t *buffer;
    uint32_t dataLength;
    struct _usb_uart_buffer_struct *next;
} usb_uart_buffer_struct_t;

typedef struct _cdc_instance_struct
{
    usb_device_handle deviceHandle;
    usb_host_class_handle classHandle;
    usb_host_interface_handle controlInterfaceHandle;
    usb_host_interface_handle dataInterfaceHandle;
    usb_host_cdc_acm_state_struct_t state;
    uint16_t bulkInMaxPacketSize;
    uint8_t deviceState;
    uint8_t previousState;
    uint8_t runState;
    uint8_t runWaitState;
} cdc_instance_struct_t;

/*! @brief USB host cdc instance global variable */
extern cdc_instance_struct_t g_cdc;

/*! @brief host app run status */
typedef enum HostCdcBmRunState
{
    kUSB_HostCdcRunIdle = 0,                /*!< idle */
    kUSB_HostCdcRunSetControlInterface,     /*!< execute set interface code */
    kUSB_HostCdcRunWaitSetControlInterface, /*!< wait set interface done */
    kUSB_HostCdcRunSetControlInterfaceDone, /*!< set interface is done, execute next step */
    kUSB_HostCdcRunSetDataInterface,        /*!< execute set interface code */
    kUSB_HostCdcRunWaitSetDataInterface,    /*!< wait set interface done */
    kUSB_HostCdcRunSetDataInterfaceDone,    /*!< set interface is done, execute next step */
    kUSB_HostCdcRunWaitDataReceived,        /*!< wait data receive done */
    kUSB_HostCdcRunDataReceivedDone,        /*!< data receive is done, execute next step  */
    kUSB_HostCdcRunWaitGetState,            /*!< wait get state done*/
    kUSB_HostCdcRunGetStateDone,            /*!< get state done , execute next step*/
    kUSB_HostCdcRunWaitDataSend,            /*!< wait data send done */
    kUSB_HostCdcRunCheckData,               /*!< check whether uart has store data in send buffer*/
    kUSB_HostCdcRunPrimeDateSend,           /*!< prime bulk out send ,  execute data send*/
    kUSB_HostCdcRunPrimeDateReceive,        /*!< prime bulk in receive,  start data receive */
    kUSB_HostCdcRunWaitGetLineCode,         /*!< wait get line code  */
    kUSB_HostCdcRunGetLineCodeDone,         /*!< get line code done, execute next step*/
    kUSB_HostCdcRunWaitSetCtrlState,        /*!< wait set control state*/
    kUSB_HostCdcRunSetCtrlStateDone,        /*!< set control state done , execute next step*/
} host_cdc_bm_run_state;

/*!
 * @brief USB_HostCdcGetUartData function.
 *
 * search in the ring buffer, if the crlfFlag is set , or the buffer is full if data ,then return the buffer address.
 *
 *
 * @retval NULL              No data in buffer chain.
 * @retval uint8_t*         the address of the bufffer containing data.
 */
extern uint8_t *USB_HostCdcGetUartData(void);
/*!
 * @brief USB_HostCdcGetRecvData function.
 *
 * search in the receive buffer, if the buffer contain data ,then return the buffer address.
 *
 *
 * @retval NULL              No data in buffer chain.
 * @retval uint8_t*         the address of the bufffer containing data.
 */
extern uint8_t *USB_HostCdcGetRecvData(void);

/*!
 * @brief USB_HostCdcGetRecvBuffer function.
 *
 * search in the receive buffer chain, if current buffer is not used,then return the buffer address.
 * if current buffer is full, it means all the buffer has been used.
 *
 * @retval NULL              No buffer is available.
 * @retval uint8_t*         the address of the bufffer which doesn't be used.
 */
extern uint8_t *USB_HostCdcGetRecvBuffer(void);
/*!
 * @brief USB_HostCdcGetSendBuffer function.
 *
 * search in the send buffer chain, if current buffer is not used,then return the buffer address.
 * if current buffer is full, it means all the buffer has been used.
 *
 * @retval NULL              No buffer is available.
 * @retval uint8_t*         the address of the bufffer which doesn't be used.
 */
extern uint8_t *USB_HostCdcGetSendBuffer(void);
/*!
 * @brief USB_HostCdcRefreshBuffer function.
 *
 * Set the all the buffers are not used ,clear their flags.
 *
 */
extern void USB_HostCdcRefreshBuffer(void);

/*!
 * @brief USB_HostCdcInitBuffer function.
 *
 * Both send buffer and receive buffer are ring buffer, the data from the uart will be stored in send buffer
 * the data from the usb device cdc will be stored in receive buffer . all the data will be stored by order, so as to
 * the data is output to the uart by its original sequence.
 *
 */
void USB_HostCdcInitBuffer(void);

/*!
 * @brief host cdc task function.
 *
 * This function implements the host mouse action, it is used to create task.
 *
 * @param param   the host cdc instance pointer.
 */
extern void USB_HostCdcTask(void *param);

/*!
 * @brief host cdc callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param event_code           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
extern usb_status_t USB_HostCdcEvent(usb_device_handle deviceHandle,
                                     usb_host_configuration_handle configurationHandle,
                                     uint32_t event_code);

#endif /*__HOST_CDC_H__*/
