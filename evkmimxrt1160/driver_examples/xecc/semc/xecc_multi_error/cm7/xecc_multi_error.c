/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "app.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_semc.h"
#include "fsl_xecc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SEMC_EXAMPLE_DATALEN (0x1000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern status_t BOARD_InitSEMC(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_xecc_multi_error = false;
static volatile bool s_bus_fault_flag   = false;
volatile uint32_t sdram_writeBuffer[SEMC_EXAMPLE_DATALEN];
volatile uint32_t sdram_readBuffer[SEMC_EXAMPLE_DATALEN];

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t BOARD_InitSEMC(void)
{
    semc_config_t config;
    semc_sdram_config_t sdramconfig;
    uint32_t clockFrq = EXAMPLE_SEMC_CLK_FREQ;

    /* Initializes the MAC configure structure to zero. */
    memset(&config, 0, sizeof(semc_config_t));
    memset(&sdramconfig, 0, sizeof(semc_sdram_config_t));

    /* Initialize SEMC. */
    SEMC_GetDefaultConfig(&config);
    config.dqsMode = kSEMC_Loopbackdqspad; /* For more accurate timing. */
    SEMC_Init(SEMC, &config);

    /* Configure SDRAM. */
    sdramconfig.csxPinMux           = kSEMC_MUXCSX0;
    sdramconfig.address             = 0x80000000;
    sdramconfig.memsize_kbytes      = 32 * 1024; /* 32MB = 32*1024*1KBytes*/
    sdramconfig.portSize            = kSEMC_PortSize16Bit;
    sdramconfig.burstLen            = kSEMC_Sdram_BurstLen8;
    sdramconfig.columnAddrBitNum    = kSEMC_SdramColunm_9bit;
    sdramconfig.casLatency          = kSEMC_LatencyThree;
    sdramconfig.tPrecharge2Act_Ns   = 15; /* tRP 15ns */
    sdramconfig.tAct2ReadWrite_Ns   = 15; /* tRCD 15ns */
    sdramconfig.tRefreshRecovery_Ns = 70; /* Use the maximum of the (Trfc , Txsr). */
    sdramconfig.tWriteRecovery_Ns   = 2;  /* tWR 2ns */
    sdramconfig.tCkeOff_Ns =
        42; /* The minimum cycle of SDRAM CLK off state. CKE is off in self refresh at a minimum period tRAS.*/
    sdramconfig.tAct2Prechage_Ns       = 40; /* tRAS 40ns */
    sdramconfig.tSelfRefRecovery_Ns    = 70;
    sdramconfig.tRefresh2Refresh_Ns    = 60;
    sdramconfig.tAct2Act_Ns            = 2; /* tRC/tRDD 2ns */
    sdramconfig.tPrescalePeriod_Ns     = 160 * (1000000000 / clockFrq);
    sdramconfig.refreshPeriod_nsPerRow = 64 * 1000000 / 8192; /* 64ms/8192 */
    sdramconfig.refreshUrgThreshold    = sdramconfig.refreshPeriod_nsPerRow;
    sdramconfig.refreshBurstLen        = 1;
    sdramconfig.delayChain             = 2;

    return SEMC_ConfigureSDRAM(SEMC, kSEMC_SDRAM_CS0, &sdramconfig, clockFrq);
}

void EXAMPLE_XECC_BUSFAULT_HANDLER(void)
{
    /* BusFault is active and XECC multiple error interrupt is pending. */
    s_bus_fault_flag = true;
}

