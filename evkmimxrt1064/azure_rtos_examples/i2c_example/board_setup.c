
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_setup.h"

void board_setup(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*Clock setting for LPI2C*/
    CLOCK_SetMux(kCLOCK_Lpi2cMux, LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, LPI2C_CLOCK_SOURCE_DIVIDER);

    /* Set IRQ priority */
    NVIC_SetPriority(I2C_MASTER_IRQ, 3);
    NVIC_SetPriority(I2C_SLAVE_IRQ, 2);
}
