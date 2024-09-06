/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_xspi.h"
#include "app.h"
#include "fsl_debug_console.h"
#include "fsl_xspi_edma.h"
#include "fsl_edma.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_edma_soc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern status_t xspi_nor_flash_erase_sector(XSPI_Type *base, uint32_t address);
status_t xspi_nor_flash_page_program(XSPI_Type *base, uint32_t dstAddr, uint32_t *src);
status_t xspi_nor_read_data(XSPI_Type *base, uint32_t startAddress, uint32_t *buffer, uint32_t length);
extern status_t xspi_nor_get_vendor_id(XSPI_Type *base, uint8_t *vendorId);
extern status_t xspi_nor_get_sfdp(XSPI_Type *base, uint32_t *sfdp);
extern void xspi_nor_flash_init(XSPI_Type *base);
extern void xspi_callback(XSPI_Type *base, xspi_edma_handle_t *handle, status_t status, void *userData);
extern status_t xspi_nor_enable_octal_mode(XSPI_Type *base);
/*******************************************************************************
 * Variables
 ******************************************************************************/
xspi_device_ddr_config_t flashDDrConfig = 
{
    .ddrDataAlignedClk = kXSPI_DDRDataAlignedWith2xInternalRefClk,
    .enableDdr = true,
    .enableByteSwapInOctalMode = false,
};
/*!
 * @brief Configuration for MX25UW51345GXD100.
 */
xspi_device_config_t deviceConfig =
{
    .xspiRootClk = 400000000,  /*!< 400MHz */
    .enableCknPad = false,  /*!< Do not support differential clock */     
    .deviceInterface = kXSPI_StrandardExtendedSPI,  /*!< Support Single IO and Octal IO */
    .interfaceSettings.strandardExtendedSPISettings.pageSize = FLASH_PAGE_SIZE,   /*!< 256 byte page buffer. */
    .CSHoldTime = 3,
    .CSSetupTime = 3,
    .sampleClkConfig.sampleClkSource = kXSPI_SampleClkFromExternalDQS,  /*!< Device support Data strobe signal.  */
    .sampleClkConfig.enableDQSLatency = false,
    .sampleClkConfig.dllConfig.dllMode = kXSPI_AutoUpdateMode,
    .sampleClkConfig.dllConfig.useRefValue = true,
    .sampleClkConfig.dllConfig.enableCdl8 = false,
    .ptrDeviceDdrConfig = &flashDDrConfig,
    .addrMode = kXSPI_DeviceByteAddressable,
    .columnAddrWidth = 0U,
    .enableCASInterleaving = false,
    .deviceSize[0] = FLASH_SIZE,
    .deviceSize[1] = FLASH_SIZE,   /*!< Single DIE flash, so deviceSize1 should equal to deviceSize0. */
    .ptrDeviceRegInfo = NULL,      /*!< Not used in this example. */
};

