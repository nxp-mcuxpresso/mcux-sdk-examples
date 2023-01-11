/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2020 - 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "virtual_nic_enetif.h"

#include "fsl_enet.h"
#include "fsl_phy.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_cdc_rndis.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "virtual_nic_enet_adapter.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_ENET_BASEADDR BOARD_GetExampleEnetBase()
#define BOARD_PHY_SYS_CLOCK BOARD_GetPhySysClock()
#define BOARD_PHY_OPS       BOARD_GetPhyOps()
#define BOARD_PHY_RESOURCE  BOARD_GetPhyResource()
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ENETIF_Input(enet_handle_t *handle, void *param);
void VNIC_EnetCallback(pbuf_t *pbuffer);
void VNIC_EnetRxBufFree(pbuf_t *pbuf);
uint8_t *VNIC_EnetRxBufAlloc(void);
extern ENET_Type *BOARD_GetExampleEnetBase(void);
extern uint32_t BOARD_GetPhySysClock(void);
extern const phy_operations_t *BOARD_GetPhyOps(void);
extern void *BOARD_GetPhyResource(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
enet_handle_t g_handle;
phy_handle_t phyHandle;
extern uint8_t g_hwaddr[ENET_MAC_ADDR_SIZE];

AT_NONCACHEABLE_SECTION_ALIGN(enet_rx_bd_struct_t RxBuffDescrip[ENET_RXBD_NUM], ENET_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_tx_bd_struct_t TxBuffDescrip[ENET_TXBD_NUM], ENET_BUFF_ALIGNMENT);

SDK_ALIGN(uint8_t RxDataBuff[ENET_RXBD_NUM][SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT);
SDK_ALIGN(uint8_t TxDataBuff[ENET_TXBD_NUM][SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT)],
          APP_ENET_BUFF_ALIGNMENT);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Services an ethernet interrupt.
 *
 * @return none.
 */
#if FSL_FEATURE_ENET_QUEUE > 1
void ENETIF_Callback(ENET_Type *base,
                     enet_handle_t *handle,
                     uint32_t ringId,
                     enet_event_t event,
                     enet_frame_info_t *frameInfo,
                     void *param)
#else
void ENETIF_Callback(
    ENET_Type *base, enet_handle_t *handle, enet_event_t event, enet_frame_info_t *frameInfo, void *param)
#endif
{
    switch (event)
    {
        case kENET_RxEvent:
            ENETIF_Input(handle, param);
            break;
        default:
            break;
    }
}

/*!
 * @brief Get the ethernet speed.
 *
 * @return Value of ethernet speed in Mbps.
 */
uint32_t ENETIF_GetSpeed(void)
{
    bool link = false;
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t count = 0;
    speed          = kPHY_Speed100M;
    while ((count < ENET_PHY_TIMEOUT) && (!link))
    {
        PHY_GetLinkStatus(&phyHandle, &link);
        if (link)
        {
            /* Get the actual PHY link speed. */
            PHY_GetLinkSpeedDuplex(&phyHandle, &speed, &duplex);
        }

        count++;
    }

    if (count == ENET_PHY_TIMEOUT)
    {
        usb_echo("\r\nPHY Link down, please check the cable connection.\r\n");
    }
    return (kPHY_Speed100M == speed ? 100 : 10);
}

/*!
 * @brief Get the ethernet link status.
 *
 * @return Ture if linked, otherwise false.
 */
bool ENETIF_GetLinkStatus(void)
{
    bool link = false;

    uint32_t count = 0;
    while ((count < ENET_PHY_TIMEOUT) && (!link))
    {
        PHY_GetLinkStatus(&phyHandle, &link);

        count++;
    }

    if (count == ENET_PHY_TIMEOUT)
    {
        usb_echo("\r\nPHY Link down, please check the cable connection.\r\n");
    }
    return link;
}

/*!
 * @brief Transmit the packet.
 *
 * @return Error code.
 */

enet_err_t ENETIF_Output(pbuf_t *packetBuffer)
{
    if (packetBuffer->length >= ENET_FRAME_MAX_FRAMELEN)
    {
        return ENET_ERROR;
    }
    /* Send a frame out. */
    if (kStatus_Success ==
        ENET_SendFrame(BOARD_ENET_BASEADDR, &g_handle, packetBuffer->payload, packetBuffer->length, 0, false, NULL))
    {
        return ENET_OK;
    }
    return ENET_ERROR;
}

/*!
 * @brief Handle the receptions of packets from the network interface.
 *
 * @return none.
 */
void ENETIF_Input(enet_handle_t *handle, void *param)
{
    enet_header_t *ethhdr;
    status_t status;
    uint32_t length = 0;
    enet_data_error_stats_t eErrStatic;

    /* Read all data from ring buffer and send to uper layer */
    do
    {
        /* Get the Frame size */
        status = ENET_GetRxFrameSize(handle, &length, 0);

        /* Call ENET_ReadFrame when there is a received frame. */
        if (length != 0)
        {
            pbuf_t packetBuffer;
            packetBuffer.payload = VNIC_EnetRxBufAlloc();
            /* Received valid frame. Deliver the rx buffer with the size equal to length. */
            if (packetBuffer.payload == NULL)
            {
                /* No packet buffer available, ignore this packet. */
                ENET_ReadFrame(BOARD_ENET_BASEADDR, handle, NULL, length, 0, NULL);
                return;
            }
            packetBuffer.length = (uint16_t)length;
            ENET_ReadFrame(BOARD_ENET_BASEADDR, handle, packetBuffer.payload, packetBuffer.length, 0, NULL);

            /* points to packet payload, which starts with an Ethernet header */
            ethhdr = (enet_header_t *)packetBuffer.payload;
            switch (USB_SHORT_FROM_BIG_ENDIAN(ethhdr->type))
            {
                /* IP or ARP packet? */
                case ETHTYPE_IP:
                case ETHTYPE_ARP:
                    /* send packet to application to process */
                    VNIC_EnetCallback(&packetBuffer);
                    break;

                default:
                    VNIC_EnetRxBufFree(&packetBuffer);
                    break;
            }
        }
        else
        {
            /* Update the received buffer when error happened. */
            if (status != kStatus_Success)
            {
                /* Get the error information of the received g_frame. */
                ENET_GetRxErrBeforeReadFrame(handle, &eErrStatic, 0);
                /* update the receive buffer. */
                ENET_ReadFrame(BOARD_ENET_BASEADDR, handle, NULL, 0, 0, NULL);
            }
        }
    } while (kStatus_ENET_RxFrameEmpty != status);
}

/*!
 * @brief Initialize the ethernet.
 *
 * @return Error code.
 */
enet_err_t ENETIF_Init(void)
{
    enet_err_t result = ENET_OK;
    enet_config_t config;
    uint32_t count = 0;
    phy_speed_t speed;
    phy_duplex_t duplex;
    bool link              = false;
    bool autonego          = false;
    phy_config_t phyConfig = {0};

    /* initialize the hardware */
    /* set MAC hardware address */
    g_hwaddr[0] = configMAC_ADDR0;
    g_hwaddr[1] = configMAC_ADDR1;
    g_hwaddr[2] = configMAC_ADDR2;
    g_hwaddr[3] = configMAC_ADDR3;
    g_hwaddr[4] = configMAC_ADDR4;
    g_hwaddr[5] = configMAC_ADDR5;

    /* prepare the buffer configuration. */
    enet_buffer_config_t buffCfg[] = {{
        ENET_RXBD_NUM,
        ENET_TXBD_NUM,
        SDK_SIZEALIGN(ENET_RXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
        SDK_SIZEALIGN(ENET_TXBUFF_SIZE, APP_ENET_BUFF_ALIGNMENT),
        &RxBuffDescrip[0],
        &TxBuffDescrip[0],
        &RxDataBuff[0][0],
        &TxDataBuff[0][0],
        true,
        true,
        NULL,
    }};

    ENET_GetDefaultConfig(&config);
#if FSL_FEATURE_ENET_QUEUE > 1
    config.miiMode = kENET_RmiiMode;
#else

#endif

    phyConfig.phyAddr  = BOARD_ENET0_PHY_ADDRESS;
    phyConfig.ops      = BOARD_PHY_OPS;
    phyConfig.resource = BOARD_PHY_RESOURCE;
    phyConfig.autoNeg  = true;
    PHY_Init(&phyHandle, &phyConfig);

    while ((count < ENET_PHY_TIMEOUT) && (!(link && autonego)))
    {
        PHY_GetAutoNegotiationStatus(&phyHandle, &autonego);
        PHY_GetLinkStatus(&phyHandle, &link);
        count++;
    }

    if (count == ENET_PHY_TIMEOUT)
    {
        usb_echo("\r\nPHY Link down, please check the cable connection.\r\n");
    }

    if (link)
    {
        /* Get the actual PHY link speed. */
        PHY_GetLinkSpeedDuplex(&phyHandle, &speed, &duplex);
        /* Change the MII speed and duplex for actual link status. */
        config.miiSpeed  = (enet_mii_speed_t)speed;
        config.miiDuplex = (enet_mii_duplex_t)duplex;
        config.interrupt = kENET_RxFrameInterrupt | kENET_TxFrameInterrupt;
    }

    ENET_Init(BOARD_ENET_BASEADDR, &g_handle, &config, &buffCfg[0], &g_hwaddr[0], BOARD_PHY_SYS_CLOCK);
    ENET_SetCallback(&g_handle, ENETIF_Callback, NULL);

    ENET_ActiveRead(BOARD_ENET_BASEADDR);

    return result;
}
