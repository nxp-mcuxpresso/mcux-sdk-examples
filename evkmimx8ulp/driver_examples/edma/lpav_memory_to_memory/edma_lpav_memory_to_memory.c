/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_ad_edma.h"

#include "fsl_upower.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_DMA_CHANNEL       30U
#define EXAMPLE_DMA_CHANNEL_CLOCK kCLOCK_Dma2Ch30
#define EXAMPLE_DMA_BASEADDR      DMA2
#define EXAMPLE_SIM_SEC_BASEADDR  SIM_SEC
#define EXAMPLE_DATA_SRCADDR      ((uint32_t *)0xa8400000)
#define EXAMPLE_DATA_DSTADDR      ((uint32_t *)0xa8400100)
#define BUFF_LENGTH 4U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
edma_handle_t g_EDMA_Handle;
volatile bool g_Transfer_Done = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* User callback function for EDMA transfer. */
void EDMA_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (transferDone)
    {
        g_Transfer_Done = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    edma_transfer_config_t transferConfig;
    edma_config_t userConfig;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

    UPOWER_PowerOnMemPart((uint32_t)kUPOWER_MP0_DMA2, 0U);
    CLOCK_EnableClock(EXAMPLE_DMA_CHANNEL_CLOCK);
    SIM_SEC_Type *base            = EXAMPLE_SIM_SEC_BASEADDR;
    base->LPAV_DMA2_CH_ALLOC_CTRL = 0x0;

    PRINTF("EDMA LPAV memory to memory transfer example begin.\r\n\r\n");
    uint32_t *srcAddr  = EXAMPLE_DATA_SRCADDR;
    uint32_t *destAddr = EXAMPLE_DATA_DSTADDR;
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        srcAddr[i]  = i + 1;
        destAddr[i] = 0x0;
    }

    /* Print destination buffer */
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }

    /* Configure EDMA one shot transfer */
    /*
     * userConfig.enableMasterIdReplication = true;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableDebugMode = false;
     * userConfig.enableBufferedWrites = false;
     */
    EDMA_AD_GetDefaultConfig(&userConfig);
    EDMA_AD_Init(EXAMPLE_DMA_BASEADDR, &userConfig);
    EDMA_AD_CreateHandle(&g_EDMA_Handle, EXAMPLE_DMA_BASEADDR, EXAMPLE_DMA_CHANNEL);
    EDMA_AD_SetCallback(&g_EDMA_Handle, EDMA_Callback, NULL);

    EDMA_AD_PrepareTransfer(&transferConfig, srcAddr, sizeof(srcAddr[0]), destAddr, sizeof(destAddr[0]),
                         sizeof(srcAddr) * 4, sizeof(srcAddr) * 4, kEDMA_MemoryToMemory);
    EDMA_AD_SubmitTransfer(&g_EDMA_Handle, &transferConfig);

    /* Trigger transfer start */
    EDMA_AD_TriggerChannelStart(EXAMPLE_DMA_BASEADDR, EXAMPLE_DMA_CHANNEL);

    /* Wait for EDMA transfer finish */
    while (g_Transfer_Done != true)
    {
    }
    /* Print destination buffer */
    PRINTF("\r\n\r\nEDMA LPAV memory to memory transfer example finish.\r\n\r\n");
    PRINTF("Destination Buffer:\r\n");
    for (i = 0; i < BUFF_LENGTH; i++)
    {
        PRINTF("%d\t", destAddr[i]);
    }
    while (1)
    {
    }
}
