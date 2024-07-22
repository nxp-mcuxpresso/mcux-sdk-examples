/*
 * Copyright 2021-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "fsl_netc_endpoint.h"
#include "fsl_netc_switch.h"
#include "fsl_netc_mdio.h"
#include "fsl_phyrtl8211f.h"
#include "fsl_phyrtl8201.h"
#include "fsl_msgintr.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_NETC_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Netc)

/* Ethernet port identifier. */
#define EXAMPLE_EP_NUM   1U
#define EXAMPLE_EP0_PORT 0x00U
#define EXAMPLE_EP_SI    \
    {                    \
        kNETC_ENETC0PSI0 \
    }
#define EXAMPLE_MSGINTR   MSGINTR1
#define EXAMPLE_SWT_PORT0 0x01U
#define EXAMPLE_SWT_PORT1 0x02U
#define EXAMPLE_SWT_PORT2 0x03U
#define EXAMPLE_SWT_PORT3 0x04U

/* Buffer desciptor configuration. */
#define EXAMPLE_EP_RING_NUM          3U
#define EXAMPLE_EP_RXBD_NUM          8U
#define EXAMPLE_EP_TXBD_NUM          8U
#define EXAMPLE_EP_BUFF_SIZE_ALIGN   64U
#define EXAMPLE_EP_RXBUFF_SIZE       1518U
#define EXAMPLE_EP_RXBUFF_SIZE_ALIGN SDK_SIZEALIGN(EXAMPLE_EP_RXBUFF_SIZE, EXAMPLE_EP_BUFF_SIZE_ALIGN)

#define EXAMPLE_EP_TEST_FRAME_SIZE 1000U
#define EXAMPLE_EP_TXFRAME_NUM     20U
#define EXAMPLE_SWT_MAX_PORT_NUM   4U
/*! Note: Be careful that some ports are multiplexed with SEMC. */
#if !defined(EXAMPLE_SWT_USED_PORT_BITMAP)
#define EXAMPLE_SWT_USED_PORT_BITMAP 0xFU /*! Enabled Switch port bit map, bit n represents port n. */
#endif
/*!< PHY reset pins. */
#define EXAMPLE_EP0_PORT_PHY_RESET_PIN  RGPIO4, 13
#define EXAMPLE_SWT_PORT0_PHY_RESET_PIN RGPIO4, 25
#define EXAMPLE_SWT_PORT1_PHY_RESET_PIN RGPIO6, 13
#define EXAMPLE_SWT_PORT2_PHY_RESET_PIN RGPIO4, 28
#define EXAMPLE_SWT_PORT3_PHY_RESET_PIN RGPIO6, 15

#define PHY_PAGE_SELECT_REG 0x1FU /*!< The PHY page select register. */
#define EXAMPLE_EP_BD_ALIGN       128U
#define EXAMPLE_TX_INTR_MSG_DATA  1U
#define EXAMPLE_RX_INTR_MSG_DATA  2U
#define EXAMPLE_TX_MSIX_ENTRY_IDX 0U
#define EXAMPLE_RX_MSIX_ENTRY_IDX 1U
#define EXAMPLE_FRAME_FID         1U

#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t APP_MDIO_Init(void);
status_t APP_PHY_Init(void);
status_t APP_PHY_GetLinkStatus(uint32_t port, bool *link);
status_t APP_PHY_GetLinkModeSpeedDuplex(uint32_t port,
                                        netc_hw_mii_mode_t *mode,
                                        netc_hw_mii_speed_t *speed,
                                        netc_hw_mii_duplex_t *duplex);
/* Rx buffer memeory type. */
typedef uint8_t rx_buffer_t[EXAMPLE_EP_RXBUFF_SIZE_ALIGN];

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* PHY operation. */
#ifdef EXAMPLE_PHY_USE_PORT_MDIO
static netc_mdio_handle_t s_mdio_handle[5];
#else
static netc_mdio_handle_t s_emdio_handle;
#endif
static phy_rtl8211f_resource_t s_phy_resource[5];
static phy_handle_t s_phy_handle[5];
/* EP resource. */
static ep_handle_t g_ep_handle;
static netc_hw_si_idx_t g_siIndex[EXAMPLE_EP_NUM] = EXAMPLE_EP_SI;

#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
/* SWT resource. */
static swt_handle_t g_swt_handle;
static swt_config_t g_swt_config;
static swt_transfer_config_t swtTxRxConfig;
#endif

