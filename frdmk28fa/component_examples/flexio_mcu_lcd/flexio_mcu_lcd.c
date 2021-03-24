/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_ssd1963.h"
#include "fsl_ft5406.h"
#if (defined(FSL_FEATURE_SOC_EDMA_COUNT) && FSL_FEATURE_SOC_EDMA_COUNT)
#include "fsl_dmamux.h"
#endif
#include "pictures.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_qspi.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Using FLEXIO EDMA MCU LCD driver for DBI data transfer. */
#define DEMO_USE_DBI_FLEXIO_EDMA 1

/* Macros for the LCD controller. */
#define DEMO_SSD1963_XTAL_FREQ     10000000U
#define DEMO_SSD1963_PCLK_FREQ     30000000U
#define DEMO_SSD1963_HSW           48U
#define DEMO_SSD1963_HFP           40U
#define DEMO_SSD1963_HBP           0U
#define DEMO_SSD1963_VSW           3U
#define DEMO_SSD1963_VFP           13U
#define DEMO_SSD1963_VBP           18U
#define DEMO_SSD1963_POLARITY_FLAG 0U
#define DEMO_SSD1963_RST_GPIO      GPIOA
#define DEMO_SSD1963_RST_PIN       5
#define DEMO_SSD1963_TE_GPIO       GPIOA
#define DEMO_SSD1963_TE_PIN        21
#define DEMO_SSD1963_CS_GPIO       GPIOA
#define DEMO_SSD1963_CS_PIN        22
#define DEMO_SSD1963_RS_GPIO       GPIOA
#define DEMO_SSD1963_RS_PIN        6

#define DEMO_DMA    DMA0
#define DEMO_DMAMUX DMAMUX0

#define DEMO_FLEXIO              FLEXIO0
#define DEMO_FLEXIO_CLOCK_FREQ   CLOCK_GetCoreSysClkFreq()
#define DEMO_FLEXIO_BAUDRATE_BPS 160000000U

/* I2C for the touch. */
#define DEMO_TOUCH_I2C            I2C1
#define DEMO_TOUCH_I2C_CLOCK_FREQ CLOCK_GetBusClkFreq()

/* Macros for the touch interrupt. */
#define DEMO_TOUCH_INT_IRQn       PORTA_IRQn
#define DEMO_TOUCH_INT_IRQHandler PORTA_IRQHandler
#define DEMO_TOUCH_INT_PORT       PORTA
#define DEMO_TOUCH_INT_PIN        2
#define DEMO_CONFIG_TOUCH_INTERRUPT() \
    PORT_SetPinInterruptConfig(DEMO_TOUCH_INT_PORT, DEMO_TOUCH_INT_PIN, kPORT_InterruptFallingEdge)
#define DEMO_TOUCH_INTERRUPT_PENDING() ((1U << DEMO_TOUCH_INT_PIN) & PORT_GetPinsInterruptFlags(DEMO_TOUCH_INT_PORT))
#define DEMO_CLEAR_TOUCH_INTERRUPT_PENDING() \
    PORT_ClearPinsInterruptFlags(DEMO_TOUCH_INT_PORT, (1U << DEMO_TOUCH_INT_PIN))

/* Macros for LCD EDMA. */
#define DEMO_FLEXIO_TX_DMA_CHANNEL 16
#define DEMO_FLEXIO_RX_DMA_CHANNEL 17
#define DEMO_FLEXIO_TX_DMA_REQUEST kDmaRequestMux0Group1FlexIO0Channel0
#define DEMO_FLEXIO_RX_DMA_REQUEST kDmaRequestMux0Group1FlexIO0Channel7

