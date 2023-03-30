/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sema4.h"
#include "fsl_mu.h"
#include "demo_common.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_MU                     MUA
#define DEMO_SEMA4                  SEMA4
#define DEMO_PROC_NUM               0 /* Fixed value by system integration. */
#define CORE1_BOOT_ADDRESS          0x20200000
#define DEMO_EnableSema4Interrupt() NVIC_EnableIRQ(SEMA4_CP0_IRQn)
#define DEMO_SEMA4IRQHandler        SEMA4_CP0_IRQHandler

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif defined(__GNUC__)
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
#define DEMO_INVALID_GATE_NUM 0xFFFFFFFFU

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile uint32_t s_lastNotifyGateNum = DEMO_INVALID_GATE_NUM;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DEMO_BootCore1(void);
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
static void DEMO_WaitCore1Stat(uint32_t stat);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * For the dual core project, generally primary core starts first, initializes
 * the system, then starts the secondary core to run.
 * In the case that debugging dual-core at the same time (for example, using IAR+DAPLink),
 * the secondary core is started by debugger. Then the secondary core might
 * run when the primary core initialization not finished. The SRC->GPR is used
 * here to indicate whether secondary core could go. When started, the secondary core
 * should check and wait the flag in SRC->GPR, the primary core sets the
 * flag in SRC->GPR when its initialization work finished.
 */
#define BOARD_SECONDARY_CORE_GO_FLAG 0xa5a5a5a5u
#define BOARD_SECONDARY_CORE_SRC_GPR kSRC_GeneralPurposeRegister20

void BOARD_SetSecondaryCoreGoFlag(void)
{
    SRC_SetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR, BOARD_SECONDARY_CORE_GO_FLAG);
}

void DEMO_BootCore1(void)
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

    BOARD_SetSecondaryCoreGoFlag();
}

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size()
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

void DEMO_SEMA4IRQHandler(void)
{
    uint32_t notifyStatus;
    uint32_t notifyGateNum;

    notifyStatus = SEMA4_GetGateNotifyStatus(DEMO_SEMA4, DEMO_PROC_NUM);

    if (0U == notifyStatus)
    {
        notifyGateNum = DEMO_INVALID_GATE_NUM;
    }
    else
    {
        notifyGateNum = 31U - __CLZ(notifyStatus);

        SEMA4_ResetGateNotify(DEMO_SEMA4, s_lastNotifyGateNum);
    }

    s_lastNotifyGateNum = notifyGateNum;

    __DSB();
}

static void DEMO_WaitCore1Stat(uint32_t stat)
{
    while (stat != MU_ReceiveMsg(DEMO_MU, DEMO_MU_CH))
    {
    }
}

/*!
 * @brief Function to copy core1 image to execution address.
 */
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

int main(void)
{
    uint32_t gateNum;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    APP_CopyCore1Image();

    PRINTF("SEMA4 dual core example start\r\n");

    MU_Init(DEMO_MU);

    SEMA4_Init(DEMO_SEMA4);

    SEMA4_ResetAllGates(DEMO_SEMA4);

    SEMA4_ResetAllGateNotify(DEMO_SEMA4);

    DEMO_EnableSema4Interrupt();

    DEMO_BootCore1();

    /* Wait for core 1 boot up. */
    DEMO_WaitCore1Stat(DEMO_STAT_CORE1_READY);

    for (gateNum = 0U; gateNum < FSL_FEATURE_SEMA4_GATE_COUNT; gateNum++)
    {
        PRINTF("\r\nGate %d/%d: ", gateNum + 1, FSL_FEATURE_SEMA4_GATE_COUNT);

        SEMA4_EnableGateNotifyInterrupt(DEMO_SEMA4, DEMO_PROC_NUM, (1U << gateNum));

        /* Let core 1 lock gate. */
        MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_CMD_LOCK_GATE(gateNum));

        /* Wait for gate locked by core 1. */
        DEMO_WaitCore1Stat(DEMO_STAT_GATE_LOCKED(gateNum));

        /* Try to lock the gate, return error. */
        if (kStatus_Success == SEMA4_TryLock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM))
        {
            PRINTF("Gate should be locked by core1 but not.\r\n");
            break;
        }

        /* Let core 1 unlock the gate. */
        MU_SendMsg(DEMO_MU, DEMO_MU_CH, DEMO_CMD_UNLOCK_GATE(gateNum));

        /* Wait for gate locked by core 1. */
        DEMO_WaitCore1Stat(DEMO_STAT_GATE_UNLOCKED(gateNum));

        /* Try to lock the gate, return success. */
        if (kStatus_Success != SEMA4_TryLock(DEMO_SEMA4, gateNum, DEMO_PROC_NUM))
        {
            PRINTF("Gate should be unlocked by core1 but not.\r\n");
            break;
        }

        SEMA4_DisableGateNotifyInterrupt(DEMO_SEMA4, DEMO_PROC_NUM, (1U << gateNum));

        /* Check the notify status. */
        if (s_lastNotifyGateNum != gateNum)
        {
            break;
        }

        PRINTF("Pass\r\n");
    }

    if (gateNum < FSL_FEATURE_SEMA4_GATE_COUNT)
    {
        PRINTF("\r\nExample finished with error\r\n");
    }
    else
    {
        PRINTF("\r\nExample finished successfully\r\n");
    }

    while (1)
    {
    }
}
