
#include "fsl_common.h"
#include "fsl_lpuart.h"
#include "board_setup.h"

#include "tx_api.h"
#include "tx_uart.h"

static void _uart_callback(LPUART_Type *base, lpuart_handle_t *state,
                           status_t status, void *user_data)
{
    tx_uart_context_t *context = user_data;

    switch (status)
    {
        case kStatus_LPUART_TxIdle:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_TX_COMPLETE, TX_OR);
            break;
        case kStatus_LPUART_RxIdle:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_RX_COMPLETE, TX_OR);
            break;
        case kStatus_LPUART_RxRingBufferOverrun:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_RING_BUFFER_OVERRUN, TX_OR);
            break;
        case kStatus_LPUART_RxHardwareOverrun:
            tx_event_flags_set(&context->event_group,
                               RTOS_UART_EVENT_HARDWARE_BUFFER_OVERRUN, TX_OR);
            break;
        default:
            break;
    }
}

UINT tx_uart_init(tx_uart_context_t *context)
{
    lpuart_config_t config;
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

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = LPUART_TRANSFER_BAUDRATE;

    ret = LPUART_Init(context->base, &config, board_uart_clock_freq());
    if (ret != kStatus_Success)
    {
        return TX_FEATURE_NOT_ENABLED;
    }

    LPUART_TransferCreateHandle(context->base, &context->handle,
                                _uart_callback, context);

    LPUART_TransferStartRingBuffer(context->base, &context->handle,
                                   context->ring_buffer, context->buffer_size);

    LPUART_EnableTx(context->base, true);
    LPUART_EnableRx(context->base, true);

    return TX_SUCCESS;
}

UINT tx_uart_send(tx_uart_context_t *context, uint8_t *buffer, uint32_t length)
{
    UINT status = TX_SUCCESS;
    lpuart_transfer_t *transfer;
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

    ret = LPUART_TransferSendNonBlocking(context->base, &context->handle,
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
    lpuart_transfer_t *transfer;
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
    ret = LPUART_TransferReceiveNonBlocking(context->base, &context->handle,
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
        LPUART_TransferAbortReceive(context->base, &context->handle);
        status = TX_NOT_DONE;
    }

    tx_mutex_put(&context->lock);

    return status;
}
