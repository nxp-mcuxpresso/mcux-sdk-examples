
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_power.h"
#include "board.h"
#include "board_setup.h"

#define USART_NVIC_PRIO    5

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

void board_setup(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    /* wait for the debug port to be ready */
    delay_ms(2000);

    NVIC_SetPriority(LPUART_IRQ_NUM, USART_NVIC_PRIO);
}

uint32_t board_uart_clock_freq(void)
{
    return CLOCK_GetFlexCommClkFreq(0U);
}
