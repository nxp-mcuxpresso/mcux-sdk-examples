/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_dmic.h"
#include "fsl_dma.h"
#include "fsl_dmic_dma.h"
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_audio_config.h"
#include "usb_device_descriptor.h"
#include "fsl_device_registers.h"
#include "audio_data_dmic.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FIFO_DEPTH  (15U)
#define BUFFER_SIZE (982U)
#define BUFFER_NUM  (2U)

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE \
    (FS_ISO_IN_ENDP_PACKET_SIZE > HS_ISO_IN_ENDP_PACKET_SIZE ? FS_ISO_IN_ENDP_PACKET_SIZE : HS_ISO_IN_ENDP_PACKET_SIZE)
#endif

#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE (FS_ISO_IN_ENDP_PACKET_SIZE)
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define AUDIO_ENDPOINT_MAX_PACKET_SIZE (FS_ISO_IN_ENDP_PACKET_SIZE)
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
dmic_dma_handle_t g_dmicDmaHandle;
dma_handle_t g_dmicRxDmaHandle;
volatile unsigned int first_int = 0;
dma_handle_t g_DMA_Handle; /*!< The DMA RX Handles. */

/*! @brief Static table of descriptors */
#if defined(__ICCARM__)
#pragma data_alignment              = 16U
dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__CC_ARM) || (defined(__ARMCC_VERSION))
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__GNUC__)
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#endif

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t s_wavBuff[AUDIO_ENDPOINT_MAX_PACKET_SIZE];
uint32_t audioPosition = 0U;

USB_DMA_NONINIT_DATA_ALIGN(4) static uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUM];
static dmic_dma_handle_t s_dmicDmaHandle;
static dma_handle_t s_dmicRxDmaHandle;
SDK_ALIGN(dma_descriptor_t s_dmaDescriptorPingpong[2], 16);

static dmic_transfer_t s_receiveXfer[2U] = {
    /* transfer configurations for channel0 */
    {
        .data                   = s_buffer,
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave1xWidth,
        .linkTransfer           = &s_receiveXfer[1],
    },

    {
        .data                   = &s_buffer[BUFFER_SIZE],
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave1xWidth,
        .linkTransfer           = &s_receiveXfer[0],
    },
};
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Audio wav data prepare function.
 *
 * This function prepare audio wav data before send.
 */
void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size)
{
    uint8_t k;
    /* copy audio wav data from flash to buffer */
    for (k = 0U; k < size; k++)
    {
        if (audioPosition > (BUFFER_SIZE * BUFFER_NUM - 1U))
        {
            audioPosition = 0U;
        }
        *(buffer + k) = s_buffer[audioPosition];
        audioPosition++;
    }
}

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    if (first_int == 0U)
    {
        audioPosition = 0U;
        first_int     = 1U;
    }
}

void Board_DMIC_DMA_Init(void)
{
    dmic_channel_config_t dmic_channel_cfg;

    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_DMIC_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&s_dmicRxDmaHandle, DEMO_DMA, DEMO_DMIC_RX_CHANNEL);

    memset(&dmic_channel_cfg, 0U, sizeof(dmic_channel_config_t));

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = DEMO_DMIC_OSR;
    dmic_channel_cfg.gainshft            = 2U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
#if defined(FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND) && (FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND)
    dmic_channel_cfg.enableSignExtend = true;
#endif
    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmDual);
#endif
    DMIC_Use2fs(DMIC0, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL, true);
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL, kDMIC_Left, &dmic_channel_cfg);

    /* FIFO disabled */
    DMIC_FifoChannel(DMIC0, DEMO_DMIC_CHANNEL, FIFO_DEPTH, true, true);
    DMIC_EnableChannnel(DMIC0, DEMO_DMIC_CHANNEL_ENABLE);

    DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle, dmic_Callback, NULL, &s_dmicRxDmaHandle);
    DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle, s_dmaDescriptorPingpong, 2U);
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle, s_receiveXfer, DEMO_DMIC_CHANNEL);
}
