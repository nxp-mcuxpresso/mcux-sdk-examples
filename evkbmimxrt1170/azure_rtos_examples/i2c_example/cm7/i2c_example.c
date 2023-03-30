
#include "fsl_common.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_api.h"
#include "tx_i2c.h"

#define I2C_SLAVE_ADDRESS   (0x7EU)
#define DATA_LENGTH         (32)
#define DEMO_STACK_SIZE     (4 * 1024)

uint8_t tx_buffer[DATA_LENGTH];
uint8_t rx_buffer[DATA_LENGTH];
uint8_t temp_buffer[DATA_LENGTH];

TX_THREAD demo_thread_tx;
TX_THREAD demo_thread_rx;

ULONG demo_stack_tx[DEMO_STACK_SIZE / sizeof(ULONG)];
ULONG demo_stack_rx[DEMO_STACK_SIZE / sizeof(ULONG)];

tx_i2c_master_context_t master_context;
tx_i2c_slave_context_t slave_context;

TX_SEMAPHORE sync_semaphore;

static VOID demo_thread_tx_entry(ULONG arg)
{
    tx_i2c_request_t request;
    UINT is_equal;
    UINT status;
    UINT i;

    TX_THREAD_NOT_USED(arg);

    /* start to transfer data after the slave side is ready */
    status = tx_semaphore_get(&sync_semaphore, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
        return;

    status = tx_init_i2c_master(&master_context);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_init_i2c_master() -> 0x%x\r\n", status);
        return;
    }

    /* init buffer: 0, 1, 2, ... */
    for (i = 0U; i < DATA_LENGTH; i++)
    {
        tx_buffer[i] = i;
    }

    do {

        PRINTF("\r\n");
        PRINTF("Master will send data:");
        for (i = 0U; i < DATA_LENGTH; i++)
        {
            if (i % 8 == 0)
            {
                PRINTF("\r\n");
            }
            PRINTF("0x%2x  ", tx_buffer[i]);
        }
        PRINTF("\r\n\r\n");

        request.dev_address = I2C_SLAVE_ADDRESS;
        request.data = tx_buffer;
        request.data_size = DATA_LENGTH;

        status = tx_write_i2c_master(&master_context, &request);
        if (status != TX_SUCCESS)
        {
            PRINTF("ERR: tx_write_i2c_master() -> 0x%x\r\n", status);
        }

        /* read the transferred data */
        request.dev_address = I2C_SLAVE_ADDRESS;
        request.data = temp_buffer;
        request.data_size = DATA_LENGTH;

        status = tx_read_i2c_master(&master_context, &request);
        if (status != TX_SUCCESS)
        {
            PRINTF("ERR: tx_read_i2c_master() -> 0x%x\r\n", status);
        }

        /* sync to prevent massy printf output */
        status = tx_semaphore_get(&sync_semaphore, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
            return;

        /* compare data */
        is_equal = 1;
        for (i = 0; i < DATA_LENGTH; i++)
        {
            if (temp_buffer[i] != tx_buffer[i])
            {
                is_equal = 0;
                break;
            }
        }

        if (is_equal)
        {
            PRINTF("OK. Data is matched.\r\n");
        }
        else
        {
            PRINTF("ERR: data does not match\r\n");
        }

        /* wait for 3 seconds */
        tx_thread_sleep(3 * TX_TIMER_TICKS_PER_SECOND);

    } while(1);
}

static VOID demo_thread_rx_entry(ULONG arg)
{
    tx_i2c_request_t request;
    UINT status;
    UINT i;

    TX_THREAD_NOT_USED(arg);

    status = tx_init_i2c_slave(&slave_context, I2C_SLAVE_ADDRESS, DATA_LENGTH);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_init_i2_slave() -> 0x%x\r\n", status);
        return;
    }

    status = tx_semaphore_put(&sync_semaphore);
    if (status != TX_SUCCESS)
        return;

    do {

        request.data = rx_buffer;
        request.data_size = DATA_LENGTH;

        status = tx_read_i2c_slave(&slave_context, &request);
        if (status != TX_SUCCESS)
        {
            PRINTF("ERR: tx_read_i2c_master() -> 0x%x\r\n", status);
            continue;
        }

        PRINTF("Slave received data:");
        for (i = 0U; i < DATA_LENGTH; i++)
        {
            if (i % 8 == 0)
            {
                PRINTF("\r\n");
            }
            PRINTF("0x%2x  ", rx_buffer[i]);
        }
        PRINTF("\r\n\r\n");

        status = tx_semaphore_put(&sync_semaphore);
        if (status != TX_SUCCESS)
            return;

    } while(1);
}

void tx_application_define(void *first_unused_memory)
{
    UINT status;

    TX_THREAD_NOT_USED(first_unused_memory);

    status = tx_semaphore_create(&sync_semaphore, "SYNC", 0);
    if (status != TX_SUCCESS)
        return;

    status = tx_thread_create(&demo_thread_rx, "I2C demo rx",
                              demo_thread_rx_entry, 0,
                              (VOID *)demo_stack_rx, DEMO_STACK_SIZE,
                              15, 15, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the demo_thread_rx thread, 0x%x\r\n", status);
        return;
    }

    status = tx_thread_create(&demo_thread_tx, "I2C demo tx",
                              demo_thread_tx_entry, 0,
                              (VOID *)demo_stack_tx, DEMO_STACK_SIZE,
                              15, 15, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the demo_thread_tx thread, 0x%x\r\n", status);
        return;
    }
}

int main(void)
{
    board_setup();

    PRINTF("Start i2c_example\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}
