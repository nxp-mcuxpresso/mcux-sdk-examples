
#ifndef _BOARD_SETUP_H
#define _BOARD_SETUP_H

#ifdef BOARD_LPSPI_B2B_MASTER

#define EXAMPLE_LPSPI_MASTER_BASEADDR           (LPSPI1)
#define EXAMPLE_LPSPI_MASTER_IRQN               (LPSPI1_IRQn)

#define EXAMPLE_LPSPI_MASTER_PCS_FOR_INIT       (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER   (kLPSPI_MasterPcs0)

#define EXAMPLE_LPSPI_MASTER_CLOCK_FREQ     \
    (CLOCK_GetFreq(kCLOCK_Usb1PllPfd0Clk) / (EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER + 1U))

#else

#define EXAMPLE_LPSPI_SLAVE_BASEADDR            (LPSPI1)
#define EXAMPLE_LPSPI_SLAVE_IRQN                (LPSPI1_IRQn)

#define EXAMPLE_LPSPI_SLAVE_PCS_FOR_INIT        (kLPSPI_Pcs0)
#define EXAMPLE_LPSPI_SLAVE_PCS_FOR_TRANSFER    (kLPSPI_SlavePcs0)

#endif

/* Select USB1 PLL PFD0 (720 MHz) as lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpspi clock source */
#define EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER (7U)


#if defined(__cplusplus)
extern "C" {
#endif

void master_board_setup(void);
void slave_board_setup(void);

void board_led_toggle(void);

#if defined(__cplusplus)
}
#endif

#endif
