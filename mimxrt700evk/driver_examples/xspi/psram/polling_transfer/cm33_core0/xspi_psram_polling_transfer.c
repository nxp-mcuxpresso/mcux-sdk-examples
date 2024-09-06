/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_xspi.h"
#include "fsl_debug_console.h"
#include "app.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
xspi_device_ddr_config_t deviceDdrConfig = {
    .ddrDataAlignedClk = kXSPI_DDRDataAlignedWith2xInternalRefClk,
    .enableByteSwapInOctalMode = false,
    .enableDdr = true,
};

xspi_device_config_t deviceConfig = {
#if defined(DEMO_USE_XSPI2) && DEMO_USE_XSPI2
   .xspiRootClk = 500000000, /*!< 500MHz for W958D6NBKX4I. */
#elif defined(DEMO_USE_XSPI1) && DEMO_USE_XSPI1
   .xspiRootClk = 400000000, /*!< 400MHz for W958D6NBKX5I. */
#endif
   .enableCknPad = true,
   .deviceInterface = kXSPI_HyperBus,
   .interfaceSettings.hyperBusSettings.x16Mode = kXSPI_x16ModeEnabledOnlyData, /*!< Only Data use x16 mode. */
#if defined(XSPI_ENABLE_VARIABLE_LATENCY) && XSPI_ENABLE_VARIABLE_LATENCY
   .interfaceSettings.hyperBusSettings.enableVariableLatency = true, /*!< Enable additional latency to increase performance. */
#else
   .interfaceSettings.hyperBusSettings.enableVariableLatency = false, /*!< Enable additional latency to increase performance. */
#endif
   .interfaceSettings.hyperBusSettings.forceBit10To1 = false,
   .CSHoldTime = 3,
   .CSSetupTime = 3,
   .sampleClkConfig.sampleClkSource = kXSPI_SampleClkFromExternalDQS,
   .sampleClkConfig.enableDQSLatency = false,
   .sampleClkConfig.dllConfig.dllMode = kXSPI_AutoUpdateMode,
   .sampleClkConfig.dllConfig.useRefValue = true,
   .sampleClkConfig.dllConfig.enableCdl8 = false,
   .addrMode = kXSPI_Device4ByteAddressable,
   .columnAddrWidth = 3U,
   .enableCASInterleaving = false,
   .deviceSize[0] = DRAM_SIZE,
   .deviceSize[1] = DRAM_SIZE,
   .ptrDeviceRegInfo = NULL,
   .ptrDeviceDdrConfig = &deviceDdrConfig,
};

const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
#if defined(XSPI_ENABLE_VARIABLE_LATENCY) && (XSPI_ENABLE_VARIABLE_LATENCY)
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xA0, 
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10, 
                                                                    kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 6), /* Dummy cycle: 2 * 6 + 2 */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ + 2] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04, 
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0x0),

        /* Memory Write */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE + 0] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x20,
                                                                    kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                    kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 6), /* Dummy cycle: 2 * 6 + 2 */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE + 2] = XSPI_LUT_SEQ(kXSPI_Command_WRITE_DDR, kXSPI_8PAD, 0x04,
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0X0),

        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xE0,
                                                            kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x6), /* Dummy cycle: 2 * 6 + 2 */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ + 2] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04,
                                                                kXSPI_Command_STOP, kXSPI_1PAD, 0x0),

        /* Register write */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x60,
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                kXSPI_Command_WRITE_DDR, kXSPI_8PAD, 0x02),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE + 2] = XSPI_LUT_SEQ(kXSPI_Command_STOP, kXSPI_1PAD, 0x0,
                                                                kXSPI_Command_STOP, kXSPI_1PAD, 0x0),
#else
        /* Memory Read */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xA0,
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 13),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_READ + 2] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04,
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0x0),

        /* Memory Write */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x20,
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 13), /* Dummy cycle: 13 + 1 */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_BURST_WRITE + 2] = XSPI_LUT_SEQ(kXSPI_Command_WRITE_DDR, kXSPI_8PAD, 0x04,
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0X0),

        /* Register Read */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xE0,
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                    kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 13),  /* Dummy cycle: 13 + 1 */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_READ + 2] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04,
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0x0),

        /* Register write */
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x60,
                                                                kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x18),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE + 1] = XSPI_LUT_SEQ(kXSPI_Command_CADDR_DDR, kXSPI_8PAD, 0x10,
                                                                    kXSPI_Command_WRITE_DDR, kXSPI_8PAD, 0x02),
        [5 * HYPERRAM_CMD_LUT_SEQ_IDX_REG_WRITE + 2] = XSPI_LUT_SEQ(kXSPI_Command_STOP, kXSPI_1PAD, 0x0,
                                                                    kXSPI_Command_STOP, kXSPI_1PAD, 0x0),