/* Macros for FlexIO shifter, timer, and pins. */
#define DEMO_FLEXIO_WR_PIN           7
#define DEMO_FLEXIO_RD_PIN           8
#define DEMO_FLEXIO_DATA_PIN_START   16
#define DEMO_FLEXIO_TX_START_SHIFTER 0
#define DEMO_FLEXIO_RX_START_SHIFTER 0
#define DEMO_FLEXIO_TX_END_SHIFTER   7
#define DEMO_FLEXIO_RX_END_SHIFTER   7
#define DEMO_FLEXIO_TIMER            0

/* Macros of the arrow. */
#define DEMO_ARROW_START_X 680
#define DEMO_ARROW_START_Y 380
#define DEMO_ARROW_SIZE    64

/* Macros of the QSPI flash. */
#define QSPI_CLK_FREQ      CLOCK_GetCoreSysClkFreq()
#define FLASH_PAGE_SIZE    256U
#define FLASH_SECTORE_SIZE 4096U
#define FLASH_SIZE         0x00400000U
#define DEMO_I2C_BAUDRATE 100000U

/* Color to fill the arraw. */
#if DEMO_BYTE_PER_PIXEL == 1
#define DEMO_ARROW_COLOR 0x03
#elif DEMO_BYTE_PER_PIXEL == 2
#define DEMO_ARROW_COLOR 0x001F
#elif DEMO_BYTE_PER_PIXEL == 3
#define DEMO_ARROW_COLOR 0x00FF
#elif DEMO_BYTE_PER_PIXEL == 4
#define DEMO_ARROW_COLOR 0x00FF
#endif

