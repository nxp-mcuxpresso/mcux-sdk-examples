

#include <string.h>

#include "fsl_common.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_api.h"
#include "tx_spi.h"

#define DATA_LENGTH         (32)
#define DEMO_STACK_SIZE     (4 * 1024)

static uint8_t master_tx_buffer[DATA_LENGTH];
static uint8_t master_rx_buffer[DATA_LENGTH];

static TX_THREAD demo_master_thread;
static TX_THREAD led_thread;

static ULONG demo_stack_master[DEMO_STACK_SIZE / sizeof(ULONG)];
static ULONG led_thread_stack[DEMO_STACK_SIZE / sizeof(ULONG)];

static tx_spi_master_context_t master_context;

static TX_MUTEX print_lock;

static void print_array(uint8_t *buffer, uint32_t size)
{
    UINT status;
    int i;

    status = tx_mutex_get(&print_lock, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return;
    }

    for (i = 0; i < size; i++)
    {
        if ((i % 8 == 0) && (i != 0))
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%1x%1x  ", buffer[i] >> 4, buffer[i] & 0x0f);
    }
    PRINTF("\r\n");

    tx_mutex_put(&print_lock);
}

static VOID demo_master_thread_entry(ULONG arg)
{
    int result;
    UINT status;
    UINT i;

    TX_THREAD_NOT_USED(arg);

    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND * 2);

    status = tx_init_spi_master(&master_context);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_init_spi_master() -> 0x%x\r\n", status);
        return;
    }

    /* init buffer: 0, 1, 2, ... */
    for (i = 0U; i < DATA_LENGTH; i++)
    {
        master_tx_buffer[i] = i % 256;
    }

    memset(master_rx_buffer, 0, DATA_LENGTH);

    status = tx_spi_master_transfer(&master_context, master_tx_buffer,
                                    master_rx_buffer, DATA_LENGTH);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_spi_master_write() -> 0x%x\r\n", status);
        return;
    }

    PRINTF("Master transmitted:\r\n");
    print_array(master_tx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    PRINTF("Master received:\r\n");
    print_array(master_rx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    result = memcmp(master_rx_buffer, master_tx_buffer, DATA_LENGTH);
    if (result == 0)
    {
        PRINTF("Slave-to-master data verified ok.\r\n");
    }
    else
    {
        PRINTF("Mismatch in slave-to-master data!\r\n");
    }
    PRINTF("\r\n");
}

static VOID led_thread_entry(ULONG arg)
{
    TX_THREAD_NOT_USED(arg);

    do {
        board_led_toggle();
        tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 2);
    } while(1);
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    status = tx_mutex_create(&print_lock, "PRINT LOCK", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the print_lock mutex\r\n");
        return;
    }

    status = tx_thread_create(&demo_master_thread, "master thread",
                              demo_master_thread_entry, 0,
                              (VOID *)demo_stack_master, DEMO_STACK_SIZE,
                              15, 15, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the demo_master_thread thread, 0x%x\r\n", status);
        return;
    }

    status = tx_thread_create(&led_thread, "LED thread",
                              led_thread_entry, 0,
                              (VOID *)led_thread_stack,
                              DEMO_STACK_SIZE,
                              20, 20, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the led_thread thread, 0x%x\r\n", status);
        return;
    }
}

int main(void)
{
    master_board_setup();

    PRINTF("Start the SPI board-to-board master example\r\n\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}
