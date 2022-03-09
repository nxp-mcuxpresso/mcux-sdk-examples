
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_setup.h"

#define SPI_NVIC_PRIO        2

void board_setup(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI3 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);

    /* attach 12 MHz clock to SPI7 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC3_RST_SHIFT_RSTn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    NVIC_SetPriority(EXAMPLE_SPI_MASTER_IRQ, SPI_NVIC_PRIO + 1);
    NVIC_SetPriority(EXAMPLE_SPI_SLAVE_IRQ, SPI_NVIC_PRIO);

}
