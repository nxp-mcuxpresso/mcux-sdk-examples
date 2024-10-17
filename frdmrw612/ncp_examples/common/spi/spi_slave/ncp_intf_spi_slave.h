/** @file ncp_intf_spi_slave.h
 *
 *  @brief main file
 *
 *  Copyright 2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef __NCP_INTF_SPI_SLAVE_H__
#define __NCP_INTF_SPI_SLAVE_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef CONFIG_NCP_SPI_DEBUG
#define CONFIG_NCP_SPI_DEBUG 0
#endif
#define NCP_SPI_SLAVE        SPI0
#define NCP_SPI_SLAVE_DMA              DMA0
#define NCP_SPI_SLAVE_DMA_RX_CHANNEL 0
#define NCP_SPI_SLAVE_DMA_TX_CHANNEL 1
#define NCP_SPI_SLAVE_SPOL   kSPI_SpolActiveAllLow

#define NCP_SPI_GPIO_TX        27
/* gpio11, port0 */
#define NCP_SPI_GPIO_RX_READY  11

#define NCP_SPI_SLAVE_GPIO_TX_MASK 0x8000000
#define NCP_SPI_SLAVE_GPIO_RX_READY_MASK 0x800

#define NCP_HOST_GPIO_INTA_IRQHandler GPIO_INTA_DriverIRQHandler
#define SLAVE_TX_ENABLE_EVENT       1 << 1
#define SLAVE_RX_ENABLE_EVENT       1 << 2
#define INTERRUPT_TYPE              kGPIO_InterruptA

#define NCP_SPI_TX          1 << 0

#define SPI_GPIO_ISR_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define SPI_DMA_ISR_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)

typedef enum
{
    NCP_SLAVE_SPI_IDLE = 0,
    NCP_SLAVE_SPI_TX,
    NCP_SLAVE_SPI_RX,
    NCP_SLAVE_SPI_END,
} ncp__slave_state;

#if CONFIG_NCP_SPI_DEBUG
#define ncp_dev_spi(...) ncplog("NCP", ##__VA_ARGS__)
#else
#define ncp_dev_spi(...)
#endif
/*******************************************************************************
 * API
 ******************************************************************************/
int ncp_spi_txrx_is_finish(void);
#endif /* __NCP_INTF_SPI_SLAVE_H__ */
