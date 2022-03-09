
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

    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);

    NVIC_SetPriority(EXAMPLE_LPSPI_MASTER_IRQN, 3);
    NVIC_SetPriority(EXAMPLE_LPSPI_SLAVE_IRQN, 2);
}
