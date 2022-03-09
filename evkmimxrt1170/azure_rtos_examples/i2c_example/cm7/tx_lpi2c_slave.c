
#include "fsl_common.h"
#include "fsl_lpi2c.h"
#include "board_setup.h"

#include "tx_i2c.h"
#include "tx_api.h"

static void _i2c_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer,
                          void *user_data)
{
    tx_i2c_slave_context_t *context = user_data;

    switch (xfer->event)
    {
        case kLPI2C_SlaveTransmitEvent:
            xfer->data     = context->buffer;
            xfer->dataSize = context->block_size;
            context->state = TX_I2C_TRANSFER;
            break;
        case kLPI2C_SlaveReceiveEvent:
            xfer->data     = context->buffer;
            xfer->dataSize = context->block_size;
            context->state = TX_I2C_RECEIVE;
            break;
        case kLPI2C_SlaveCompletionEvent:
            tx_semaphore_put(&context->io_semaphore);
            break;
        default:
            break;
    }
}

UINT tx_init_i2c_slave(tx_i2c_slave_context_t *context, UINT slave_address,
                       UINT block_size)
{
    lpi2c_slave_config_t config;
    UINT status = TX_SUCCESS;
    status_t ret;

    TX_MEMSET(context, 0, sizeof(tx_i2c_slave_context_t));

    status = tx_mutex_create(&context->io_mutex, "IO MUTEX", TX_NO_INHERIT);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    status = tx_semaphore_create(&context->io_semaphore, "IO SEMAPHORE", 0);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    context->base = I2C_SLAVE_BASE;
    context->block_size = block_size;

    LPI2C_SlaveGetDefaultConfig(&config);
    config.address0 = (uint8_t)slave_address;
    config.address1 = 0;

    LPI2C_SlaveInit(context->base, &config, I2C_SLAVE_CLOCK_FREQUENCY);

    /* Create the I2C handle for the non-blocking transfer */
    LPI2C_SlaveTransferCreateHandle(context->base, &context->slave_handle,
                                  _i2c_callback, context);

    ret = LPI2C_SlaveTransferNonBlocking(context->base, &context->slave_handle,
                                       kLPI2C_SlaveCompletionEvent);
    if (ret != kStatus_Success)
    {
        return TX_NOT_DONE;
    }

    return TX_SUCCESS;
}

UINT tx_read_i2c_slave(tx_i2c_slave_context_t *context, tx_i2c_request_t *request)
{
    UINT status = TX_SUCCESS;

    status = tx_mutex_get(&context->io_mutex, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    do {
        /* wait for completion */
        status = tx_semaphore_get(&context->io_semaphore, TX_WAIT_FOREVER);
        if (status != TX_SUCCESS)
        {
            tx_mutex_put(&context->io_mutex);
            return status;
        }

        /* Check if it's a completion signal for receiving data. */
        if (context->state == TX_I2C_RECEIVE)
            break;

    } while (1);

    if (request->data_size <= context->block_size)
    {
        memcpy(request->data, context->buffer, request->data_size);
    }
    else
    {
        status = TX_NOT_DONE;
    }

    tx_mutex_put(&context->io_mutex);
    return status;
}
