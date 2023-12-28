
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "fsl_power.h"
#include "board.h"
#include "board_setup.h"

void board_setup(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to FLEXCOMM2 (I2C master) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    /* attach 12 MHz clock to FLEXCOMM7 (I2C master) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set interrupt priorities */
    NVIC_SetPriority(I2C_SLAVE_IRQ, 2);
    NVIC_SetPriority(I2C_MASTER_IRQ, 3);
}

