
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H


#define I2C_MASTER_BASE                 ((I2C_Type *)I2C2_BASE)
#define I2C_MASTER_IRQ                  FLEXCOMM2_IRQn
#define I2C_MASTER_CLOCK_FREQUENCY      (12000000U)  /* 12 MHz*/

#define I2C_SLAVE_BASE                  ((I2C_Type *)I2C7_BASE)
#define I2C_SLAVE_IRQ                   FLEXCOMM7_IRQn
#define I2C_SLAVE_CLOCK_FREQUENCY       (12000000U)  /* 12 MHz*/

#define I2C_BAUDRATE                    (100000U)

#if defined(__cplusplus)
extern "C" {
#endif

void board_setup(void);

#if defined(__cplusplus)
}
#endif

#endif
