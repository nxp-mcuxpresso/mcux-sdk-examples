/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "demo_config.h"
#include "demo_info.h"
#include "image.h"
#include "image_utils.h"
#include "model.h"
#include "output_postproc.h"
#include "timer.h"
#include "fsl_gpc.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "lpm.h"
#include "pin_mux.h"
#include "board.h"
#include "board_init.h"
#include "fsl_mu.h"
#include "fsl_soc_src.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS 0x83000000

/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

#define GPC_CPU_MODE_CTRL GPC_CPU_MODE_CTRL_0

static volatile uint32_t g_msgRecv;
/* Loop number after wakeup, then enter sleep mode */
static volatile int  wakeuptime;


#ifdef CORE1_IMAGE_COPY_TO_RAM

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
//#elif (defined(__GNUC__))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif



/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint32_t get_core1_image_size(void);

uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

void APP_BootCore1(void)
{
    IOMUXC_LPSR_GPR->GPR0 = IOMUXC_LPSR_GPR_GPR0_CM4_INIT_VTOR_LOW(CORE1_BOOT_ADDRESS >> 3);
    IOMUXC_LPSR_GPR->GPR1 = IOMUXC_LPSR_GPR_GPR1_CM4_INIT_VTOR_HIGH(CORE1_BOOT_ADDRESS >> 16);

    /* Read back to make sure write takes effect. */
    (void)IOMUXC_LPSR_GPR->GPR0;
    (void)IOMUXC_LPSR_GPR->GPR1;

    /* If CM4 is already running (released by debugger), then reset it.
       If CM4 is not running, release it. */
    if ((SRC->SCR & SRC_SCR_BT_RELEASE_M4_MASK) == 0)
    {
        SRC_ReleaseCoreReset(SRC, kSRC_CM4Core);
    }
    else
    {
        SRC_AssertSliceSoftwareReset(SRC, kSRC_M4CoreSlice);
    }
}

static void APP_CopyCore1Image(void)
{
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
       image to the target memory during startup automatically */
    uint32_t core1_image_size = get_core1_image_size();

    PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}

void MU_IRQ_HANDLER(void)
{
    wakeuptime = TIMER_GetTimeInUS();
    if (kMU_Rx0FullFlag & MU_GetStatusFlags(MU_BASE))
    {
        g_msgRecv     = MU_ReceiveMsgNonBlocking(MU_BASE, 0);
        /* We do not disable MU interrupt here since we always get input from core0. */
    }
    if (kMU_GenInt0Flag & MU_GetStatusFlags(MU_BASE))
    {
        MU_ClearStatusFlags(MU_BASE, kMU_GenInt0Flag);
    }
    SDK_ISR_EXIT_BARRIER;
}

static void APP_SetWakeupConfig(void)
{
    /* Enable receive interrupt */
    MU_EnableInterrupts(MU_BASE, kMU_Rx0FullInterruptEnable);
    /* Enable the Interrupt */
    EnableIRQ(MU_IRQ);
    /* Mask all interrupt first */
    GPC_DisableAllWakeupSource(GPC_CPU_MODE_CTRL);
    /* Enable GPC interrupt */
    GPC_EnableWakeupSource(MU_IRQ);
}


int main(void)
{
    BOARD_Init();
    TIMER_Init();

    /* MU init */
    MU_Init(MU_BASE);

    /* Boot Secondary core application */
    APP_CopyCore1Image();
    PRINTF("Starting Secondary core.\r\n");
    APP_BootCore1();

    /* Wait Core 1 is Boot Up */
    while (BOOT_FLAG != MU_GetFlags(MU_BASE))
    {
    }
    PRINTF("The secondary core application has been started.\r\n");

    if (MODEL_Init() != kStatus_Success)
    {
        PRINTF("Failed initializing model" EOL);
        for (;;) {}
    }

    tensor_dims_t inputDims;
    tensor_type_t inputType;
    uint8_t* inputData = MODEL_GetInputTensorData(&inputDims, &inputType);

    tensor_dims_t outputDims;
    tensor_type_t outputType;
    uint8_t* outputData = MODEL_GetOutputTensorData(&outputDims, &outputType);

    GPC_CM_RequestRunModeSetPointTransition(GPC_CPU_MODE_CTRL, 0);

    wakeuptime = TIMER_GetTimeInUS();
    while (1)
    {
        /* Expected tensor dimensions: [batches, height, width, channels] */
        if (IMAGE_GetImage(inputData, inputDims.data[2], inputDims.data[1], inputDims.data[3]) != kStatus_Success)
        {
            PRINTF("Failed retrieving input image" EOL);
            for (;;) {}
        }
        MODEL_ConvertInput(inputData, &inputDims, inputType);

        int startTime = TIMER_GetTimeInUS();
        MODEL_RunInference();
        int endTime = TIMER_GetTimeInUS();
        MODEL_ProcessOutput(outputData, &outputDims, outputType, endTime - startTime);

        /* wakeup 10 seconds then enter sleep mode again */
        if ((endTime - wakeuptime)/1000000 > 10)
        {
            APP_SetWakeupConfig();
            PRINTF("CPU enter Sleep mode ..." EOL);
            GPIO_PinWrite(BOARD_MIPI_PANEL_BL_GPIO, BOARD_MIPI_PANEL_BL_PIN, 0);
            CpuModeTransition(kGPC_StopMode, false);
            //waked up from sleep
            GPIO_PinWrite(BOARD_MIPI_PANEL_BL_GPIO, BOARD_MIPI_PANEL_BL_PIN, 1);

        }
    }
}

