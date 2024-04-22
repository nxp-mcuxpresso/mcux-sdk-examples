/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_netc_endpoint.h"
#include "fsl_netc_switch.h"
#include "fsl_netc_mdio.h"
#include "fsl_msgintr.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcmgr.h"

#include "fsl_rgpio.h"
#include "fsl_phyrtl8211f.h"
volatile bool g_pinSet = false;
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_INIT()                    \
    rgpio_pin_config_t led_config = { \
        kRGPIO_DigitalOutput,         \
        0,                            \
    };                                \
    RGPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &led_config);

// #define LED_TOGGLE() GPIO_PortToggle(BOARD_USER_LED_GPIO, 1u << BOARD_USER_LED_GPIO_PIN);
#define LED_TOGGLE()                                                      \
    if (g_pinSet)                                                         \
    {                                                                     \
        RGPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, 0U); \
        g_pinSet = false;                                                 \
    }                                                                     \
    else                                                                  \
    {                                                                     \
        RGPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, 1U); \
        g_pinSet = true;                                                  \
    }

#define EXAMPLE_EP_RXBD_NUM          8U
#define EXAMPLE_EP_TXBD_NUM          8U
#define EXAMPLE_EP_BD_ALIGN          128U
#define EXAMPLE_EP_BUFF_SIZE_ALIGN   64U
#define EXAMPLE_EP_RXBUFF_SIZE       1518U
#define EXAMPLE_EP_RXBUFF_SIZE_ALIGN SDK_SIZEALIGN(EXAMPLE_EP_RXBUFF_SIZE, EXAMPLE_EP_BUFF_SIZE_ALIGN)
#define EXAMPLE_EP_TEST_FRAME_SIZE   1000U
#define EXAMPLE_TX_INTR_MSG_DATA      0U
#define EXAMPLE_RX_INTR_MSG_DATA      1U
#define EXAMPLE_SI_COM_INTR_MSG_DATA  2U
#define EXAMPLE_TX_MSIX_ENTRY_IDX     0U
#define EXAMPLE_RX_MSIX_ENTRY_IDX     1U
#define EXAMPLE_SI_COM_MSIX_ENTRY_IDX 2U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Buffer descriptor and buffer memeory. */
typedef uint8_t rx_buffer_t[EXAMPLE_EP_RXBUFF_SIZE_ALIGN];

AT_NONCACHEABLE_SECTION_ALIGN(static netc_rx_bd_t g_rxBuffDescrip[EXAMPLE_EP_RXBD_NUM], EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static netc_tx_bd_t g_txBuffDescrip[EXAMPLE_EP_TXBD_NUM], EXAMPLE_EP_BD_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static rx_buffer_t g_rxDataBuff[EXAMPLE_EP_RXBD_NUM], EXAMPLE_EP_BUFF_SIZE_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t g_txFrame[EXAMPLE_EP_TEST_FRAME_SIZE], EXAMPLE_EP_BUFF_SIZE_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t vsiMsgBuff[32], 64);
AT_NONCACHEABLE_SECTION(static uint8_t g_rxFrame[EXAMPLE_EP_RXBUFF_SIZE_ALIGN]);
uint64_t rxBuffAddrArray[EXAMPLE_EP_RXBD_NUM];

static ep_handle_t g_ep_handle;
static netc_tx_frame_info_t g_txDirty[EXAMPLE_EP_TXBD_NUM];
static netc_tx_frame_info_t txFrameInfo = {0};
static volatile bool txOver;
static volatile bool vsiMsgTxOver;
static volatile bool vsiMsgRxReady;

/* MAC address. */
uint8_t g_macAddr[6] = {0x54, 0x27, 0x8d, 0xAA, 0xBB, 0xCC};

/*******************************************************************************
 * Code
 ******************************************************************************/

