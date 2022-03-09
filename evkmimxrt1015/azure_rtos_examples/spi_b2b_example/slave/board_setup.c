
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_iomuxc.h"
#include "board.h"
#include "board_setup.h"

/*
 * This is the relationship between signal name, pin name, and Arduino interface.
 * Signal      |   pin name         |   GPIO        |   Arduino Interface
 * LPSPI1_PCS0 |   GPIO_AD_B0_11    |   GPIO1[11]   |   J19-3
 * LPSPI1_SDO  |   GPIO_AD_B0_12    |   GPIO1[12]   |   J19-4
 * LPSPI1_SDI  |   GPIO_AD_B0_13    |   GPIO1[13]   |   J19-5
 * LPSPI1_SCK  |   GPIO_AD_B0_10    |   GPIO1[10]   |   J19-6
 *
 * The LED pin:
 * GPIO3[21] -> GPIO_SD_B1_01 -> GREEN LED D26
 * 0 -> LED on
 * 1 -> LED off
 */

#define LPSPI1_SDO_GPIO_NUM         12U
#define LPSPI1_SDI_GPIO_NUM         13U

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
        .outputLogic = 0U,      /* LED on */
        .interruptMode = kGPIO_NoIntmode
    };

    CLOCK_EnableClock(kCLOCK_Iomuxc);

    GPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &USER_LED_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B1_01_GPIO3_IO21, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B1_01_GPIO3_IO21, 0x10B0U);
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

    GPIO_PinInit(GPIO1, 11U, &LPSPI1_PCS0_config);
    GPIO_PinInit(GPIO1, 12U, &LPSPI1_SDO_config);
    GPIO_PinInit(GPIO1, 13U, &LPSPI1_SDI_config);
    GPIO_PinInit(GPIO1, 10U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_12_GPIO1_IO12, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_12_GPIO1_IO12, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0x30A0U);
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

    GPIO_PinInit(GPIO1, 11U, &LPSPI1_PCS0_config);
    GPIO_PinInit(GPIO1, 12U, &LPSPI1_SDO_config);
    GPIO_PinInit(GPIO1, 13U, &LPSPI1_SDI_config);
    GPIO_PinInit(GPIO1, 10U, &LPSPI1_SCK_config);

    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_12_GPIO1_IO12, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0U);

    /* Pull_Down_100K_Ohm */
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_11_GPIO1_IO11, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_12_GPIO1_IO12, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_13_GPIO1_IO13, 0x30A0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_AD_B0_10_GPIO1_IO10, 0x30A0U);
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
