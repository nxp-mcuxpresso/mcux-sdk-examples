
#include "fsl_common.h"
#include "fsl_usart.h"
#include "board_setup.h"

#include "tx_api.h"
#include "tx_uart.h"

static void _uart_callback(USART_Type *base, usart_handle_t *state,
                           status_t status, void *user_data)
{
    tx_uart_context_t *context = user_data;

    switch (status)
    {
        case kStatus_USART_TxIdle:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_TX_COMPLETE, TX_OR);
            break;
        case kStatus_USART_RxIdle:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_RX_COMPLETE, TX_OR);
            break;
        case kStatus_USART_RxRingBufferOverrun:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_RING_BUFFER_OVERRUN, TX_OR);
            break;
        default:
            break;
    }
}

UINT tx_uart_init(tx_uart_context_t *context)
{
    usart_config_t config;
    UINT status = TX_SUCCESS;
    status_t ret;

    assert(context != NULL);

    TX_MEMSET(context, 0, sizeof(tx_uart_context_t));

    ret = tx_mutex_create(&context->lock, "LOCK", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    ret = tx_event_flags_create(&context->event_group, "EVENT");
    if (status != TX_SUCCESS)
    {
        return status;
    }

    context->base = LPUART_REG_BASE;
    context->buffer_size = TX_UART_RING_BUFFER_SIZE;

    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = LPUART_TRANSFER_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    ret = USART_Init(context->base, &config, board_uart_clock_freq());
    if (ret != kStatus_Success)
    {
        return TX_FEATURE_NOT_ENABLED;
    }

    USART_TransferCreateHandle(context->base, &context->handle,
                               _uart_callback, context);

    USART_TransferStartRingBuffer(context->base, &context->handle,
                                   context->ring_buffer, context->buffer_size);

    return TX_SUCCESS;
}

UINT tx_uart_send(tx_uart_context_t *context, uint8_t *buffer, uint32_t length)
{
    UINT status = TX_SUCCESS;
    usart_transfer_t *transfer;
    ULONG events;
    status_t ret;

    assert(context != NULL);
    assert(buffer != NULL);
    assert(length != 0);

    status = tx_mutex_get(&context->lock, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    transfer = &context->tx_transfer;
    transfer->data      = buffer;
    transfer->dataSize  = length;

    ret = USART_TransferSendNonBlocking(context->base, &context->handle,
                                        transfer);
    if (ret != kStatus_Success)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    status = tx_event_flags_get(&context->event_group,
                                RTOS_UART_EVENT_TX_COMPLETE, TX_AND_CLEAR,
                                &events, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    tx_mutex_put(&context->lock);

    return status;
}

UINT tx_uart_recv(tx_uart_context_t *context, uint8_t *buffer, uint32_t length)
{
    usart_transfer_t *transfer;
    UINT status = TX_SUCCESS;
    size_t actual_size;
    ULONG events;
    status_t ret;

    assert(context != NULL);
    assert(buffer != NULL);
    assert(length != 0);

    status = tx_mutex_get(&context->lock, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    transfer = &context->rx_transfer;
    transfer->data      = buffer;
    transfer->dataSize  = length;
    ret = USART_TransferReceiveNonBlocking(context->base, &context->handle,
                                           transfer, &actual_size);
    if (ret != kStatus_Success)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    status = tx_event_flags_get(&context->event_group,
                                RTOS_UART_EVENT_RX_EVENTS, TX_OR_CLEAR,
                                &events, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    if (events & RTOS_UART_EVENT_RX_COMPLETE)
    {
        status = TX_SUCCESS;
    }
    else
    {
        USART_TransferAbortReceive(context->base, &context->handle);
        status = TX_NOT_DONE;
    }

    tx_mutex_put(&context->lock);

    return status;
}
