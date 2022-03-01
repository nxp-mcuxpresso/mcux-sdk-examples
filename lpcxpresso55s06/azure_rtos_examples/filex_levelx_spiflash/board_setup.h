
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

#include "board.h"
#include "fsl_spi.h"

#define EXAMPLE_SPI_MASTER      (SPI8)
#define EXAMPLE_SPI_MASTER_IRQ  (LSPI_HS_IRQn)
#define EXAMPLE_MASTER_SPI_SPOL (kSPI_SpolActiveAllLow)
#define EXAMPLE_SPI_MASTER_CLK_FREQ     CLOCK_GetHsLspiClkFreq()
#define EXAMPLE_SPI_SSEL        (kSPI_Ssel1)

#if defined(__cplusplus)
extern "C" {
#endif

void board_setup(void);

#if defined(__cplusplus)
}
#endif

#endif
