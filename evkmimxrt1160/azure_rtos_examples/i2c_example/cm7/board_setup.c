
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_setup.h"

void board_setup(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Set IRQ priority */
    NVIC_SetPriority(I2C_MASTER_IRQ, 3);
    NVIC_SetPriority(I2C_SLAVE_IRQ, 2);
}
