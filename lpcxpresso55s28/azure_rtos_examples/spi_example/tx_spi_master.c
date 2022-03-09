
#include "fsl_spi.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_spi.h"
#include "tx_api.h"

#define TRANSFER_BAUDRATE   (500000U)   /* Transfer baudrate - 500k */

#define TIMEOUT  (TX_TIMER_TICKS_PER_SECOND)   /* 1 second */


static void _spi_callback(SPI_Type *base,
                          spi_master_handle_t *drv_handle,
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
    spi_master_config_t config;
    spi_transfer_t transfer;
    UINT status = TX_SUCCESS;
    status_t ret;

    assert(tx_buffer != NULL || rx_buffer != NULL);
    assert(data_size != 0);
    assert(data_size % 4 == 0);
    assert(data_size <= MAX_BYTES_PER_FRAME);

    status = tx_mutex_get(&context->lock, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    SPI_MasterGetDefaultConfig(&config);

    config.baudRate_Bps = TRANSFER_BAUDRATE;
    config.sselNum      = EXAMPLE_SPI_SSEL;
    config.sselPol      = (spi_spol_t)EXAMPLE_MASTER_SPI_SPOL;

    ret = SPI_MasterInit(context->base, &config, EXAMPLE_SPI_MASTER_CLK_FREQ);
    if (ret != kStatus_Success)
    {
        tx_mutex_put(&context->lock);
        return TX_NOT_DONE;
    }

    (void)SPI_MasterTransferCreateHandle(context->base, &context->drv_handle,
                                         _spi_callback, (void *)context);

    transfer.txData      = tx_buffer;
    transfer.rxData      = rx_buffer;
    transfer.dataSize    = data_size;
    transfer.configFlags = kSPI_FrameAssert;

    ret = SPI_MasterTransferNonBlocking(context->base, &context->drv_handle, &transfer);
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
    if (context->result != kStatus_SPI_Idle)
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

    context->base = EXAMPLE_SPI_MASTER;

    return TX_SUCCESS;
}
