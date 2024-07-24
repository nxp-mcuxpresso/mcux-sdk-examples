/*
 * Copyright 2020-2023 NXP
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
#define ENET_QOS_RXBD_NUM                     (4)
#define ENET_QOS_TXBD_NUM                     (4)
#define ENET_QOS_RXQUEUE_USE                  (3)
#define ENET_QOS_TXQUEUE_USE                  (3)
#define ENET_QOS_RXBUFF_SIZE                  (ENET_QOS_FRAME_MAX_FRAMELEN)
#define ENET_QOS_TXBUFF_SIZE                  (ENET_QOS_FRAME_MAX_FRAMELEN)
#define ENET_QOS_BuffSizeAlign(n)             ENET_QOS_ALIGN(n, ENET_QOS_BUFF_ALIGNMENT)
#define ENET_QOS_ALIGN(x, align)              ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define ENET_QOS_DATA_LENGTH                  (1000)
#define ENET_QOS_HEAD_LENGTH                  (14)
#define ENET_QOS_FRAME_LENGTH                 ENET_QOS_DATA_LENGTH + ENET_QOS_HEAD_LENGTH
#define ENET_QOS_TRANSMIT_DATA_NUM            (30)
#define ENET_QOS_ALIGN(x, align)              ((unsigned int)((x) + ((align)-1)) & (unsigned int)(~(unsigned int)((align)-1)))
#define ENET_QOS_HEAD_TYPE_OFFSET             12U     /*!< ENET head type offset. */
#define ENET_QOS_VLANTYPE                     0x8100U /*! @brief VLAN TYPE */
#define ENET_QOS_VLANTAGLEN                   4U      /*! @brief VLAN TAG length */
#define ENET_QOS_AVBTYPE                      0x22F0U /*! @brief AVB TYPE */
#define ENET_QOS_AVTPDU_IEC61883              0U /*! @brief AVTPDU formats to use the IEC 61883 protocol as subtype. */
#define ENET_QOS_AVTPDU_IEC61883_SUBTYPE_MASK 0x7FU /*! @brief AVTPDU formats to use the IEC 61883 subtype mask. */
#define ENET_QOS_AVTPDU_IEC61883_SPH_OFFSET   26U   /*! @brief AVTPDU IEC61883 format SPH feild byte-offset. */
#define ENET_QOS_AVTPDU_IEC61883_SPH_MASK     0x04U /*! @brief AVTPDU IEC61883 format SPH BIT MASK in the byte. */
#define ENET_QOS_AVTPDU_IEC61883_FMT_OFFSET   28U   /*! @brief AVTPDU IEC61883 format FMT feild byte-offset. */
#define ENET_QOS_AVTPDU_IEC61883_FMT_MASK     0x3FU /*! @brief AVTPDU IEC61883 format FMT BIT MASK in the byte. */
#define ENET_QOS_AVTPDU_IEC61883_6AUDIOTYPE   0x10U /*! @brief AVTPDU IEC61883-6 audio&music type. */
#define ENET_QOS_AVTPDU_IEC61883_8DVDTYPE     0x00U /*! @brief AVTPDU IEC61883-8 DV type. */
#define ENET_QOS_HTONS(n)                     __REV16(n)
#ifndef APP_ENET_QOS_BUFF_ALIGNMENT
#define APP_ENET_QOS_BUFF_ALIGNMENT ENET_QOS_BUFF_ALIGNMENT
#endif

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
        0xd3, 0xbe, 0xd9, 0x45, 0x22, 0x60 \
    }
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_rtl8211f_resource_t g_phy_resource;
AT_NONCACHEABLE_SECTION_ALIGN(enet_qos_rx_bd_struct_t g_rxBuffDescrip[ENET_QOS_RXQUEUE_USE][ENET_QOS_RXBD_NUM],
                              ENET_QOS_BUFF_ALIGNMENT);
AT_NONCACHEABLE_SECTION_ALIGN(enet_qos_tx_bd_struct_t g_txBuffDescrip[ENET_QOS_TXQUEUE_USE][ENET_QOS_TXBD_NUM],
                              ENET_QOS_BUFF_ALIGNMENT);
/*! @brief The data buffers can be in cacheable region or in non-cacheable region.
 * If use cacheable region, the alignment size should be the maximum size of "CACHE LINE SIZE" and
 * "ENET_QOS_BUFF_ALIGNMENT" If use non-cache region, the alignment size is the "ENET_QOS_BUFF_ALIGNMENT".
 */