const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    /*Read*/
    [5 * NOR_CMD_LUT_SEQ_IDX_READ] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0xEE, kXSPI_Command_DDR, kXSPI_8PAD, 0x11),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ + 1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x20, kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x12),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ + 2] =
        XSPI_LUT_SEQ(kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x2, kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x4),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ + 3] =
        XSPI_LUT_SEQ(kXSPI_Command_STOP, kXSPI_8PAD, 0x0, 0, 0, 0),

    /*Read status SPI*/
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, 0x05, kXSPI_Command_READ_SDR, kXSPI_1PAD, 0x04),

    /* Read Status OPI */
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x05, kXSPI_Command_DDR, kXSPI_8PAD, 0xFA),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI + 1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x20, kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x12),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI + 2] =
        XSPI_LUT_SEQ(kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x2, kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x4),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_STATUS_OPI + 3] =
        XSPI_LUT_SEQ(kXSPI_Command_STOP, kXSPI_8PAD, 0x0, 0, 0, 0),

    /*Write enable*/
    [5 * NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, 0x06, kXSPI_Command_STOP, kXSPI_1PAD, 0x04),

    /* Write Enable - OPI */
    [5 * NOR_CMD_LUT_SEQ_IDX_WRITE_ENABLE_OPI] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x06, kXSPI_Command_DDR, kXSPI_8PAD, 0xF9),

    /* Read ID */
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_ID_OPI + 0] = XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x9F, kXSPI_Command_DDR, kXSPI_8PAD, 0x60),
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_ID_OPI + 1] = XSPI_LUT_SEQ(kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x20, kXSPI_Command_DUMMY_SDR, kXSPI_8PAD, 0x04), /*address is 0x00,0x00,0x00,0x00*/
    [5 * NOR_CMD_LUT_SEQ_IDX_READ_ID_OPI + 2] = XSPI_LUT_SEQ(kXSPI_Command_READ_DDR, kXSPI_8PAD, 0x04, kXSPI_Command_STOP, kXSPI_1PAD, 0x0),

    /* Erase Sector */
    [5 * NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x21, kXSPI_Command_DDR, kXSPI_8PAD, 0xDE),
    [5 * NOR_CMD_LUT_SEQ_IDX_ERASE_SECTOR + 1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x20, kXSPI_Command_STOP, kXSPI_8PAD, 0x0),

    /* Enable OPI DDR mode */
    [5 * NOR_CMD_LUT_SEQ_IDX_ENTER_OPI] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, 0x72, kXSPI_Command_SDR, kXSPI_1PAD, 0x00),
    [5 * NOR_CMD_LUT_SEQ_IDX_ENTER_OPI + 1] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, 0x00, kXSPI_Command_SDR, kXSPI_1PAD, 0x00),
    [5 * NOR_CMD_LUT_SEQ_IDX_ENTER_OPI + 2] =
        XSPI_LUT_SEQ(kXSPI_Command_SDR, kXSPI_1PAD, 0x00, kXSPI_Command_WRITE_SDR,  kXSPI_1PAD, 0x01),

    /* Page program */
    [5 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_OCTAL] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x12, kXSPI_Command_DDR, kXSPI_8PAD, 0xED),
    [5 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_OCTAL + 1] =
        XSPI_LUT_SEQ(kXSPI_Command_RADDR_DDR, kXSPI_8PAD, 0x20, kXSPI_Command_WRITE_DDR, kXSPI_8PAD, 0x4),

    /* Erase Chip */
    [5 * NOR_CMD_LUT_SEQ_IDX_ERASE_CHIP] =
        XSPI_LUT_SEQ(kXSPI_Command_DDR, kXSPI_8PAD, 0x60, kXSPI_Command_DDR, kXSPI_8PAD, 0x9F),
};
/*Default xspi+edma driver uses 32-bit data width configuration for transfer,
this requires data buffer address should be aligned to 32-bit. */
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_nor_program_buffer[256], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_nor_read_buffer[256], 4);
edma_handle_t dmaTxHandle;
edma_handle_t dmaRxHandle;
xspi_edma_handle_t xspiHandle;
/*******************************************************************************
 * Code
 ******************************************************************************/
extern void XSPI_TX_DMA_ISR(void);
extern void XSPI_RX_DMA_ISR(void);

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;
    status_t status;
    uint8_t vendorID = 0;
    edma_config_t userConfig;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitNorFlashPins();

    CLOCK_AttachClk(kMAIN_PLL_PFD1_to_XSPI0);
    CLOCK_SetClkDiv(kCLOCK_DivXspi0Clk, 1u);     /*400MHz*/

    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    EDMA_EnableRequest(EXAMPLE_XSPI_DMA, XSPI_TX_DMA_REQUEST_SOURCE);
    EDMA_EnableRequest(EXAMPLE_XSPI_DMA, XSPI_RX_DMA_REQUEST_SOURCE);
    
#if defined(ENABLE_RAM_VECTOR_TABLE)
    InstallIRQHandler(XSPI_TX_DMA_IRQn, (uint32_t)XSPI_TX_DMA_ISR);
    InstallIRQHandler(XSPI_RX_DMA_IRQn, (uint32_t)XSPI_RX_DMA_ISR);
