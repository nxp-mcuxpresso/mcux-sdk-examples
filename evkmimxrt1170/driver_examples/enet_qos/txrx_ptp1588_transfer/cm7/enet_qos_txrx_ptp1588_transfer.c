/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include "fsl_debug_console.h"
#include "fsl_enet_qos.h"
#include "fsl_phy.h"

#include "pin_mux.h"
#include "board.h"
#include "fsl_enet_qos_mdio.h"
#include "fsl_phyrtl8211f.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_ENET_QOS_BASE ENET_QOS
#define EXAMPLE_PHY_ADDR      0x01U
#define CORE_CLK_FREQ         CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)
#define ENET_PTP_REF_CLK      50000000UL
#define ENET_QOS_RXBD_NUM               (4)
#define ENET_QOS_TXBD_NUM               (4)
#define ENET_QOS_RXBUFF_SIZE            (ENET_QOS_FRAME_MAX_FRAMELEN)
#define ENET_QOS_BuffSizeAlign(n)       ENET_QOS_ALIGN(n, ENET_QOS_BUFF_ALIGNMENT)
#define ENET_QOS_ALIGN(x, align)        ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define ENET_QOS_EXAMPLE_FRAME_HEADSIZE (14U)
#define ENET_QOS_EXAMPLE_DATA_LENGTH    (1000U)
#define ENET_QOS_EXAMPLE_FRAME_SIZE     (ENET_QOS_EXAMPLE_DATA_LENGTH + ENET_QOS_EXAMPLE_FRAME_HEADSIZE)
#define ENET_QOS_EXAMPLE_PACKAGETYPE    (4U)
#define ENET_QOS_EXAMPLE_SEND_COUNT     (20U)
#define ENET_QOS_PTP_SYNC_MSG           (0x00U)

#ifndef FSL_FEATURE_L2CACHE_LINESIZE_BYTE
#define FSL_FEATURE_L2CACHE_LINESIZE_BYTE 0
#endif
#ifndef FSL_FEATURE_L1DCACHE_LINESIZE_BYTE
#define FSL_FEATURE_L1DCACHE_LINESIZE_BYTE 0
#endif

#if (FSL_FEATURE_L2CACHE_LINESIZE_BYTE > FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#define EXAMPLE_CACHE_LINE_SIZE FSL_FEATURE_L2CACHE_LINESIZE_BYTE
#else
#define EXAMPLE_CACHE_LINE_SIZE FSL_FEATURE_L1DCACHE_LINESIZE_BYTE
#endif

#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (800000U)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif

/* @TEST_ANCHOR */

#ifndef MAC_ADDRESS
#define MAC_ADDRESS {0xd4, 0xbe, 0xd9, 0x45, 0x22, 0x60}
#endif

#ifndef MAC_ADDRESS2
#define MAC_ADDRESS2 {0x01, 0x00, 0x5e, 0x00, 0x01, 0x81}
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void ENET_QOS_BuildPtpEventFrame(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(enet_qos_rx_bd_struct_t g_rxBuffDescrip[ENET_QOS_RXBD_NUM], ENET_QOS_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_qos_tx_bd_struct_t g_txBuffDescrip[ENET_QOS_TXBD_NUM], ENET_QOS_BUFF_ALIGNMENT);

enet_qos_frame_info_t g_txDirty[ENET_QOS_RXBD_NUM];

enet_qos_handle_t g_handle = {0};
/* The MAC address for ENET device. */
uint8_t g_macAddr[6]       = MAC_ADDRESS;
uint8_t g_multicastAddr[6] = MAC_ADDRESS2;
uint8_t g_frame[ENET_QOS_EXAMPLE_PACKAGETYPE][ENET_QOS_EXAMPLE_FRAME_SIZE];
uint8_t *g_txbuff[ENET_QOS_TXBD_NUM];
uint32_t g_txIdx     = 0;
uint8_t g_txbuffIdx  = 0;
uint8_t g_txCosumIdx = 0;
uint32_t g_testIdx   = 0;

extern phy_handle_t phyHandle;
/*******************************************************************************
 * Code
 ******************************************************************************/
mdio_handle_t mdioHandle = {.resource = {.base = ENET_QOS}, .ops = &enet_qos_ops};
phy_handle_t phyHandle   = {.phyAddr = EXAMPLE_PHY_ADDR, .mdioHandle = &mdioHandle, .ops = &phyrtl8211f_ops};

void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Qos, &rootCfg);
    rootCfg.div = 10;
    CLOCK_SetRootClock(kCLOCK_Root_Enet_Timer3, &rootCfg); /* Generate 50M PTP REF clock. */

    mdioHandle.resource.csrClock_Hz = CORE_CLK_FREQ;
}

