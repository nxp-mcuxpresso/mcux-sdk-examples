/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i3c_edma.h"

#include "fsl_edma.h"
#include "fsl_trdc.h"
#include "fsl_ele_base_api.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* When CM33 set TRDC, CM7 must NOT require TRDC ownership from ELE */
#define CM33_SET_TRDC 0U

#define ELE_TRDC_AON_ID    0x74
#define ELE_TRDC_WAKEUP_ID 0x78
#define ELE_CORE_CM33_ID   0x1
#define ELE_CORE_CM7_ID    0x2

/*
 * Set ELE_STICK_FAILED_STS to 0 when ELE status check is not required,
 * which is useful when debug reset, where the core has already get the
 * TRDC ownership at first time and ELE is not able to release TRDC
 * ownership again for the following TRDC ownership request.
 */
#define ELE_STICK_FAILED_STS 1

#if ELE_STICK_FAILED_STS
#define ELE_IS_FAILED(x) (x != kStatus_Success)
#else
#define ELE_IS_FAILED(x) false
#endif
#define EXAMPLE_SLAVE                  (I3C2)
#define EXAMPLE_I2C_BAUDRATE           (400000U)
#define EXAMPLE_I3C_OD_BAUDRATE        (1500000U)
#define EXAMPLE_I3C_PP_BAUDRATE        (4000000U)
#define I3C_SLAVE_CLOCK_FREQUENCY      CLOCK_GetRootClockFreq(kCLOCK_Root_I3c2)

#define EXAMPLE_DMA                    DMA4
#define EXAMPLE_I3C_TX_DMA_CHANNEL     (0U)
#define EXAMPLE_I3C_RX_DMA_CHANNEL     (1U)
#define EXAMPLE_I3C_TX_DMA_CHANNEL_MUX (kDma4RequestMuxI3C2ToBusRequest)
#define EXAMPLE_I3C_RX_DMA_CHANNEL_MUX (kDma4RequestMuxI3C2FromBusRequest)

#define I3C_ASYNC_WAKE_UP_INTR_CLEAR                    \
    {                                                   \
        BLK_CTRL_WAKEUPMIX->I3C2_ASYNC_WAKEUP_CTRL = 1; \
    }

#define BOARD_GetEDMAConfig(config)                                              \
    {                                                                            \
        static edma_channel_config_t channelConfig = {                           \
            .enableMasterIDReplication = true,                                   \
            .securityLevel             = kEDMA_ChannelSecurityLevelSecure,       \
            .protectionLevel           = kEDMA_ChannelProtectionLevelPrivileged, \
        };                                                                       \
        config.enableMasterIdReplication = true;                                 \
        config.channelConfig[0]          = &channelConfig;                       \
        config.channelConfig[1]          = &channelConfig;                       \
    }
#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1EU
#endif

#ifndef I3C_DATA_LENGTH
#define I3C_DATA_LENGTH 33U
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
i3c_slave_edma_handle_t g_i3c_s_handle;
edma_handle_t g_tx_dma_handle;
edma_handle_t g_rx_dma_handle;
AT_NONCACHEABLE_SECTION(uint8_t g_slave_rxBuff[I3C_DATA_LENGTH]);
volatile bool g_slaveCompletionFlag  = false;
volatile bool g_slaveRequestSentFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U))
void SEI_EAR_TRDC_EDMA4_ResetPermissions()
{
    uint8_t i, j;
    /* Set the master domain access configuration for eDMA4 */
    trdc_non_processor_domain_assignment_t edma4Assignment;
    (void)memset(&edma4Assignment, 0, sizeof(edma4Assignment));
    edma4Assignment.domainId = 0x7U;
    /* Use the bus master's privileged/user attribute directly */
    edma4Assignment.privilegeAttr = kTRDC_MasterPrivilege;
    /* Use the bus master's secure/nonsecure attribute directly */
    edma4Assignment.secureAttr = kTRDC_MasterSecure;
    /* Use the DID input as the domain indentifier */
    edma4Assignment.bypassDomainId = true;
    edma4Assignment.lock           = false;
    TRDC_SetNonProcessorDomainAssignment(TRDC2, kTRDC2_MasterDMA4, &edma4Assignment);

    /* Enable all access modes for MBC and MRC. */
    trdc_hardware_config_t hwConfig;
    TRDC_GetHardwareConfig(TRDC2, &hwConfig);

    trdc_memory_access_control_config_t memAccessConfig;
    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));

    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }
}