#if DEMO_USE_DBI_FLEXIO_EDMA
#include "fsl_dbi_flexio_edma.h"
#else
#error Currently only support FLEXIO MCULCD EDMA
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SetCSPin(bool set);
void BOARD_SetRSPin(bool set);
void BOARD_SetResetPin(bool set);
static void DEMO_DbiMemoryDoneCallback(status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Touch driver handle. */
ft5406_handle_t touchHandle;
/* SSD1963 LCD controller handle. */
ssd1963_handle_t lcdHandle;
/* The FlexIO MCU LCD device. */
FLEXIO_MCULCD_Type flexioLcdDev = {
    .flexioBase          = DEMO_FLEXIO,
    .busType             = kFLEXIO_MCULCD_8080,
    .dataPinStartIndex   = DEMO_FLEXIO_DATA_PIN_START,
    .ENWRPinIndex        = DEMO_FLEXIO_WR_PIN,
    .RDPinIndex          = DEMO_FLEXIO_RD_PIN,
    .txShifterStartIndex = DEMO_FLEXIO_TX_START_SHIFTER,
    .txShifterEndIndex   = DEMO_FLEXIO_TX_END_SHIFTER,
    .rxShifterStartIndex = DEMO_FLEXIO_RX_START_SHIFTER,
    .rxShifterEndIndex   = DEMO_FLEXIO_RX_END_SHIFTER,
    .timerIndex          = DEMO_FLEXIO_TIMER,
    .setCSPin            = BOARD_SetCSPin,
    .setRSPin            = BOARD_SetRSPin,
    .setRDWRPin          = NULL /* Not used in 8080 mode. */
};

/* Touch interrupt occurs. */
volatile bool touchFlag = false;

/* Pictures filled with the arrow. */
uint8_t picWithArrow[DEMO_PIC_NUM][DEMO_ARROW_SIZE * DEMO_ARROW_SIZE * DEMO_BYTE_PER_PIXEL];

/* FlexIO MCU LCD DMA handle. */
#if DEMO_USE_DBI_FLEXIO_EDMA
static edma_handle_t rxDmaHandle;
static edma_handle_t txDmaHandle;
#endif

#if DEMO_USE_DBI_FLEXIO_EDMA

dbi_flexio_edma_xfer_handle_t g_dbiFlexioXferHandle;

#define g_dbiFlexioXferOps g_dbiFlexioEdmaXferOps

#endif

static volatile bool s_dbiMemoryDone = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_ConfigQspiFlash(void)
{
    const uint32_t lut[FSL_FEATURE_QSPI_LUT_DEPTH] = {
        /* Seq0 :Quad Read */
        /* CMD:        0xEB - Quad Read, Single pad */
        /* ADDR:       0x18 - 24bit address, Quad pads */
        /* DUMMY:      0x06 - 6 clock cyles, Quad pads */
        /* READ:       0x80 - Read 128 bytes, Quad pads */
        /* JUMP_ON_CS: 0 */
        [0] = 0x0A1804EB,
        [1] = 0x1E800E0A,
        [2] = 0x2400,

        /* Seq1: Write Enable */
        /* CMD:      0x06 - Write Enable, Single pad */
        [4] = 0x406,

        /* Seq2: Erase All */
        /* CMD:    0x60 - Erase All chip, Single pad */
        [8] = 0x460,

        /* Seq3: Read Status */
        /* CMD:    0x05 - Read Status, single pad */
        /* READ:   0x01 - Read 1 byte */
        [12] = 0x1c010405,

        /* Seq4: Page Program */
        /* CMD:    0x02 - Page Program, Single pad */
        /* ADDR:   0x18 - 24bit address, Single pad */
        /* WRITE:  0x80 - Write 128 bytes at one pass, Single pad */
        [16] = 0x08180402,
        [17] = 0x2080,

        /* Seq5: Write Register */
        /* CMD:    0x01 - Write Status Register, single pad */
        /* WRITE:  0x01 - Write 1 byte of data, single pad */
        [20] = 0x20010401,

        /* Seq6: Read Config Register */
        /* CMD:  0x05 - Read Config register, single pad */
        /* READ: 0x01 - Read 1 byte */
        [24] = 0x1c010405,

        /* Seq7: Erase Sector */
        /* CMD:  0x20 - Sector Erase, single pad */
        /* ADDR: 0x18 - 24 bit address, single pad */
        [28] = 0x08180420,

        /* Seq8: Dummy */
        /* CMD:    0xFF - Dummy command, used to force SPI flash to exit continuous read mode */
        [32] = 0x4FF,

        /* Seq9: Fast Single read */
        /* CMD:        0x0B - Fast Read, Single Pad */
        /* ADDR:       0x18 - 24bit address, Single Pad */
        /* DUMMY:      0x08 - 8 clock cyles, Single Pad */
        /* READ:       0x80 - Read 128 bytes, Single Pad */
        /* JUMP_ON_CS: 0 */
        [36] = 0x0818040B,
        [37] = 0x1C800C08,
        [38] = 0x2400,

        /* Seq10: Fast Dual read */
        /* CMD:        0x3B - Dual Read, Single Pad */
        /* ADDR:       0x18 - 24bit address, Single Pad */
        /* DUMMY:      0x08 - 8 clock cyles, Single Pad */
        /* READ:       0x80 - Read 128 bytes, Dual pads */
        /* JUMP_ON_CS: 0 */
        [40] = 0x0818043B,
        [41] = 0x1D800C08,
        [42] = 0x2400,

        /* Match MISRA rule */
        [63] = 0};

    qspi_config_t config = {0};

    qspi_flash_config_t single_config = {
        .flashA1Size = FLASH_SIZE, /* 4MB */
        .flashA2Size = 0,
#if defined(FSL_FEATURE_QSPI_SUPPORT_PARALLEL_MODE) && (FSL_FEATURE_QSPI_SUPPORT_PARALLEL_MODE)
        .flashB1Size = FLASH_SIZE,
        .flashB2Size = 0,
#endif
#if !defined(FSL_FEATURE_QSPI_HAS_NO_TDH) || (!FSL_FEATURE_QSPI_HAS_NO_TDH)
        .dataHoldTime = 0,
#endif
        .CSHoldTime        = 0,
        .CSSetupTime       = 0,
        .cloumnspace       = 0,
        .dataLearnValue    = 0,
        .endian            = kQSPI_64LittleEndian,
        .enableWordAddress = false
    };

    /*Get QSPI default settings and configure the qspi */
    QSPI_GetDefaultQspiConfig(&config);

    /*Set AHB buffer size for reading data through AHB bus */
    config.AHBbufferSize[3] = FLASH_PAGE_SIZE;
    QSPI_Init(QuadSPI0, &config, QSPI_CLK_FREQ);

    /* Copy the LUT table */
    memcpy(single_config.lookuptable, lut, sizeof(uint32_t) * FSL_FEATURE_QSPI_LUT_DEPTH);

    /*According to serial flash feature to configure flash settings */
    QSPI_SetFlashConfig(QuadSPI0, &single_config);
}


void BOARD_SetCSPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_CS_GPIO, DEMO_SSD1963_CS_PIN, (uint8_t)set);
}

