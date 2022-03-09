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
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_enet.h"
#include "fsl_enet_cmsis.h"
#include "fsl_enet_phy_cmsis.h"
#include "fsl_phy.h"
#include "stdlib.h"

#include "fsl_enet_mdio.h"
#include "fsl_phyrtl8211f.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "RTE_Device.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_ENET     Driver_ETH_MAC0
#define EXAMPLE_ENET_PHY Driver_ETH_PHY0

/* MDIO operations. */
#define EXAMPLE_MDIO_OPS enet_ops
/* PHY operations. */
#define EXAMPLE_PHY_OPS phyrtl8211_ops
#define ENET_DATA_LENGTH        (1000)
#define ENET_EXAMPLE_LOOP_COUNT (20U)

/* @TEST_ANCHOR*/

#ifndef MAC_ADDRESS
#define MAC_ADDRESS {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x61}
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_frame[ENET_DATA_LENGTH + 14];
volatile uint32_t g_testTxNum  = 0;
uint8_t g_macAddr[6]           = MAC_ADDRESS;
volatile uint32_t g_rxIndex    = 0;
volatile uint32_t g_rxCheckIdx = 0;
volatile uint32_t g_txCheckIdx = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
mdio_handle_t mdioHandle = {.resource = {.base = ENET_1G}, .ops = &EXAMPLE_MDIO_OPS};
phy_handle_t phyHandle   = {.phyAddr = RTE_ENET_PHY_ADDRESS, .mdioHandle = &mdioHandle, .ops = &phyrtl8211f_ops};

uint32_t ENET0_GetFreq(void)
{
    return CLOCK_GetRootClockFreq(kCLOCK_Root_Bus);
}

void BOARD_InitModuleClock(void)
{
    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);

    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
    mdioHandle.resource.csrClock_Hz = ENET0_GetFreq();
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
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    /* Hardware Initialization. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);

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