void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
    IOMUXC_GPR->GPR6 |= (miiMode << 3U);
    IOMUXC_GPR->GPR6 |= IOMUXC_GPR_GPR6_ENET_QOS_CLKGEN_EN_MASK; /* Set this bit to enable ENET_QOS clock generation. */
}

static void ENET_QOS_BuildPtpEventFrame(void)
{
    uint8_t index;
    for (index = 0; index < ENET_QOS_EXAMPLE_PACKAGETYPE; index++)
    {
        /* Build for PTP event message frame. */
        memcpy(&g_frame[index][0], &g_multicastAddr[0], 6);
        /* The six-byte source MAC address. */
        memcpy(&g_frame[index][6], &g_macAddr[0], 6);
        /* The type/length: if data length is used make sure it's smaller than 1500 */
        g_frame[index][12]    = 0x08U;
        g_frame[index][13]    = 0x00U;
        g_frame[index][0x0EU] = 0x40;
        g_frame[index][0x24U] = (kENET_QOS_PtpEventPort >> 8) & 0xFFU;
        g_frame[index][0x25U] = kENET_QOS_PtpEventPort & 0xFFU;
        g_frame[index][0x17U] = 0x11U;
        /* Add ptp event message type: sync message. */
        g_frame[index][0x2AU - 12U] = ENET_QOS_PTP_SYNC_MSG;
        /* Add sequence id. */
        g_frame[index][0x48U - 12U] = 0;
        g_frame[index][0x48U - 11U] = 0;

        g_frame[index][ENET_QOS_EXAMPLE_FRAME_SIZE - 1] = index % 0xFFU;
    }
}

void ENET_QOS_IntCallback(
    ENET_QOS_Type *base, enet_qos_handle_t *handle, enet_qos_event_t event, uint8_t channel, void *param)
{
    switch (event)
    {
        case kENET_QOS_TxIntEvent:
            /* Free tx buffers. */
            free(g_txbuff[g_txCosumIdx]);
            g_txCosumIdx = (g_txCosumIdx + 1) % ENET_QOS_TXBD_NUM;
            break;
        default:
            break;
    }
}

