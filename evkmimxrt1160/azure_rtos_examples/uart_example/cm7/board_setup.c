

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "board_setup.h"

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

void board_setup(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    /* wait for the debug port to be ready */
    delay_ms(2000);

    NVIC_SetPriority(LPUART_IRQ_NUM, 5);
}

uint32_t board_uart_clock_freq(void)
{
    return BOARD_DebugConsoleSrcFreq();
}