/* Buffer descriptor resource. */
AT_NONCACHEABLE_SECTION_ALIGN(static netc_rx_bd_t g_rxBuffDescrip[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM],
                              EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_tx_bd_t g_txBuffDescrip[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_TXBD_NUM],
                              EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static rx_buffer_t g_rxDataBuff[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM],
                              EXAMPLE_EP_BUFF_SIZE_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t g_txFrame[EXAMPLE_EP_TEST_FRAME_SIZE], EXAMPLE_EP_BUFF_SIZE_ALIGN);
#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
AT_NONCACHEABLE_SECTION_ALIGN(static netc_tx_bd_t g_mgmtTxBuffDescrip[EXAMPLE_EP_TXBD_NUM], EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_cmd_bd_t g_cmdBuffDescrip[EXAMPLE_EP_TXBD_NUM], EXAMPLE_EP_BD_ALIGN);
#endif
AT_NONCACHEABLE_SECTION(static uint8_t g_rxFrame[EXAMPLE_EP_RXBUFF_SIZE_ALIGN]);
static uint64_t rxBuffAddrArray[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM];
#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
static netc_tx_frame_info_t g_mgmtTxDirty[EXAMPLE_EP_TXBD_NUM];
static netc_tx_frame_info_t mgmtTxFrameInfo;
#endif
static netc_tx_frame_info_t g_txDirty[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_TXBD_NUM];
static netc_tx_frame_info_t txFrameInfo;
static uint8_t txFrameNum, rxFrameNum;
static volatile bool txOver;

/* MAC address. */
static uint8_t g_macAddr[6] = {0x54, 0x27, 0x8d, 0x00, 0x00, 0x00};

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t APP_MDIO_Init(void)
{
    status_t result = kStatus_Success;

    netc_mdio_config_t mdioConfig = {
        .isPreambleDisable = false,
        .isNegativeDriven  = false,
        .srcClockHz        = EXAMPLE_NETC_FREQ,
    };

#ifdef EXAMPLE_PHY_USE_PORT_MDIO
    /* Usually should call EP_Init/SWT_Init then init port MDIO, here just an quick enablement example. */
    NETC_F2_PCI_HDR_TYPE0->PCI_CFH_CMD |=
        (ENETC_PCI_TYPE0_PCI_CFH_CMD_MEM_ACCESS_MASK | ENETC_PCI_TYPE0_PCI_CFH_CMD_BUS_MASTER_EN_MASK);
    NETC_F3_PCI_HDR_TYPE0->PCI_CFH_CMD |=
        (ENETC_PCI_TYPE0_PCI_CFH_CMD_MEM_ACCESS_MASK | ENETC_PCI_TYPE0_PCI_CFH_CMD_BUS_MASTER_EN_MASK);

    for (int i = 0U; i < 5U; i++)
    {
        mdioConfig.mdio.port = (netc_hw_eth_port_idx_t)((uint32_t)kNETC_ENETC0EthPort + i);
        result               = NETC_MDIOInit(&s_mdio_handle[i], &mdioConfig);
        if (result != kStatus_Success)
        {
            return result;
        }
    }
#else
    mdioConfig.mdio.type = kNETC_EMdio;
    result               = NETC_MDIOInit(&s_emdio_handle, &mdioConfig);
    if (result != kStatus_Success)
    {
        return result;
    }
#endif

    return result;
}

#ifdef EXAMPLE_PHY_USE_PORT_MDIO
static status_t APP_PMDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    status_t result = kStatus_Success;
    netc_mdio_handle_t *mdioHandle;

    switch (phyAddr)
    {
        case BOARD_EP0_PHY_ADDR:
            mdioHandle = &s_mdio_handle[0];
            break;
        case BOARD_SWT_PORT0_PHY_ADDR:
            mdioHandle = &s_mdio_handle[1];
            break;
        case BOARD_SWT_PORT1_PHY_ADDR:
            mdioHandle = &s_mdio_handle[2];
            break;
        case BOARD_SWT_PORT2_PHY_ADDR:
            mdioHandle = &s_mdio_handle[3];
            break;
        case BOARD_SWT_PORT3_PHY_ADDR:
            mdioHandle = &s_mdio_handle[4];
            break;
        default:
            result = kStatus_InvalidArgument;
            break;
    }

    if (result != kStatus_Success)
    {
        return result;
    }

    return NETC_MDIOWrite(mdioHandle, phyAddr, regAddr, data);
}

