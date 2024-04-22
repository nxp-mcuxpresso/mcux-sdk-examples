/*
 * Copyright 2024 NXP
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
#define EXAMPLE_EP_NUM 1U
#define EXAMPLE_EP_SI    \
    {                    \
        kNETC_ENETC0PSI0 \
    }
#define EXAMPLE_MSGINTR   MSGINTR1
#define EXAMPLE_EP0_PORT  0x00U
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
#define EXAMPLE_EP_BD_ALIGN          128U
#define EXAMPLE_TX_INTR_MSG_DATA     1U
#define EXAMPLE_RX_INTR_MSG_DATA     2U
#define EXAMPLE_EP_TX_INTR_MSG_DATA  3U
#define EXAMPLE_TX_MSIX_ENTRY_IDX    0U
#define EXAMPLE_RX_MSIX_ENTRY_IDX    1U
#define EXAMPLE_EP_TX_MSIX_ENTRY_IDX 2U
#define EXAMPLE_FRAME_FID            1U

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
status_t APP_PHY_EnableLoopback(uint32_t port, bool enable);
/* Rx buffer memeory type. */
typedef uint8_t rx_buffer_t[EXAMPLE_EP_RXBUFF_SIZE_ALIGN];

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* PHY operation. */
static netc_mdio_handle_t s_emdio_handle;
static phy_rtl8211f_resource_t s_phy_resource[5];
static phy_handle_t s_phy_handle[5];
/* EP resource. */
static ep_handle_t g_ep_handle;

/* SWT resource. */
static swt_handle_t g_swt_handle;
static swt_config_t g_swt_config;
static swt_transfer_config_t swtTxRxConfig;

/* Buffer descriptor resource. */
AT_NONCACHEABLE_SECTION_ALIGN(static netc_rx_bd_t g_rxBuffDescrip[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM],
                              EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_tx_bd_t g_txBuffDescrip[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_TXBD_NUM],
                              EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static rx_buffer_t g_rxDataBuff[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM],
                              EXAMPLE_EP_BUFF_SIZE_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t g_txFrame[EXAMPLE_EP_TEST_FRAME_SIZE], EXAMPLE_EP_BUFF_SIZE_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_tx_bd_t g_mgmtTxBuffDescrip[EXAMPLE_EP_TXBD_NUM], EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_cmd_bd_t g_cmdBuffDescrip[EXAMPLE_EP_TXBD_NUM], EXAMPLE_EP_BD_ALIGN);

static uint64_t rxBuffAddrArray[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_RXBD_NUM];
static netc_tx_frame_info_t g_mgmtTxDirty[EXAMPLE_EP_TXBD_NUM];
static netc_tx_frame_info_t mgmtTxFrameInfo;
static netc_tx_frame_info_t g_txDirty[EXAMPLE_EP_RING_NUM][EXAMPLE_EP_TXBD_NUM];
static netc_tx_frame_info_t txFrameInfo;
static volatile bool txOver;

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
    mdioConfig.mdio.type = kNETC_EMdio;
    result               = NETC_MDIOInit(&s_emdio_handle, &mdioConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    return result;
}

static status_t APP_EMDIOWrite(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return NETC_MDIOWrite(&s_emdio_handle, phyAddr, regAddr, data);
}

static status_t APP_EMDIORead(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return NETC_MDIORead(&s_emdio_handle, phyAddr, regAddr, pData);
}

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

    s_phy_resource[port].write = APP_EMDIOWrite;
    s_phy_resource[port].read  = APP_EMDIORead;

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

status_t APP_PHY_EnableLoopback(uint32_t port, bool enable)
{
    phy_speed_t speed = kPHY_Speed100M;

    switch (port)
    {
        case EXAMPLE_EP0_PORT:
        case EXAMPLE_SWT_PORT0:
            /* kPHY_Speed100M */
            break;
        case EXAMPLE_SWT_PORT1:
        case EXAMPLE_SWT_PORT2:
        case EXAMPLE_SWT_PORT3:
            speed = kPHY_Speed1000M;
            break;
        default:
            assert(false);
            break;
    }

    return PHY_EnableLoopback(&s_phy_handle[port], kPHY_LocalLoop, speed, enable);
}

/*! @brief Build Frame for single ring transmit. */
static void APP_BuildBroadCastFrame(uint32_t dstPort, uint32_t srcPort)
{
    uint32_t length = EXAMPLE_EP_TEST_FRAME_SIZE - 14U;
    uint32_t count;

    memcpy(&g_txFrame[0], &g_macAddr[0], 5U);
    g_txFrame[5] = dstPort;
    memcpy(&g_txFrame[6], &g_macAddr[0], 5U);
    g_txFrame[11] = srcPort;
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

static status_t APP_SwtReclaimCallback(swt_handle_t *handle, netc_tx_frame_info_t *frameInfo, void *userData)
{
    mgmtTxFrameInfo = *frameInfo;
    return kStatus_Success;
}

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
    /* EP Transmit interrupt */
    if ((pendingIntr & (1U << EXAMPLE_EP_TX_INTR_MSG_DATA)) != 0U)
    {
        EP_CleanTxIntrFlags(&g_ep_handle, 2, 0);
        txOver = true;
    }
}