static void APP_BuildBroadCastFrame(void)
{
    uint32_t count  = 0;
    uint32_t length = EXAMPLE_EP_TEST_FRAME_SIZE - 14U;

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

void msgintrCallback(MSGINTR_Type *base, uint8_t channel, uint32_t pendingIntr)
{
    uint32_t flags;

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
    /* VSI Rx interrupt */
    if ((pendingIntr & (1U << EXAMPLE_SI_COM_INTR_MSG_DATA)) != 0U)
    {
        flags = EP_VsiGetStatus(&g_ep_handle);
        if ((flags & kNETC_VsiMsgRxFlag) != 0U)
        {
            vsiMsgRxReady = true;
        }
        if ((flags & kNETC_VsiMsgTxFlag) != 0U)
        {
            vsiMsgTxOver = true;
        }
        EP_VsiClearStatus(&g_ep_handle, flags);
    }
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t result                  = kStatus_Success;
    netc_rx_bdr_config_t rxBdrConfig = {0};
    netc_tx_bdr_config_t txBdrConfig = {0};
    netc_bdr_config_t bdrConfig      = {.rxBdrConfig = &rxBdrConfig, .txBdrConfig = &txBdrConfig};
    netc_buffer_struct_t txBuff      = {.buffer = &g_txFrame, .length = sizeof(g_txFrame)};
    netc_frame_struct_t txFrame      = {.buffArray = &txBuff, .length = 1};
    netc_msix_entry_t msixEntry[3];
    ep_config_t ep_config;
    uint32_t startupData, i;
    mcmgr_status_t status;
    uint32_t msgAddr;
    uint32_t length;
    uint16_t msg;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    /* Configure LED */
    LED_INIT();

    /* FIXME: Wait for MCMGR fixing that MCMGR_GetStartupData may return success when master doesn't start core. */
    SDK_DelayAtLeastUs(4000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    /* Make a noticable delay after the reset */
    /* Use startup parameter from the master core... */
    for (i = 0; i < startupData; i++)
    {
        SDK_DelayAtLeastUs(1000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }

    for (uint8_t index = 0U; index < EXAMPLE_EP_RXBD_NUM; index++)
    {
        rxBuffAddrArray[index] = (uintptr_t)&g_rxDataBuff[index];
    }

    /* MSIX and interrupt configuration. */
    MSGINTR_Init(MSGINTR2, &msgintrCallback);

    msgAddr              = MSGINTR_GetIntrSelectAddr(MSGINTR2, 0);
    msixEntry[0].control = kNETC_MsixIntrMaskBit;
    msixEntry[0].msgAddr = msgAddr;
    msixEntry[0].msgData = EXAMPLE_TX_INTR_MSG_DATA;
    msixEntry[1].control = kNETC_MsixIntrMaskBit;
    msixEntry[1].msgAddr = msgAddr;
    msixEntry[1].msgData = EXAMPLE_RX_INTR_MSG_DATA;
    msixEntry[2].control = kNETC_MsixIntrMaskBit;
    msixEntry[2].msgAddr = msgAddr;
    msixEntry[2].msgData = EXAMPLE_SI_COM_INTR_MSG_DATA;

    bdrConfig.txBdrConfig[0].msixEntryIdx  = EXAMPLE_TX_MSIX_ENTRY_IDX;
    bdrConfig.txBdrConfig[0].enIntr        = true;
    bdrConfig.rxBdrConfig[0].msixEntryIdx  = EXAMPLE_RX_MSIX_ENTRY_IDX;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].enThresIntr   = true;
    bdrConfig.rxBdrConfig[0].enCoalIntr    = true;
    bdrConfig.rxBdrConfig[0].intrThreshold = 1;

    bdrConfig.rxBdrConfig[0].bdArray       = &g_rxBuffDescrip[0];
    bdrConfig.rxBdrConfig[0].len           = EXAMPLE_EP_RXBD_NUM;
    bdrConfig.rxBdrConfig[0].extendDescEn  = false;
    bdrConfig.rxBdrConfig[0].buffAddrArray = &rxBuffAddrArray[0];
    bdrConfig.rxBdrConfig[0].buffSize      = EXAMPLE_EP_RXBUFF_SIZE_ALIGN;
    bdrConfig.txBdrConfig[0].bdArray       = &g_txBuffDescrip[0];
    bdrConfig.txBdrConfig[0].len           = EXAMPLE_EP_TXBD_NUM;
    bdrConfig.txBdrConfig[0].dirtyArray    = &g_txDirty[0];

    (void)EP_GetDefaultConfig(&ep_config);
    ep_config.si                 = kNETC_ENETC1VSI1;
    ep_config.siConfig.txRingUse = 1;
    ep_config.siConfig.rxRingUse = 1;
    ep_config.reclaimCallback    = APP_ReclaimCallback;
    ep_config.siComEntryIdx      = EXAMPLE_SI_COM_MSIX_ENTRY_IDX;
    ep_config.msixEntry          = &msixEntry[0];
    ep_config.entryNum           = 3;
#ifdef EXAMPLE_ENABLE_CACHE_MAINTAIN
    ep_config.rxCacheMaintain = true;
    ep_config.txCacheMaintain = true;
#endif
    result = EP_Init(&g_ep_handle, &g_macAddr[0], &ep_config, &bdrConfig);
    if (result != kStatus_Success)
    {
        return result;
    }

    APP_BuildBroadCastFrame();

    /* Unmask MSIX message interrupt. */
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_TX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_RX_MSIX_ENTRY_IDX, false);
    EP_MsixSetEntryMask(&g_ep_handle, EXAMPLE_SI_COM_MSIX_ENTRY_IDX, false);

    EP_VsiClearStatus(&g_ep_handle, kNETC_VsiMsgRxFlag | kNETC_VsiMsgTxFlag);
    EP_VsiEnableInterrupt(&g_ep_handle, kNETC_VsiMsgRxFlag | kNETC_VsiMsgTxFlag, true);

    /* Customized message command to indicate VSI ready. */
    vsiMsgBuff[0] = 0;
    vsiMsgBuff[1] = 1;

    /* Send SI message to set PSI MAC address. */
    vsiMsgTxOver = false;
    do
    {
        result = EP_VsiSendMsg(&g_ep_handle, (uintptr_t)&vsiMsgBuff[0], 32);
        assert(result != kStatus_InvalidArgument);
    } while (result != kStatus_Success);
    while (!vsiMsgTxOver)
    {
    }

    /* Customized message command to indicate to set PSI MAC address. */
    vsiMsgBuff[0] = 0;
    vsiMsgBuff[1] = 0;
    vsiMsgBuff[2] = 0;
    vsiMsgBuff[3] = 0;
    vsiMsgBuff[4] = 0x54;
    vsiMsgBuff[5] = 0x27;
    vsiMsgBuff[6] = 0x8D;
    vsiMsgBuff[7] = 0x00;
    vsiMsgBuff[8] = 0x00;
    vsiMsgBuff[9] = 0x00;

    while (1)
    {
        while (!vsiMsgRxReady)
        {
        }
        do
        {
            result = EP_VsiReceiveMsg(&g_ep_handle, &msg);
        } while (result != kStatus_Success);
        vsiMsgRxReady = false;

        txOver = false;
        result = EP_SendFrame(&g_ep_handle, 0, &txFrame, NULL, NULL);
        assert(result == kStatus_Success);
        while (!txOver)
        {
        }

        EP_ReclaimTxDescriptor(&g_ep_handle, 0);
        if (txFrameInfo.status != kNETC_EPTxSuccess)
        {
            while (1)
            {
            }
        }

        do
        {
            result = EP_GetRxFrameSize(&g_ep_handle, 0, &length);
            if ((result != kStatus_NETC_RxFrameEmpty) && (result != kStatus_Success))
            {
                result = EP_ReceiveFrameCopy(&g_ep_handle, 0, g_rxFrame, 1, NULL);
                assert(result == kStatus_Success);
            }
            else if (length != 0U)
            {
                result = EP_ReceiveFrameCopy(&g_ep_handle, 0, g_rxFrame, length, NULL);
                assert(result == kStatus_Success);
            }
            else
            {
                break;
            }
        } while (1);

        /* Toggle LED everytime send one frame. */
        LED_TOGGLE();

        /* Send SI message to set PSI MAC address. */
        vsiMsgBuff[9] = msg;
        vsiMsgTxOver  = false;
        do
        {
            result = EP_VsiSendMsg(&g_ep_handle, (uintptr_t)&vsiMsgBuff[0], 32);
            assert(result != kStatus_InvalidArgument);
        } while (result != kStatus_Success);
        while (!vsiMsgTxOver)
        {
        }
    }
}
