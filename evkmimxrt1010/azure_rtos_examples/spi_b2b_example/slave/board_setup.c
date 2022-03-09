
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_iomuxc.h"
#include "board.h"
#include "board_setup.h"

/*
 * This is the relationship between signal name, pin name, and Arduino interface.
 * Signal      |   pin name    |   GPIO        |   Arduino Interface
 * LPSPI1_PCS0 |   GPIO_AD_05  |   GPIO1[19]   |   J57-6
 * LPSPI1_SDO  |   GPIO_AD_04  |   GPIO1[18]   |   J57-8
 * LPSPI1_SDI  |   GPIO_AD_03  |   GPIO1[17]   |   J57-10
 * LPSPI1_SCK  |   GPIO_AD_06  |   GPIO1[20]   |   J57-12
 *
 * The LED pin:
 * GPIO1[11] -> GPIO_11 -> GREEN LED D25
 * 0 -> LED off
 * 1 -> LED on
 */

#define LPSPI1_SDO_GPIO_NUM         18U
#define LPSPI1_SDI_GPIO_NUM         17U

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

/*
 * Setup the pin GPIO_11 to control the on-board LED D25.
 */
static void light_led(void)
{
    gpio_pin_config_t GPIO_11_config = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1U,          /* LED on */
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    GPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &GPIO_11_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_11_GPIOMUX_IO11, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_11_GPIOMUX_IO11, 0x10A0U);
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

    GPIO_PinInit(GPIO1, 19U, &LPSPI1_PCS0_config);
    GPIO_PinInit(GPIO1, 18U, &LPSPI1_SDO_config);
    GPIO_PinInit(GPIO1, 17U, &LPSPI1_SDI_config);
    GPIO_PinInit(GPIO1, 20U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_05_GPIOMUX_IO19, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_04_GPIOMUX_IO18, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_03_GPIOMUX_IO17, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_06_GPIOMUX_IO20, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_05_GPIOMUX_IO19, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_04_GPIOMUX_IO18, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_03_GPIOMUX_IO17, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_06_GPIOMUX_IO20, 0x30A0U);
}

/*
 * Set the pin as an output GPIO, and send a pulse on this pin.
 */
static void send_pulse(void)
{
    /* send a pulse */
    GPIO_PinWrite(GPIO1, LPSPI1_SDO_GPIO_NUM, 1);
    delay_ms(200);
    GPIO_PinWrite(GPIO1, LPSPI1_SDO_GPIO_NUM, 0);
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
    /* GPIO configuration of LPSPI1_PCS0 on GPIO_AD_05 (pin 55) */
    gpio_pin_config_t LPSPI1_PCS0_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    /* GPIO configuration of LPSPI1_SDO on GPIO_AD_04 (pin 56) */
    gpio_pin_config_t LPSPI1_SDO_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    /* GPIO configuration of LPSPI1_SDI on GPIO_AD_03 (pin 57) */
    gpio_pin_config_t LPSPI1_SDI_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };
    /* GPIO configuration of LPSPI1_SCK on GPIO_AD_06 (pin 52) */
    gpio_pin_config_t LPSPI1_SCK_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0U,
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    /* Initialize GPIO functionality on GPIO_AD_05 (pin 55) */
    GPIO_PinInit(GPIO1, 19U, &LPSPI1_PCS0_config);
    /* Initialize GPIO functionality on GPIO_AD_04 (pin 56) */
    GPIO_PinInit(GPIO1, 18U, &LPSPI1_SDO_config);
    /* Initialize GPIO functionality on GPIO_AD_03 (pin 57) */
    GPIO_PinInit(GPIO1, 17U, &LPSPI1_SDI_config);
    /* Initialize GPIO functionality on GPIO_AD_06 (pin 52) */
    GPIO_PinInit(GPIO1, 20U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_05_GPIOMUX_IO19, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_04_GPIOMUX_IO18, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_03_GPIOMUX_IO17, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_06_GPIOMUX_IO20, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_05_GPIOMUX_IO19, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_04_GPIOMUX_IO18, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_03_GPIOMUX_IO17, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_06_GPIOMUX_IO20, 0x30A0U);
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
    while(!GPIO_PinRead(GPIO1, LPSPI1_SDI_GPIO_NUM))
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