void EXAMPLE_XECC_IRQ_HANDLER(void)
{
    uint32_t intStatus;

    intStatus = XECC_GetStatusFlags(EXAMPLE_XECC);
    XECC_ClearStatusFlags(EXAMPLE_XECC, intStatus);

    if ((intStatus & (uint32_t)kXECC_MultiErrorInterruptFlag) != 0x00U)
    {
        s_xecc_multi_error = true;
    }
    SDK_DelayAtLeastUs(100000, SystemCoreClock);

    /* Avoid to loop between multiple error interrupt and BusFault infinitely*/
    XECC_Deinit(EXAMPLE_XECC);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t multiErrorData;
    uint32_t uncorrectedData;
    uint32_t sdramAddress    = EXAMPLE_SEMC_START_ADDRESS;
    volatile uint32_t *sdram = (uint32_t *)EXAMPLE_SEMC_START_ADDRESS; /* SDRAM start address. */
    bool errorFlag           = false;
    xecc_config_t config;
    xecc_multi_error_info_t info;

    /* Hardware initialize. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    XECC_Deinit(EXAMPLE_XECC);

    PRINTF("XECC Multiple Errors Example Start!\r\n");
    if (BOARD_InitSEMC() != kStatus_Success)
    {
        PRINTF("\r\n SEMC SDRAM Init Failed\r\n");
    }

    /* Get default configuration */
    XECC_GetDefaultConfig(&config);
    /* Enable ECC function for write and read */
    config.enableXECC     = true;
    config.enableWriteECC = true;
    config.enableReadECC  = true;

    /* Set ECC regions */
    config.Region0BaseAddress = EXAMPLE_SEMC_START_ADDRESS;
    config.Region0EndAddress  = EXAMPLE_SEMC_START_ADDRESS + EXAMPLE_XECC_AREA_SIZE;
    config.Region1BaseAddress = EXAMPLE_SEMC_START_ADDRESS + 0X2000U;
    config.Region1EndAddress  = EXAMPLE_SEMC_START_ADDRESS + 0X2000U + EXAMPLE_XECC_AREA_SIZE;
    config.Region2BaseAddress = EXAMPLE_SEMC_START_ADDRESS + 0x4000U;
    config.Region2EndAddress  = EXAMPLE_SEMC_START_ADDRESS + 0x4000U + EXAMPLE_XECC_AREA_SIZE;
    config.Region3BaseAddress = EXAMPLE_SEMC_START_ADDRESS + 0x6000U;
    config.Region3EndAddress  = EXAMPLE_SEMC_START_ADDRESS + 0x6000U + EXAMPLE_XECC_AREA_SIZE;

    /* Initialize XECC */
    XECC_Init(EXAMPLE_XECC, &config);
    /* Enable IRQ */
    EnableIRQ(EXAMPLE_XECC_IRQ);

    /*
     * When multiple bits error is occurred for read operation on xecc block, the xecc will generate
     * a fatal interrupt if xecc interrupt enable is active. At the same time, xecc will also output
     * an read response error signal on axi bus to the core. If the BusFault interrupt is enabled in NVIC,
     * then the core will also receive an bus fault interrupt (interrupt_number=5), which is more higher priority
     * than xecc fatal error interrupt. The core will response the BusFault interrupt firstly, then the xecc interrupt
     * ISR.
     */
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;

    /* Enable XECC multiple error interrupt */
    XECC_EnableInterrupts(EXAMPLE_XECC, kXECC_MultiErrorInterruptStatusEnable);

    multiErrorData       = 0x03;
    sdram_writeBuffer[0] = 0xDDCCBBAAU;
    uncorrectedData      = 0xDDCCBBA9U;

    /* Multiple error injection. */
    XECC_ErrorInjection(EXAMPLE_XECC, multiErrorData, 0);

    /* Write data into SDRAM. */
    sdram[0] = sdram_writeBuffer[0];

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    /* Disable D cache to avoid cache pre-fetch more data from external memory, which include ECC data.
       Otherwise, XECC will decode ECC data itself and generate another error interrupt. */
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
    SCB_DisableDCache();
#elif (defined __CORTEX_M) && (__CORTEX_M == 4U)
    L1CACHE_DisableSystemCache();
#endif
#endif

    /* Read data from SDRAM. */
    sdram_readBuffer[0] = sdram[0];

    /* Waiting for multiple error interrupt */
    while (!s_xecc_multi_error)
    {
    }

    if (s_bus_fault_flag)
    {
        PRINTF("First level BusFault interrupt finished.\r\n");
    }
    else
    {
        PRINTF("First level BusFault interrupt failed.\r\n");
    }

    /* Verify multiple error with injection operation. */
    (void)XECC_GetMultiErrorInfo(EXAMPLE_XECC, &info);

    if ((sdramAddress == info.multiErrorAddress) && (uncorrectedData == info.multiErrorData))
    {
        PRINTF("Uncorrecdted read data: 0x%x\r\n", uncorrectedData);
        PRINTF("Multiple error address: 0x%x\r\n", info.multiErrorAddress);
        PRINTF("Multiple error read data: 0x%x\r\n", info.multiErrorData);
        PRINTF("Multiple error ECC code: 0x%x\r\n", info.multiErrorEccCode);
        PRINTF("Multiple error bit field: 0x%x\r\n", info.multiErrorBitField);
    }
    else
    {
        errorFlag = true;
    }

    if (errorFlag)
    {
        PRINTF("XECC multiple errors example failed!\r\n");
    }
    else
    {
        PRINTF("XECC multiple errors example successfully!\r\n");
    }

    while (1)
    {
    }
}