static status_t APP_PMDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    status_t result = kStatus_Success;
    netc_mdio_handle_t *mdioHandle;

    switch (phyAddr)
    {
        case BOARD_EP0_PHY_ADDR:
            mdioHandle = &s_mdio_handle[0];
            break;
        case BOARD_SWT_PORT0_PHY_ADDR:
            mdioHandle = &s_mdio_handle[1];
            break;
        case BOARD_SWT_PORT1_PHY_ADDR:
            mdioHandle = &s_mdio_handle[2];
            break;
        case BOARD_SWT_PORT2_PHY_ADDR:
            mdioHandle = &s_mdio_handle[3];
            break;
        case BOARD_SWT_PORT3_PHY_ADDR:
            mdioHandle = &s_mdio_handle[4];
            break;
        default:
            result = kStatus_InvalidArgument;
            break;
    }

    if (result != kStatus_Success)
    {
        return result;
    }

    return NETC_MDIORead(mdioHandle, phyAddr, regAddr, pData);
}
#else
static status_t APP_EMDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return NETC_MDIOWrite(&s_emdio_handle, phyAddr, regAddr, data);
}

static status_t APP_EMDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return NETC_MDIORead(&s_emdio_handle, phyAddr, regAddr, pData);
}
#endif

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

static status_t APP_PHY_SetPort(uint32_t port, phy_config_t *phyConfig)
{
    status_t result = kStatus_Success;

#ifdef EXAMPLE_PHY_USE_PORT_MDIO
    s_phy_resource[port].write = APP_PMDIOWrite;
    s_phy_resource[port].read  = APP_PMDIORead;
#else
    s_phy_resource[port].write = APP_EMDIOWrite;
    s_phy_resource[port].read  = APP_EMDIORead;
#endif
    result = PHY_Init(&s_phy_handle[port], phyConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    return PHY_EnableLoopback(&s_phy_handle[port], kPHY_LocalLoop, phyConfig->speed, true);
}

status_t APP_PHY_Init(void)
{
    status_t result            = kStatus_Success;
    phy_config_t phy8211Config = {
        .autoNeg   = false,
        .speed     = kPHY_Speed1000M,
        .duplex    = kPHY_FullDuplex,
        .enableEEE = false,
        .ops       = &phyrtl8211f_ops,
    };
    phy_config_t phy8201Config = {
        .autoNeg   = false,
        .speed     = kPHY_Speed100M,
        .duplex    = kPHY_FullDuplex,
        .enableEEE = false,
        .ops       = &phyrtl8201_ops,
    };

    /* Reset all PHYs even some are not used in case unstable status has effect on other PHYs. */
    /* Reset PHY8201 for ETH4(EP), ETH0(Switch port0). Power on 150ms, reset 10ms, wait 150ms. */
    /* Reset PHY8211 for ETH1(Switch port1), ETH2(Switch port2), ETH3(Switch port3). Reset 10ms, wait 30ms. */
    RGPIO_PinWrite(EXAMPLE_EP0_PORT_PHY_RESET_PIN, 0);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT0_PHY_RESET_PIN, 0);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT1_PHY_RESET_PIN, 0);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT2_PHY_RESET_PIN, 0);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT3_PHY_RESET_PIN, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    RGPIO_PinWrite(EXAMPLE_EP0_PORT_PHY_RESET_PIN, 1);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT0_PHY_RESET_PIN, 1);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT1_PHY_RESET_PIN, 1);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT2_PHY_RESET_PIN, 1);
    RGPIO_PinWrite(EXAMPLE_SWT_PORT3_PHY_RESET_PIN, 1);
    SDK_DelayAtLeastUs(150000, CLOCK_GetFreq(kCLOCK_CpuClk));

    /* Initialize PHY for EP. */
    phy8201Config.resource = &s_phy_resource[EXAMPLE_EP0_PORT];
    phy8201Config.phyAddr  = BOARD_EP0_PHY_ADDR;
    result                 = APP_PHY_SetPort(EXAMPLE_EP0_PORT, &phy8201Config);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = APP_Phy8201SetUp(&s_phy_handle[EXAMPLE_EP0_PORT]);
    if (result != kStatus_Success)
    {
        return result;
    }
#if defined(EXAMPLE_PORT_USE_100M_HALF_DUPLEX_MODE)
    uint16_t phyRegValue;
    (void)PHY_Write(&s_phy_handle[EXAMPLE_EP0_PORT], 0x1F, 7);
    (void)PHY_Read(&s_phy_handle[EXAMPLE_EP0_PORT], 20, &phyRegValue);
    (void)PHY_Write(&s_phy_handle[EXAMPLE_EP0_PORT], 20, (phyRegValue | 0x900U));
    (void)PHY_Write(&s_phy_handle[EXAMPLE_EP0_PORT], 0x1F, 0);
