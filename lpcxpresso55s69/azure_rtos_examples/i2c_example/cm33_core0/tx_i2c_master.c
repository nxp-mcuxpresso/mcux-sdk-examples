
#include "fsl_common.h"
#include "fsl_i2c.h"
#include "fsl_debug_console.h"
#include "board_setup.h"

#include "tx_i2c.h"
#include "tx_api.h"

#define TIMEOUT  (TX_TIMER_TICKS_PER_SECOND / 2)   /* 0.5 second */

static void _i2c_callback(I2C_Type *base,
                          i2c_master_handle_t *handle,
                          status_t status,
                          void *user_data)
{
    tx_i2c_master_context_t *context = user_data;
    UINT ret;

    context->result = status;
    while (true)
    {
        ret = tx_semaphore_put(&context->io_semaphore);
        if (ret == TX_SUCCESS)
            break;

        tx_thread_sleep(1);
    }
}

static UINT _tx_tranfer_i2c(tx_i2c_master_context_t *context, tx_i2c_request_t *request,
                           i2c_direction_t dir)
{
    i2c_master_transfer_t transfer;
    status_t ret;
    UINT status = TX_SUCCESS;

    assert(request != NULL);
    assert(request->data != NULL);

    status = tx_mutex_get(&context->io_mutex, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS)
    {
        return status;
    }

    transfer.slaveAddress   = request->dev_address;
    transfer.direction      = dir;
    transfer.subaddress     = 0;
    transfer.subaddressSize = 0;
    transfer.data           = request->data;
    transfer.dataSize       = request->data_size;
    transfer.flags          = kI2C_TransferDefaultFlag;

    /* call the non-blocking function to transfer data */
    ret = I2C_MasterTransferNonBlocking(context->base, &context->master_handle,
                                        &transfer);
    if (ret != kStatus_Success)
    {
        tx_mutex_put(&context->io_mutex);
        return TX_NOT_DONE;
    }

    /* wait for completion */
    status = tx_semaphore_get(&context->io_semaphore, TIMEOUT);
    if (status != TX_SUCCESS)
    {
        tx_mutex_put(&context->io_mutex);
        return status;
    }

    /* check the result */
    if (context->result != kStatus_Success)
    {
        PRINTF("ERR: I2C communication error, %d\r\n", context->result);
        status = TX_NOT_DONE;
        if (context->result == kStatus_I2C_Addr_Nak)
            PRINTF("ERR: the slave device did not response\r\n");
    }
    else
    {
        status = TX_SUCCESS;
    }

    tx_mutex_put(&context->io_mutex);

    return status;
}

UINT tx_init_i2c_master(tx_i2c_master_context_t *context)
{
    i2c_master_config_t config;
    UINT status = TX_SUCCESS;

    TX_MEMSET(context, 0, sizeof(tx_i2c_master_context_t));

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

    context->base = I2C_MASTER_BASE;

    I2C_MasterGetDefaultConfig(&config);
    config.baudRate_Bps = I2C_BAUDRATE;

    I2C_MasterInit(context->base, &config, I2C_MASTER_CLOCK_FREQUENCY);

    /* create a new handle for the non-blocking APIs */
    I2C_MasterTransferCreateHandle(context->base, &context->master_handle,
                                   _i2c_callback, context);

    return TX_SUCCESS;
}

UINT tx_write_i2c_master(tx_i2c_master_context_t *context, tx_i2c_request_t *request)
{
   return _tx_tranfer_i2c(context, request, kI2C_Write);
}

UINT tx_read_i2c_master(tx_i2c_master_context_t *context, tx_i2c_request_t *request)
{
   return _tx_tranfer_i2c(context, request, kI2C_Read);
}
