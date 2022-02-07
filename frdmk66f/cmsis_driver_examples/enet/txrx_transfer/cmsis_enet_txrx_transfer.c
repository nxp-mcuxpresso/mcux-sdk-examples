/*
 * Copyright 2017-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "Driver_ETH_MAC.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_enet.h"
#include "fsl_enet_cmsis.h"
#include "fsl_enet_phy_cmsis.h"
#include "fsl_phy.h"
#include "stdlib.h"

#include "fsl_common.h"
#include "fsl_sysmpu.h"
#include "fsl_phyksz8081.h"
#include "fsl_enet_mdio.h"
#include "RTE_Device.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* ENET base address */
#define EXAMPLE_ENET     Driver_ETH_MAC0
#define EXAMPLE_ENET_PHY Driver_ETH_PHY0
#define ENET_DATA_LENGTH        (1000)
#define ENET_EXAMPLE_LOOP_COUNT (20U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_frame[ENET_DATA_LENGTH + 14];
volatile uint32_t g_testTxNum  = 0;
uint8_t g_macAddr[6]           = {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x61};
volatile uint32_t g_rxIndex    = 0;
volatile uint32_t g_rxCheckIdx = 0;
volatile uint32_t g_txCheckIdx = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
mdio_handle_t mdioHandle = {.ops = &enet_ops};
phy_handle_t phyHandle   = {.phyAddr = RTE_ENET_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &phyksz8081_ops};

uint32_t ENET0_GetFreq(void)
{
    return CLOCK_GetFreq(kCLOCK_CoreSysClk);
}

void ENET_SignalEvent_t(uint32_t event)
{
    if (event == ARM_ETH_MAC_EVENT_RX_FRAME)
    {
        uint32_t size;
        uint32_t len;

        /* Get the Frame size */
        size = EXAMPLE_ENET.GetRxFrameSize();
        /* Call ENET_ReadFrame when there is a received frame. */
        if (size != 0)
        {
            /* Received valid frame. Deliver the rx buffer with the size equal to length. */
            uint8_t *data = (uint8_t *)malloc(size);
            if (data)
            {
                len = EXAMPLE_ENET.ReadFrame(data, size);
                if (size == len)
                {
                    /* Increase the received frame numbers. */
                    if (g_rxIndex < ENET_EXAMPLE_LOOP_COUNT)
                    {
                        g_rxIndex++;
                    }
                }
                free(data);
            }
        }
    }
    if (event == ARM_ETH_MAC_EVENT_TX_FRAME)
    {
        g_testTxNum++;
    }
}

/*! @brief Build Frame for transmit. */
static void ENET_BuildBroadCastFrame(void)
{
    uint32_t count  = 0;
    uint32_t length = ENET_DATA_LENGTH - 14;

    for (count = 0; count < 6U; count++)
    {
        g_frame[count] = 0xFFU;
    }
    memcpy(&g_frame[6], &g_macAddr[0], 6U);
    g_frame[12] = (length >> 8) & 0xFFU;
    g_frame[13] = length & 0xFFU;

    for (count = 0; count < length; count++)
    {
        g_frame[count + 14] = count % 0xFFU;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t txnumber = 0;
    ARM_ETH_LINK_INFO linkInfo;

    /* Hardware Initialization. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Disable SYSMPU. */
    SYSMPU_Enable(SYSMPU, false);
    /* Set RMII clock src. */
    CLOCK_SetRmii0Clock(1U);

    mdioHandle.resource.base        = ENET;
    mdioHandle.resource.csrClock_Hz = ENET0_GetFreq();

    PRINTF("\r\nENET example start.\r\n");

    /* Initialize the ENET module. */
    EXAMPLE_ENET.Initialize(ENET_SignalEvent_t);
    EXAMPLE_ENET.PowerControl(ARM_POWER_FULL);
    EXAMPLE_ENET.SetMacAddress((ARM_ETH_MAC_ADDR *)g_macAddr);

    PRINTF("Wait for PHY init...\r\n");
    while (EXAMPLE_ENET_PHY.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK)
    {
        PRINTF("PHY Auto-negotiation failed, please check the cable connection and link partner setting.\r\n");
    }

    EXAMPLE_ENET.Control(ARM_ETH_MAC_CONTROL_RX, 1);
    EXAMPLE_ENET.Control(ARM_ETH_MAC_CONTROL_TX, 1);
    PRINTF("Wait for PHY link up...\r\n");
    do
    {
        if (EXAMPLE_ENET_PHY.GetLinkState() == ARM_ETH_LINK_UP)
        {
            linkInfo = EXAMPLE_ENET_PHY.GetLinkInfo();
            EXAMPLE_ENET.Control(ARM_ETH_MAC_CONFIGURE, linkInfo.speed << ARM_ETH_MAC_SPEED_Pos |
                                                            linkInfo.duplex << ARM_ETH_MAC_DUPLEX_Pos |
                                                            ARM_ETH_MAC_ADDRESS_BROADCAST);
            break;
        }
    } while (1);

#if defined(PHY_STABILITY_DELAY_US) && PHY_STABILITY_DELAY_US
    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
#endif

    /* Build broadcast for sending. */
    ENET_BuildBroadCastFrame();

    while (1)
    {
        /* Check the total number of received number. */
        if (g_testTxNum && (g_txCheckIdx != g_testTxNum))
        {
            g_txCheckIdx = g_testTxNum;
            PRINTF("The %d frame transmitted success!\r\n", g_txCheckIdx);
        }
        if (g_rxCheckIdx != g_rxIndex)
        {
            g_rxCheckIdx = g_rxIndex;
            PRINTF("A total of %d frame(s) has been successfully received!\r\n", g_rxCheckIdx);
        }
        /* Get the Frame size */
        if (txnumber < ENET_EXAMPLE_LOOP_COUNT)
        {
            txnumber++;
            /* Send a multicast frame when the PHY is link up. */
            if (EXAMPLE_ENET.SendFrame(&g_frame[0], ENET_DATA_LENGTH, ARM_ETH_MAC_TX_FRAME_EVENT) == ARM_DRIVER_OK)
            {
                SDK_DelayAtLeastUs(1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            }
            else
            {
                PRINTF(" \r\nTransmit frame failed!\r\n");
            }
        }
    }
}
