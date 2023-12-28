
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_power.h"
#include "board.h"
#include "board_setup.h"

#define SPI_NVIC_PRIO        2

void board_setup(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    /* attach 12 MHz clock to HSLSPI */
    CLOCK_AttachClk(kFRO12M_to_HSLSPI);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kHSLSPI_RST_SHIFT_RSTn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    NVIC_SetPriority(EXAMPLE_SPI_MASTER_IRQ, SPI_NVIC_PRIO + 1);
    NVIC_SetPriority(EXAMPLE_SPI_SLAVE_IRQ, SPI_NVIC_PRIO);

}
