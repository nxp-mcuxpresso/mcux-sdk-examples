
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

#include "board.h"

#define EXAMPLE_SPI_MASTER      (SPI7)
#define EXAMPLE_SPI_MASTER_IRQ  (FLEXCOMM7_IRQn)
#define EXAMPLE_MASTER_SPI_SPOL (kSPI_SpolActiveAllLow)
#define EXAMPLE_SPI_MASTER_CLK_FREQ     CLOCK_GetFlexCommClkFreq(7U)

#define EXAMPLE_SPI_SLAVE       (SPI2)
#define EXAMPLE_SPI_SLAVE_IRQ   (FLEXCOMM2_IRQn)
#define EXAMPLE_SPI_SSEL        (kSPI_Ssel1)
#define EXAMPLE_SLAVE_SPI_SPOL  (kSPI_SpolActiveAllLow)

#if defined(__cplusplus)
extern "C" {
#endif

void board_setup(void);

#if defined(__cplusplus)
}
#endif

#endif
