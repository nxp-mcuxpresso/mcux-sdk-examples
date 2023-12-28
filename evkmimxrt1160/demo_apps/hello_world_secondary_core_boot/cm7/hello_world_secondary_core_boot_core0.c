/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU            MUA
#define BOOT_CORE1_BY_MU 0
/* Set core1 boot address*/
#define CORE1_BOOT_ADDRESS 0x08802000
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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_BootCore1(void);

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_SetSecondaryCoreGoFlag(void)
{
    SRC_SetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR, BOARD_SECONDARY_CORE_GO_FLAG);
}

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

    BOARD_SetSecondaryCoreGoFlag();
}
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("Prepare to wake up core1 booted from flash, console will be transferred to it.\r\n");
    /* Boot core 1. */
#if BOOT_CORE1_BY_MU
    MU_BootCoreB(APP_MU, APP_CORE1_BOOT_MODE);
#else
    APP_BootCore1();
#endif

    while (1)
    {
    }
}
