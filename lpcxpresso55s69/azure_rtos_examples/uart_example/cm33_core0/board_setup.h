
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

#define LPUART_REG_BASE             USART0
#define LPUART_IRQ_NUM              FLEXCOMM0_IRQn

#define LPUART_TRANSFER_BAUDRATE    (115200U)

#if defined(__cplusplus)
extern "C" {
#endif

void board_setup(void);

uint32_t board_uart_clock_freq(void);

#if defined(__cplusplus)
}
#endif

#endif