enet_qos_frame_info_t g_txDirty[ENET_QOS_TXQUEUE_USE][ENET_QOS_RXBD_NUM];
uint32_t rxbuffer[ENET_QOS_RXQUEUE_USE][ENET_QOS_RXBD_NUM];
static enet_qos_handle_t g_handle = {0};
static phy_handle_t phyHandle;

/* The MAC address for ENET device. */
uint8_t g_macAddr[6]  = MAC_ADDRESS;
uint8_t g_macAddr2[6] = MAC_ADDRESS2;
uint8_t g_frame[ENET_QOS_TXQUEUE_USE][ENET_QOS_FRAME_LENGTH];
volatile uint32_t g_rxIndex  = 0;
volatile uint32_t g_rxIndex1 = 0;
volatile uint32_t g_rxIndex2 = 0;
uint32_t g_txIndex           = 0;
uint32_t g_txIndex1          = 0;
uint32_t g_txIndex2          = 0;
uint32_t g_txSuccessFlag     = false;
uint32_t g_rxSuccessFlag     = false;
uint32_t g_txMessageOut      = false;

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

/*! @brief Build Frame for transmit. */
static void ENET_QOS_BuildFrame(void)
{
    uint32_t count = 0;
    uint8_t index  = 0;

    /* memset */
    for (index = 0; index < ENET_QOS_TXQUEUE_USE; index++)
    {
        memset(&g_frame[index], 0, ENET_QOS_FRAME_LENGTH);
    }

    /* Create the mac addresses. */
    for (index = 0; index < 2U; index++)
    {
        for (count = 0; count < 6U; count++)
        {
            g_frame[index][count] = 0xFFU;
        }
        memcpy(&g_frame[index][6], &g_macAddr[0], 6U);
    }

    /* Create the different type frame from the ethernet type offset:
     * first: for normal frame
     * second: for Class A data flag specififc AVB frame with VLAN tag.
     */
    /* First frame - broadcast frame. */
    g_frame[0][ENET_QOS_HEAD_TYPE_OFFSET] = (ENET_QOS_DATA_LENGTH >> 8) & 0xFFU;
    g_frame[0][13]                        = ENET_QOS_DATA_LENGTH & 0xFFU;
    g_frame[0][16]                        = 0x33;
    g_frame[0][17]                        = 0x55;
    g_frame[0][18]                        = 0xDD;
    g_frame[0][19]                        = 0xEE;
    for (count = ENET_QOS_HEAD_LENGTH + 6U; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[0][count] = count % 0xFFU;
    }

    /* Second frame. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET] = ENET_QOS_HTONS(ENET_QOS_VLANTYPE); /* VLAN TAG type. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN / 2] =
        ENET_QOS_HTONS((5 << 13U)); /* Prio 5. */
    *(uint16_t *)&g_frame[1][ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN] =
        ENET_QOS_HTONS(ENET_QOS_AVBTYPE); /* AVTP type. */
    uint32_t offset         = ENET_QOS_HEAD_TYPE_OFFSET + ENET_QOS_VLANTAGLEN + 2;
    g_frame[1][offset]      = 0; /* AVTPDU set subtype with IEC61883 in common header. */
    g_frame[1][26 + offset] = 0; /* AVTPDU set SPH field. */
    g_frame[1][28 + offset] = (1 << 7) | ENET_QOS_AVTPDU_IEC61883_6AUDIOTYPE;
    for (count = 50; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[1][count] = count % 0xFFU;
    }

    /* Third frame - unicast frame. */
    memcpy(&g_frame[2][0], &g_macAddr2[0], 6U);
    memcpy(&g_frame[2][6], &g_macAddr[0], 6U);
    g_frame[2][ENET_QOS_HEAD_TYPE_OFFSET] = (ENET_QOS_DATA_LENGTH >> 8) & 0xFFU;
    g_frame[2][13]                        = ENET_QOS_DATA_LENGTH & 0xFFU;
    for (count = ENET_QOS_HEAD_LENGTH; count < ENET_QOS_FRAME_LENGTH; count++)
    {
        g_frame[2][count] = count % 0xFFU;
    }

    /* make sure the tx frames are written to memory before DMA bursts*/
    DCACHE_CleanByRange((uint32_t)g_frame[0], ENET_QOS_FRAME_LENGTH);
    DCACHE_CleanByRange((uint32_t)g_frame[1], ENET_QOS_FRAME_LENGTH);
    DCACHE_CleanByRange((uint32_t)g_frame[2], ENET_QOS_FRAME_LENGTH);
}