void BOARD_SetRSPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_RS_GPIO, DEMO_SSD1963_RS_PIN, (uint8_t)set);
}

void BOARD_SetResetPin(bool set)
{
    GPIO_PinWrite(DEMO_SSD1963_RST_GPIO, DEMO_SSD1963_RST_PIN, (uint8_t)set);
}

static void DEMO_DbiMemoryDoneCallback(status_t status, void *userData)
{
    s_dbiMemoryDone = true;
}

void DEMO_Delay(uint32_t loops)
{
    while (loops--)
    {
        __NOP();
    }
}

void DEMO_TOUCH_INT_IRQHandler(void)
{
    if (DEMO_TOUCH_INTERRUPT_PENDING())
    {
        touchFlag = true;
        DEMO_CLEAR_TOUCH_INTERRUPT_PENDING();
    }
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_InitDma(void)
{
#if DEMO_USE_DBI_FLEXIO_EDMA
    edma_config_t edmaConfig;

    /*
     * edmaConfig.enableContinuousLinkMode = false;
     * edmaConfig.enableHaltOnError = true;
     * edmaConfig.enableRoundRobinArbitration = false;
     * edmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(DEMO_DMA, &edmaConfig);

    DMAMUX_Init(DEMO_DMAMUX);

    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_FLEXIO_RX_DMA_CHANNEL, DEMO_FLEXIO_RX_DMA_REQUEST);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_FLEXIO_TX_DMA_CHANNEL, DEMO_FLEXIO_TX_DMA_REQUEST);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_FLEXIO_RX_DMA_CHANNEL);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_FLEXIO_TX_DMA_CHANNEL);

    EDMA_CreateHandle(&rxDmaHandle, DEMO_DMA, DEMO_FLEXIO_RX_DMA_CHANNEL);
    EDMA_CreateHandle(&txDmaHandle, DEMO_DMA, DEMO_FLEXIO_TX_DMA_CHANNEL);
#endif
}

void DEMO_InitTouch(void)
{
    i2c_master_config_t masterConfig;

    /*
     * masterConfig.enableMaster = true;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.enableTimeout = false;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = DEMO_I2C_BAUDRATE;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(DEMO_TOUCH_I2C, &masterConfig, DEMO_TOUCH_I2C_CLOCK_FREQ);

    /* Initialize the touch handle. */
    FT5406_Init(&touchHandle, DEMO_TOUCH_I2C);

    /* Enable the interupt for touch data. */
    NVIC_EnableIRQ(DEMO_TOUCH_INT_IRQn);

    /* Enable the touch panel I2C interrupt. */
    DEMO_CONFIG_TOUCH_INTERRUPT();
}

status_t DEMO_InitLcd(void)
{
    status_t status;

    flexio_mculcd_config_t flexioMcuLcdConfig;

    const ssd1963_config_t ssd1963Config = {
        .pclkFreq_Hz = DEMO_SSD1963_PCLK_FREQ,
#if SSD1963_DATA_WITDH == 16
        .pixelInterface = kSSD1963_PixelInterface16Bit565,
#else
        .pixelInterface = kSSD1963_PixelInterface8BitBGR888,
#endif
        .panelDataWidth = kSSD1963_PanelData24Bit,
        .polarityFlags  = DEMO_SSD1963_POLARITY_FLAG,
        .panelWidth     = DEMO_PIC_WIDTH,
        .panelHeight    = DEMO_PIC_HEIGHT,
        .hsw            = DEMO_SSD1963_HSW,
        .hfp            = DEMO_SSD1963_HFP,
        .hbp            = DEMO_SSD1963_HBP,
        .vsw            = DEMO_SSD1963_VSW,
        .vfp            = DEMO_SSD1963_VFP,
        .vbp            = DEMO_SSD1963_VBP
    };

    /* Initialize the flexio MCU LCD. */
    /*
     * flexioMcuLcdConfig.enable = true;
     * flexioMcuLcdConfig.enableInDoze = false;
     * flexioMcuLcdConfig.enableInDebug = true;
     * flexioMcuLcdConfig.enableFastAccess = true;
     * flexioMcuLcdConfig.baudRate_Bps = 96000000U;
     */
    FLEXIO_MCULCD_GetDefaultConfig(&flexioMcuLcdConfig);
    flexioMcuLcdConfig.baudRate_Bps = DEMO_FLEXIO_BAUDRATE_BPS;

    status = FLEXIO_MCULCD_Init(&flexioLcdDev, &flexioMcuLcdConfig, DEMO_FLEXIO_CLOCK_FREQ);

    if (kStatus_Success != status)
    {
        return status;
    }

    /* Create the DBI XFER handle. */
#if DEMO_USE_DBI_FLEXIO_EDMA
    status = DBI_FLEXIO_EDMA_CreateXferHandle(&g_dbiFlexioXferHandle, &flexioLcdDev, &txDmaHandle, &rxDmaHandle);
#endif

    if (kStatus_Success != status)
    {
        return status;
    }

    /* Reset the SSD1963 LCD controller. */
    BOARD_SetResetPin(false);
    DEMO_Delay(2); /* Delay 10ns. */
    BOARD_SetResetPin(true);
    DEMO_Delay(250000); /* Delay 5ms. */

    status =
        SSD1963_Init(&lcdHandle, &ssd1963Config, &g_dbiFlexioXferOps, &g_dbiFlexioXferHandle, DEMO_SSD1963_XTAL_FREQ);

    if (kStatus_Success == status)
    {
        SSD1963_SetMemoryDoneCallback(&lcdHandle, DEMO_DbiMemoryDoneCallback, NULL);
    }

    return status;
}

/*
 * Fill arrow to the picture,
 */
void DEMO_FillArrow(uint8_t pic[])
{
    uint32_t i;
    uint32_t j;

    /*
     *                 |\
     *                 | \
     *                 |  \
     *       ----------    \
     *      |               \
     *      |   1        2  /
     *       ----------    /
     *                 |  /
     *                 | /
     *                 |/
     *
     */

    /* Draw part 1. */
    for (j = DEMO_ARROW_SIZE / 4; j <= DEMO_ARROW_SIZE * 3 / 4; j++)
    {
        for (i = 0; i < DEMO_ARROW_SIZE / 2; i++)
        {
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 0] = (DEMO_ARROW_COLOR >> 0U) & 0xFF;
#if (DEMO_BYTE_PER_PIXEL > 1)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 1] = (DEMO_ARROW_COLOR >> 8U) & 0xFF;
#elif (DEMO_BYTE_PER_PIXEL > 2)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 2] = (DEMO_ARROW_COLOR >> 16U) & 0xFF;
#elif (DEMO_BYTE_PER_PIXEL > 3)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 3] = (DEMO_ARROW_COLOR >> 24U) & 0xFF;
#endif
        }
    }

    /* Draw part 2. */
    for (j = 0; j <= DEMO_ARROW_SIZE / 2; j++)
    {
        for (i = DEMO_ARROW_SIZE / 2; i < DEMO_ARROW_SIZE / 2 + j; i++)
        {
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 0] = (DEMO_ARROW_COLOR >> 0U) & 0xFF;
            pic[((DEMO_ARROW_SIZE - j - 1) * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 0] =
                (DEMO_ARROW_COLOR >> 0U) & 0xFF;
#if (DEMO_BYTE_PER_PIXEL > 1)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 1] = (DEMO_ARROW_COLOR >> 8U) & 0xFF;
            pic[((DEMO_ARROW_SIZE - j - 1) * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 1] =
                (DEMO_ARROW_COLOR >> 8U) & 0xFF;
#elif (DEMO_BYTE_PER_PIXEL > 2)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 2] = (DEMO_ARROW_COLOR >> 16U) & 0xFF;
            pic[((DEMO_ARROW_SIZE - j - 1) * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 2] =
                (DEMO_ARROW_COLOR >> 16U) & 0xFF;
#elif (DEMO_BYTE_PER_PIXEL > 3)
            pic[(j * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 3] = (DEMO_ARROW_COLOR >> 24U) & 0xFF;
            pic[((DEMO_ARROW_SIZE - j - 1) * DEMO_ARROW_SIZE + i) * DEMO_BYTE_PER_PIXEL + 3] =
                (DEMO_ARROW_COLOR >> 24U) & 0xFF;
#endif
        }
    }
}

/*
 * This function reads the arrow's area from the pictures, and saves them to the
 * array picWithArrow. Then fill the arrow to array picWithArrow.
 * When switching the pictures, the original pictures are sent first, then the
 * array picWithArrow will be sent to show the arrow.
 */
void DEMO_InitArrow(void)
{
    uint32_t i, j;

    /* Draw arrow to the pictures in SDRAM. */
    for (i = 0; i < DEMO_PIC_NUM; i++)
    {
        /* Copy the area. */
        for (j = 0; j < DEMO_ARROW_SIZE; j++)
        {
            memcpy(&picWithArrow[i][j * DEMO_ARROW_SIZE * DEMO_BYTE_PER_PIXEL],
                   &demoPictures[i][((DEMO_ARROW_START_Y + j) * DEMO_PIC_WIDTH + DEMO_ARROW_START_X) *
                                    DEMO_BYTE_PER_PIXEL],
                   DEMO_ARROW_SIZE * DEMO_BYTE_PER_PIXEL);
        }

        DEMO_FillArrow(picWithArrow[i]);
    }
}

int main(void)
{
    int touchX;
    int touchY;
    status_t status;
    touch_event_t touchEvent;
    uint32_t picIdx = 0U;

    /* Init board hardware. */
    const gpio_pin_config_t resetPinConfig = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic  = 1,
    };

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_ConfigQspiFlash();

    /* Use system clock as FlexIO clock source. */
    CLOCK_SetFlexio0Clock(0U);

    /* Set SSD1963 CS, RS, and reset pin to output. */
    GPIO_PinInit(DEMO_SSD1963_RST_GPIO, DEMO_SSD1963_RST_PIN, &resetPinConfig);
    GPIO_PinInit(DEMO_SSD1963_CS_GPIO, DEMO_SSD1963_CS_PIN, &resetPinConfig);
    GPIO_PinInit(DEMO_SSD1963_RS_GPIO, DEMO_SSD1963_RS_PIN, &resetPinConfig);

    PRINTF("FLEXIO MCU LCD example:\r\n");

    DEMO_InitArrow();

    /* Initialize DMA. */
    DEMO_InitDma();

    /* Initialize the LCD controller. */
    status = DEMO_InitLcd();
    if (kStatus_Success != status)
    {
        while (1)
        {
        }
    }

    /* Send first picture. */
    SSD1963_SelectArea(&lcdHandle, 0, 0, DEMO_PIC_WIDTH - 1, DEMO_PIC_HEIGHT - 1);

    s_dbiMemoryDone = false;
    SSD1963_WriteMemory(&lcdHandle, demoPictures[picIdx], DEMO_PIC_HEIGHT * DEMO_PIC_WIDTH * DEMO_BYTE_PER_PIXEL);
    while (s_dbiMemoryDone == false)
    {
    }

    /* Send the arrow. */
    SSD1963_SelectArea(&lcdHandle, DEMO_ARROW_START_X, DEMO_ARROW_START_Y, DEMO_ARROW_START_X + DEMO_ARROW_SIZE - 1,
                       DEMO_ARROW_START_Y + DEMO_ARROW_SIZE - 1);

    s_dbiMemoryDone = false;
    SSD1963_WriteMemory(&lcdHandle, picWithArrow[picIdx], DEMO_ARROW_SIZE * DEMO_ARROW_SIZE * DEMO_BYTE_PER_PIXEL);
    while (s_dbiMemoryDone == false)
    {
    }

    SSD1963_StartDisplay(&lcdHandle);

    SSD1963_SetBackLight(&lcdHandle, 128);

    /* Initialize the touch. */
    DEMO_InitTouch();

    while (1)
    {
        PRINTF("Touch the arrow to show next picture...\r\n");
        while (1)
        {
            /* Wait touch interrupt. */
            while (!touchFlag)
            {
            }

            NVIC_DisableIRQ(DEMO_TOUCH_INT_IRQn);

            status = FT5406_GetSingleTouch(&touchHandle, &touchEvent, &touchX, &touchY);

            touchFlag = false;
            NVIC_EnableIRQ(DEMO_TOUCH_INT_IRQn);

            if (kStatus_Success == status)
            {
                /* Only handle the up event. */
                if (touchEvent == kTouch_Up)
                {
                    PRINTF("Touch Position: %03d %03d\r\n", touchX, touchY);

                    /* If press the arrow. */
                    if ((touchX >= DEMO_ARROW_START_X) && (touchX < DEMO_ARROW_START_X + DEMO_ARROW_SIZE) &&
                        (touchY >= DEMO_ARROW_START_Y) && (touchY < DEMO_ARROW_START_Y + DEMO_ARROW_SIZE))
                    {
                        /* Arrow touched, break out to show next picture. */
                        break;
                    }
                    else
                    {
                        PRINTF("Not touch the arrow...\r\n");
                    }
                }
            }
            else
            {
                PRINTF("Error reading touch controller\r\n");
            }
        }

        /* Show the next picture. */
        picIdx++;
        if (DEMO_PIC_NUM == picIdx)
        {
            picIdx = 0;
        }

        /* Send picture. */
        SSD1963_SelectArea(&lcdHandle, 0, 0, DEMO_PIC_WIDTH - 1, DEMO_PIC_HEIGHT - 1);

        s_dbiMemoryDone = false;
        SSD1963_WriteMemory(&lcdHandle, demoPictures[picIdx], DEMO_PIC_HEIGHT * DEMO_PIC_WIDTH * DEMO_BYTE_PER_PIXEL);
        while (s_dbiMemoryDone == false)
        {
        }

        /* Send arrow. */
        SSD1963_SelectArea(&lcdHandle, DEMO_ARROW_START_X, DEMO_ARROW_START_Y, DEMO_ARROW_START_X + DEMO_ARROW_SIZE - 1,
                           DEMO_ARROW_START_Y + DEMO_ARROW_SIZE - 1);

        s_dbiMemoryDone = false;
        SSD1963_WriteMemory(&lcdHandle, picWithArrow[picIdx], DEMO_ARROW_SIZE * DEMO_ARROW_SIZE * DEMO_BYTE_PER_PIXEL);
        while (s_dbiMemoryDone == false)
        {
        }
    }
}