#endif

    /* Initialize PHY for switch port0. */
    phy8201Config.resource = &s_phy_resource[EXAMPLE_SWT_PORT0];
    phy8201Config.phyAddr  = BOARD_SWT_PORT0_PHY_ADDR;
    result                 = APP_PHY_SetPort(EXAMPLE_SWT_PORT0, &phy8201Config);
    if (result != kStatus_Success)
    {
        return result;
    }
    result = APP_Phy8201SetUp(&s_phy_handle[EXAMPLE_SWT_PORT0]);
    if (result != kStatus_Success)
    {
        return result;
    }
#if defined(EXAMPLE_PORT_USE_100M_HALF_DUPLEX_MODE)
    (void)PHY_Write(&s_phy_handle[EXAMPLE_SWT_PORT0], 0x1F, 7);
    (void)PHY_Read(&s_phy_handle[EXAMPLE_SWT_PORT0], 20, &phyRegValue);
    (void)PHY_Write(&s_phy_handle[EXAMPLE_SWT_PORT0], 20, (phyRegValue | 0x900U));
    (void)PHY_Write(&s_phy_handle[EXAMPLE_SWT_PORT0], 0x1F, 0);
#endif

    /* Initialize PHY for switch port1. */
    phy8211Config.resource = &s_phy_resource[EXAMPLE_SWT_PORT1];
    phy8211Config.phyAddr  = BOARD_SWT_PORT1_PHY_ADDR;
    result                 = APP_PHY_SetPort(EXAMPLE_SWT_PORT1, &phy8211Config);
    if (result != kStatus_Success)
    {
        return result;
    }

    if (((1U << 2) & EXAMPLE_SWT_USED_PORT_BITMAP) != 0U)
    {
        /* Initialize PHY for switch port2. */
        phy8211Config.resource = &s_phy_resource[EXAMPLE_SWT_PORT2];
        phy8211Config.phyAddr  = BOARD_SWT_PORT2_PHY_ADDR;
        result                 = APP_PHY_SetPort(EXAMPLE_SWT_PORT2, &phy8211Config);
        if (result != kStatus_Success)
        {
            return result;
        }
    }

    if (((1U << 3) & EXAMPLE_SWT_USED_PORT_BITMAP) != 0U)
    {
        /* Initialize PHY for switch port3. */
        phy8211Config.resource = &s_phy_resource[EXAMPLE_SWT_PORT3];
        phy8211Config.phyAddr  = BOARD_SWT_PORT3_PHY_ADDR;
        result                 = APP_PHY_SetPort(EXAMPLE_SWT_PORT3, &phy8211Config);
        if (result != kStatus_Success)
        {
            return result;
        }
    }

    return result;
}

status_t APP_PHY_GetLinkStatus(uint32_t port, bool *link)
{
    return PHY_GetLinkStatus(&s_phy_handle[port], link);
}

status_t APP_PHY_GetLinkModeSpeedDuplex(uint32_t port,
                                        netc_hw_mii_mode_t *mode,
                                        netc_hw_mii_speed_t *speed,
                                        netc_hw_mii_duplex_t *duplex)
{
    switch (port)
    {
        case EXAMPLE_EP0_PORT:
            *mode = kNETC_RmiiMode;
            break;
        case EXAMPLE_SWT_PORT0:
            *mode = kNETC_RmiiMode;
            break;
        case EXAMPLE_SWT_PORT1:
            *mode = kNETC_RgmiiMode;
            break;
        case EXAMPLE_SWT_PORT2:
            *mode = kNETC_RgmiiMode;
            break;
        case EXAMPLE_SWT_PORT3:
            *mode = kNETC_RgmiiMode;
            break;
        default:
            assert(false);
            break;
    }

    return PHY_GetLinkSpeedDuplex(&s_phy_handle[port], (phy_speed_t *)speed, (phy_duplex_t *)duplex);
}

/*! @brief Build Frame for single ring transmit. */
static void APP_BuildBroadCastFrame(void)
{
    uint32_t length = EXAMPLE_EP_TEST_FRAME_SIZE - 14U;
    uint32_t count;

    for (count = 0; count < 6U; count++)
    {
        g_txFrame[count] = 0xFFU;
    }
    memcpy(&g_txFrame[6], &g_macAddr[0], 6U);
    g_txFrame[12] = (length >> 8U) & 0xFFU;
    g_txFrame[13] = length & 0xFFU;

    for (count = 0; count < length; count++)
    {
        g_txFrame[count + 14U] = count % 0xFFU;
    }
}

static status_t APP_ReclaimCallback(ep_handle_t *handle, uint8_t ring, netc_tx_frame_info_t *frameInfo, void *userData)
{
    txFrameInfo = *frameInfo;
    return kStatus_Success;
}