void ENET_QOS_IntCallback(
    ENET_QOS_Type *base, enet_qos_handle_t *handle, enet_qos_event_t event, uint8_t channel, void *userData)
{
    status_t status;
    uint32_t length = 0U;

    switch (event)
    {
        case kENET_QOS_RxIntEvent:
            /* Get the Frame size */
            do
            {
                status = ENET_QOS_GetRxFrameSize(base, &g_handle, &length, channel);
                if ((status == kStatus_Success) && (length != 0))
                {
                    /* Received valid frame. Deliver the rx buffer with the size equal to length. */
                    uint8_t *data = (uint8_t *)malloc(length);
                    status        = ENET_QOS_ReadFrame(base, &g_handle, data, length, channel, NULL);
                    if (status == kStatus_Success)
                    {
                        if (channel == 0)
                        {
                            g_rxIndex++;
                        }
                        else if (channel == 1)
                        {
                            g_rxIndex1++;
                        }
                        else if (channel == 2)
                        {
                            g_rxIndex2++;
                        }
                    }
                    free(data);
                }
                else if (status == kStatus_ENET_QOS_RxFrameError)
                {
                    /* update the receive buffer. */
                    ENET_QOS_ReadFrame(base, &g_handle, NULL, 0, channel, NULL);
                }
            } while (length != 0U);

            /* Set rx success flag. */
            if ((g_rxIndex + g_rxIndex1 + g_rxIndex2) == ENET_QOS_TRANSMIT_DATA_NUM)
            {
                g_rxSuccessFlag = true;
            }
            break;
        case kENET_QOS_TxIntEvent:
            switch (channel)
            {
                case 0:
                    g_txIndex++;
                    break;
                case 1:
                    g_txIndex1++;
                    break;
                case 2:
                    g_txIndex2++;
                    break;
                default:
                    break;
            }
            if ((g_txIndex + g_txIndex1 + g_txIndex2) == ENET_QOS_TRANSMIT_DATA_NUM)
            {
                g_txSuccessFlag = true;
            }
            break;
        default:
            break;
    }
}

