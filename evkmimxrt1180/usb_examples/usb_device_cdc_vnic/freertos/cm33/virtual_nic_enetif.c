/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2020 - 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "board.h"
#include "fsl_debug_console.h"
#include "virtual_nic_enetif.h"
#include "fsl_netc_endpoint.h"
#include "fsl_netc_switch.h"
#include "fsl_netc_mdio.h"
#if defined(BOARD_USE_NETC_PHY_RTL8201)
#include "fsl_phyrtl8201.h"
#elif defined(BOARD_USE_NETC_PHY_RTL8211F)
#include "fsl_phyrtl8211f.h"
#endif
#include "fsl_msgintr.h"

#include "fsl_netc.h"
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
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NETC_EP0_PORT                0x00U
#define NETC_EP0_PHY_ADDR            0x03U
#define PHY_PAGE_SELECT_REG          0x1FU /*!< The PHY page select register. */
#define EXAMPLE_EP_BUFF_SIZE_ALIGN   64U
#define EXAMPLE_EP_RXBUFF_SIZE       1518U
#define EXAMPLE_EP_RXBUFF_SIZE_ALIGN SDK_SIZEALIGN(EXAMPLE_EP_RXBUFF_SIZE, EXAMPLE_EP_BUFF_SIZE_ALIGN)
#define EXAMPLE_TX_INTR_MSG_DATA     1U
#define EXAMPLE_RX_INTR_MSG_DATA     2U
#define EXAMPLE_TX_MSIX_ENTRY_IDX    0U
#define EXAMPLE_RX_MSIX_ENTRY_IDX    1U
#define EXAMPLE_EP_TEST_FRAME_SIZE   1000U

#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ENETIF_Input(pbuf_t *pbuffer);
void VNIC_EnetCallback(pbuf_t *pbuffer);
void VNIC_EnetRxBufFree(pbuf_t *pbuf);
uint8_t *VNIC_EnetRxBufAlloc(void);
extern NETC_ENETC_Type *BOARD_GetExampleEnetBase(void);
extern uint32_t BOARD_GetPhySysClock(void);
status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data);
status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData);
extern uint8_t g_hwaddr[ENET_MAC_ADDR_SIZE];
status_t APP_PHY_Init(void);
status_t APP_PHY_GetLinkModeSpeedDuplex(netc_hw_mii_mode_t *mode,
                                        netc_hw_mii_speed_t *speed,
                                        netc_hw_mii_duplex_t *duplex);
status_t APP_PHY_SetPort(phy_config_t *phyConfig);
/*******************************************************************************
 * Variables
 ******************************************************************************/