#endif
        
   /* reset */
    [5 * HYPERRAM_CMD_LUT_SEQ_IDX_RESET] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xFF, kXSPI_Command_DDR, kXSPI_8PAD, 0xFF),
    [5 * HYPERRAM_CMD_LUT_SEQ_IDX_RESET + 1] = 
        XSPI_LUT_SEQ(kXSPI_Command_STOP, kXSPI_8PAD, 0x0, kXSPI_Command_STOP, kXSPI_8PAD, 0x0),
};
static uint8_t s_hyper_ram_write_buffer[256];
/* psram enable x16 mode address will offset 8 byte*/
static uint8_t s_hyper_ram_read_buffer[256];
extern void xspi_hyper_ram_init(XSPI_Type *base);
extern void xspi_hyper_ram_ahbcommand_write_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length);
extern void xspi_hyper_ram_ahbcommand_read_data(XSPI_Type *base, uint32_t address, uint32_t *buffer, uint32_t length);
extern status_t xspi_hyper_ram_ipcommand_write_data(XSPI_Type *base,
                                                    uint32_t address,
                                                    uint32_t *buffer,
                                                    uint32_t length);
extern status_t xspi_hyper_ram_ipcommand_read_data(XSPI_Type *base,
                                                   uint32_t address,
                                                   uint32_t *buffer,
                                                   uint32_t length);
/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    uint32_t i  = 0;
    status_t st = kStatus_Success;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
#if defined(DEMO_USE_XSPI2) && DEMO_USE_XSPI2
    BOARD_InitPsRamPins_Xspi2();
    CLOCK_AttachClk(kMAIN_PLL_PFD3_to_XSPI2);
    CLOCK_SetClkDiv(kCLOCK_DivXspi2Clk, 1u);     /*500MHz*/
#elif defined(DEMO_USE_XSPI1) && DEMO_USE_XSPI1
    BOARD_InitPsRamPins_Xspi1();
    CLOCK_AttachClk(kAUDIO_PLL_PFD1_to_XSPI1);
    CLOCK_SetClkDiv(kCLOCK_DivXspi1Clk, 1u);     /*400MHz*/
#endif
    /* XSPI init */
    xspi_hyper_ram_init(EXAMPLE_XSPI);

    PRINTF("XSPI example started!\r\n");

    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = (i + 0xFFU);
    }

    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));

    for (i = 0; i < DRAM_SIZE; i += 1024)
    {
        xspi_hyper_ram_ahbcommand_write_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                             sizeof(s_hyper_ram_write_buffer));
        xspi_hyper_ram_ahbcommand_read_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                            sizeof(s_hyper_ram_read_buffer));
        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("AHB Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 1023);
            return -1;
        }
    }

    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = i;
    }
    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));

    for (i = 1; i < DRAM_SIZE - 1024; i += 1024)
    {
        xspi_hyper_ram_ahbcommand_write_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                             sizeof(s_hyper_ram_write_buffer));
        xspi_hyper_ram_ahbcommand_read_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                            sizeof(s_hyper_ram_read_buffer));

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("AHB Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 1023);
            return -1;
        }
    }

    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = (i + 0xFFU);
    }
    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));

    for (i = 2; i < DRAM_SIZE - 1024; i += 1024)
    {
        xspi_hyper_ram_ahbcommand_write_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                             sizeof(s_hyper_ram_write_buffer));
        xspi_hyper_ram_ahbcommand_read_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                            sizeof(s_hyper_ram_read_buffer));

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("AHB Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 1023);
            return -1;
        }
    }

    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = i;
    }
    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));

    for (i = 3; i < DRAM_SIZE - 1024; i += 1024)
    {
        xspi_hyper_ram_ahbcommand_write_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                             sizeof(s_hyper_ram_write_buffer));
        xspi_hyper_ram_ahbcommand_read_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                            sizeof(s_hyper_ram_read_buffer));

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("AHB Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 1023);
            return -1;
        }
    }

    PRINTF("AHB Command Read/Write data successfully at all address range !\r\n");

    memset(s_hyper_ram_read_buffer, 0, sizeof(s_hyper_ram_read_buffer));
    for (i = 0; i < sizeof(s_hyper_ram_write_buffer); i++)
    {
        s_hyper_ram_write_buffer[i] = i;
    }

    /* IP command write/read, should notice that the start address should be even address and the write address/size
     * should be 1024 aligned.*/
    for (i = 0; i < DRAM_SIZE; i += 256)
    {
        st = xspi_hyper_ram_ipcommand_write_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_write_buffer,
                                                 sizeof(s_hyper_ram_write_buffer));

        if (st != kStatus_Success)
        {
            st = kStatus_Fail;
            PRINTF("IP Command Write data Failure at 0x%x!\r\n", i);
        }

        st = xspi_hyper_ram_ipcommand_read_data(EXAMPLE_XSPI, i, (uint32_t *)s_hyper_ram_read_buffer,
                                                sizeof(s_hyper_ram_read_buffer));
        if (st != kStatus_Success)
        {
            st = kStatus_Fail;
            PRINTF("IP Command Read data Failure at 0x%x!\r\n", i);
        }

        if (memcmp(s_hyper_ram_read_buffer, s_hyper_ram_write_buffer, sizeof(s_hyper_ram_write_buffer)) != 0)
        {
            PRINTF("IP Command Read/Write data Failure at 0x%x - 0x%x!\r\n", i, i + 255);
            return -1;
        }
    }

    PRINTF("IP Command Read/Write data successfully at all address range !\r\n");

    while (1)
    {
    }
}
