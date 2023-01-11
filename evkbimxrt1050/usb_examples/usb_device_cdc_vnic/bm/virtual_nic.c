/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
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
#include "virtual_nic.h"
#include "virtual_nic_enet_adapter.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#include "usb_phy.h"
#include "fsl_iomuxc.h"
#include "fsl_phyksz8081.h"
#include "fsl_phy.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Base unit for ENIT layer is 1Mbps while for RNDIS its 100bps*/
#define ENET_CONVERT_FACTOR (10000)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif
void VNIC_EnetRxBufFree(pbuf_t *pbuf);
void VNIC_EnetTxBufFree(pbuf_t *pbuf);
bool VNIC_EnetGetLinkStatus(void);
uint32_t VNIC_EnetGetSpeed(void);
uint8_t *VNIC_EnetTxBufAlloc(void);
usb_status_t VNIC_EnetSend(uint8_t *buf, uint32_t len);
usb_status_t USB_DeviceCdcVnicCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
usb_status_t VNIC_EnetTxDone(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_ksz8081_resource_t g_phy_resource;
extern usb_cdc_vnic_t g_cdcVnic;
extern usb_device_endpoint_struct_t g_cdcVnicDicEp[];
extern usb_device_class_struct_t g_cdcVnicClass;
extern queue_t g_enetRxServiceQueue;
extern queue_t g_enetTxServiceQueue;
extern uint8_t g_hwaddr[ENET_MAC_ADDR_SIZE];
/* Data structure of virtual nic device. */
usb_cdc_vnic_t g_cdcVnic;
/* Buffer to receive data. */
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0))
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[HS_CDC_VNIC_BULK_OUT_PACKET_SIZE];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[FS_CDC_VNIC_BULK_OUT_PACKET_SIZE];
#endif

/* Part 1 of usb transmit buffer. */
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0))
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_usbTxPartOneBuffer[HS_CDC_VNIC_BULK_OUT_PACKET_SIZE];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_usbTxPartOneBuffer[FS_CDC_VNIC_BULK_OUT_PACKET_SIZE];
#endif

/* USB receive buffer to store the rndis packet. size is calculated as
 * ENET_FRAME_MAX_FRAMELEN + sizeof(enet_header_t) + RNDIS_USB_HEADER_SIZE.
 */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_usbRxRndisPacketBuffer[1581];
/* USB receive buffer to store the rndis packet. size is calculated as
 * ENET_FRAME_MAX_FRAMELEN + sizeof(enet_header_t).
 */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_usbTxRndisPacketBuffer[1536];

/* Append byte for zero length packet. */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_zeroSend = 0x00;

/* USB device class information */
static usb_device_class_config_struct_t s_cdcAcmConfig[1] = {{
    USB_DeviceCdcVnicCallback,
    0,
    &g_cdcVnicClass,
}};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t s_cdcAcmConfigList = {
    s_cdcAcmConfig,
    USB_DeviceCallback,
    1,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
ENET_Type *BOARD_GetExampleEnetBase(void)
{
    return ENET;
}

uint32_t BOARD_GetPhySysClock(void)
{
    return CLOCK_GetFreq(kCLOCK_IpgClk);
}

const phy_operations_t *BOARD_GetPhyOps(void)
{
    return &phyksz8081_ops;
}

void *BOARD_GetPhyResource(void)
{
    return (void *)&g_phy_resource;
}

void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {.enableClkOutput = true, .enableClkOutput25M = false, .loopDivider = 1};
    CLOCK_InitEnetPll(&config);
}

static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET)]);
    ENET_SetSMI(ENET, CLOCK_GetFreq(kCLOCK_IpgClk), false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(ENET, phyAddr, regAddr, pData);
}


void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_cdcVnic.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_cdcVnic.deviceHandle);
}

