
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define I2C_MASTER_BASE                 ((LPI2C_Type *)LPI2C1_BASE)
#define I2C_MASTER_IRQ                  LPI2C1_IRQn
#define I2C_MASTER_CLOCK_FREQUENCY      LPI2C_CLOCK_FREQUENCY

#define I2C_SLAVE_BASE                  ((LPI2C_Type *)LPI2C3_BASE)
#define I2C_SLAVE_IRQ                   LPI2C3_IRQn
#define I2C_SLAVE_CLOCK_FREQUENCY       LPI2C_CLOCK_FREQUENCY

#define I2C_BAUDRATE                    (100000U)

#if defined(__cplusplus)
extern "C" {
#endif

void board_setup(void);

#if defined(__cplusplus)
}
#endif

#endif
