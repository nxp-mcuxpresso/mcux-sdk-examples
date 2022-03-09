

#include "fsl_common.h"
#include "board.h"
#include "board_setup.h"

#include "tx_api.h"
#include "tx_uart.h"
#include "string.h"


#define DEMO_STACK_SIZE     (4 * 1024)
#define DATA_LENGTH         (32)

static TX_THREAD demo_thread;
static ULONG demo_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

static uint8_t rx_buffer[DATA_LENGTH];
static tx_uart_context_t uart_context;

static const char *welcome = "Start the UART example...\r\n";
static const char *prompt = "Please input 4 characters:\r\n";
static const char *recv_error = "\r\nreceive error\r\n";


static VOID demo_thread_entry(ULONG arg)
{
    UINT status;

    TX_THREAD_NOT_USED(arg);

    status = tx_uart_init(&uart_context);
    if (status != TX_SUCCESS)
        return;

    status = tx_uart_send(&uart_context, (uint8_t *)welcome, strlen(welcome));
    if (status != TX_SUCCESS)
        return;

    status = tx_uart_send(&uart_context, (uint8_t *)prompt, strlen(prompt));
    if (status != TX_SUCCESS)
        return;

    do {

        memset(rx_buffer, 0, DATA_LENGTH);

        status = tx_uart_recv(&uart_context, rx_buffer, 4);
        if (status == TX_SUCCESS)
        {
            status = tx_uart_send(&uart_context, (uint8_t *)rx_buffer,
                                  strlen((const char *)rx_buffer));
            if (status != TX_SUCCESS)
                return;
        }
        else
        {
            status = tx_uart_send(&uart_context, (uint8_t *)recv_error,
                                  strlen((const char *)recv_error));
            if (status != TX_SUCCESS)
                return;
        }

    } while (1);
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    status = tx_thread_create(&demo_thread, "UART demo",
                              demo_thread_entry, 0,
                              (VOID *)demo_stack, DEMO_STACK_SIZE,
                              15, 15, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        return;
    }
}

int main(void)
{
    board_setup();

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}