void USB_DeviceClockInit(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceEhciTaskFunction(deviceHandle);
}
#endif
/*!
 * @brief Set the state of the usb transmit direction
 *
 * @param state The state of the usb transmit direction.
 * @return A USB error code or kStatus_USB_Success.
 */
static inline usb_status_t USB_DeviceVnicTransmitSetState(usb_cdc_vnic_tx_state_t state)
{
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    g_cdcVnic.nicTrafficInfo.usbTxState = state;
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return kStatus_USB_Success;
}

/*!
 * @brief Set the state of the usb receive direction.
 *
 * @param state The state of the usb receive direction.
 * @return A USB error code or kStatus_USB_Success.
 */
static inline usb_status_t USB_DeviceVnicReceiveSetState(usb_cdc_vnic_rx_state_t state)
{
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    USB_DEVICE_VNIC_ENTER_CRITICAL();
    g_cdcVnic.nicTrafficInfo.usbRxState = state;
    USB_DEVICE_VNIC_EXIT_CRITICAL();
    return kStatus_USB_Success;
}

/*!
 * @brief State process for usb transmit direction.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceVnicTransmit(void)
{
    usb_status_t error = kStatus_USB_Error;
    vnic_enet_transfer_t cdcAcmTransfer;
    pbuf_t *enetPbuf;
    uint32_t usbTxLen;
    uint32_t usbTxPart_1Len;
    uint8_t *nicData;
    uint32_t length;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    uint8_t *firstSendBuff = s_usbTxPartOneBuffer;
    switch (g_cdcVnic.nicTrafficInfo.usbTxState)
    {
        case TX_IDLE:
        {
            /* Initialize the tx variables */
            g_cdcVnic.nicTrafficInfo.usbTxNicData         = NULL;
            g_cdcVnic.nicTrafficInfo.usbTxEnetPcb.payload = NULL;
            g_cdcVnic.nicTrafficInfo.usbTxEnetPcb.length  = 0;
            g_cdcVnic.nicTrafficInfo.usbTxRndisPkgLength  = 0;
            g_cdcVnic.nicTrafficInfo.usbTxPart_1Len       = 0;

            /* Get a transfer request from the enet queue */
            error = VNIC_EnetQueueGet(&g_enetRxServiceQueue, &cdcAcmTransfer);
            if (kStatus_USB_Success == error)
            {
                enetPbuf = &(g_cdcVnic.nicTrafficInfo.usbTxEnetPcb);

                enetPbuf->payload = cdcAcmTransfer.buffer;
                enetPbuf->length  = cdcAcmTransfer.length;
                length            = cdcAcmTransfer.length;
                nicData           = cdcAcmTransfer.buffer;

                usbTxLen       = length + RNDIS_USB_OVERHEAD_SIZE;
                usbTxPart_1Len = 0;
                g_cdcVnic.nicTrafficInfo.enetRxUsb2hostSent++;
                g_cdcVnic.nicTrafficInfo.usbTxNicData        = nicData;
                g_cdcVnic.nicTrafficInfo.usbTxRndisPkgLength = usbTxLen;

                /* RNDIS Protocol defines 1 byte call of 0x00, if transfer size is multiple of endpoint packet size */
                g_cdcVnic.nicTrafficInfo.usbTxZeroSendFlag =
                    (uint8_t)((usbTxLen % g_cdcVnicDicEp[0].maxPacketSize) ? 0 : 1);

                /* Whichever is smaller but not less than RNDIS_USB_OVERHEAD_SIZE */
                usbTxPart_1Len =
                    usbTxLen > g_cdcVnicDicEp[0].maxPacketSize ? g_cdcVnicDicEp[0].maxPacketSize : usbTxLen;

                if (usbTxPart_1Len < RNDIS_USB_OVERHEAD_SIZE)
                {
                    /* For g_cdcVnicDicEp[0].maxPacketSize as 8, 16 or 32 minimum  usbTxPart_1Len has to be
                     either usbTxLen
                     (which is definitely greater than RNDIS_USB_OVERHEAD_SIZE) or at least 64 which is the next allowed
                     multiple of
                     g_cdcVnicDicEp[0].maxPacketSize */
                    usbTxPart_1Len = usbTxLen > 64 ? 64 : usbTxLen;
                }
                g_cdcVnic.nicTrafficInfo.usbTxPart_1Len = usbTxPart_1Len;

                /* Prepare USB Header */
                ((rndis_packet_msg_format_t *)firstSendBuff)->messageType = USB_LONG_TO_LITTLE_ENDIAN(RNDIS_PACKET_MSG);
                ((rndis_packet_msg_format_t *)firstSendBuff)->messageLen  = USB_LONG_TO_LITTLE_ENDIAN(usbTxLen);
                ((rndis_packet_msg_format_t *)firstSendBuff)->dataOffset = USB_LONG_TO_LITTLE_ENDIAN(RNDIS_DATA_OFFSET);
                ((rndis_packet_msg_format_t *)firstSendBuff)->dataLen    = USB_LONG_TO_LITTLE_ENDIAN(length);
                /* Fill rest of firstSendBuff buffers with payload as much as possible */
                memcpy(firstSendBuff + RNDIS_USB_OVERHEAD_SIZE, nicData, usbTxPart_1Len - RNDIS_USB_OVERHEAD_SIZE);

                USB_DEVICE_VNIC_ENTER_CRITICAL();
                error = USB_DeviceCdcAcmSend(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_IN_ENDPOINT, firstSendBuff,
                                             usbTxPart_1Len);
                if (kStatus_USB_Error != error)
                {
                    USB_DeviceVnicTransmitSetState(TX_PART_ONE_PROCESS);
                }
                else
                {
                    usb_echo("Part One of RNDIS packet send failed, 0x%x\n", error);
                    VNIC_EnetRxBufFree(enetPbuf);
                }
                USB_DEVICE_VNIC_EXIT_CRITICAL();
            }
        }
        break;
        case TX_PART_ONE_PROCESS:
            break;
        case TX_PART_ONE_DONE:
        {
            usbTxLen             = g_cdcVnic.nicTrafficInfo.usbTxRndisPkgLength;
            usbTxPart_1Len       = g_cdcVnic.nicTrafficInfo.usbTxPart_1Len;
            nicData              = g_cdcVnic.nicTrafficInfo.usbTxNicData;
            enetPbuf             = &(g_cdcVnic.nicTrafficInfo.usbTxEnetPcb);
            uint8_t returnStatus = kStatus_USB_Success;

            if (usbTxLen > usbTxPart_1Len)
            {
                /* Send the part 2 of the RNDIS packet */
                USB_DEVICE_VNIC_ENTER_CRITICAL();
                memcpy(s_usbTxRndisPacketBuffer, nicData + (usbTxPart_1Len - RNDIS_USB_OVERHEAD_SIZE),
                       usbTxLen - usbTxPart_1Len);
                error = USB_DeviceCdcAcmSend(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_IN_ENDPOINT,
                                             s_usbTxRndisPacketBuffer, usbTxLen - usbTxPart_1Len);

                if (kStatus_USB_Error != error)
                {
                    USB_DeviceVnicTransmitSetState(TX_PART_TWO_PROCESS);
                }
                else
                {
                    usb_echo("Part Two of RNDIS packet send failed, 0x%x\n", returnStatus);
                    VNIC_EnetRxBufFree(enetPbuf);
                }
                USB_DEVICE_VNIC_EXIT_CRITICAL();
            }
            else
            {
                USB_DeviceVnicTransmitSetState(TX_PART_TWO_DONE);
            }
        }
        break;
        case TX_PART_TWO_PROCESS:
            break;
        case TX_PART_TWO_DONE:
        {
            enetPbuf = &(g_cdcVnic.nicTrafficInfo.usbTxEnetPcb);
            if (g_cdcVnic.nicTrafficInfo.usbTxZeroSendFlag == 1)
            {
                /* Send a zero length packet */
                USB_DEVICE_VNIC_ENTER_CRITICAL();
                error = USB_DeviceCdcAcmSend(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_IN_ENDPOINT, &s_zeroSend,
                                             sizeof(uint8_t));
                if (kStatus_USB_Error != error)
                {
                    USB_DeviceVnicTransmitSetState(TX_ZLP_PROCESS);
                }
                else
                {
                    usb_echo("Zero length packet send failed, 0x%x\n", error);
                }
                USB_DEVICE_VNIC_EXIT_CRITICAL();
            }
            else
            {
                VNIC_EnetRxBufFree(enetPbuf);
                g_cdcVnic.nicTrafficInfo.enetRxUsb2host++;
                USB_DeviceVnicTransmitSetState(TX_IDLE);
            }
        }
        break;
        case TX_ZLP_PROCESS:
            break;
        case TX_ZLP_DONE:
        {
            enetPbuf = &(g_cdcVnic.nicTrafficInfo.usbTxEnetPcb);
            VNIC_EnetRxBufFree(enetPbuf);
            g_cdcVnic.nicTrafficInfo.enetRxUsb2host++;
            USB_DeviceVnicTransmitSetState(TX_IDLE);
        }
        break;
        default:
            break;
    }
    return error;
}

