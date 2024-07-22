/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include "fsl_debug_console.h"
#include "fsl_enet_qos.h"
#include "fsl_phy.h"
#include "fsl_silicon_id.h"
#include "fsl_cache.h"

#include "fsl_phyrtl8211f.h"
#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
extern phy_rtl8211f_resource_t g_phy_resource;
#define EXAMPLE_ENET_QOS_BASE      ENET_QOS
#define EXAMPLE_PHY_ADDR           (0x01U)
#define EXAMPLE_PHY_OPS            &phyrtl8211f_ops
#define EXAMPLE_PHY_RESOURCE       &g_phy_resource
#define ENET_QOS_SYSTEM_CLOCK_ROOT kCLOCK_Root_WakeupAxi
#define CORE_CLK_FREQ              CLOCK_GetIpFreq(ENET_QOS_SYSTEM_CLOCK_ROOT)
#define ENET_QOS_PTP_CLOCK_ROOT    kCLOCK_Root_EnetTimer2
#define ENET_PTP_REF_CLK           CLOCK_GetIpFreq(ENET_QOS_PTP_CLOCK_ROOT)
#define ENET_QOS_CLOCK_ROOT        kCLOCK_Root_Enet
#define ENET_QOS_CLOCK_GATE        kCLOCK_Enet_Qos
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

#if defined(FSL_ETH_ENABLE_CACHE_CONTROL)
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
#else                             /* No FSL_ETH_ENABLE_CACHE_CONTROL defined */
#define EXAMPLE_CACHE_LINE_SIZE 1 /*!< No need to align cache line size */
#endif                            /* FSL_ETH_ENABLE_CACHE_CONTROL */

#ifndef PHY_LINKUP_TIMEOUT_COUNT
#define PHY_LINKUP_TIMEOUT_COUNT (800000U)
#endif
#ifndef PHY_AUTONEGO_TIMEOUT_COUNT
#define PHY_AUTONEGO_TIMEOUT_COUNT (800000U)
#endif
#ifndef PHY_STABILITY_DELAY_US
#define PHY_STABILITY_DELAY_US (500000U)
#endif
#ifndef EXAMPLE_PHY_LINK_INTR_SUPPORT
#define EXAMPLE_PHY_LINK_INTR_SUPPORT (0U)
#endif

/* @TEST_ANCHOR */

#ifndef MAC_ADDRESS
#define MAC_ADDRESS                        \
    {                                      \
        0x54, 0x27, 0x8d, 0x00, 0x00, 0x00 \
    }
#else
#define USER_DEFINED_MAC_ADDRESS
#endif

#ifndef MAC_ADDRESS2
#define MAC_ADDRESS2                       \
    {                                      \
        0x01, 0x00, 0x5e, 0x00, 0x01, 0x81 \
    }
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void ENET_QOS_BuildPtpEventFrame(void);