#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
static status_t APP_SwtReclaimCallback(swt_handle_t *handle, netc_tx_frame_info_t *frameInfo, void *userData)
{
    mgmtTxFrameInfo = *frameInfo;
    return kStatus_Success;
}
#endif

void msgintrCallback(MSGINTR_Type *base, uint8_t channel, uint32_t pendingIntr)
{
    /* Transmit interrupt */
    if ((pendingIntr & (1U << EXAMPLE_TX_INTR_MSG_DATA)) != 0U)
    {
        EP_CleanTxIntrFlags(&g_ep_handle, 1, 0);
        txOver = true;
    }
    /* Receive interrupt */
    if ((pendingIntr & (1U << EXAMPLE_RX_INTR_MSG_DATA)) != 0U)
    {
        EP_CleanRxIntrFlags(&g_ep_handle, 1);
    }
}

status_t APP_EP_XferLoopBack(uint32_t index)
{
    status_t result                  = kStatus_Success;
    netc_rx_bdr_config_t rxBdrConfig = {0};
    netc_tx_bdr_config_t txBdrConfig = {0};
    netc_bdr_config_t bdrConfig      = {.rxBdrConfig = &rxBdrConfig, .txBdrConfig = &txBdrConfig};
    netc_buffer_struct_t txBuff      = {.buffer = &g_txFrame, .length = sizeof(g_txFrame)};
    netc_frame_struct_t txFrame      = {.buffArray = &txBuff, .length = 1};
    bool link                        = false;
    netc_msix_entry_t msixEntry[2];
    netc_hw_mii_mode_t phyMode;
    netc_hw_mii_speed_t phySpeed;
    netc_hw_mii_duplex_t phyDuplex;
    ep_config_t ep_config;
    uint32_t msgAddr;
    uint32_t length;

    PRINTF("\r\nNETC EP%d frame loopback example start.\r\n", index);

    /* MSIX and interrupt configuration. */
    MSGINTR_Init(EXAMPLE_MSGINTR, &msgintrCallback);
    msgAddr              = MSGINTR_GetIntrSelectAddr(EXAMPLE_MSGINTR, 0);
    msixEntry[0].control = kNETC_MsixIntrMaskBit;
    msixEntry[0].msgAddr = msgAddr;
    msixEntry[0].msgData = EXAMPLE_TX_INTR_MSG_DATA;
    msixEntry[1].control = kNETC_MsixIntrMaskBit;
    msixEntry[1].msgAddr = msgAddr;
    msixEntry[1].msgData = EXAMPLE_RX_INTR_MSG_DATA;

    /* BD ring configuration. */
    bdrConfig.rxBdrConfig[0].bdArray       = &g_rxBuffDescrip[0][0];
    bdrConfig.rxBdrConfig[0].len           = EXAMPLE_EP_RXBD_NUM;
    bdrConfig.rxBdrConfig[0].buffAddrArray = &rxBuffAddrArray[0][0];
    bdrConfig.rxBdrConfig[0].buffSize      = EXAMPLE_EP_RXBUFF_SIZE_ALIGN;
    bdrConfig.rxBdrConfig[0].msixEntryIdx  = EXAMPLE_RX_MSIX_ENTRY_IDX;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].enThresIntr   = true;
    bdrConfig.rxBdrConfig[0].enCoalIntr    = true;
    bdrConfig.rxBdrConfig[0].intrThreshold = 1;

    bdrConfig.txBdrConfig[0].bdArray      = &g_txBuffDescrip[0][0];
    bdrConfig.txBdrConfig[0].len          = EXAMPLE_EP_TXBD_NUM;
    bdrConfig.txBdrConfig[0].dirtyArray   = &g_txDirty[0][0];
    bdrConfig.txBdrConfig[0].msixEntryIdx = EXAMPLE_TX_MSIX_ENTRY_IDX;
    bdrConfig.txBdrConfig[0].enIntr       = true;

    /* Wait PHY link up. */
    PRINTF("Wait for PHY link up...\r\n");
    do
    {
        result = APP_PHY_GetLinkStatus(index, &link);
    } while ((result != kStatus_Success) || (!link));
    result = APP_PHY_GetLinkModeSpeedDuplex(index, &phyMode, &phySpeed, &phyDuplex);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Endpoint configuration. */
    (void)EP_GetDefaultConfig(&ep_config);
    ep_config.si                    = g_siIndex[index];
    ep_config.siConfig.txRingUse    = 1;
    ep_config.siConfig.rxRingUse    = 1;
    ep_config.reclaimCallback       = APP_ReclaimCallback;
    ep_config.msixEntry             = &msixEntry[0];
    ep_config.entryNum              = 2;
    ep_config.port.ethMac.miiMode   = phyMode;
    ep_config.port.ethMac.miiSpeed  = phySpeed;
    ep_config.port.ethMac.miiDuplex = phyDuplex;