/*!
 * @brief State process for usb receive direction.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceVnicReceive(void)
{
    usb_status_t error         = kStatus_USB_Error;
    uint8_t *rndisPktMsgData   = NULL;
    uint32_t frameRemainingLen = 0;
    uint32_t messageLen        = 0;
    uint8_t *buffer;
    uint32_t len;
    USB_DEVICE_VNIC_CRITICAL_ALLOC();
    switch (g_cdcVnic.nicTrafficInfo.usbRxState)
    {
        case RX_IDLE:
        {
            /* Prime for next receive */
            USB_DEVICE_VNIC_ENTER_CRITICAL();
            error = USB_DeviceCdcAcmRecv(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                         g_cdcVnicDicEp[1].maxPacketSize);
            if (kStatus_USB_Error != error)
            {
                USB_DeviceVnicReceiveSetState(RX_PART_ONE_PROCESS);
            }
            else
            {
                usb_echo("Part One of RNDIS packet recv failed\r\n");
            }
            USB_DEVICE_VNIC_EXIT_CRITICAL();
        }
        break;
        case RX_PART_ONE_PROCESS:
            break;
        case RX_PART_ONE_DONE:
        {
            buffer = (uint8_t *)g_cdcVnic.nicTrafficInfo.usbRxPartOneBuf;
            len    = g_cdcVnic.nicTrafficInfo.usbRxPartOneLen;
            if (USB_CANCELLED_TRANSFER_LENGTH == len)
            {
                USB_DeviceVnicReceiveSetState(RX_IDLE);
                break;
            }
            messageLen = USB_LONG_TO_LITTLE_ENDIAN(*((uint32_t *)(buffer) + 1));

            if (!(messageLen % g_cdcVnicDicEp[1].maxPacketSize))
            {
                /* RNDIS Protocol defines 1 byte call of 0x00, if transfer size is multiple of endpoint packet size */
                messageLen++;
            }
            rndisPktMsgData = s_usbRxRndisPacketBuffer;
            memcpy(rndisPktMsgData, buffer, len);

            frameRemainingLen = messageLen - len;

            /* Receive the second part of RNDIS packet from host */
            if (frameRemainingLen)
            {
                /* Required when ethernet packet + usb header is larger than maxPacketSize */
                USB_DEVICE_VNIC_ENTER_CRITICAL();
                error = USB_DeviceCdcAcmRecv(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_OUT_ENDPOINT,
                                             rndisPktMsgData + g_cdcVnicDicEp[1].maxPacketSize, frameRemainingLen);
                if (kStatus_USB_Error != error)
                {
                    USB_DeviceVnicReceiveSetState(RX_PART_TWO_PROCESS);
                }
                else
                {
                    usb_echo("Part Two of RNDIS packet recv failed\r\n");
                }
                USB_DEVICE_VNIC_EXIT_CRITICAL();
            }
            else
            {
                /* Send the ethernet packet */
                USB_DeviceVnicReceiveSetState(RX_USB2ENET_PROCESS);

                error = VNIC_EnetSend((uint8_t *)(rndisPktMsgData + RNDIS_USB_OVERHEAD_SIZE),
                                      messageLen - RNDIS_USB_OVERHEAD_SIZE);

                if (kStatus_USB_Success != error)
                {
                    usb_echo("RX_PART_ONE_DONE, VNIC_EnetSend failed\n");
                }
            }
        }
        break;
        case RX_PART_TWO_PROCESS:
            break;
        case RX_PART_TWO_DONE:
        {
            buffer = (uint8_t *)g_cdcVnic.nicTrafficInfo.usbRxPartTwoBuf;
            len    = g_cdcVnic.nicTrafficInfo.usbRxPartTwoLen;
            if (USB_CANCELLED_TRANSFER_LENGTH == len)
            {
                USB_DeviceVnicReceiveSetState(RX_IDLE);
                break;
            }
            /* Entire ethernet packet with USB header was not received as one transaction */
            rndisPktMsgData = buffer - g_cdcVnicDicEp[1].maxPacketSize;

            /* Re-calculate messageLen as it might have changed because of 1 byte of zero recv */
            messageLen = USB_LONG_TO_LITTLE_ENDIAN(*((uint32_t *)rndisPktMsgData + 1));

            /* Send the ethernet packet */
            USB_DeviceVnicReceiveSetState(RX_USB2ENET_PROCESS);
            error = VNIC_EnetSend((uint8_t *)(rndisPktMsgData + RNDIS_USB_OVERHEAD_SIZE),
                                  messageLen - RNDIS_USB_OVERHEAD_SIZE);

            if (kStatus_USB_Success != error)
            {
                usb_echo("RX_PART_TWO_DONE, VNIC_EnetSend failed\n");
            }
        }
        break;
        case RX_USB2ENET_PROCESS:
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)) || \
    (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0))
            VNIC_EnetTxDone();