#if (defined(EXAMPLE_PHY_LINK_INTR_SUPPORT) && (EXAMPLE_PHY_LINK_INTR_SUPPORT))
void GPIO_EnableLinkIntr(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_rtl8211f_resource_t g_phy_resource;
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
static phy_handle_t phyHandle;

#if (defined(EXAMPLE_PHY_LINK_INTR_SUPPORT) && (EXAMPLE_PHY_LINK_INTR_SUPPORT))
static bool linkChange = false;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
void ENET_QOS_EnableClock(bool enable)
{
    BLK_CTRL_WAKEUPMIX->GPR =
        (BLK_CTRL_WAKEUPMIX->GPR & (~BLK_CTRL_WAKEUPMIX_GPR_ENABLE_MASK)) | BLK_CTRL_WAKEUPMIX_GPR_ENABLE(enable);
}
void ENET_QOS_SetSYSControl(enet_qos_mii_mode_t miiMode)
{
    BLK_CTRL_WAKEUPMIX->GPR |= BLK_CTRL_WAKEUPMIX_GPR_MODE(miiMode);
}

static void MDIO_Init(void)
{
    /* Set SMI first. */
    CLOCK_EnableClock(s_enetqosClock[ENET_QOS_GetInstance(EXAMPLE_ENET_QOS_BASE)]);
    ENET_QOS_SetSMI(EXAMPLE_ENET_QOS_BASE, CORE_CLK_FREQ);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_QOS_MDIOWrite(EXAMPLE_ENET_QOS_BASE, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_QOS_MDIORead(EXAMPLE_ENET_QOS_BASE, phyAddr, regAddr, pData);
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

#if (defined(EXAMPLE_PHY_LINK_INTR_SUPPORT) && (EXAMPLE_PHY_LINK_INTR_SUPPORT))
void PHY_LinkStatusChange(void)
{
    linkChange = true;
}
#endif

int main(void)
{
    enet_qos_config_t config;
    uint32_t rxbuffer[ENET_QOS_RXBD_NUM];
    uint8_t index;
    void *buff;
    uint32_t refClock; /* PTP REF clock. */
    phy_speed_t speed;
    phy_duplex_t duplex;
    uint32_t length = 0;
    uint8_t *buffer;
    uint32_t count = 0;
    status_t status;
    enet_qos_ptp_config_t ptpConfig = {0};
    uint64_t second;
    uint32_t nanosecond;
    bool link              = false;
#if !defined(EXAMPLE_PHY_LOOPBACK_ENABLE)
    bool autonego                   = false;
#endif
    bool tempLink          = false;
    phy_config_t phyConfig = {0};

    /* Hardware Initialization. */
    /* clang-format off */

    /* enetqosSysClk 250MHz */
    const clock_root_config_t enetqosSysClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_WAKEUPAXI_ClockRoot_MuxSysPll1Pfd0, // 1000MHz
	.div = 4
    };

    /* enetqosPtpClk 100MHz */
    const clock_root_config_t enetqosPtpClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_ENETTSTMR2_ClockRoot_MuxSysPll1Pfd1Div2, // 400MHz
	.div = 4
    };

    /* enetqosClk 250MHz (For 125MHz TX_CLK ) */
    const clock_root_config_t enetqosClkCfg = {
        .clockOff = false,
	.mux = kCLOCK_ENET_ClockRoot_MuxSysPll1Pfd0Div2, // 500MHz
	.div = 2
    };

    const clock_root_config_t lpi2cClkCfg = {
        .clockOff = false,
	.mux = 0, // 24MHz oscillator source
	.div = 1
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(ENET_QOS_SYSTEM_CLOCK_ROOT, &enetqosSysClkCfg);
    CLOCK_SetRootClock(ENET_QOS_PTP_CLOCK_ROOT, &enetqosPtpClkCfg);
    CLOCK_SetRootClock(ENET_QOS_CLOCK_ROOT, &enetqosClkCfg);
    CLOCK_EnableClock(ENET_QOS_CLOCK_GATE);
    CLOCK_SetRootClock(BOARD_PCAL6524_I2C_CLOCK_ROOT, &lpi2cClkCfg);
    CLOCK_EnableClock(BOARD_PCAL6524_I2C_CLOCK_GATE);

    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    pcal6524_handle_t handle;
    BOARD_InitPCAL6524(&handle);
    PCAL6524_SetDirection(&handle, (1 << BOARD_PCAL6524_ENET1_NRST), kPCAL6524_Output);
    PCAL6524_ClearPins(&handle, (1 << BOARD_PCAL6524_ENET1_NRST));
    SDK_DelayAtLeastUs(10000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    PCAL6524_SetPins(&handle, (1 << BOARD_PCAL6524_ENET1_NRST));
    SDK_DelayAtLeastUs(30000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

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

    phyConfig.phyAddr  = EXAMPLE_PHY_ADDR;
    phyConfig.autoNeg  = true;
    phyConfig.ops      = EXAMPLE_PHY_OPS;
    phyConfig.resource = EXAMPLE_PHY_RESOURCE;
#if (defined(EXAMPLE_PHY_LINK_INTR_SUPPORT) && (EXAMPLE_PHY_LINK_INTR_SUPPORT))
    phyConfig.intrType = kPHY_IntrActiveLow;
#endif

    PRINTF("Wait for PHY init...\r\n");
#if defined(EXAMPLE_PHY_LOOPBACK_ENABLE)
    /* Initialize PHY and enable loopback. */
    do
    {
        status = PHY_Init(&phyHandle, &phyConfig);
        if (status != kStatus_Success)
        {
            PRINTF("Failed to init PHY\r\n");
            return status;
        }

        status = PHY_EnableLoopback(&phyHandle, kPHY_LocalLoop, kPHY_Speed1000M, true);
        if (status != kStatus_Success)
        {
            PRINTF("Failed to enable PHY loopback\r\n");
            return status;
        }

        /* Wait link up */
        PRINTF("Wait for PHY link up...\r\n");
        count = PHY_LINKUP_TIMEOUT_COUNT;
        do
        {
            PHY_GetLinkStatus(&phyHandle, &link);
            if (link)
            {
                break;
            }
        } while (--count);
    } while (!link);
#else
    /* Initialize PHY and wait auto-negotiation over. */
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
#endif

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

#ifndef USER_DEFINED_MAC_ADDRESS
    /* Set special address for each chip. */
    SILICONID_ConvertToMacAddr(&g_macAddr);
#endif

    /* Initialize ENET. */
    /* Shoule enable the multicast receive and enable the store and forward
     * to make the timestamp is always updated correclty in the descriptors. */
    config.specialControl        = kENET_QOS_HashMulticastEnable | kENET_QOS_StoreAndForward;
    config.csrClock_Hz           = CORE_CLK_FREQ;
    ptpConfig.tsRollover         = kENET_QOS_DigitalRollover;
    refClock                     = ENET_PTP_REF_CLK;
    ptpConfig.systemTimeClock_Hz = refClock;
    config.ptpConfig             = &ptpConfig;
    ENET_QOS_Init(EXAMPLE_ENET_QOS_BASE, &config, &g_macAddr[0], 1, refClock);

    /* Initialize Descriptor. */
    ENET_QOS_DescriptorInit(EXAMPLE_ENET_QOS_BASE, &config, &buffConfig);

    /* Create the handler. */
    ENET_QOS_CreateHandler(EXAMPLE_ENET_QOS_BASE, &g_handle, &config, &buffConfig, ENET_QOS_IntCallback, NULL);

    /* Add to multicast group to receive ptp multicast frame. */
    ENET_QOS_AddMulticastGroup(EXAMPLE_ENET_QOS_BASE, &g_multicastAddr[0]);

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
        /* PHY link status update. */
#if (defined(EXAMPLE_PHY_LINK_INTR_SUPPORT) && (EXAMPLE_PHY_LINK_INTR_SUPPORT))
        if (linkChange)
        {
            linkChange = false;
            PHY_ClearInterrupt(&phyHandle);
            PHY_GetLinkStatus(&phyHandle, &link);
            GPIO_EnableLinkIntr();
        }
#else
        PHY_GetLinkStatus(&phyHandle, &link);
#endif
        if (tempLink != link)
        {
            PRINTF("PHY link changed, link status = %u\r\n", link);
            tempLink = link;
        }

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
                                                              ENET_QOS_EXAMPLE_FRAME_SIZE, 0, true, buffer, kENET_QOS_TxOffloadDisable))
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
        }
    }
}
