/*
 * @brief Main module
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdio.h>
#include <string.h>

#include "pin_mux.h"
#include "board.h"
#include "delay.h"

#include "app_usbd_cfg.h"
#include "audio_usbd.h"
//#include "Power_Tasks.h"
#include "fsl_dmic.h"
#include "fsl_dma.h"
#include "fsl_dmic_dma.h"
#include "fsl_debug_console.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FIFO_DEPTH              8U
#define EXAMPLE_DMA             DMA0
#define DMAREQ_DMIC             DMAREQ_DMIC1
#define DMAREQ_DMIC0            16
#define DMAREQ_DMIC1            17
#define APP_DMIC_CHANNEL        kDMIC_Channel1
#define APP_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH1(1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void BOARD_InitHardware(void);

extern ErrorCode_t usbd_init(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
unsigned short g_data_buffer[BUFFER_LENGTH] = {0};
dmic_dma_handle_t g_dmicDmaHandle;
dma_handle_t g_dmicRxDmaHandle;
volatile unsigned int first_int = 0;
dma_handle_t g_DMA_Handle; /*!< The DMA RX Handles. */
extern volatile uint32_t audioPosition;

/*! @brief Static table of descriptors */
#if defined(__ICCARM__)
#pragma data_alignment              = 16U
dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#elif defined(__GNUC__)
__attribute__((aligned(16U))) dma_descriptor_t g_pingpong_desc[2] = {0};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    if (tcds == kDMA_IntB)
    {
    }
    if (tcds == kDMA_IntA)
    {
    }
    if (first_int == 0U)
    {
        audioPosition = 0U;
        first_int     = 1U;
    }
}
/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
    ErrorCode_t ret = LPC_OK;
    dmic_channel_config_t dmic_channel_cfg;
    dma_transfer_config_t transferConfig;
    // uint32_t pllFreq;

    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /* DMIC uses 12MHz FRO clock */
    CLOCK_AttachClk(kFRO12M_to_DMIC);
    /*12MHz divided by 5 = 2.4MHz PDM clock --> gives 48kHz sample rate */
    /*12MHz divided by 15 = 800 KHz PDM clock --> gives 16kHz sample rate */
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 14, false);
    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));

#if defined(LOWPOWEROPERATION)
    /* Setup up basic power prior to USB setup. USB setup also sets up
     * the audio interfaces and any necessary interface specific pin
     * muxing. */
    Power_Init();
#endif

    /* Initialize USB subsystem */
    ret = usbd_init();
    /* pllFreq = CLOCK_GetPllOutFreq();*/
    /* printf("Audio PLL frequency: %ld.%ldMHz\n", pllFreq / 1000000, (pllFreq - (pllFreq / 1000000) * 1000000) /
     * 1000);*/

    if (ret != LPC_OK)
    {
        return 0;
    }

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = 25U;
    dmic_channel_cfg.gainshft            = 2U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
    DMIC_Init(DMIC0);

    DMIC_ConfigIO(DMIC0, kDMIC_PdmDual);
    DMIC_Use2fs(DMIC0, true);
    DMIC_SetOperationMode(DMIC0, kDMIC_OperationModeDma);
    DMIC_ConfigChannel(DMIC0, APP_DMIC_CHANNEL, kDMIC_Left, &dmic_channel_cfg);

    DMIC_FifoChannel(DMIC0, APP_DMIC_CHANNEL, FIFO_DEPTH, true, true);

    DMIC_EnableChannnel(DMIC0, APP_DMIC_CHANNEL_ENABLE);

    DMA_Init(DMA0);

    DMA_EnableChannel(DMA0, DMAREQ_DMIC);

    /* Request dma channels from DMA manager. */
    DMA_CreateHandle(&g_DMA_Handle, DMA0, DMAREQ_DMIC);

    DMA_SetCallback(&g_DMA_Handle, DMA_Callback, NULL);
    DMA_PrepareTransfer(&transferConfig, (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, g_data_buffer, 2,
                        BUFFER_LENGTH, kDMA_PeripheralToMemory, &g_pingpong_desc[1]);
    DMA_SubmitTransfer(&g_DMA_Handle, &transferConfig);
    transferConfig.xfercfg.intA = false;
    transferConfig.xfercfg.intB = true;
    DMA_CreateDescriptor(&g_pingpong_desc[1], &transferConfig.xfercfg,
                         (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, &g_data_buffer[BUFFER_LENGTH / 2],
                         &g_pingpong_desc[0]);
    transferConfig.xfercfg.intA = true;
    transferConfig.xfercfg.intB = false;
    DMA_CreateDescriptor(&g_pingpong_desc[0], &transferConfig.xfercfg,
                         (void *)&DMIC0->CHANNEL[APP_DMIC_CHANNEL].FIFO_DATA, &g_data_buffer[0], &g_pingpong_desc[1]);
    DMA_StartTransfer(&g_DMA_Handle);
    PRINTF("USB audio dmic demo:\n");
    /*PRINTF("build date: " __DATE__ " build time: " __TIME__ "\r\n");*/
    /*PRINTF("processor frequency: %ldMHz\n", SystemCoreClock / 1000000);*/
    while (1U)
    {
#if defined(LOWPOWEROPERATION)
        Power_Tasks();
#else
        __WFI();
#endif
    }
}
