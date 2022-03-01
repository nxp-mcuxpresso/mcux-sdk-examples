
#ifndef _TX_UART_H
#define _TX_UART_H

#include "fsl_common.h"

#if FSL_FEATURE_SOC_LPUART_COUNT > 0
#include "fsl_lpuart.h"
#elif FSL_FEATURE_SOC_USART_COUNT > 0
#include "fsl_usart.h"
#else
#error "Do not have a supported UART peripheral."
#endif

#include "tx_api.h"

#define TX_UART_RING_BUFFER_SIZE    128U

/* events */
#define RTOS_UART_EVENT_TX_COMPLETE     (1U)
#define RTOS_UART_EVENT_RX_COMPLETE     (2U)
#define RTOS_UART_EVENT_RING_BUFFER_OVERRUN     (4U)
#define RTOS_UART_EVENT_HARDWARE_BUFFER_OVERRUN (8U)

#define RTOS_UART_EVENT_RX_EVENTS   \
            RTOS_UART_EVENT_RX_COMPLETE | \
            RTOS_UART_EVENT_RING_BUFFER_OVERRUN | \
            RTOS_UART_EVENT_HARDWARE_BUFFER_OVERRUN


#if FSL_FEATURE_SOC_LPUART_COUNT > 0
typedef struct tx_uart_context {
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    LPUART_Type *base;                  /*!< UART base address */
    lpuart_handle_t handle;             /*!< LPUART handle */
    lpuart_transfer_t tx_transfer;      /*!< TX transfer structure */
    lpuart_transfer_t rx_transfer;      /*!< RX transfer structure */
    TX_EVENT_FLAGS_GROUP event_group;
    uint8_t ring_buffer[TX_UART_RING_BUFFER_SIZE];  /*!< ring buffer for background reception */
    uint32_t buffer_size;               /*!< Size of the ring buffer for background reception */
} tx_uart_context_t;

#elif FSL_FEATURE_SOC_USART_COUNT > 0

typedef struct tx_uart_context {
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    USART_Type *base;                   /*!< USART base address */
    usart_handle_t handle;              /*!< USART handle structure */
    usart_transfer_t tx_transfer;       /*!< TX transfer structure */
    usart_transfer_t rx_transfer;       /*!< RX transfer structure */
    TX_EVENT_FLAGS_GROUP event_group;
    uint8_t ring_buffer[TX_UART_RING_BUFFER_SIZE];  /*!< ring buffer for background reception */
    uint32_t buffer_size;               /*!< Size of the ring buffer for background reception */
} tx_uart_context_t;

#endif

#if defined(__cplusplus)
extern "C" {
#endif

UINT tx_uart_init(tx_uart_context_t *context);

UINT tx_uart_send(tx_uart_context_t *context, uint8_t *buffer, uint32_t length);

UINT tx_uart_recv(tx_uart_context_t *context, uint8_t *buffer, uint32_t length);

#if defined(__cplusplus)
}
#endif

#endif /* _TX_UART_H */
