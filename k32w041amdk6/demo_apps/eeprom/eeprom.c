/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "Eeprom.h"
#include "board.h"

#include "pin_mux.h"
#include "Panic.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EEPROM_PAGE_SIZE (256)
#define EEPROM_BLOCK_SIZE (32 * 1024)
#define EEPROM_START_ADDRESS (0x00000000)
#define EEPROM_BUFF_SIZE EEPROM_PAGE_SIZE
#define EEPROM_START_PAGE (EEPROM_START_ADDRESS / EEPROM_PAGE_SIZE)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t writeData[EEPROM_BUFF_SIZE];
uint8_t readData[EEPROM_BUFF_SIZE] = {0};

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t BOARD_GetSpiClock(uint32_t instance)
{
    panic(0, 0, 0, 0);
    return 0;
}

void BOARD_InitSPI(void)
{
    panic(0, 0, 0, 0);
}

#define IOCON_QSPI_MODE_FUNC  (7U)
#define LP_DIGITAL_PULLUP_CFG IOCON_PIO_FUNC(0) |\
                              IOCON_PIO_MODE(0) |\
                              IOCON_PIO_DIGIMODE(1)

#define SPIFI_FAST_IO_MODE                                              \
    IOCON_PIO_FUNC(IOCON_QSPI_MODE_FUNC) |                              \
    /* No addition pin function */                                      \
    /* Fast mode, slew rate control is enabled */                       \
    IOCON_PIO_SLEW0(1) | IOCON_PIO_SLEW1(1) |                           \
    /* Input function is not inverted */                                \
    IOCON_PIO_INV_DI |                                                  \
    /* Enables digital function */                                      \
    IOCON_PIO_DIGITAL_EN |                                              \
    /* Input filter disabled */                                         \
    IOCON_PIO_INPFILT_OFF |                                             \
    /* Open drain is disabled */                                        \
    IOCON_PIO_OPENDRAIN_DI |                                            \
    /* SSEL is disabled */                                              \
    IOCON_PIO_SSEL_DI

#define SPIFI_FAST_IO_MODE_INACT   (SPIFI_FAST_IO_MODE | IOCON_PIO_MODE(2))
#define SPIFI_FAST_IO_MODE_PULLUP  (SPIFI_FAST_IO_MODE | IOCON_PIO_MODE(0))

const iocon_group_t sfifi_io_cfg[] = {
    [0] = {
        .port = 0,
        .pin =  16,               /* SPIFI Chip Select */
        .modefunc = SPIFI_FAST_IO_MODE_PULLUP,
    },
    [1] = {
        .port = 0,
        .pin =  17,                 /*SPIFI DIO3 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [2] = {
        .port = 0,
        .pin =  18,                 /*SPIFI CLK */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [3] = {
        .port = 0,
        .pin =  19,                 /*SPIFI DIO0 */
        .modefunc =SPIFI_FAST_IO_MODE_INACT,
    },
    [4] = {
        .port = 0,
        .pin =  20,                 /*SPIFI DIO2 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [5] = {
        .port = 0,
        .pin =  21,                 /*SPIFI DIO1 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
};

static void _BOARD_InitSPIFIPins(void)
{
    /* Enables the clock for the I/O controller block. 0: Disable. 1: Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);
    for (int i = 0; i < 6; i++)
    {
        if ((i==1 || i==4) && CHIP_USING_SPIFI_DUAL_MODE())
            continue;
        IOCON_PinMuxSet(IOCON,
                        sfifi_io_cfg[i].port,
                        sfifi_io_cfg[i].pin,
                        sfifi_io_cfg[i].modefunc);
    }
}

static void _BOARD_DeInitSPIFIPins(void)
{
    /* Enables the clock for the I/O controller block. 0: Disable. 1: Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Enables the clock for the GPIO0 module */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Turn all pins into analog inputs */
    for (int i = 0; i < 6; i++)
    {
        if ((i==1 || i==4) && CHIP_USING_SPIFI_DUAL_MODE())
            continue;
        /* Initialize GPIO functionality on each pin : all inputs */
        GPIO_PinInit(GPIO,
                     sfifi_io_cfg[i].port,
                     sfifi_io_cfg[i].pin,
                     &((const gpio_pin_config_t){kGPIO_DigitalInput, 1}));
        IOCON_PinMuxSet(IOCON,
                        sfifi_io_cfg[i].port,
                        sfifi_io_cfg[i].pin,
                        LP_DIGITAL_PULLUP_CFG);
    }
}

void BOARD_InitSPIFI(void)
{
    uint32_t divisor;

    _BOARD_InitSPIFIPins();
    RESET_SetPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
    /* Enable clock to block and set divider */
    CLOCK_AttachClk(kMAIN_CLK_to_SPIFI);
    divisor = CLOCK_GetSpifiClkFreq() / BOARD_SPIFI_CLK_RATE;
    CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, divisor ? divisor : 1, false);
    RESET_ClearPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
}

void BOARD_SetSpiFi_LowPowerEnter(void)
{
    _BOARD_DeInitSPIFIPins();
}

/*!
 * @brief Main function
 */
int main(void)
{
    ee_err_t status = ee_ok;

    /* Init board hardware. */
    /* Security code to allow debug access */
    SYSCON->CODESECURITYPROT = 0x87654320;

    /* attach clock for USART(debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset FLEXCOMM for USART */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kGPIO0_RST_SHIFT_RSTn);

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitPins();

    PRINTF("EEPROM Demo\r\n\r\n");
    PRINTF("This demo will write data into the EEPROM and then read it back.\r\n\r\n");

    /* setup some test data */
    srand(0xBADD10DE);
    for (uint32_t i = 0; i < EEPROM_BUFF_SIZE; i++)
    {
        writeData[i] = (uint8_t)(rand());
    }

    status = EEPROM_Init();

    if (ee_ok != status)
    {
        PRINTF("EEPROM init error: %d\r\n\r\n", status);
        goto exit;
    }

    status = EEPROM_EraseBlock(EEPROM_START_ADDRESS, EEPROM_BLOCK_SIZE);

    if (ee_ok != status)
    {
        PRINTF("EEPROM Erase Block error: %d\r\n\r\n", status);
        goto exit;
    }

    status = EEPROM_WriteData(EEPROM_BUFF_SIZE, EEPROM_START_PAGE, writeData);

    if (ee_ok != status)
    {
        PRINTF("EEPROM Write Data error: %d\r\n\r\n", status);
        goto exit;
    }

    status = EEPROM_ReadData(EEPROM_BUFF_SIZE, EEPROM_START_PAGE, readData);

    if (ee_ok != status)
    {
        PRINTF("EEPROM Read Data error: %d\r\n\r\n", status);
        goto exit;
    }

    if (!memcmp(readData, writeData, sizeof(readData)))
    {
        PRINTF("%d bytes verified successfully via API EEPROM read.\r\n", EEPROM_BUFF_SIZE);
    }
    else
    {
        PRINTF("Data error\r\n");
    }

    /* Place the EEPROM into a minimum power consumption state, Deep Power down mode */
    EEPROM_DeInit();

    PRINTF("\r\nEnd of the demo, EEPROM is placed into deep power down mode.\n\r");

exit:
    while (1)
    {
    }
}
