
#include "fsl_common.h"
#include "fsl_lpspi.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_spi.h"
#include "tx_api.h"

#define TRANSFER_BAUDRATE   (500000U)   /* Transfer baudrate - 500k */
#define NANOSEC_PER_SECOND 	(1000000000U)

#define TIMEOUT  (TX_TIMER_TICKS_PER_SECOND)   /* 1 second */

static void _spi_callback(LPSPI_Type *base,
                          lpspi_master_handle_t *drv_handle,
                          status_t status, void *user_data)
{
    tx_spi_master_context_t *context = user_data;

    context->result = status;
    tx_semaphore_put(&context->sema_sync);
}

UINT tx_spi_master_transfer(tx_spi_master_context_t *context,
                            uint8_t *tx_buffer, uint8_t *rx_buffer,
                            uint32_t data_size)
{
    lpspi_master_config_t config;
    lpspi_transfer_t transfer;
    UINT status = TX_SUCCESS;
    status_t ret;

    assert(tx_buffer != NULL || rx_buffer != NULL);
    assert(data_size != 0);
    assert(data_size <= MAX_BYTES_PER_FRAME);

    status = tx_mutex_get(&context->lock, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    LPSPI_MasterGetDefaultConfig(&config);
    config.baudRate      = TRANSFER_BAUDRATE;
    config.whichPcs      = EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT;
    config.cpol          = kLPSPI_ClockPolarityActiveHigh;
    config.cpha          = kLPSPI_ClockPhaseFirstEdge;

    LPSPI_MasterInit(context->base, &config, EXAMPLE_LPSPI_MASTER_CLOCK_FREQ);

    LPSPI_MasterTransferCreateHandle(context->base, &context->drv_handle,
                                     _spi_callback, (void *)context);

    transfer.txData      = tx_buffer;
    transfer.rxData      = rx_buffer;
    transfer.dataSize    = data_size;
    transfer.configFlags = EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER |
                           kLPSPI_MasterPcsContinuous |
                           kLPSPI_SlaveByteSwap;

    ret = LPSPI_MasterTransferNonBlocking(context->base,
                                          &context->drv_handle,
                                          &transfer);
    if (ret != kStatus_Success)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    /* wait for completion */
    status = tx_semaphore_get(&context->sema_sync, TIMEOUT);
    if (status != TX_SUCCESS)
    {
        tx_mutex_put(&context->lock);
        return status;
    }

    /* check the result */
    if (context->result != kStatus_Success)
    {
        PRINTF("ERR: SPI communication error, %d\r\n", context->result);
        status = TX_NOT_DONE;
    }

    tx_mutex_put(&context->lock);

    return status;
}

UINT tx_spi_master_read(tx_spi_master_context_t *context,
                        uint8_t *rx_buffer, uint32_t data_size)
{
    assert(rx_buffer != NULL);

    return tx_spi_master_transfer(context, NULL, rx_buffer, data_size);
}

UINT tx_spi_master_write(tx_spi_master_context_t *context,
                         uint8_t *tx_buffer, uint32_t data_size)
{
    assert(tx_buffer != NULL);

    return tx_spi_master_transfer(context, tx_buffer, NULL, data_size);
}

UINT tx_init_spi_master(tx_spi_master_context_t *context)
{
    UINT status = TX_SUCCESS;

    TX_MEMSET(context, 0, sizeof(tx_spi_master_context_t));

    status = tx_mutex_create(&context->lock, "LOCK", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    status = tx_semaphore_create(&context->sema_sync, "SYNC", 0);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    context->base = EXAMPLE_LPSPI_MASTER_BASEADDR;

    return TX_SUCCESS;
}
