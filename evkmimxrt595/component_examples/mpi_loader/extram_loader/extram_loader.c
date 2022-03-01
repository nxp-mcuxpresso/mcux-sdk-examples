/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
#include "fsl_mpi_loader.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_EXTRAM_VECTOR_TABLE (0x28000000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
extern const clock_sys_pll_config_t g_sysPllConfig_BOARD_BootClockRUN;
extern const clock_audio_pll_config_t g_audioPllConfig_BOARD_BootClockRUN;


/* NOTE: Limitation on running extram_loader from flash (XIP): The FlexSPI flash boot source clock cannot be PLL. */
static void APP_BootClockRUN()
{
    /* Configure LPOSC 1M */
    POWER_DisablePD(kPDRUNCFG_PD_LPOSC); /* Power on LPOSC (1MHz) */
    CLOCK_EnableLpOscClk();              /* Wait until LPOSC stable */

    /* Configure FRO clock source */
    POWER_DisablePD(kPDRUNCFG_PD_FFRO); /* Power on FFRO (192MHz) */
    /* FFRO DIV1(192 MHz) is always enabled and used as Main clock during PLL update. */
    CLOCK_EnableFroClk(kCLOCK_FroAllOutEn); /* Enable all FRO outputs */

    /* Let CPU run on FRO with divider 2 (96Mhz) for safe switching. */
    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 2);
    CLOCK_AttachClk(kFRO_DIV1_to_MAIN_CLK);

    /* Configure SYSOSC clock source. */
    POWER_DisablePD(kPDRUNCFG_PD_SYSXTAL);                       /* Power on SYSXTAL */
    CLOCK_EnableSysOscClk(true, true, BOARD_SYSOSC_SETTLING_US); /* Enable system OSC */

    /* Configure SysPLL0 clock source. */
    CLOCK_InitSysPll(&g_sysPllConfig_BOARD_BootClockRUN);
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 24); /* Enable MAIN PLL clock */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24); /* Enable AUX0 PLL clock */

    /* Configure Audio PLL clock source. */
    CLOCK_InitAudioPll(&g_audioPllConfig_BOARD_BootClockRUN);
    CLOCK_InitAudioPfd(kCLOCK_Pfd0, 26); /* Enable Audio PLL clock */

    CLOCK_SetClkDiv(kCLOCK_DivSysCpuAhbClk, 2U); /* Set SYSCPUAHBCLKDIV divider to value 2 */

    /* Set up clock selectors - Attach clocks to the peripheries. */
    CLOCK_AttachClk(kMAIN_PLL_to_MAIN_CLK);        /* Switch MAIN_CLK to MAIN_PLL */
    CLOCK_AttachClk(kMAIN_CLK_DIV_to_SYSTICK_CLK); /* Switch SYSTICK_CLK to MAIN_CLK_DIV */
    CLOCK_AttachClk(kFRO_DIV2_to_CLKOUT);          /* Switch CLKOUT to FRO192M */

    /* Set up dividers. */
    CLOCK_SetClkDiv(kCLOCK_DivAudioPllClk, 15U); /* Set AUDIOPLLCLKDIV divider to value 15 */
    CLOCK_SetClkDiv(kCLOCK_DivPLLFRGClk, 11U);   /* Set FRGPLLCLKDIV divider to value 11 */
    CLOCK_SetClkDiv(kCLOCK_DivSystickClk, 2U);   /* Set SYSTICKFCLKDIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivPfc0Clk, 2U);      /* Set PFC0DIV divider to value 2 */
    CLOCK_SetClkDiv(kCLOCK_DivPfc1Clk, 4U);      /* Set PFC1DIV divider to value 4 */
    CLOCK_SetClkDiv(kCLOCK_DivClockOut, 100U);   /* Set CLKOUTFCLKDIV divider to value 100 */
}

/* Enable multicore packed image loader for DSP context copy. */
void SystemInitHook(void)
{
    status_t status;

    BOARD_InitPins();
    BOARD_InitPsRamPins();
    APP_BootClockRUN();

    status = BOARD_InitPsRam();
    if (status != kStatus_Success)
    {
        assert(false);
    }

    MPI_LoadMultiImages();
}
/*!
 * @brief Main function
 */
int main(void)
{
    void (*appEntry)(void);

    /* Init board hardware. */
    /* Need to set global variables. */
    POWER_UpdateOscSettlingTime(BOARD_SYSOSC_SETTLING_US); /* Updated XTAL oscillator settling time */
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);              /* Sets external XTAL OSC freq */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;

    BOARD_InitDebugConsole();

    PRINTF("Loading finished, now enter external RAM application.\r\n");
    appEntry = (void (*)())(*((uint32_t *)((APP_EXTRAM_VECTOR_TABLE) + 4)));
    appEntry();

    /* Will never return. */
    assert(false);
    return 0;
}
