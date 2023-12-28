
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_iomuxc.h"
#include "board.h"
#include "board_setup.h"
#include "fsl_debug_console.h"

/*
 * This is the relationship between signal name, pin name, and Arduino interface.
 * Signal      |   pin name         |   GPIO        |   Arduino Interface
 * LPSPI1_PCS0 |   GPIO_SD_B0_01    |   GPIO3[13]   |   J17-3
 * LPSPI1_SDO  |   GPIO_SD_B0_02    |   GPIO3[14]   |   J17-4
 * LPSPI1_SDI  |   GPIO_SD_B0_03    |   GPIO3[15]   |   J17-5
 * LPSPI1_SCK  |   GPIO_SD_B0_00    |   GPIO3[12]   |   J17-6
 *
 * The LED pin:
 * GPIO1[8] -> GPIO_AD_B0_08 -> GREEN LED D8
 * 0 -> LED off
 * 1 -> LED on
 */

#define LPSPI1_SDO_GPIO_NUM         14U
#define LPSPI1_SDI_GPIO_NUM         15U

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

/*
 * Setup the pin GPIO_SD_B1_01 to control the on-board LED D26.
 */
static void light_led(void)
{
    gpio_pin_config_t USER_LED_config = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1U,      /* LED on */
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    GPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &USER_LED_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_08_GPIO1_IO08, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_08_GPIO1_IO08, 0x10B0U);
}

#ifdef BOARD_LPSPI_B2B_MASTER

static void init_gpio(void)
{
    gpio_pin_config_t LPSPI1_PCS0_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SDO_config = {
        .direction = kGPIO_DigitalOutput,   /* output */
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SDI_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SCK_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);


    GPIO_PinInit(GPIO3, 13U, &LPSPI1_PCS0_config);
    GPIO_PinInit(GPIO3, 14U, &LPSPI1_SDO_config);
    GPIO_PinInit(GPIO3, 15U, &LPSPI1_SDI_config);
    GPIO_PinInit(GPIO3, 12U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_01_GPIO3_IO13, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_GPIO3_IO12, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_GPIO3_IO13, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_GPIO3_IO12, 0x30A0U);
}

/*
 * Set the pin as an output GPIO, and send a pulse on this pin.
 */
static void send_pulse(void)
{
    /* send a pulse */
    GPIO_PinWrite(GPIO3, LPSPI1_SDO_GPIO_NUM, 1);
    delay_ms(200);
    GPIO_PinWrite(GPIO3, LPSPI1_SDO_GPIO_NUM, 0);
}

/*
 * This setup function is for boards worked as LPSPI master.
 */
void master_board_setup(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootClocks();

    /* make the lpspi pins low */
    init_gpio();
    /* turn on-board LED on */
    light_led();

    /* wait for a moment to make sure the slave device is ready */
    delay_ms(2000);     /* about 2 seconds */

    /* send out a pulse to inform the slave device to be ready for tranfer */
    send_pulse();

    BOARD_InitBootPins();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    NVIC_SetPriority(EXAMPLE_LPSPI_MASTER_IRQN, 3);
}
#else

/*
 * When the lpspi pins of two boards are connected, a high voltage in these pins
 * will affect the bootup process. Boards may not boot up correctly. To avoid this
 * case, power up the slave device first and make the lpspi pins on the slave device low,
 * then boot up the master device.
 *
 * This function will make the lpspi pins low at the boot time.
 */
static void zero_lpspi_pins(void)
{
    gpio_pin_config_t LPSPI1_PCS0_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SDO_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SDI_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    gpio_pin_config_t LPSPI1_SCK_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    GPIO_PinInit(GPIO3, 13U, &LPSPI1_PCS0_config);
    GPIO_PinInit(GPIO3, 14U, &LPSPI1_SDO_config);
    GPIO_PinInit(GPIO3, 15U, &LPSPI1_SDI_config);
    GPIO_PinInit(GPIO3, 12U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_01_GPIO3_IO13, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_GPIO3_IO12, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_GPIO3_IO13, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_GPIO3_IO14, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_GPIO3_IO15, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_GPIO3_IO12, 0x30A0U);
}

/*
 * This setup function is for boards worked as LPSPI slave.
 */
void slave_board_setup(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootClocks();

    /* make the lpspi pins low at the boot time */
    zero_lpspi_pins();

    /*
     * Light the LED to show the device is ready to start.
     * Inform that the master device can power up now.
     */
    light_led();

    /* wait for a moment to make sure the level of LPSPI1_SDI is stable */
    delay_ms(1000);

    /*
     * Check the signal sent from the master device.
     * If received a pulse, it will continue the program.
     */
    while(!GPIO_PinRead(GPIO3, LPSPI1_SDI_GPIO_NUM))
    {
        delay_ms(50);
    }

    BOARD_InitBootPins();
    BOARD_InitDebugConsole();

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    NVIC_SetPriority(EXAMPLE_LPSPI_SLAVE_IRQN, 2);
}
#endif

/*
 * This function provides an API for applications to control the on-board LED.
 */
void board_led_toggle(void)
{
    static bool set_pin = true;

    GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, set_pin ? 1U : 0U);
    set_pin = !set_pin;
}