#ifdef EXAMPLE_ENABLE_CACHE_MAINTAIN
    ep_config.rxCacheMaintain = true;
    ep_config.txCacheMaintain = true;
#endif
    result = EP_Init(&g_ep_handle, &g_macAddr[0], &ep_config, &bdrConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Unmask MSIX message interrupt. */
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_TX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_RX_MSIX_ENTRY_IDX, false);

    APP_BuildBroadCastFrame();
    txFrameNum = 0;
    rxFrameNum = 0;

    while (txFrameNum < EXAMPLE_EP_TXFRAME_NUM)
    {
        txOver = false;
        result = EP_SendFrame(&g_ep_handle, 0, &txFrame, NULL, NULL);
        if (result != kStatus_Success)
        {
            PRINTF("\r\nTransmit frame failed!\r\n");
            return result;
        }
        while (!txOver)
        {
        }
        EP_ReclaimTxDescriptor(&g_ep_handle, 0);
        if (txFrameInfo.status != kNETC_EPTxSuccess)
        {
            PRINTF("\r\nTransmit frame failed!\r\n");
            return kStatus_Fail;
        }
        txFrameNum++;
        PRINTF("The %d frame transmitted success!\r\n", txFrameNum);

        do
        {
            result = EP_GetRxFrameSize(&g_ep_handle, 0, &length);
        } while (result == kStatus_NETC_RxFrameEmpty);
        if (result != kStatus_Success)
        {
            return result;
        }

        result = EP_ReceiveFrameCopy(&g_ep_handle, 0, g_rxFrame, length, NULL);
        if (result != kStatus_Success)
        {
            return result;
        }
        rxFrameNum++;
        PRINTF(" A frame received. The length is %d ", length);
        PRINTF(" Dest Address %02x:%02x:%02x:%02x:%02x:%02x Src Address %02x:%02x:%02x:%02x:%02x:%02x \r\n",
               g_rxFrame[0], g_rxFrame[1], g_rxFrame[2], g_rxFrame[3], g_rxFrame[4], g_rxFrame[5], g_rxFrame[6],
               g_rxFrame[7], g_rxFrame[8], g_rxFrame[9], g_rxFrame[10], g_rxFrame[11]);

        if (memcmp(g_rxFrame, g_txFrame, sizeof(g_txFrame)))
        {
            PRINTF("\r\nTx/Rx frames don't match!\r\n");
            return kStatus_Fail;
        }
    }

    EP_Deinit(&g_ep_handle);

    return result;
}

#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
status_t APP_SWT_XferLoopBack(void)
{
    status_t result                  = kStatus_Success;
    netc_rx_bdr_config_t rxBdrConfig = {0};
    netc_tx_bdr_config_t txBdrConfig = {0};
    netc_bdr_config_t bdrConfig      = {.rxBdrConfig = &rxBdrConfig, .txBdrConfig = &txBdrConfig};
    netc_buffer_struct_t txBuff      = {.buffer = &g_txFrame, .length = sizeof(g_txFrame)};
    netc_frame_struct_t txFrame      = {.buffArray = &txBuff, .length = 1};
    swt_mgmt_tx_arg_t txArg          = {0};
    bool link                        = false;
    netc_msix_entry_t msixEntry[2];
    netc_hw_mii_mode_t phyMode;
    netc_hw_mii_speed_t phySpeed;
    netc_hw_mii_duplex_t phyDuplex;
    ep_config_t g_ep_config;
    uint32_t entryID;
    uint32_t msgAddr;
    uint32_t length;
    uint32_t i;

    PRINTF("\r\nNETC Switch frame loopback example start.\r\n");

    /* MSIX and interrupt configuration. */
    MSGINTR_Init(EXAMPLE_MSGINTR, &msgintrCallback);
    msgAddr              = MSGINTR_GetIntrSelectAddr(EXAMPLE_MSGINTR, 0);
    msixEntry[0].control = kNETC_MsixIntrMaskBit;
    msixEntry[0].msgAddr = msgAddr;
    msixEntry[0].msgData = EXAMPLE_TX_INTR_MSG_DATA;
    msixEntry[1].control = kNETC_MsixIntrMaskBit;
    msixEntry[1].msgAddr = msgAddr;
    msixEntry[1].msgData = EXAMPLE_RX_INTR_MSG_DATA;

    bdrConfig.rxBdrConfig[0].bdArray       = &g_rxBuffDescrip[0][0];
    bdrConfig.rxBdrConfig[0].len           = EXAMPLE_EP_RXBD_NUM;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].buffAddrArray = &rxBuffAddrArray[0][0];
    bdrConfig.rxBdrConfig[0].buffSize      = EXAMPLE_EP_RXBUFF_SIZE_ALIGN;
    bdrConfig.rxBdrConfig[0].msixEntryIdx  = EXAMPLE_RX_MSIX_ENTRY_IDX;
    bdrConfig.rxBdrConfig[0].enThresIntr   = true;
    bdrConfig.rxBdrConfig[0].enCoalIntr    = true;
    bdrConfig.rxBdrConfig[0].intrThreshold = 1;

    (void)EP_GetDefaultConfig(&g_ep_config);
    g_ep_config.si                 = kNETC_ENETC1PSI0;
    g_ep_config.siConfig.txRingUse = 1;
    g_ep_config.siConfig.rxRingUse = 1;
    g_ep_config.reclaimCallback    = APP_ReclaimCallback;
    g_ep_config.msixEntry          = &msixEntry[0];
    g_ep_config.entryNum           = 2;