ep_handle_t g_handle;
netc_mdio_handle_t s_mdio_handle;
ep_config_t g_ep_config;
typedef uint8_t rx_buffer_t[EXAMPLE_EP_RXBUFF_SIZE_ALIGN];
AT_NONCACHEABLE_SECTION_ALIGN(netc_rx_bd_t RxBuffDescrip[NETC_RXBD_NUM], NETC_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(netc_tx_bd_t TxBuffDescrip[NETC_TXBD_NUM], NETC_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(rx_buffer_t RxDataBuff[NETC_RXBD_NUM], EXAMPLE_EP_BUFF_SIZE_ALIGN);
uint64_t rxBuffAddr[NETC_RXBD_NUM];
#if defined(BOARD_USE_NETC_PHY_RTL8201)
static phy_rtl8201_resource_t s_phy_resource;
#elif defined(BOARD_USE_NETC_PHY_RTL8211F)
static phy_rtl8211f_resource_t s_phy_resource;
#endif
static phy_handle_t s_phy_handle;
static uint8_t s_phy_address = NETC_EP0_PHY_ADDR;
static netc_tx_frame_info_t g_txDirty[NETC_TXBD_NUM];
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Get the ethernet speed.
 *
 * @return Value of ethernet speed in Mbps.
 */
uint32_t NETCIF_GetSpeed(void)
{
    bool link = false;
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t count = 0;
    speed          = kPHY_Speed100M;
    while ((count < ENET_PHY_TIMEOUT) && (!link))
    {
        PHY_GetLinkStatus(&s_phy_handle, &link);
        if (link)
        {
            /* Get the actual PHY link speed. */
            PHY_GetLinkSpeedDuplex(&s_phy_handle, &speed, &duplex);
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
        PHY_GetLinkStatus(&s_phy_handle, &link);

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

enet_err_t NETCIF_Output(pbuf_t *packetBuffer)
{
    netc_buffer_struct_t txBuff = {.buffer = packetBuffer->payload, .length = packetBuffer->length};
    netc_frame_struct_t txFrame = {.buffArray = &txBuff, .length = 1};

    if (packetBuffer->length >= ENET_FRAME_MAX_FRAMELEN)
    {
        return ENET_ERROR;
    }

    EP_ReclaimTxDescriptor(&g_handle, 0);

    /* Send a frame out. */
    if (kStatus_Success == EP_SendFrame(&g_handle, 0, &txFrame, NULL, NULL))
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
void ENETIF_Input(pbuf_t *pbuffer)
{
    netc_header_t *ethhdr;

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = (netc_header_t *)pbuffer->payload;
    switch (USB_SHORT_FROM_BIG_ENDIAN(ethhdr->type))
    {
        /* IP or ARP packet? */
        case ETHTYPE_IP:
        case ETHTYPE_ARP:
            /* send packet to application to process */
            VNIC_EnetCallback(pbuffer);
            break;

        default:
            VNIC_EnetRxBufFree(pbuffer);
            break;
    }
}

static status_t APP_ReclaimCallback(ep_handle_t *handle, uint8_t ring, netc_tx_frame_info_t *frameInfo, void *userData)
{
    return kStatus_Success;
}

status_t APP_EP0_MDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return NETC_MDIOWrite(&s_mdio_handle, phyAddr, regAddr, data);
}

status_t APP_EP0_MDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return NETC_MDIORead(&s_mdio_handle, phyAddr, regAddr, pData);
}

#ifdef BOARD_USE_NETC_PHY_RTL8201
static status_t APP_Phy8201SetUp(phy_handle_t *handle)
{
    status_t result;
    uint16_t data;

    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 7);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Read(handle, 16, &data);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* CRS/DV pin is RXDV signal. */
    data |= (1U << 2);
    result = PHY_Write(handle, 16, data);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = PHY_Write(handle, PHY_PAGE_SELECT_REG, 0);

    return result;
}
#endif /* BOARD_USE_NETC_PHY_RTL8201 */

void msgintrCallback(MSGINTR_Type *base, uint8_t channel, uint32_t pendingIntr)
{
    uint32_t length;
    status_t result = kStatus_Success;

    /* Transmit interrupt */
    if ((pendingIntr & (1U << EXAMPLE_TX_INTR_MSG_DATA)) != 0U)
    {
        EP_CleanTxIntrFlags(&g_handle, 1, 0);
    }
    /* Receive interrupt */
    if ((pendingIntr & (1U << EXAMPLE_RX_INTR_MSG_DATA)) != 0U)
    {
        g_handle.hw.si->BDR[0].RBIER &= ~ENETC_SI_RBIER_RXTIE_MASK;
        do
        {
            result = EP_GetRxFrameSize(&g_handle, 0, &length);
            if (0U == length)
            {
                break;
            }
            else if (result != kStatus_Success)
            {
                EP_ReceiveFrameCopy(&g_handle, 0, NULL, 1, NULL);
                break;
            }

            pbuf_t packetBuffer;
            packetBuffer.payload = VNIC_EnetRxBufAlloc();
            /* Received valid frame. Deliver the rx buffer with the size equal to length. */
            if (packetBuffer.payload == NULL)
            {
                /* No packet buffer available, ignore this packet. */
                EP_ReceiveFrameCopy(&g_handle, 0, NULL, 1, NULL);
                break;
            }
            packetBuffer.length = length;
            result              = EP_ReceiveFrameCopy(&g_handle, 0, packetBuffer.payload, packetBuffer.length, NULL);
            if (result != kStatus_Success)
            {
                break;
            }
            ENETIF_Input(&packetBuffer);
        } while (1);

        EP_CleanRxIntrFlags(&g_handle, 1);
        g_handle.hw.si->BDR[0].RBIER = ENETC_SI_RBIER_RXTIE_MASK;
    }
}

status_t NETC_PHY_GetLinkStatus(uint32_t port, bool *link)
{
    return PHY_GetLinkStatus(&s_phy_handle, link);
}

status_t APP_PHY_GetLinkModeSpeedDuplex(netc_hw_mii_mode_t *mode,
                                        netc_hw_mii_speed_t *speed,
                                        netc_hw_mii_duplex_t *duplex)
{
#if defined(BOARD_USE_NETC_PHY_RTL8201)
    *mode = kNETC_RmiiMode;
#elif defined(BOARD_USE_NETC_PHY_RTL8211F)
    *mode = kNETC_RgmiiMode;
#endif

    return PHY_GetLinkSpeedDuplex(&s_phy_handle, (phy_speed_t *)speed, (phy_duplex_t *)duplex);
}

status_t APP_PHY_SetPort(phy_config_t *phyConfig)
{
    status_t result = kStatus_Success;

    s_phy_resource.write = APP_EP0_MDIOWrite;
    s_phy_resource.read  = APP_EP0_MDIORead;

    result = PHY_Init(&s_phy_handle, phyConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    return result;
}

status_t APP_PHY_Init(void)
{
    status_t result        = kStatus_Success;
    phy_config_t phyConfig = {
        .autoNeg   = false,
        .speed     = kPHY_Speed100M,
        .duplex    = kPHY_FullDuplex,
        .enableEEE = false,
#if defined(BOARD_USE_NETC_PHY_RTL8201)
        .ops = &phyrtl8201_ops,
#elif defined(BOARD_USE_NETC_PHY_RTL8211F)
        .ops = &phyrtl8211f_ops,
#endif
    };

    /* Initialize PHY for EP. */
    phyConfig.resource = &s_phy_resource;
    phyConfig.phyAddr  = s_phy_address;
    result             = APP_PHY_SetPort(&phyConfig);
    if (result != kStatus_Success)
    {
        return result;
    }
#ifdef BOARD_USE_NETC_PHY_RTL8201
    result = APP_Phy8201SetUp(&s_phy_handle);
    if (result != kStatus_Success)
    {
        return result;
    }
#if defined(EXAMPLE_PORT_USE_100M_HALF_DUPLEX_MODE)
    uint16_t phyRegValue;
    (void)PHY_Write(&s_phy_handle, 0x1F, 7);
    (void)PHY_Read(&s_phy_handle, 20, &phyRegValue);
    (void)PHY_Write(&s_phy_handle, 20, (phyRegValue | 0x900U));
    (void)PHY_Write(&s_phy_handle, 0x1F, 0);
#endif /* EXAMPLE_PORT_USE_100M_HALF_DUPLEX_MODE */
#endif /* BOARD_USE_NETC_PHY_RTL8201 */
    return result;
}

/*!
 * @brief Initialize the ethernet.
 *
 * @return Error code.
 */
enet_err_t NETCIF_Init(void)
{
    enet_err_t result                = ENET_OK;
    bool link                        = false;
    netc_rx_bdr_config_t rxBdrConfig = {0};
    netc_tx_bdr_config_t txBdrConfig = {0};
    netc_bdr_config_t bdrConfig      = {.rxBdrConfig = &rxBdrConfig, .txBdrConfig = &txBdrConfig};
    netc_hw_mii_mode_t phyMode;
    netc_hw_mii_speed_t phySpeed;
    netc_hw_mii_duplex_t phyDuplex;
    netc_msix_entry_t msixEntry[2];
    uint32_t msgAddr;

    /* initialize the hardware */
    /* set MAC hardware address */
    g_hwaddr[0] = configMAC_ADDR0;
    g_hwaddr[1] = configMAC_ADDR1;
    g_hwaddr[2] = configMAC_ADDR2;
    g_hwaddr[3] = configMAC_ADDR3;
    g_hwaddr[4] = configMAC_ADDR4;
    g_hwaddr[5] = configMAC_ADDR5;

    result = APP_PHY_Init();
    if (result != kStatus_Success)
    {
        PRINTF("\r\nPHY Init failed!\r\n");
        return result;
    }

    for (uint8_t index = 0U; index < NETC_RXBD_NUM; index++)
    {
        rxBuffAddr[index] = (uint64_t)(uintptr_t)&RxDataBuff[index];
    }

    /* MSIX and interrupt configuration. */
    MSGINTR_Init(MSGINTR1, &msgintrCallback);
    NVIC_SetPriority(MSGINTR1_IRQn, ENET_INTERRUPT_PRIORITY);
    msgAddr              = MSGINTR_GetIntrSelectAddr(MSGINTR1, 0);
    msixEntry[0].control = kNETC_MsixIntrMaskBit;
    msixEntry[0].msgAddr = msgAddr;
    msixEntry[0].msgData = EXAMPLE_TX_INTR_MSG_DATA;
    msixEntry[1].control = kNETC_MsixIntrMaskBit;
    msixEntry[1].msgAddr = msgAddr;
    msixEntry[1].msgData = EXAMPLE_RX_INTR_MSG_DATA;

    /* BD ring configuration. */
    bdrConfig.rxBdrConfig[0].bdArray       = &RxBuffDescrip[0];
    bdrConfig.rxBdrConfig[0].len           = NETC_RXBD_NUM;
    bdrConfig.rxBdrConfig[0].buffAddrArray = &rxBuffAddr[0];
    bdrConfig.rxBdrConfig[0].buffSize      = EXAMPLE_EP_RXBUFF_SIZE_ALIGN;
    bdrConfig.rxBdrConfig[0].msixEntryIdx  = EXAMPLE_RX_MSIX_ENTRY_IDX;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].enThresIntr   = true;
    bdrConfig.rxBdrConfig[0].enCoalIntr    = true;
    bdrConfig.rxBdrConfig[0].intrThreshold = 1;

    bdrConfig.txBdrConfig[0].bdArray      = &TxBuffDescrip[0];
    bdrConfig.txBdrConfig[0].len          = NETC_TXBD_NUM;
    bdrConfig.txBdrConfig[0].dirtyArray   = &g_txDirty[0];
    bdrConfig.txBdrConfig[0].msixEntryIdx = EXAMPLE_TX_MSIX_ENTRY_IDX;
    bdrConfig.txBdrConfig[0].enIntr       = true;

    /* Wait PHY link up. */
    do
    {
        result = PHY_GetLinkStatus(&s_phy_handle, &link);
    } while ((result != kStatus_Success) || (!link));

    result = APP_PHY_GetLinkModeSpeedDuplex(&phyMode, &phySpeed, &phyDuplex);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Endpoint configuration. */
    (void)EP_GetDefaultConfig(&g_ep_config);
    g_ep_config.si                    = kNETC_ENETC0PSI0;
    g_ep_config.siConfig.txRingUse    = 1;
    g_ep_config.siConfig.rxRingUse    = 1;
    g_ep_config.reclaimCallback       = APP_ReclaimCallback;
    g_ep_config.msixEntry             = &msixEntry[0];
    g_ep_config.entryNum              = 2;
    g_ep_config.port.ethMac.miiMode   = phyMode;
    g_ep_config.port.ethMac.miiSpeed  = phySpeed;
    g_ep_config.port.ethMac.miiDuplex = phyDuplex;
    g_ep_config.rxCacheMaintain       = true;
    g_ep_config.txCacheMaintain       = true;
    result                            = EP_Init(&g_handle, &g_hwaddr[0], &g_ep_config, &bdrConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Unmask MSIX message interrupt. */
    EP_MsixSetEntryMask(&g_handle, EXAMPLE_TX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_handle, EXAMPLE_RX_MSIX_ENTRY_IDX, false);

    return result;
}
