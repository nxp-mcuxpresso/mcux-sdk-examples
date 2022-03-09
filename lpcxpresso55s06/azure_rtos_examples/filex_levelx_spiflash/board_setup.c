
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_power.h"
#include "board.h"
#include "board_setup.h"

void board_setup(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivFrohfClk, 2U, false);

    /* attach 48 MHz clock to HSLSPI */
    CLOCK_AttachClk(kFRO_HF_DIV_to_HSLSPI);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kHSLSPI_RST_SHIFT_RSTn);
}
