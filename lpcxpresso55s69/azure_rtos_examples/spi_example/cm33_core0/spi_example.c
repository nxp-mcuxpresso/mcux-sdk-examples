
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
static uint8_t slave_tx_buffer[DATA_LENGTH];
static uint8_t slave_rx_buffer[DATA_LENGTH];

static TX_THREAD demo_master_thread;
static TX_THREAD demo_slave_thread;

static ULONG demo_stack_master[DEMO_STACK_SIZE / sizeof(ULONG)];
static ULONG demo_stack_slave[DEMO_STACK_SIZE / sizeof(ULONG)];

static tx_spi_master_context_t master_context;
static tx_spi_slave_context_t slave_context;

static TX_SEMAPHORE sync_semaphore;
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

    status = tx_init_spi_master(&master_context);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_init_spi_master() -> 0x%x\r\n", status);
        return;
    }

    for (i = 0U; i < DATA_LENGTH; i++)
    {
        master_tx_buffer[i] = i % 256;
    }

    memset(master_rx_buffer, 0, DATA_LENGTH);

    /* start to transfer data after the slave side is ready */
    status = tx_semaphore_get(&sync_semaphore, TX_WAIT_FOREVER);
    assert(status == TX_SUCCESS);

    status = tx_spi_master_transfer(&master_context, master_tx_buffer,
                                    master_rx_buffer, DATA_LENGTH);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_spi_master_transfer() -> 0x%x\r\n", status);
        return;
    }

    PRINTF("Master transmited:\r\n");
    print_array(master_tx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    PRINTF("Slave received:\r\n");
    print_array(slave_rx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    PRINTF("Slave transmited:\r\n");
    print_array(slave_tx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    PRINTF("Master received:\r\n");
    print_array(master_rx_buffer, DATA_LENGTH);
    PRINTF("\r\n");

    result = memcmp(master_tx_buffer, slave_rx_buffer, DATA_LENGTH);
    if (result == 0)
    {
        PRINTF("Master-to-slave data verified ok.\r\n");
    }
    else
    {
        PRINTF("Mismatch in master-to-slave data!\r\n");
    }

    result = memcmp(slave_tx_buffer, master_rx_buffer, DATA_LENGTH);
    if (result == 0)
    {
        PRINTF("Slave-to-master data verified ok.\r\n");
    }
    else
    {
        PRINTF("Mismatch in slave-to-master data!\r\n");
    }
}

static VOID demo_slave_thread_entry(ULONG arg)
{
    UINT status;
    UINT i;

    TX_THREAD_NOT_USED(arg);

    status = tx_init_spi_slave(&slave_context);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: tx_init_spi_slave() -> 0x%x\r\n", status);
        return;
    }

    for (i = 0U; i < DATA_LENGTH; i++)
    {
        slave_tx_buffer[i] = ~(i % 256);
    }

    memset(slave_rx_buffer, 0, DATA_LENGTH);

    status = tx_mutex_get(&slave_context.lock, TX_WAIT_FOREVER);
    assert(status == TX_SUCCESS);

    status = tx_spi_slave_transfer(&slave_context, slave_tx_buffer,
                                   slave_rx_buffer, DATA_LENGTH);
    if (status != TX_SUCCESS)
    {
        tx_mutex_put(&slave_context.lock);
        PRINTF("ERR: tx_spi_slave_transfer() -> 0x%x\r\n", status);
        return;
    }

    /* signal that the slave side is ready */
    status = tx_semaphore_put(&sync_semaphore);
    assert(status == TX_SUCCESS);

    /* wait for tranfer completion */
    status = tx_semaphore_get(&slave_context.sema_sync, TX_WAIT_FOREVER);
    assert(status == TX_SUCCESS);

    if (slave_context.result != SPI_SLAVE_RETURN_OK)
    {
        PRINTF("ERR: SPI communication error, %d\r\n", slave_context.result);
    }

    tx_mutex_put(&slave_context.lock);
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

    status = tx_semaphore_create(&sync_semaphore, "SYNC", 0);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the sync_semaphore semaphore\r\n");
        return;
    }

    status = tx_thread_create(&demo_slave_thread, "slave thread",
                              demo_slave_thread_entry, 0,
                              (VOID *)demo_stack_slave, DEMO_STACK_SIZE,
                              15, 15, 1, TX_AUTO_START);
    if (status != TX_SUCCESS)
    {
        PRINTF("ERR: create the demo_slave_thread thread, 0x%x\r\n", status);
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
}

int main(void)
{
    board_setup();

    PRINTF("\r\nStart the SPI example\r\n\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}
