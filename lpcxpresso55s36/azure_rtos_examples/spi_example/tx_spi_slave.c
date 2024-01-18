
#include "fsl_common.h"
#include "fsl_spi.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_spi.h"
#include "tx_api.h"

static void _spi_callback(SPI_Type *base,
                          spi_slave_handle_t *drv_handle,
                          status_t status, void *user_data)
{
    tx_spi_slave_context_t *context = user_data;

    context->result = status;
    tx_semaphore_put(&context->sema_sync);
}

UINT tx_spi_slave_transfer(tx_spi_slave_context_t *context,
                           uint8_t *tx_buffer, uint8_t *rx_buffer,
                           uint32_t data_size)
{
    spi_slave_config_t config;
    spi_transfer_t transfer;
    UINT status = TX_SUCCESS;
    status_t ret;

    assert(tx_buffer != NULL || rx_buffer != NULL);
    assert(data_size != 0);
    assert(data_size % 4 == 0);
    assert(data_size <= MAX_BYTES_PER_FRAME);

    SPI_SlaveGetDefaultConfig(&config);

    config.sselPol = (spi_spol_t)EXAMPLE_SLAVE_SPI_SPOL;

    SPI_SlaveInit(context->base, &config);

    SPI_SlaveTransferCreateHandle(context->base, &context->drv_handle,
                                  _spi_callback, (void *)context);

    transfer.txData         = tx_buffer;
    transfer.rxData         = rx_buffer;
    transfer.dataSize       = data_size;
    transfer.configFlags    = 0;

    ret = SPI_SlaveTransferNonBlocking(context->base,
                                       &context->drv_handle,
                                       &transfer);
    if (ret != kStatus_Success)
    {
        return TX_NOT_DONE;
    }

    return status;
}

UINT tx_init_spi_slave(tx_spi_slave_context_t *context)
{
    UINT status = TX_SUCCESS;

    TX_MEMSET(context, 0, sizeof(tx_spi_slave_context_t));

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

    context->base = EXAMPLE_SPI_SLAVE;

    return TX_SUCCESS;
}
