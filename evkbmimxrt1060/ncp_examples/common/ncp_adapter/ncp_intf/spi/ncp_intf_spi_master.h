/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#if CONFIG_NCP_SPI
#ifndef __NCP_INTF_SPI_MASTER_H__
#define __NCP_INTF_SPI_MASTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NCP_HOST_SPI_MASTER            SPI0
#define NCP_HOST_SPI_MASTER_CLK_FREQ   CLOCK_GetFlexCommClkFreq(0U)
#define NCP_HOST_SPI_MASTER_RX_CHANNEL 0
#define NCP_HOST_SPI_MASTER_TX_CHANNEL 1
#define SPI_DMA_MAX_TRANSFER_COUNT         1024

#define NCP_HOST_SPI_SSEL        kSPI_Ssel0
#define NCP_HOST_DMA             DMA0
#define NCP_HOST_MASTER_SPI_SPOL kSPI_SpolActiveAllLow

#define NCP_HOST_MASTER_TX    1
#define NCP_HOST_MASTER_RX    2

#define EXAMPLE_LPSPI_MASTER_BASEADDR              (LPSPI1)
#define EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE          (DMAMUX)
#define EXAMPLE_LPSPI_MASTER_DMA_RX_REQUEST_SOURCE kDmaRequestMuxLPSPI1Rx
#define EXAMPLE_LPSPI_MASTER_DMA_TX_REQUEST_SOURCE kDmaRequestMuxLPSPI1Tx
#define EXAMPLE_LPSPI_MASTER_DMA_BASE              (DMA0)
#define EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL        0U
#define EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL        1U

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT     (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER (kLPSPI_MasterPcs0)

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (1U)

#define LPSPI_MASTER_CLK_FREQ (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))
#define NCP_SPI_MASTER_CLOCK   500000U

#define NCP_HOST_GPIO               GPIO1
#define NCP_HOST_GPIO_NUM           1
#define NCP_HOST_GPIO_PIN_RX        16U
#define NCP_HOST_GPIO_PIN_RX_READY  17U
#define NCP_HOST_GPIO_IRQ           GPIO1_Combined_16_31_IRQn
#define NCP_HOST_GPIO_RX_MASK       0x10000
#define NCP_HOST_GPIO_RX_READY_MASK 0x20000


#define NCP_HOST_GPIO_IRQ_HANDLER GPIO1_Combined_16_31_IRQHandler

#define NCP_HOST_GPIO_IRQ_PRIO 3
#define NCP_HOST_DMA_IRQ_PRIO  4

#define MASTER_TX_ENABLE_EVENT       1 << 1
#define MASTER_RX_ENABLE_EVENT       1 << 2

/*******************************************************************************
 * API
 ******************************************************************************/
typedef enum
{
    NCP_MASTER_SPI_IDLE = 0,
    NCP_MASTER_SPI_TX,
    NCP_MASTER_SPI_RX,
    NCP_MASTER_SPI_DROP_SLAVE_TX,
    NCP_MASTER_SPI_END,
} ncp_state;

#if CONFIG_NCP_SPI_DEBUG
#define mcu_host_spi_debug(...) wmlog("spi master", ##__VA_ARGS__)
#else
#define mcu_host_spi_debug(...)
#endif
#endif /* __NCP_INTF_SPI_MASTER_H__ */
#endif /* CONFIG_NCP_SPI */