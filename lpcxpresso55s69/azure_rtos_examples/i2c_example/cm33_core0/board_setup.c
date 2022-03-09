
#include "fsl_common.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "fsl_power.h"
#include "board.h"
#include "board_setup.h"

void board_setup(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);

    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to FLEXCOMM1 (I2C master) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    /* attach 12 MHz clock to FLEXCOMM4 (I2C master) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set interrupt priorities */
    NVIC_SetPriority(I2C_SLAVE_IRQ, 2);
    NVIC_SetPriority(I2C_MASTER_IRQ, 3);
}