#endif

    PRINTF("\r\nXSPI edma example started!\r\n");

    /* EDMA init */
    /*
     * userConfig.enableRoundRobinArbitration = false;
     * userConfig.enableHaltOnError = true;
     * userConfig.enableContinuousLinkMode = false;
     * userConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&userConfig);
    /* To unlock TBDR, configure the master ID of EDMA to be the same as the cm33_core0 */
    userConfig.enableMasterIdReplication = true;
    EDMA_Init(EXAMPLE_XSPI_DMA, &userConfig);
    /* To unlock TBDR, configure the master ID of EDMA to be the same as the cm33_core0 */
    EDMA_EnableChannelMasterIDReplication(EXAMPLE_XSPI_DMA, XSPI_TX_DMA_CHANNEL, true);
    /* Create the EDMA channel handles */
    EDMA_CreateHandle(&dmaTxHandle, EXAMPLE_XSPI_DMA, XSPI_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&dmaRxHandle, EXAMPLE_XSPI_DMA, XSPI_RX_DMA_CHANNEL);

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_XSPI_DMA, XSPI_TX_DMA_CHANNEL, XSPI_TX_DMA_REQUEST_SOURCE);
    EDMA_SetChannelMux(EXAMPLE_XSPI_DMA, XSPI_RX_DMA_CHANNEL, XSPI_RX_DMA_REQUEST_SOURCE);
#endif

    /* Create handle for xspi. */
    XSPI_TransferCreateHandleEDMA(EXAMPLE_XSPI, &xspiHandle, xspi_callback, NULL, &dmaTxHandle, &dmaRxHandle);
    /* XSPI init */
    xspi_nor_flash_init(EXAMPLE_XSPI);

#if defined(FLASH_ADESTO) && FLASH_ADESTO
    /* Get vendor ID. */
    status = xspi_nor_get_vendor_id(EXAMPLE_XSPI, &vendorID);
    if (status != kStatus_Success)
    {
        return status;
    }
    PRINTF("Vendor ID: 0x%x\r\n", vendorID);

    /* Enter quad mode. */
    status = xspi_nor_enable_octal_mode(EXAMPLE_XSPI);
    if (status != kStatus_Success)
    {
        return status;
    }
#else
    /* Enter quad mode. */
    status = xspi_nor_enable_octal_mode(EXAMPLE_XSPI);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Get vendor ID. */
    status = xspi_nor_get_vendor_id(EXAMPLE_XSPI, &vendorID);
    if (status != kStatus_Success)
    {
        return status;
    }
    PRINTF("Vendor ID: 0x%x\r\n", vendorID);
#endif

    /* Erase sectors. */
    PRINTF("Erasing Serial NOR over XSPI...\r\n");

    /* Disable I cache to avoid cache pre-fatch instruction with branch prediction from flash
       and application operate flash synchronously in multi-tasks. */

    status = xspi_nor_flash_erase_sector(EXAMPLE_XSPI, EXAMPLE_SECTOR * SECTOR_SIZE);

    if (status != kStatus_Success)
    {
        PRINTF("Erase sector failure !\r\n");
        return -1;
    }

    memset(s_nor_program_buffer, 0xFFU, sizeof(s_nor_program_buffer));
    memcpy(s_nor_read_buffer, (void *)(EXAMPLE_XSPI_AMBA_BASE + EXAMPLE_SECTOR * SECTOR_SIZE),
           sizeof(s_nor_read_buffer));

    if (memcmp(s_nor_program_buffer, s_nor_read_buffer, sizeof(s_nor_program_buffer)))
    {
        PRINTF("Erase data -  read out data value incorrect !\r\n ");
        return -1;
    }
    else
    {
        PRINTF("Erase data - successfully. \r\n");
    }

    for (i = 0; i < FLASH_PAGE_SIZE; i++)
    {
        s_nor_program_buffer[i] = i;
    }
    status = xspi_nor_flash_page_program(EXAMPLE_XSPI, EXAMPLE_SECTOR * SECTOR_SIZE, (void *)s_nor_program_buffer);

    if (status != kStatus_Success)
    {
        PRINTF("Page program failure !\r\n");
        return -1;
    }

#if defined(DEMO_INVALIDATE_CACHES)
    DEMO_INVALIDATE_CACHES;
#endif /*  defined(DEMO_INVALIDATE_CACHES) */
    memcpy(s_nor_read_buffer, (void *)(EXAMPLE_XSPI_AMBA_BASE + EXAMPLE_SECTOR * SECTOR_SIZE),
           sizeof(s_nor_read_buffer));

    if (memcmp(s_nor_read_buffer, s_nor_program_buffer, sizeof(s_nor_program_buffer)) != 0)
    {
        PRINTF("Program data -  read out data value incorrect !\r\n ");
        return -1;
    }
    else
    {
        PRINTF("EDMA Program data - successfully. \r\n");
    }

    while (1)
    {
    }
}