void BOARD_SetDMA4Permission(void)
{
    status_t sts;

    /* Get ELE FW status */
    do
    {
        uint32_t ele_fw_sts;
        sts = ELE_BaseAPI_GetFwStatus(MU_RT_S3MUA, &ele_fw_sts);
    } while (sts != kStatus_Success);

    /* Release TRDC A to CM33 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_AON_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    /* Release TRDC W to CM33 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_WAKEUP_ID, ELE_CORE_CM7_ID);
    } while (ELE_IS_FAILED(sts));

    SEI_EAR_TRDC_EDMA4_ResetPermissions();
}
#endif /* !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U)) */


static void i3c_slave_callback(I3C_Type *base, i3c_slave_edma_transfer_t *xfer, void *userData)
{
    switch ((uint32_t)xfer->event)
    {
        /*  Transfer done */
        case kI3C_SlaveCompletionEvent:
            if (xfer->completionStatus == kStatus_Success)
            {
                g_slaveCompletionFlag = true;
            }
            break;

        case kI3C_SlaveRequestSentEvent:
            g_slaveRequestSentFlag = true;
            break;

#if defined(I3C_ASYNC_WAKE_UP_INTR_CLEAR)
        /*  Handle async wake up interrupt on specific platform. */
        case kI3C_SlaveAddressMatchEvent:
            I3C_ASYNC_WAKE_UP_INTR_CLEAR
            break;
#endif

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t eventMask = kI3C_SlaveCompletionEvent;
#if defined(I3C_ASYNC_WAKE_UP_INTR_CLEAR)
    eventMask |= kI3C_SlaveAddressMatchEvent;
#endif
    i3c_slave_edma_transfer_t slaveXfer;
    i3c_slave_config_t slaveConfig;
    edma_config_t config;
    status_t result;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
#if !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U))
    BOARD_SetDMA4Permission();
#endif /* !(defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U)) */

    PRINTF("\r\nI3C board2board EDMA example -- Slave transfer.\r\n");

    I3C_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    slaveConfig.vendorID   = 0x123U;
    slaveConfig.offline    = false;
    I3C_SlaveInit(EXAMPLE_SLAVE, &slaveConfig, I3C_SLAVE_CLOCK_FREQUENCY);

    PRINTF("\r\nCheck I3C master I3C SDR transfer.\r\n");

    /* Create I3C DMA tx/rx handle. */
    EDMA_GetDefaultConfig(&config);
#if defined(BOARD_GetEDMAConfig)
    BOARD_GetEDMAConfig(config);
#endif
    EDMA_Init(EXAMPLE_DMA, &config);
    EDMA_CreateHandle(&g_tx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&g_rx_dma_handle, EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_TX_DMA_CHANNEL, EXAMPLE_I3C_TX_DMA_CHANNEL_MUX);
    EDMA_SetChannelMux(EXAMPLE_DMA, EXAMPLE_I3C_RX_DMA_CHANNEL, EXAMPLE_I3C_RX_DMA_CHANNEL_MUX);

    /* Create slave handle. */
    I3C_SlaveTransferCreateHandleEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, i3c_slave_callback, NULL, &g_rx_dma_handle,
                                      &g_tx_dma_handle);

    /* Start slave non-blocking transfer. */
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    memset(g_slave_rxBuff, 0, sizeof(g_slave_rxBuff));
    slaveXfer.rxData     = g_slave_rxBuff;
    slaveXfer.rxDataSize = I3C_DATA_LENGTH;
    result = I3C_SlaveTransferEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer, eventMask);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait for master transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("Slave received data :");
    for (uint32_t i = 0U; i < g_slave_rxBuff[0]; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_rxBuff[i + 1]);
    }
    PRINTF("\r\n");

    /* Update slave tx buffer according to the received buffer. */
    memset(&slaveXfer, 0, sizeof(slaveXfer));
    slaveXfer.txData     = (uint8_t *)&g_slave_rxBuff[1];
    slaveXfer.txDataSize = g_slave_rxBuff[0];
    result = I3C_SlaveTransferEDMA(EXAMPLE_SLAVE, &g_i3c_s_handle, &slaveXfer,
                                   (eventMask | kI3C_SlaveRequestSentEvent));
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Notify master that slave tx data is prepared, ibi data is the data size slave want to transmit. */
    uint8_t ibiData = g_slave_rxBuff[0];
    I3C_SlaveRequestIBIWithData(EXAMPLE_SLAVE, &ibiData, 1);
    PRINTF("\r\nI3C slave request IBI event with one mandatory data byte 0x%x.", ibiData);

    while (!g_slaveRequestSentFlag)
    {
    }
    g_slaveRequestSentFlag = false;

    PRINTF("\r\nI3C slave request IBI event sent.\r\n", ibiData);

    /* Wait for slave transmit completed. */
    while (!g_slaveCompletionFlag)
    {
    }
    g_slaveCompletionFlag = false;

    PRINTF("\r\nI3C master I3C SDR transfer finished.\r\n");

    while (1)
    {
    }
}