int main(void)
{
    enet_qos_config_t config;
    uint32_t rxbuffer[ENET_QOS_RXBD_NUM];
    uint8_t index;
    void *buff;
    uint32_t refClock = ENET_PTP_REF_CLK; /* PTP REF clock. */
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t length = 0;
    uint8_t *buffer;
    uint32_t count = 0;
    status_t status;
    enet_qos_ptp_config_t ptpConfig = {0};
    uint64_t second;
    uint32_t nanosecond;
    bool link     = false;
    bool autonego = false;

    /* Hardware Initialization. */
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    /* Hardware Initialization. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitModuleClock();
    BOARD_InitDebugConsole();

    IOMUXC_GPR->GPR6 |= IOMUXC_GPR_GPR6_ENET_QOS_RGMII_EN_MASK; /* Set this bit to enable ENET_QOS RGMII TX clock output
                                                                   on TX_CLK pad. */

    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);

    for (index = 0; index < ENET_QOS_RXBD_NUM; index++)
    {
        /* This is for rx buffers, static alloc and dynamic alloc both ok. use as your wish.rx buffer
            malloc should align with cache line to avoid side impact during buffer invalidate.. */
        buff = SDK_Malloc(ENET_QOS_RXBUFF_SIZE, EXAMPLE_CACHE_LINE_SIZE);

        /* Clean the alloc buffer to avoid there's any dirty data. */
        DCACHE_CleanByRange((uint32_t)buff, ENET_QOS_RXBUFF_SIZE);

        if (buff)
        {
            rxbuffer[index] = (uint32_t)buff;
        }
        else
        {
            PRINTF("Mem Alloc fail\r\n");
        }
    }

    /* prepare the buffer configuration. */
    enet_qos_buffer_config_t buffConfig = {
        ENET_QOS_RXBD_NUM,
        ENET_QOS_TXBD_NUM,
        &g_txBuffDescrip[0],
        &g_txBuffDescrip[0],
        &g_txDirty[0],
        &g_rxBuffDescrip[0],
        &g_rxBuffDescrip[ENET_QOS_RXBD_NUM],
        &rxbuffer[0],
        ENET_QOS_BuffSizeAlign(ENET_QOS_RXBUFF_SIZE),
        true,
    };

    PRINTF("\r\nENET example start.\r\n");

    phy_config_t phyConfig;
    phyConfig.phyAddr = EXAMPLE_PHY_ADDR;
    phyConfig.autoNeg = true;

    /* Initialize PHY and wait auto-negotiation over. */
    PRINTF("Wait for PHY init...\r\n");
    do
    {
        status = PHY_Init(&phyHandle, &phyConfig);
        if (status == kStatus_Success)
        {
            PRINTF("Wait for PHY link up...\r\n");
            /* Wait for auto-negotiation success and link up */
            count = PHY_AUTONEGO_TIMEOUT_COUNT;
            do
            {
                PHY_GetAutoNegotiationStatus(&phyHandle, &autonego);
                PHY_GetLinkStatus(&phyHandle, &link);
                if (autonego && link)
                {
                    break;
                }
            } while (--count);
            if (!autonego)
            {
                PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
            }
        }
    } while (!(link && autonego));

    /* Wait a moment for PHY status to be stable. */
    SDK_DelayAtLeastUs(PHY_STABILITY_DELAY_US, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    PHY_GetLinkSpeedDuplex(&phyHandle, &speed, &duplex);

    /* Get default configuration 100M RMII. */
    ENET_QOS_GetDefaultConfig(&config);

    /* Use the actual speed and duplex when phy success to finish the autonegotiation. */
    switch (speed)
    {
        case kPHY_Speed10M:
            config.miiSpeed = kENET_QOS_MiiSpeed10M;
            break;
        case kPHY_Speed100M:
            config.miiSpeed = kENET_QOS_MiiSpeed100M;
            break;
        case kPHY_Speed1000M:
            config.miiSpeed = kENET_QOS_MiiSpeed1000M;
            break;
        default:
            break;
    }
    config.miiDuplex = (enet_qos_mii_duplex_t)duplex;

    /* Initialize ENET. */
    /* Shoule enable the multicast receive and enable the store and forward
     * to make the timestamp is always updated correclty in the descriptors. */
    config.specialControl        = kENET_QOS_HashMulticastEnable | kENET_QOS_StoreAndForward;
    config.csrClock_Hz           = CORE_CLK_FREQ;
    ptpConfig.tsRollover         = kENET_QOS_DigitalRollover;
    ptpConfig.systemTimeClock_Hz = refClock;
    config.ptpConfig             = &ptpConfig;
    ENET_QOS_Init(EXAMPLE_ENET_QOS_BASE, &config, &g_macAddr[0], 1, refClock);

    /* Add to multicast group to receive ptp multicast frame. */
    ENET_QOS_AddMulticastGroup(EXAMPLE_ENET_QOS_BASE, &g_multicastAddr[0]);

    /* Initialize Descriptor. */
    ENET_QOS_DescriptorInit(EXAMPLE_ENET_QOS_BASE, &config, &buffConfig);

    /* Create the handler. */
    ENET_QOS_CreateHandler(EXAMPLE_ENET_QOS_BASE, &g_handle, &config, &buffConfig, ENET_QOS_IntCallback, NULL);
    /* Active TX/RX. */
    ENET_QOS_StartRxTx(EXAMPLE_ENET_QOS_BASE, 1, 1);

    /* Build ptp message for sending and active for receiving. */
    ENET_QOS_BuildPtpEventFrame();

    /* Check if the timestamp is running */
    for (index = 1; index <= 10; index++)
    {
        ENET_QOS_Ptp1588GetTimer(EXAMPLE_ENET_QOS_BASE, &second, &nanosecond);
        PRINTF(" Get the %d-th time", index);
        PRINTF(" %d second,", (uint32_t)second);
        PRINTF(" %d nanosecond  \r\n", nanosecond);
        SDK_DelayAtLeastUs(200000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    }

    PRINTF("\r\nTransmission start now!\r\n");

    while (1)
    {
        /* Get the Frame size */
        length = 0;
        status = ENET_QOS_GetRxFrameSize(EXAMPLE_ENET_QOS_BASE, &g_handle, &length, 0);
        /* Call ENET_QOS_ReadFrame when there is a received frame. */
        if ((status == kStatus_Success) && (length != 0))
        {
            /* Received valid frame. Deliver the rx buffer with the size equal to length. */
            uint8_t *data = (uint8_t *)malloc(length);
            enet_qos_ptp_time_t rxFrameTimeStamp;
            if (data)
            {
                status = ENET_QOS_ReadFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, data, length, 0, &rxFrameTimeStamp);
                if (status == kStatus_Success)
                {
                    PRINTF(" One frame received. the length %d \r\n", length);
                    PRINTF(" the timestamp is %d second,", (uint32_t)rxFrameTimeStamp.second);
                    PRINTF(" %d nanosecond  \r\n", rxFrameTimeStamp.nanosecond);
                }

                free(data);
            }
            else
            {
                /* Discard due to the lack of buffers. */
                ENET_QOS_ReadFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, NULL, 0, 0, NULL);
                PRINTF("No availabe memory.\r\n");
            }
        }
        else if (status == kStatus_ENET_QOS_RxFrameError)
        {
            /* update the receive buffer. */
            ENET_QOS_ReadFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, NULL, 0, 0, NULL);
        }

        if (g_testIdx < ENET_QOS_EXAMPLE_SEND_COUNT)
        {
            /* Send a multicast frame when the PHY is link up. */
            PHY_GetLinkStatus(&phyHandle, &link);
            if (link)
            {
                /* Create the frame to be send. */
                buffer = (uint8_t *)malloc(ENET_QOS_EXAMPLE_FRAME_SIZE);
                if (buffer)
                {
                    memcpy(buffer, &g_frame[g_txIdx], ENET_QOS_EXAMPLE_FRAME_SIZE);
                    /* Make each transmit different.*/
                    g_txIdx = (g_txIdx + 1) % ENET_QOS_EXAMPLE_PACKAGETYPE;
                    /* Store the buffer for mem free.*/
                    g_txbuff[g_txbuffIdx] = buffer;
                    g_txbuffIdx           = (g_txbuffIdx + 1) % ENET_QOS_TXBD_NUM;

                    /* Do cache clean operation to ensure buffer is filled with data for transmit. */
                    DCACHE_CleanByRange((uint32_t)buffer, ENET_QOS_EXAMPLE_FRAME_SIZE);

                    if (kStatus_Success == ENET_QOS_SendFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, buffer,
                                                              ENET_QOS_EXAMPLE_FRAME_SIZE, 0, true, buffer))
                    {
                        g_testIdx++;
                        PRINTF("The %d frame transmitted success!\r\n", g_testIdx);
                        enet_qos_frame_info_t sentFrameInfo;
                        ENET_QOS_GetTxFrame(&g_handle, &sentFrameInfo, 0);
                        if ((sentFrameInfo.context == buffer) && (sentFrameInfo.isTsAvail == true))
                        {
                            PRINTF(" the timestamp is %d second,", (uint32_t)sentFrameInfo.timeStamp.second);
                            PRINTF(" %d nanosecond  \r\n", sentFrameInfo.timeStamp.nanosecond);
                        }
                    }
                }
                else
                {
                    PRINTF("No avail tx buffers\r\n");
                }
            }
            else
            {
                PRINTF(" \r\nThe PHY link down!\r\n");
            }
        }
    }
}