status_t APP_SWT_MacLearning_Forwarding(void)
{
    status_t result                  = kStatus_Success;
    netc_rx_bdr_config_t rxBdrConfig = {0};
    netc_tx_bdr_config_t txBdrConfig = {0};
    netc_bdr_config_t bdrConfig      = {.rxBdrConfig = &rxBdrConfig, .txBdrConfig = &txBdrConfig};
    netc_buffer_struct_t txBuff      = {.buffer = &g_txFrame, .length = sizeof(g_txFrame)};
    netc_frame_struct_t txFrame      = {.buffArray = &txBuff, .length = 1};
    swt_mgmt_tx_arg_t txArg          = {0};
    bool link                        = false;
    netc_port_phy_mac_traffic_statistic_t swtTxStatis;
    netc_tb_fdb_search_criteria_t sCriteria;
    netc_tb_fdb_rsp_data_t resp;
    netc_msix_entry_t msixEntry[3];
    netc_hw_mii_mode_t phyMode;
    netc_hw_mii_speed_t phySpeed;
    netc_hw_mii_duplex_t phyDuplex;
    ep_config_t g_ep_config;
    uint32_t msgAddr;
    uint32_t txCount;
    uint32_t i;

    PRINTF("\r\nNETC Switch example start.\r\n");

    for (uint8_t ring = 0U; ring < EXAMPLE_EP_RING_NUM; ring++)
    {
        for (uint8_t index = 0U; index < EXAMPLE_EP_RXBD_NUM; index++)
        {
            rxBuffAddrArray[ring][index] = (uint64_t)(uintptr_t)&g_rxDataBuff[ring][index];
        }
    }

    /* MSIX and interrupt configuration. */
    MSGINTR_Init(EXAMPLE_MSGINTR, &msgintrCallback);
    msgAddr              = MSGINTR_GetIntrSelectAddr(EXAMPLE_MSGINTR, 0);
    msixEntry[0].control = kNETC_MsixIntrMaskBit;
    msixEntry[0].msgAddr = msgAddr;
    msixEntry[0].msgData = EXAMPLE_TX_INTR_MSG_DATA;
    msixEntry[1].control = kNETC_MsixIntrMaskBit;
    msixEntry[1].msgAddr = msgAddr;
    msixEntry[1].msgData = EXAMPLE_RX_INTR_MSG_DATA;
    msixEntry[2].control = kNETC_MsixIntrMaskBit;
    msixEntry[2].msgAddr = msgAddr;
    msixEntry[2].msgData = EXAMPLE_EP_TX_INTR_MSG_DATA;

    bdrConfig.rxBdrConfig[0].bdArray       = &g_rxBuffDescrip[0][0];
    bdrConfig.rxBdrConfig[0].len           = EXAMPLE_EP_RXBD_NUM;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].buffAddrArray = &rxBuffAddrArray[0][0];
    bdrConfig.rxBdrConfig[0].buffSize      = EXAMPLE_EP_RXBUFF_SIZE_ALIGN;
    bdrConfig.rxBdrConfig[0].msixEntryIdx  = EXAMPLE_RX_MSIX_ENTRY_IDX;
    bdrConfig.rxBdrConfig[0].enThresIntr   = true;
    bdrConfig.rxBdrConfig[0].enCoalIntr    = true;
    bdrConfig.rxBdrConfig[0].intrThreshold = 1;

    bdrConfig.txBdrConfig[0].bdArray      = &g_txBuffDescrip[0][0];
    bdrConfig.txBdrConfig[0].len          = EXAMPLE_EP_TXBD_NUM;
    bdrConfig.txBdrConfig[0].dirtyArray   = &g_txDirty[0][0];
    bdrConfig.txBdrConfig[0].msixEntryIdx = EXAMPLE_EP_TX_MSIX_ENTRY_IDX;
    bdrConfig.txBdrConfig[0].enIntr       = true;

    (void)EP_GetDefaultConfig(&g_ep_config);
    g_ep_config.si                 = kNETC_ENETC1PSI0;
    g_ep_config.siConfig.txRingUse = 1;
    g_ep_config.siConfig.rxRingUse = 1;
    g_ep_config.reclaimCallback    = APP_ReclaimCallback;
    g_ep_config.msixEntry          = &msixEntry[0];
    g_ep_config.entryNum           = 3;

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
        g_swt_config.ports[i].bridgeCfg.isRxVlanAware = false;
    }

    g_swt_config.bridgeCfg.dVFCfg.portMembership = 0x1FU;
    g_swt_config.bridgeCfg.dVFCfg.enUseFilterID = true;
    g_swt_config.bridgeCfg.dVFCfg.filterID = EXAMPLE_FRAME_FID;
    g_swt_config.bridgeCfg.dVFCfg.mfo = kNETC_FDBLookUpWithDiscard;
    g_swt_config.bridgeCfg.dVFCfg.mlo = kNETC_HardwareMACLearn;

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

    result = SWT_ManagementTxRxConfig(&g_swt_handle, &g_ep_handle, &swtTxRxConfig);
    if (kStatus_Success != result)
    {
        return result;
    }

    /* Unmask MSIX message interrupt. */
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_TX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_RX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_EP_TX_MSIX_ENTRY_IDX, false);

    PRINTF("\r\nMAC learning.\r\n");
    i = 0;
    while (1)
    {
        /* Only use the enabled port. */
        if (((1U << i) & EXAMPLE_SWT_USED_PORT_BITMAP) == 0U)
        {
            i++;
            i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
            if (i == 0U)
            {
                break;
            }
            continue;
        }

        /* Send the frame on port and loopback, let MAC learning work. */
        APP_BuildBroadCastFrame(0xF0, i);
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
        PRINTF("The frame received from port %u.", i);
        PRINTF(" Dest Address %02x:%02x:%02x:%02x:%02x:%02x Src Address %02x:%02x:%02x:%02x:%02x:%02x.",
               g_txFrame[0], g_txFrame[1], g_txFrame[2], g_txFrame[3], g_txFrame[4], g_txFrame[5], g_txFrame[6],
               g_txFrame[7], g_txFrame[8], g_txFrame[9], g_txFrame[10], g_txFrame[11]);

        memset(&sCriteria, 0, sizeof(sCriteria));
        sCriteria.resumeEntryId   = 0xFFFFFFFF;
        sCriteria.cfgeMc          = kNETC_FDBCfgeMacthPortBitmap;
        sCriteria.cfge.portBitmap = 1U << i;
        if (SWT_BridgeSearchFDBTableEntry(&g_swt_handle, &sCriteria, &resp) != kStatus_Success)
        {
            PRINTF("\r\nGet FDB entry error!\r\n");
            return kStatus_Fail;
        }
        PRINTF(" Port %u bounds to MAC %02x:%02x:%02x:%02x:%02x:%02x.\r\n", i, resp.keye.macAddr[0], 
               resp.keye.macAddr[1], resp.keye.macAddr[2], resp.keye.macAddr[3], resp.keye.macAddr[4], resp.keye.macAddr[5]);

        i++;
        i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
        if (i == 0U)
        {
            break;
        }
    }

    PRINTF("\r\nFrame forwarding.\r\n");
    while (1)
    {
        /* Only use the enabled port. */
        if (((1U << i) & EXAMPLE_SWT_USED_PORT_BITMAP) == 0U)
        {
            i++;
            i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
            if (i == 0U)
            {
                break;
            }
            continue;
        }

        /* Disable the PHY loopback on this port */
        result = APP_PHY_EnableLoopback(i+1U, false);
        if (kStatus_Success != result)
        {
            return result;
        }

        NETC_PortGetPhyMacTxStatistic(g_swt_handle.hw.ports[i].eth, kNETC_ExpressMAC, &swtTxStatis);
        txCount = swtTxStatis.total511To1023BPacket;

        APP_BuildBroadCastFrame(i, 0xF0);
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

        do
        {
            NETC_PortGetPhyMacTxStatistic(g_swt_handle.hw.ports[i].eth, kNETC_ExpressMAC, &swtTxStatis);
        } while (swtTxStatis.total511To1023BPacket != txCount + 1U);

        PRINTF("The frame received from pseudo port. Dest Address %02x:%02x:%02x:%02x:%02x:%02x Src Address %02x:%02x:%02x:%02x:%02x:%02x.",
               g_txFrame[0], g_txFrame[1], g_txFrame[2], g_txFrame[3], g_txFrame[4], g_txFrame[5], g_txFrame[6],
               g_txFrame[7], g_txFrame[8], g_txFrame[9], g_txFrame[10], g_txFrame[11]);
        PRINTF(" Frame forwarding to port %u.\r\n", i);

        i++;
        i = ((i >= EXAMPLE_SWT_MAX_PORT_NUM) ? 0 : i);
        if (i == 0U)
        {
            break;
        }
    }
    
    return result;
}

int main(void)
{
    status_t result = kStatus_Success;

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

    APP_SWT_MacLearning_Forwarding();

    while (1)
    {
    }
}
