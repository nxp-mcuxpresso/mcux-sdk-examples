/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/* GPIO_AD_24 (coord L13), LPUART1_TXD/J31[2] */
/* Routed pin properties */
#define BOARD_INITPINS_LPUART1_TXD_PERIPHERAL                            LPUART1   /*!< Peripheral name */
#define BOARD_INITPINS_LPUART1_TXD_SIGNAL                                    TXD   /*!< Signal name */

/* GPIO_AD_25 (coord M15), LPUART1_RXD/J32[2] */
/* Routed pin properties */
#define BOARD_INITPINS_LPUART1_RXD_PERIPHERAL                            LPUART1   /*!< Peripheral name */
#define BOARD_INITPINS_LPUART1_RXD_SIGNAL                                    RXD   /*!< Signal name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);                    /* Function assigned for the Cortex-M7F */

#define BOARD_INITENETPINS_IOMUXC_GPR_GPR42_GPIO_MUX3_GPIO_SEL_LOW_MASK 0x0800U /*!< GPIO3 and CM7_GPIO3 share same IO MUX function, GPIO_MUX3 selects one GPIO function: affected bits mask */

/* GPIO_LPSR_12 (coord U5), JTAG_TDI/J1[5]/J58[8]/LPSPI6_SDI/J26[5]/DMIC_DATA3/U45[5]/ENET_RST/U7[32] */
/* Routed pin properties */
#define BOARD_INITENETPINS_PHY_RESET_PERIPHERAL                           GPIO12   /*!< Peripheral name */
#define BOARD_INITENETPINS_PHY_RESET_SIGNAL                              gpio_io   /*!< Signal name */
#define BOARD_INITENETPINS_PHY_RESET_CHANNEL                                 12U   /*!< Signal channel */

/* Symbols to be used with GPIO driver */
#define BOARD_INITENETPINS_PHY_RESET_GPIO                                 GPIO12   /*!< GPIO peripheral base pointer */
#define BOARD_INITENETPINS_PHY_RESET_GPIO_PIN                                12U   /*!< GPIO pin number */
#define BOARD_INITENETPINS_PHY_RESET_GPIO_PIN_MASK                   (1U << 12U)   /*!< GPIO pin mask */

/* GPIO_AD_12 (coord P17), SPDIF_LOCK/J26[6]/ENET_INT/U7[21] */
/* Routed pin properties */
#define BOARD_INITENETPINS_PHY_INTR_PERIPHERAL                             GPIO3   /*!< Peripheral name */
#define BOARD_INITENETPINS_PHY_INTR_SIGNAL                           gpio_mux_io   /*!< Signal name */
#define BOARD_INITENETPINS_PHY_INTR_CHANNEL                                  11U   /*!< Signal channel */

/* GPIO_AD_32 (coord K16), ENET_MDC/U7[12]/SD1_CD_B/J15[9] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_MDC_PERIPHERAL                              ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_MDC_SIGNAL                              enet_mdc   /*!< Signal name */

/* GPIO_AD_33 (coord H17), ENET_MDIO/U7[11] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_MDIO_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_MDIO_SIGNAL                            enet_mdio   /*!< Signal name */

/* GPIO_DISP_B2_05 (coord C9), ENET_TX_REF_CLK/U7[9]/BT_CFG[11] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_TX_REF_CLK_PERIPHERAL                       ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_TX_REF_CLK_SIGNAL                   enet_ref_clk   /*!< Signal name */

/* GPIO_DISP_B2_04 (coord C7), ENET_TXEN/U7[23]/BT_CFG[10] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_TXEN_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_TXEN_SIGNAL                           enet_tx_en   /*!< Signal name */

/* GPIO_DISP_B2_02 (coord E9), ENET_TXD0/U7[24]/BT_CFG[8] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_TXD0_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_TXD0_SIGNAL                           enet_tdata   /*!< Signal name */
#define BOARD_INITENETPINS_ENET_TXD0_CHANNEL                                  0U   /*!< Signal channel */

/* GPIO_DISP_B2_03 (coord D7), ENET_TXD1/U7[25]/BT_CFG[9] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_TXD1_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_TXD1_SIGNAL                           enet_tdata   /*!< Signal name */
#define BOARD_INITENETPINS_ENET_TXD1_CHANNEL                                  1U   /*!< Signal channel */

/* GPIO_DISP_B2_08 (coord B5), ENET_CRS_DV/U7[18] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_CRS_DV_PERIPHERAL                           ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_CRS_DV_SIGNAL                         enet_rx_en   /*!< Signal name */

/* GPIO_DISP_B2_09 (coord D8), ENET_RXER/U7[20] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_RXER_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_RXER_SIGNAL                           enet_rx_er   /*!< Signal name */

/* GPIO_DISP_B2_06 (coord C6), ENET_RXD0/U7[16] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_RXD0_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_RXD0_SIGNAL                           enet_rdata   /*!< Signal name */
#define BOARD_INITENETPINS_ENET_RXD0_CHANNEL                                  0U   /*!< Signal channel */

/* GPIO_DISP_B2_07 (coord D6), ENET_RXD1/U7[15] */
/* Routed pin properties */
#define BOARD_INITENETPINS_ENET_RXD1_PERIPHERAL                             ENET   /*!< Peripheral name */
#define BOARD_INITENETPINS_ENET_RXD1_SIGNAL                           enet_rdata   /*!< Signal name */
#define BOARD_INITENETPINS_ENET_RXD1_CHANNEL                                  1U   /*!< Signal channel */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitENETPins(void);                /* Function assigned for the Cortex-M7F */

#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