#ifdef EXAMPLE_ENABLE_CACHE_MAINTAIN
    g_ep_config.rxCacheMaintain = true;
    g_ep_config.txCacheMaintain = true;
#endif
    result = EP_Init(&g_ep_handle, &g_macAddr[0], &g_ep_config, &bdrConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    SWT_GetDefaultConfig(&g_swt_config);

    /* Wait PHY link up. */
    PRINTF("Wait for PHY link up...\r\n");
    for (i = 0; i < EXAMPLE_SWT_MAX_PORT_NUM; i++)
    {
        /* Only check the enabled port. */
        if (((1U << i) & EXAMPLE_SWT_USED_PORT_BITMAP) == 0U)
        {
            continue;
        }

        do
        {
            result = APP_PHY_GetLinkStatus(EXAMPLE_SWT_PORT0 + i, &link);
        } while ((result != kStatus_Success) || (!link));
        result = APP_PHY_GetLinkModeSpeedDuplex(EXAMPLE_SWT_PORT0 + i, &phyMode, &phySpeed, &phyDuplex);
        if (result != kStatus_Success)
        {
            return result;
        }
        g_swt_config.ports[i].ethMac.miiMode   = phyMode;
        g_swt_config.ports[i].ethMac.miiSpeed  = phySpeed;
        g_swt_config.ports[i].ethMac.miiDuplex = phyDuplex;
    }

    g_swt_config.bridgeCfg.dVFCfg.portMembership = 0x1FU;
    g_swt_config.bridgeCfg.dVFCfg.enUseFilterID = true;
    g_swt_config.bridgeCfg.dVFCfg.filterID = EXAMPLE_FRAME_FID;
    g_swt_config.bridgeCfg.dVFCfg.mfo = kNETC_FDBLookUpWithFlood;
    g_swt_config.bridgeCfg.dVFCfg.mlo = kNETC_DisableMACLearn;

    g_swt_config.cmdRingUse            = 1U;
    g_swt_config.cmdBdrCfg[0].bdBase   = &g_cmdBuffDescrip[0];
    g_swt_config.cmdBdrCfg[0].bdLength = 8U;

    result = SWT_Init(&g_swt_handle, &g_swt_config);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Configure switch transfer resource. */
    swtTxRxConfig.enUseMgmtRxBdRing            = false;
    swtTxRxConfig.enUseMgmtTxBdRing            = true;
    swtTxRxConfig.mgmtTxBdrConfig.bdArray      = &g_mgmtTxBuffDescrip[0];
    swtTxRxConfig.mgmtTxBdrConfig.len          = EXAMPLE_EP_TXBD_NUM;
    swtTxRxConfig.mgmtTxBdrConfig.dirtyArray   = &g_mgmtTxDirty[0];
    swtTxRxConfig.mgmtTxBdrConfig.msixEntryIdx = EXAMPLE_TX_MSIX_ENTRY_IDX;
    swtTxRxConfig.mgmtTxBdrConfig.enIntr       = true;
    swtTxRxConfig.reclaimCallback              = APP_SwtReclaimCallback;
#ifdef EXAMPLE_ENABLE_CACHE_MAINTAIN
    swtTxRxConfig.rxCacheMaintain = true;
    swtTxRxConfig.txCacheMaintain = true;
#endif
    result = SWT_ManagementTxRxConfig(&g_swt_handle, &g_ep_handle, &swtTxRxConfig);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Unmask MSIX message interrupt. */
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_TX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_RX_MSIX_ENTRY_IDX, false);

    APP_BuildBroadCastFrame();
    rxFrameNum = 0;
    txFrameNum = 0;

    /* Set FDB table, input frame only forwards to pseudo MAC port. */
    netc_tb_fdb_config_t fdbEntryCfg = {.keye.fid = EXAMPLE_FRAME_FID, .cfge.portBitmap = 0x10U, .cfge.dynamic = 1};
    memset(&fdbEntryCfg.keye.macAddr[0], 0xFF, 6U);
    result = SWT_BridgeAddFDBTableEntry(&g_swt_handle, &fdbEntryCfg, &entryID);
    if ((kStatus_Success != result) || (0U == entryID))
    {
        return kStatus_Fail;
    }

    i = 0;
    while (txFrameNum < EXAMPLE_EP_TXFRAME_NUM)
    {
        /* Only use the enabled port. */
        if (((1U << i) & EXAMPLE_SWT_USED_PORT_BITMAP) == 0U)
        {
            i++;
            i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
            continue;
        }

        txOver     = false;
        txArg.ring = 0;
        result     = SWT_SendFrame(&g_swt_handle, txArg, (netc_hw_port_idx_t)(kNETC_SWITCH0Port0 + i), false, &txFrame, NULL, NULL);
        if (result != kStatus_Success)
        {
            PRINTF("\r\nTransmit frame failed!\r\n");
            return result;
        }
        while (!txOver)
        {
        }
        SWT_ReclaimTxDescriptor(&g_swt_handle, false, 0);
        if (mgmtTxFrameInfo.status != kNETC_EPTxSuccess)
        {
            PRINTF("\r\nTransmit frame has error!\r\n");
            return kStatus_Fail;
        }
        txFrameNum++;
        PRINTF("The %u frame transmitted success on port %u!\r\n", txFrameNum, (uint8_t)(kNETC_SWITCH0Port0 + i));

        do
        {
            result = EP_GetRxFrameSize(&g_ep_handle, 0, &length);
        } while (result == kStatus_NETC_RxFrameEmpty);
        if (result != kStatus_Success)
        {
            return result;
        }

        result = EP_ReceiveFrameCopy(&g_ep_handle, 0, g_rxFrame, length, NULL);
        if (result != kStatus_Success)
        {
            return result;
        }
        rxFrameNum++;
        PRINTF(" A frame received. The length is %d ", length);
        PRINTF(" Dest Address %02x:%02x:%02x:%02x:%02x:%02x Src Address %02x:%02x:%02x:%02x:%02x:%02x \r\n",
               g_rxFrame[0], g_rxFrame[1], g_rxFrame[2], g_rxFrame[3], g_rxFrame[4], g_rxFrame[5], g_rxFrame[6],
               g_rxFrame[7], g_rxFrame[8], g_rxFrame[9], g_rxFrame[10], g_rxFrame[11]);

        if (memcmp(g_rxFrame, g_txFrame, sizeof(g_txFrame)))
        {
            PRINTF("\r\nTx/Rx frames don't match!\r\n");
            return kStatus_Fail;
        }

        i++;
        i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
    }

    return result;
}
#endif

