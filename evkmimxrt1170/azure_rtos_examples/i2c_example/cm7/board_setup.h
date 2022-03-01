
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

/* Get frequency of lpi2c clock */
#define LPI2C_CLOCK_FREQUENCY (CLOCK_GetFreq(kCLOCK_OscRc48MDiv2))

#define I2C_MASTER_BASE                 ((LPI2C_Type *)LPI2C1_BASE)
#define I2C_MASTER_IRQ                  LPI2C1_IRQn
#define I2C_MASTER_CLOCK_FREQUENCY      LPI2C_CLOCK_FREQUENCY

#define I2C_SLAVE_BASE                  ((LPI2C_Type *)LPI2C5_BASE)
#define I2C_SLAVE_IRQ                   LPI2C5_IRQn
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