int main(void)
{
    enet_qos_ptp_config_t ptpConfig = {0};
    phy_config_t phyConfig          = {0};
    bool link                       = false;
#if !defined(EXAMPLE_PHY_LOOPBACK_ENABLE)
    bool autonego                   = false;
#endif
    uint32_t ringId                 = 0;
    uint32_t testTxNum              = 0;
    uint32_t count                  = 0;
    enet_qos_config_t config;
    phy_speed_t speed;
    phy_duplex_t duplex;
    status_t status;
    uint8_t *buff;

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

    for (uint8_t ringId = 0; ringId < ENET_QOS_RXQUEUE_USE; ringId++)
    {
        for (uint8_t index = 0; index < ENET_QOS_RXBD_NUM; index++)
        {
            /* This is for rx buffers, static alloc and dynamic alloc both ok. use as your wish, rx buffer
            malloc should align with cache line to avoid side impact during buffer invalidate.. */
            buff = SDK_Malloc(ENET_QOS_RXBUFF_SIZE, EXAMPLE_CACHE_LINE_SIZE);

            /* Clean the alloc buffer to avoid there's any dirty data. */
            DCACHE_CleanByRange((uint32_t)buff, ENET_QOS_RXBUFF_SIZE);

            if (buff)
            {
                rxbuffer[ringId][index] = (uint32_t)buff;
            }
            else
            {
                PRINTF("Mem Alloc fail\r\n");
            }
        }
    }
    /* prepare the buffer configuration. */
    enet_qos_buffer_config_t buffConfig[ENET_QOS_TXQUEUE_USE] = {{
                                                                     ENET_QOS_RXBD_NUM,
                                                                     ENET_QOS_TXBD_NUM,
                                                                     &g_txBuffDescrip[0][0],
                                                                     &g_txBuffDescrip[0][0],
                                                                     &g_txDirty[0][0],
                                                                     &g_rxBuffDescrip[0][0],
                                                                     &g_rxBuffDescrip[0][ENET_QOS_RXBD_NUM],
                                                                     &rxbuffer[0][0],
                                                                     ENET_QOS_BuffSizeAlign(ENET_QOS_RXBUFF_SIZE),
                                                                     true,

                                                                 },
                                                                 {
                                                                     ENET_QOS_RXBD_NUM,
                                                                     ENET_QOS_TXBD_NUM,
                                                                     &g_txBuffDescrip[1][0],
                                                                     &g_txBuffDescrip[1][0],
                                                                     &g_txDirty[1][0],
                                                                     &g_rxBuffDescrip[1][0],
                                                                     &g_rxBuffDescrip[1][ENET_QOS_RXBD_NUM],
                                                                     &rxbuffer[1][0],
                                                                     ENET_QOS_BuffSizeAlign(ENET_QOS_RXBUFF_SIZE),
                                                                     true,
                                                                 },
                                                                 {
                                                                     ENET_QOS_RXBD_NUM,
                                                                     ENET_QOS_TXBD_NUM,
                                                                     &g_txBuffDescrip[2][0],
                                                                     &g_txBuffDescrip[2][0],
                                                                     &g_txDirty[2][0],
                                                                     &g_rxBuffDescrip[2][0],
                                                                     &g_rxBuffDescrip[2][ENET_QOS_RXBD_NUM],
                                                                     &rxbuffer[2][0],
                                                                     ENET_QOS_BuffSizeAlign(ENET_QOS_RXBUFF_SIZE),
                                                                     true,
                                                                 }};

    PRINTF("\r\nENET multi-ring txrx example start.\r\n");

    phyConfig.phyAddr  = EXAMPLE_PHY_ADDR;
    phyConfig.ops      = EXAMPLE_PHY_OPS;
    phyConfig.resource = EXAMPLE_PHY_RESOURCE;
    phyConfig.autoNeg  = true;

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

    /* Get default configuration 1000M RGMII. */
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
    /* Shoule enable the promiscuous mode and enable the store and forward
     * to make the timestamp is always updated correclty in the descriptors. */
    config.specialControl = kENET_QOS_PromiscuousEnable | kENET_QOS_StoreAndForward;
    config.csrClock_Hz    = CORE_CLK_FREQ;

    ptpConfig.tsRollover         = kENET_QOS_DigitalRollover;
    ptpConfig.systemTimeClock_Hz = ENET_PTP_REF_CLK;
    config.ptpConfig             = &ptpConfig;

    /* Multi-queue config */
    enet_qos_multiqueue_config_t multiQueue = {
        .burstLen   = kENET_QOS_BurstLen1,
        .txQueueUse = ENET_QOS_TXQUEUE_USE,
        .mtltxSche  = kENET_QOS_txWeightRR,
        .txQueueConfig =
            {
                {
                    .mode      = kENET_QOS_DCB_Mode,
                    .weight    = 0x10U,
                    .priority  = 0x0U,
                    .cbsConfig = NULL,
                },
                {
                    .mode      = kENET_QOS_DCB_Mode,
                    .weight    = 0x10U,
                    .priority  = 0x1U,
                    .cbsConfig = NULL,
                },
                {
                    .mode      = kENET_QOS_DCB_Mode,
                    .weight    = 0x10U,
                    .priority  = 0x2U,
                    .cbsConfig = NULL,
                },
            },
        .rxQueueUse = ENET_QOS_RXQUEUE_USE,
        .mtlrxSche  = kENET_QOS_rxStrPrio,
        .rxQueueConfig =
            {
                {
                    .mode        = kENET_QOS_DCB_Mode,
                    .mapChannel  = 0x0U,
                    .priority    = 0x0U,
                    .packetRoute = kENET_QOS_PacketNoQ,
                },
                {
                    .mode        = kENET_QOS_AVB_Mode,
                    .mapChannel  = 0x1U,
                    .priority    = 0x1U,
                    .packetRoute = kENET_QOS_PacketNoQ,
                },
                {
                    .mode        = kENET_QOS_DCB_Mode,
                    .mapChannel  = 0x2U,
                    .priority    = 0x2U,
                    .packetRoute = kENET_QOS_PacketNoQ,
                },
            },
    };
    config.multiqueueCfg = &multiQueue;

    ENET_QOS_Init(EXAMPLE_ENET_QOS_BASE, &config, &g_macAddr[0], 1, ENET_PTP_REF_CLK);

    enet_qos_rxp_config_t rxpConfig[3] = {
        {
            .matchData    = 0x45D9BED3U, /* match DA at frame offset 0 bytes in g_frame[2] */
            .matchEnable  = 0xFFFFFFFFU,
            .acceptFrame  = 1,
            .rejectFrame  = 0,
            .inverseMatch = 0,
            .nextControl  = 0,
            .reserved     = 0,
            .frameOffset  = 0x0U,
            .okIndex      = 0U,
            .dmaChannel   = kENET_QOS_Rxp_DMAChn2, /* Channel 2*/
            .reserved2    = 0,
        },
        {
            .matchData    = 0x00A00081U, /* match frame pattern at offset 12 bytes in g_frame[1] */
            .matchEnable  = 0xFFFFFFFFU,
            .acceptFrame  = 1,
            .rejectFrame  = 0,
            .inverseMatch = 0,
            .nextControl  = 0,
            .reserved     = 0,
            .frameOffset  = 0x3U,
            .okIndex      = 0U,
            .dmaChannel   = kENET_QOS_Rxp_DMAChn1, /* Channel 1*/
            .reserved2    = 0,
        },
        {
            .matchData    = 0xEEDD5533U, /* match frame pattern at offset 16 bytes in g_frame[0] */
            .matchEnable  = 0xFFFFFFFFU,
            .acceptFrame  = 1,
            .rejectFrame  = 0,
            .inverseMatch = 0,
            .nextControl  = 0,
            .reserved     = 0,
            .frameOffset  = 0x4U,
            .okIndex      = 0U,
            .dmaChannel   = kENET_QOS_Rxp_DMAChn0, /* Channel 0*/
            .reserved2    = 0,
        }};

    /* Configure rx parser. */
    ENET_QOS_ConfigureRxParser(EXAMPLE_ENET_QOS_BASE, &rxpConfig[0], 3);

    /* Enable the rx interrupt. */
    ENET_QOS_EnableInterrupts(EXAMPLE_ENET_QOS_BASE, kENET_QOS_DmaRx);

    /* Initialize Descriptor. */
    ENET_QOS_DescriptorInit(EXAMPLE_ENET_QOS_BASE, &config, &buffConfig[0]);

    /* Create the handler. */
    ENET_QOS_CreateHandler(EXAMPLE_ENET_QOS_BASE, &g_handle, &config, &buffConfig[0], ENET_QOS_IntCallback, NULL);

    /* Active TX/RX. */
    ENET_QOS_StartRxTx(EXAMPLE_ENET_QOS_BASE, multiQueue.txQueueUse, multiQueue.rxQueueUse);

    /* Build broadcast for sending and active for receiving. */
    ENET_QOS_BuildFrame();

    /* Delay some time before executing send and receive operation. */
    SDK_DelayAtLeastUs(1000000ULL, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    PRINTF("\r\n%d frames will be sent in %d queues, and frames will be received in %d queues.\r\n",
           ENET_QOS_TRANSMIT_DATA_NUM, ENET_QOS_TXQUEUE_USE, ENET_QOS_RXQUEUE_USE);

    /* Start with Ring 2 because ring(queue) 2 has higher priority on tx DMA channel.
    tx Ring N uses DMA channel N and channel N has higher priority than N-1. */
    ringId = 2;
    while (1)
    {
        if (testTxNum < ENET_QOS_TRANSMIT_DATA_NUM)
        {
            /* Send a multicast frame when the PHY is link up. */
            PHY_GetLinkStatus(&phyHandle, &link);
            if (link)
            {
                testTxNum++;
                while (kStatus_Success != ENET_QOS_SendFrame(EXAMPLE_ENET_QOS_BASE, &g_handle, &g_frame[ringId][0],
                                                             ENET_QOS_FRAME_LENGTH, ringId, false, NULL, kENET_QOS_TxOffloadDisable))
                {
                }
                /* Wait for Rx over every three frames to prevent Rx FIFO overflow when Rx interrupt handling is slow. */
                if (ringId == 0U)
                {
                    while (g_rxIndex != (testTxNum / 3U));
                }

                ringId = (ringId + 2) % 3;
            }
        }
        if (g_txSuccessFlag)
        {
            PRINTF("The frame transmitted from the ring 0, 1, 2 is %d, %d, %d!\r\n", g_txIndex, g_txIndex1, g_txIndex2);
            PRINTF("%d frames transmitted succeed!\r\n", ENET_QOS_TRANSMIT_DATA_NUM);
            g_txSuccessFlag = false;
            g_txMessageOut  = true;
        }
        if (g_txMessageOut && g_rxSuccessFlag)
        {
            PRINTF("The frames successfully received from the ring 0, 1, 2 is %d, %d, %d!\r\n", g_rxIndex, g_rxIndex1,
                   g_rxIndex2);
            g_rxSuccessFlag = false;
            g_txMessageOut  = false;
        }
    }
}