#endif
            USB_DeviceVnicReceiveSetState(RX_USB2ENET_DONE);
            break;
        case RX_USB2ENET_DONE:
        {
            USB_DeviceVnicReceiveSetState(RX_IDLE);
        }
        break;
        default:
            break;
    }
    return error;
}

/*!
 * @brief Callback for RNDIS specific requests.
 *
 * @param handle The class handle of the CDC ACM class.
 * @param event The event for the RNDIS device.
 * @param param The pointer to parameter of the callback.
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcRndisCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    usb_device_cdc_rndis_request_param_struct_t *rndisParam;
    rndisParam = (usb_device_cdc_rndis_request_param_struct_t *)param;

    switch (event)
    {
        case kUSB_DeviceCdcEventAppGetLinkSpeed:
            if (1 == g_cdcVnic.attach)
            {
                *((uint32_t *)rndisParam->buffer) = VNIC_EnetGetSpeed();
                *((uint32_t *)rndisParam->buffer) *= ENET_CONVERT_FACTOR;
            }
            break;
        case kUSB_DeviceCdcEventAppGetSendPacketSize:
            *((uint32_t *)rndisParam->buffer) = USB_LONG_TO_LITTLE_ENDIAN(g_cdcVnicDicEp[0].maxPacketSize);
            break;
        case kUSB_DeviceCdcEventAppGetRecvPacketSize:
            *((uint32_t *)rndisParam->buffer) = USB_LONG_TO_LITTLE_ENDIAN(g_cdcVnicDicEp[1].maxPacketSize);
            break;
        case kUSB_DeviceCdcEventAppGetMacAddress:
            memcpy(rndisParam->buffer, &g_hwaddr[0], ENET_MAC_ADDR_SIZE);
            break;
        case kUSB_DeviceCdcEventAppGetMaxFrameSize:
            *((uint32_t *)rndisParam->buffer) = (ENET_FRAME_MAX_FRAMELEN + RNDIS_USB_HEADER_SIZE);
            break;
        case kUSB_DeviceCdcEventAppGetLinkStatus:
            if (1 == g_cdcVnic.attach)
            {
                *((uint32_t *)rndisParam->buffer) = VNIC_EnetGetLinkStatus();
            }
            break;
        default:
            break;
    }
    return error;
}

/*!
 * @brief Callback for CDC RNDIS class specific requests.
 *
 * @param handle The class handle of the CDC ACM class.
 * @param event The event for the RNDIS device.
 * @param param The pointer to parameter of the callback.
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVnicCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam   = (usb_device_endpoint_callback_message_struct_t *)param;
    switch (event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if (1 == g_cdcVnic.attach)
            {
                if (NULL == epCbParam->buffer)
                {
                    return error;
                }

                switch (g_cdcVnic.nicTrafficInfo.usbTxState)
                {
                    case TX_PART_ONE_PROCESS:
                        USB_DeviceVnicTransmitSetState(TX_PART_ONE_DONE);
                        break;
                    case TX_PART_TWO_PROCESS:
                        USB_DeviceVnicTransmitSetState(TX_PART_TWO_DONE);
                        break;
                    case TX_ZLP_PROCESS:
                        USB_DeviceVnicTransmitSetState(TX_ZLP_DONE);
                        break;
                    default:
                        break;
                }
                error = kStatus_USB_Success;
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if (1 == g_cdcVnic.attach)
            {
                switch (g_cdcVnic.nicTrafficInfo.usbRxState)
                {
                    case RX_PART_ONE_PROCESS:
                        g_cdcVnic.nicTrafficInfo.usbRxPartOneBuf = epCbParam->buffer;
                        g_cdcVnic.nicTrafficInfo.usbRxPartOneLen = epCbParam->length;
                        USB_DeviceVnicReceiveSetState(RX_PART_ONE_DONE);
                        break;
                    case RX_PART_TWO_PROCESS:
                        g_cdcVnic.nicTrafficInfo.usbRxPartTwoBuf = epCbParam->buffer;
                        g_cdcVnic.nicTrafficInfo.usbRxPartTwoLen = epCbParam->length;
                        USB_DeviceVnicReceiveSetState(RX_PART_TWO_DONE);
                        break;
                    default:
                        break;
                }
                error = kStatus_USB_Success;
            }
            else
            {
                usb_echo("Discard the received data\r\n");
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error                                                 = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            if (1 == acmReqParam->isSetup)
            {
                *(acmReqParam->buffer) = g_cdcVnic.rndisHandle->rndisCommand;
                *(acmReqParam->length) = RNDIS_MAX_EXPECTED_COMMAND_SIZE;
            }
            else
            {
                /* data phase */
                USB_DeviceCdcRndisMessageSet(g_cdcVnic.rndisHandle, acmReqParam->buffer, acmReqParam->length);
            }
            error = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            error = USB_DeviceCdcRndisMessageGet(g_cdcVnic.rndisHandle, acmReqParam->buffer, acmReqParam->length);
            break;
        default:
            break;
    }

    return error;
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            uint8_t *message;
            uint32_t len;
            g_cdcVnic.attach               = 0;
            g_cdcVnic.currentConfiguration = 0U;
            error                          = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_cdcVnic.speed))
            {
                USB_DeviceSetSpeed(handle, g_cdcVnic.speed);
            }
