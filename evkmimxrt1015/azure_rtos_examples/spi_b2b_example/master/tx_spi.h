
#ifndef _TX_SPI_H
#define _TX_SPI_H

#include "fsl_common.h"

#if FSL_FEATURE_SOC_LPSPI_COUNT > 0
#include "fsl_lpspi.h"
#elif FSL_FEATURE_SOC_SPI_COUNT > 0
#include "fsl_spi.h"
#else
"Do not have a supported SPI peripheral."
#endif

#include "tx_api.h"

#define MAX_BYTES_PER_FRAME     (512U)

#if FSL_FEATURE_SOC_LPSPI_COUNT > 0

#define SPI_SLAVE_RETURN_OK     kStatus_Success

/*!
 * @brief Define type for the LPSPI driver context of the master side.
 */
typedef struct tx_spi_master_context {
    LPSPI_Type *base;                   /*!< LPSPI peripheral base address. */
    lpspi_master_handle_t drv_handle;   /*!< Driver handle for master. */
    status_t result;                    /*!< Non-blocking APIs return result. */
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    TX_SEMAPHORE sema_sync;             /*!< Semaphore for syncing. */
} tx_spi_master_context_t;

/*!
 * @brief Define type for the LPSPI driver context of the slave side.
 */
typedef struct tx_spi_slave_context {
    LPSPI_Type *base;                   /*!< LPSPI peripheral base address. */
    lpspi_slave_handle_t drv_handle;    /*!< Driver handle for slave. */
    status_t result;                    /*!< Non-blocking APIs return result. */
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    TX_SEMAPHORE sema_sync;             /*!< Semaphore for syncing. */
} tx_spi_slave_context_t;

#elif FSL_FEATURE_SOC_SPI_COUNT > 0

#define SPI_SLAVE_RETURN_OK     kStatus_SPI_Idle

/*!
 * @brief Define type for the SPI driver context of the master side.
 */
typedef struct tx_spi_master_context {
    SPI_Type *base;                   /*!< LPSPI peripheral base address. */
    spi_master_handle_t drv_handle;   /*!< Driver handle for master. */
    status_t result;                    /*!< Non-blocking APIs return result. */
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    TX_SEMAPHORE sema_sync;             /*!< Semaphore for syncing. */
} tx_spi_master_context_t;

/*!
 * @brief Define type for the SPI driver context of the slave side.
 */
typedef struct tx_spi_slave_context {
    SPI_Type *base;                   /*!< LPSPI peripheral base address. */
    spi_slave_handle_t drv_handle;    /*!< Driver handle for slave. */
    status_t result;                    /*!< Non-blocking APIs return result. */
    TX_MUTEX lock;                      /*!< Mutex to protect the context. */
    TX_SEMAPHORE sema_sync;             /*!< Semaphore for syncing. */
} tx_spi_slave_context_t;

#endif

#if defined(__cplusplus)
extern "C" {
#endif

UINT tx_init_spi_master(tx_spi_master_context_t *context);

UINT tx_spi_master_read(tx_spi_master_context_t *context,
                        uint8_t *rx_buffer, uint32_t data_size);

UINT tx_spi_master_write(tx_spi_master_context_t *context,
                         uint8_t *tx_buffer, uint32_t data_size);

UINT tx_spi_master_transfer(tx_spi_master_context_t *context,
                            uint8_t *tx_buffer, uint8_t *rx_buffer,
                            uint32_t data_size);

UINT tx_init_spi_slave(tx_spi_slave_context_t *context);

UINT tx_spi_slave_transfer(tx_spi_slave_context_t *context,
                           uint8_t *tx_buffer, uint8_t *rx_buffer,
                           uint32_t data_size);

#if defined(__cplusplus)
}
#endif

#endif