int main(void)
{
    status_t result = kStatus_Success;
    uint32_t index;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    if (((1U << 2) & EXAMPLE_SWT_USED_PORT_BITMAP) != 0U)
    {
        BOARD_InitSwtPort2Pins();
    }
    if (((1U << 3) & EXAMPLE_SWT_USED_PORT_BITMAP) != 0U)
    {
        BOARD_InitSwtPort3Pins();
    }
    BOARD_NETC_Init();

    result = APP_MDIO_Init();
    if (result != kStatus_Success)
    {
        PRINTF("\r\nMDIO Init failed!\r\n");
        return result;
    }

    result = APP_PHY_Init();
    if (result != kStatus_Success)
    {
        PRINTF("\r\nPHY Init failed!\r\n");
        return result;
    }

    for (uint8_t ring = 0U; ring < EXAMPLE_EP_RING_NUM; ring++)
    {
        for (uint8_t index = 0U; index < EXAMPLE_EP_RXBD_NUM; index++)
        {
            rxBuffAddrArray[ring][index] = (uint64_t)(uintptr_t)&g_rxDataBuff[ring][index];
        }
    }

    for (index = 0; index < EXAMPLE_EP_NUM; index++)
    {
        APP_EP_XferLoopBack(index);
    }
#if !(defined(FSL_FEATURE_NETC_HAS_NO_SWITCH) && FSL_FEATURE_NETC_HAS_NO_SWITCH)
    APP_SWT_XferLoopBack();
#endif

    while (1)
    {
    }
}