#endif
            USB_DeviceCdcRndisHaltCommand(g_cdcVnic.rndisHandle);
            USB_DeviceCdcRndisResetCommand(g_cdcVnic.rndisHandle, &message, &len);
            VNIC_EnetClearEnetQueue();
            if ((g_cdcVnic.nicTrafficInfo.usbTxEnetPcb.payload != NULL))
            {
                VNIC_EnetRxBufFree(&(g_cdcVnic.nicTrafficInfo.usbTxEnetPcb));
            }
            USB_DeviceVnicTransmitSetState(TX_IDLE);
            USB_DeviceVnicReceiveSetState(RX_PART_ONE_PROCESS);
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_cdcVnic.attach               = 0;
                g_cdcVnic.currentConfiguration = 0U;
                error                          = kStatus_USB_Success;
            }
            else if (USB_CDC_VNIC_CONFIGURE_INDEX == (*temp8))
            {
                g_cdcVnic.attach               = 1;
                g_cdcVnic.currentConfiguration = *temp8;
                /* Schedule buffer for receive */
                error = USB_DeviceCdcAcmRecv(g_cdcVnic.cdcAcmHandle, USB_CDC_VNIC_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                             g_cdcVnicDicEp[0].maxPacketSize);
                if (kStatus_USB_Error == error)
                {
                    usb_echo("kUSB_DeviceEventSetConfiguration, USB_DeviceCdcAcmRecv failed.\r\n");
                }
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_cdcVnic.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);

                if (interface == USB_CDC_VNIC_COMM_INTERFACE_INDEX)
                {
                    if (alternateSetting < USB_CDC_VNIC_COMM_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_cdcVnic.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                 = kStatus_USB_Success;
                    }
                }
                else if (interface == USB_CDC_VNIC_DATA_INTERFACE_INDEX)
                {
                    if (alternateSetting < USB_CDC_VNIC_DATA_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_cdcVnic.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                 = kStatus_USB_Success;
                    }
                }
                else
                {
                    /* no action, return kStatus_USB_InvalidRequest */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get current configuration request */
                *temp8 = g_cdcVnic.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_CDC_VNIC_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_cdcVnic.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            /* no action, return kStatus_USB_InvalidRequest */
            break;
    }

    return error;
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void APPInit(void)
{
    usb_device_cdc_rndis_config_struct_t rndisConfig;

    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Initialize the FEC interface */
    if (ENET_OK != VNIC_EnetInit())
    {
        usb_echo("VNIC_EnetInit failed\r\n");
    }

    g_cdcVnic.speed        = USB_SPEED_FULL;
    g_cdcVnic.attach       = 0;
    g_cdcVnic.cdcAcmHandle = (class_handle_t)NULL;
    g_cdcVnic.deviceHandle = NULL;

    USB_DeviceVnicReceiveSetState(RX_PART_ONE_PROCESS);

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &s_cdcAcmConfigList, &g_cdcVnic.deviceHandle))
    {
        usb_echo("USB device init failed\r\n");
    }
    else
    {
        usb_echo("USB device CDC virtual nic demo\r\n");
        g_cdcVnic.cdcAcmHandle = s_cdcAcmConfigList.config->classHandle;
    }

    rndisConfig.devMaxTxSize  = ENET_FRAME_MAX_FRAMELEN + RNDIS_USB_HEADER_SIZE;
    rndisConfig.rndisCallback = USB_DeviceCdcRndisCallback;
    if (kStatus_USB_Success != USB_DeviceCdcRndisInit(g_cdcVnic.cdcAcmHandle, &rndisConfig, &(g_cdcVnic.rndisHandle)))
    {
        usb_echo("USB_DeviceCdcRndisInit failed\r\n");
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_cdcVnic.deviceHandle);
}

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
void APPTask(void)
{
    if ((1 == g_cdcVnic.attach))
    {
        /* User Code */
        USB_DeviceVnicTransmit();
        USB_DeviceVnicReceive();
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO1, 9, 1);
    SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

    APPInit();

    while (1)
    {
        APPTask();

#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_cdcVnic.deviceHandle);
#endif
    }
}
