
#ifndef _TX_I2C_H
#define _TX_I2C_H

#include "fsl_common.h"

#if FSL_FEATURE_SOC_LPI2C_COUNT > 0
#include "fsl_lpi2c.h"
#elif FSL_FEATURE_SOC_I2C_COUNT > 0
#include "fsl_i2c.h"
#else
#error "Do not have a supported I2C peripheral."
#endif

#include "tx_api.h"

/*!
 * @brief Define the size of the buffer used to store the received data.
 */
#define I2C_SLAVE_BUFFER_SIZE   (256U)

/*!
 * @brief Define type for I2C request.
 */
typedef struct tx_i2c_request {
    uint32_t dev_address;  /*!< I2C device address on I2C bus */
    void *data;            /*!< data buffer pointer */
    uint32_t data_size;    /*!< data size */
} tx_i2c_request_t;

typedef enum {
    TX_I2C_RECEIVE  = 0,
    TX_I2C_TRANSFER = 1
} tx_i2c_state_t;

#if FSL_FEATURE_SOC_LPI2C_COUNT > 0
/*!
 * @brief Define type for the I2C driver context of the master side.
 */
typedef struct tx_i2c_master_context {
    LPI2C_Type *base;                       /*!< I2C peripheral base address. */
    lpi2c_master_handle_t master_handle;    /*!< Driver handle for master */
    status_t result;                        /*!< Non-blocking APIs return result. */
    TX_MUTEX io_mutex;                      /*!< Mutex to protect the context. */
    TX_SEMAPHORE io_semaphore;              /*!< Semaphore for syncing. */
} tx_i2c_master_context_t;

/*!
 * @brief Define type for the I2C driver context of the slave side.
 */
typedef struct tx_i2c_slave_context {
    LPI2C_Type *base;
    lpi2c_slave_handle_t slave_handle;
    UINT block_size;
    status_t result;
    tx_i2c_state_t state;
    TX_MUTEX io_mutex;   /* protect the device */
    TX_SEMAPHORE io_semaphore;
    uint8_t buffer[I2C_SLAVE_BUFFER_SIZE];
} tx_i2c_slave_context_t;

#elif FSL_FEATURE_SOC_I2C_COUNT > 0

/*!
 * @brief Define type for the I2C driver context of the master side.
 */
typedef struct tx_i2c_master_context {
    I2C_Type *base;                     /*!< I2C peripheral base address. */
    i2c_master_handle_t master_handle;  /*!< Driver handle for master */
    status_t result;                    /*!< Non-blocking APIs return result. */
    TX_MUTEX io_mutex;                  /*!< Mutex to protect the context. */
    TX_SEMAPHORE io_semaphore;          /*!< Semaphore for syncing. */
} tx_i2c_master_context_t;

/*!
 * @brief Define type for the I2C driver context of the slave side.
 */
typedef struct tx_i2c_slave_context {
    I2C_Type *base;
    i2c_slave_handle_t slave_handle;
    UINT block_size;
    status_t result;
    tx_i2c_state_t state;
    TX_MUTEX io_mutex;
    TX_SEMAPHORE io_semaphore;
    uint8_t buffer[I2C_SLAVE_BUFFER_SIZE];
} tx_i2c_slave_context_t;

#endif

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Initialize I2C driver to work as master.
 * @param context I2C driver context.
 */
UINT tx_init_i2c_master(tx_i2c_master_context_t *context);

/*!
 * @brief Define write function for I2C master.
 * @param context I2C driver context.
 * @param request Point to the data buffer information for transferring.
 */
UINT tx_write_i2c_master(tx_i2c_master_context_t *context, tx_i2c_request_t *request);

/*!
 * @brief Define read function for I2C master.
 * @param context I2C driver context.
 * @param request Point to the data buffer information for receiving data.
 */
UINT tx_read_i2c_master(tx_i2c_master_context_t *context, tx_i2c_request_t *request);

/*!
 * @brief Initialize I2C driver to work as slave.
 * @param context I2C driver context.
 * @param slave_address I2C slave device address on I2C bus.
 * @param block_size Maximum buffer size for receiving data.
 */
UINT tx_init_i2c_slave(tx_i2c_slave_context_t *context, UINT slave_address, UINT block_size);

/*!
 * @brief Define read function for I2C slave.
 * @param context I2C driver context.
 * @param request Point to the data buffer information for receiving data.
 */
UINT tx_read_i2c_slave(tx_i2c_slave_context_t *context, tx_i2c_request_t *request);

#if defined(__cplusplus)
}
#endif

#endif
