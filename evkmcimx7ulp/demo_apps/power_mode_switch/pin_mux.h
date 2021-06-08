/*
 * Copyright 2020 NXP.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/* PTA25 (number AG20), A7_POW_EN# */
/* Routed pin properties */
#define BOARD_A7_POW_EN_PERIPHERAL                                           PTA   /*!< Peripheral name */
#define BOARD_A7_POW_EN_SIGNAL                                              port   /*!< Signal name */
#define BOARD_A7_POW_EN_CHANNEL                                               25   /*!< Signal channel */
#define BOARD_A7_POW_EN_PIN_NAME                                           PTA25   /*!< Routed pin name */
#define BOARD_A7_POW_EN_PIN_FUNCTION_ID                       IOMUXC_PTA25_PTA25   /*!< Pin function id */
#define BOARD_A7_POW_EN_LABEL                                       "A7_POW_EN#"   /*!< Label */
#define BOARD_A7_POW_EN_NAME                                         "A7_POW_EN"   /*!< Identifier */

/* PTA19 (number AB19), UART0_RX */
/* Routed pin properties */
#define BOARD_UART0_RX_PERIPHERAL                                        LPUART0   /*!< Peripheral name */
#define BOARD_UART0_RX_SIGNAL                                          lpuart_rx   /*!< Signal name */
#define BOARD_UART0_RX_PIN_NAME                                            PTA19   /*!< Routed pin name */
#define BOARD_UART0_RX_PIN_FUNCTION_ID                   IOMUXC_PTA19_LPUART0_RX   /*!< Pin function id */
#define BOARD_UART0_RX_LABEL                                          "UART0_RX"   /*!< Label */
#define BOARD_UART0_RX_NAME                                           "UART0_RX"   /*!< Identifier */

/* PTA18 (number AC19), UART0_TX */
/* Routed pin properties */
#define BOARD_UART0_TX_PERIPHERAL                                        LPUART0   /*!< Peripheral name */
#define BOARD_UART0_TX_SIGNAL                                          lpuart_tx   /*!< Signal name */
#define BOARD_UART0_TX_PIN_NAME                                            PTA18   /*!< Routed pin name */
#define BOARD_UART0_TX_PIN_FUNCTION_ID                   IOMUXC_PTA18_LPUART0_TX   /*!< Pin function id */
#define BOARD_UART0_TX_LABEL                                          "UART0_TX"   /*!< Label */
#define BOARD_UART0_TX_NAME                                           "UART0_TX"   /*!< Identifier */

/* PTA4 (number AD15), I2S0_MCLK */
/* Routed pin properties */
#define BOARD_I2S0_MCLK_PERIPHERAL                                          I2S0   /*!< Peripheral name */
#define BOARD_I2S0_MCLK_SIGNAL                                          i2s_mclk   /*!< Signal name */
#define BOARD_I2S0_MCLK_PIN_NAME                                            PTA4   /*!< Routed pin name */
#define BOARD_I2S0_MCLK_PIN_FUNCTION_ID                    IOMUXC_PTA4_I2S0_MCLK   /*!< Pin function id */
#define BOARD_I2S0_MCLK_LABEL                                        "I2S0_MCLK"   /*!< Label */
#define BOARD_I2S0_MCLK_NAME                                         "I2S0_MCLK"   /*!< Identifier */

/* PTA2 (number AG14), I2S0_RXD0 */
/* Routed pin properties */
#define BOARD_I2S0_RXD0_PERIPHERAL                                          I2S0   /*!< Peripheral name */
#define BOARD_I2S0_RXD0_SIGNAL                                           i2s_rxd   /*!< Signal name */
#define BOARD_I2S0_RXD0_CHANNEL                                                0   /*!< Signal channel */
#define BOARD_I2S0_RXD0_PIN_NAME                                            PTA2   /*!< Routed pin name */
#define BOARD_I2S0_RXD0_PIN_FUNCTION_ID                    IOMUXC_PTA2_I2S0_RXD0   /*!< Pin function id */
#define BOARD_I2S0_RXD0_LABEL                                        "I2S0_RXD0"   /*!< Label */
#define BOARD_I2S0_RXD0_NAME                                         "I2S0_RXD0"   /*!< Identifier */

/* PTA5 (number AC15), I2S0_TX_BCLK */
/* Routed pin properties */
#define BOARD_I2S0_TX_BCLK_PERIPHERAL                                       I2S0   /*!< Peripheral name */
#define BOARD_I2S0_TX_BCLK_SIGNAL                                    i2s_tx_bclk   /*!< Signal name */
#define BOARD_I2S0_TX_BCLK_PIN_NAME                                         PTA5   /*!< Routed pin name */
#define BOARD_I2S0_TX_BCLK_PIN_FUNCTION_ID              IOMUXC_PTA5_I2S0_TX_BCLK   /*!< Pin function id */
#define BOARD_I2S0_TX_BCLK_LABEL                                  "I2S0_TX_BCLK"   /*!< Label */
#define BOARD_I2S0_TX_BCLK_NAME                                   "I2S0_TX_BCLK"   /*!< Identifier */

/* PTA6 (number AB15), I2S0_TX_FS */
/* Routed pin properties */
#define BOARD_I2S0_TX_FS_PERIPHERAL                                         I2S0   /*!< Peripheral name */
#define BOARD_I2S0_TX_FS_SIGNAL                                        i2s_tx_fs   /*!< Signal name */
#define BOARD_I2S0_TX_FS_PIN_NAME                                           PTA6   /*!< Routed pin name */
#define BOARD_I2S0_TX_FS_PIN_FUNCTION_ID                  IOMUXC_PTA6_I2S0_TX_FS   /*!< Pin function id */
#define BOARD_I2S0_TX_FS_LABEL                                      "I2S0_TX_FS"   /*!< Label */
#define BOARD_I2S0_TX_FS_NAME                                       "I2S0_TX_FS"   /*!< Identifier */

/* PTA7 (number AD14), I2S0_TXD0 */
/* Routed pin properties */
#define BOARD_I2S0_TXD0_PERIPHERAL                                          I2S0   /*!< Peripheral name */
#define BOARD_I2S0_TXD0_SIGNAL                                           i2s_txd   /*!< Signal name */
#define BOARD_I2S0_TXD0_CHANNEL                                                0   /*!< Signal channel */
#define BOARD_I2S0_TXD0_PIN_NAME                                            PTA7   /*!< Routed pin name */
#define BOARD_I2S0_TXD0_PIN_FUNCTION_ID                    IOMUXC_PTA7_I2S0_TXD0   /*!< Pin function id */
#define BOARD_I2S0_TXD0_LABEL                                        "I2S0_TXD0"   /*!< Label */
#define BOARD_I2S0_TXD0_NAME                                         "I2S0_TXD0"   /*!< Identifier */

/* PTA3 (number AF14), VOL+ */
/* Routed pin properties */
#define BOARD_VOL_UP_PERIPHERAL                                              PTA   /*!< Peripheral name */
#define BOARD_VOL_UP_SIGNAL                                                 port   /*!< Signal name */
#define BOARD_VOL_UP_CHANNEL                                                   3   /*!< Signal channel */
#define BOARD_VOL_UP_PIN_NAME                                               PTA3   /*!< Routed pin name */
#define BOARD_VOL_UP_PIN_FUNCTION_ID                            IOMUXC_PTA3_PTA3   /*!< Pin function id */
#define BOARD_VOL_UP_LABEL                                                "VOL+"   /*!< Label */
#define BOARD_VOL_UP_NAME                                               "VOL_UP"   /*!< Identifier */

/* PTA13 (number AF16), VOL- */
/* Routed pin properties */
#define BOARD_VOL_DOWN_PERIPHERAL                                            PTA   /*!< Peripheral name */
#define BOARD_VOL_DOWN_SIGNAL                                               port   /*!< Signal name */
#define BOARD_VOL_DOWN_CHANNEL                                                13   /*!< Signal channel */
#define BOARD_VOL_DOWN_PIN_NAME                                            PTA13   /*!< Routed pin name */
#define BOARD_VOL_DOWN_PIN_FUNCTION_ID                        IOMUXC_PTA13_PTA13   /*!< Pin function id */
#define BOARD_VOL_DOWN_LABEL                                              "VOL-"   /*!< Label */
#define BOARD_VOL_DOWN_NAME                                           "VOL_DOWN"   /*!< Identifier */

/* PTA14 (number AF17), WL_REG_ON */
/* Routed pin properties */
#define BOARD_WL_REG_ON_PERIPHERAL                                           PTA   /*!< Peripheral name */
#define BOARD_WL_REG_ON_SIGNAL                                              port   /*!< Signal name */
#define BOARD_WL_REG_ON_CHANNEL                                               14   /*!< Signal channel */
#define BOARD_WL_REG_ON_PIN_NAME                                           PTA14   /*!< Routed pin name */
#define BOARD_WL_REG_ON_PIN_FUNCTION_ID                       IOMUXC_PTA14_PTA14   /*!< Pin function id */
#define BOARD_WL_REG_ON_LABEL                                        "WL_REG_ON"   /*!< Label */
#define BOARD_WL_REG_ON_NAME                                         "WL_REG_ON"   /*!< Identifier */

/* PTA15 (number AF18), BT_REG_ON */
/* Routed pin properties */
#define BOARD_BT_REG_ON_PERIPHERAL                                           PTA   /*!< Peripheral name */
#define BOARD_BT_REG_ON_SIGNAL                                              port   /*!< Signal name */
#define BOARD_BT_REG_ON_CHANNEL                                               15   /*!< Signal channel */
#define BOARD_BT_REG_ON_PIN_NAME                                           PTA15   /*!< Routed pin name */
#define BOARD_BT_REG_ON_PIN_FUNCTION_ID                       IOMUXC_PTA15_PTA15   /*!< Pin function id */
#define BOARD_BT_REG_ON_LABEL                                        "BT_REG_ON"   /*!< Label */
#define BOARD_BT_REG_ON_NAME                                         "BT_REG_ON"   /*!< Identifier */

/* PTA31 (number AF24), WL_HOST_WAKE */
/* Routed pin properties */
#define BOARD_WL_HOST_WAKE_PERIPHERAL                                        PTA   /*!< Peripheral name */
#define BOARD_WL_HOST_WAKE_SIGNAL                                           port   /*!< Signal name */
#define BOARD_WL_HOST_WAKE_CHANNEL                                            31   /*!< Signal channel */
#define BOARD_WL_HOST_WAKE_PIN_NAME                                        PTA31   /*!< Routed pin name */
#define BOARD_WL_HOST_WAKE_PIN_FUNCTION_ID                    IOMUXC_PTA31_PTA31   /*!< Pin function id */
#define BOARD_WL_HOST_WAKE_LABEL                                  "WL_HOST_WAKE"   /*!< Label */
#define BOARD_WL_HOST_WAKE_NAME                                   "WL_HOST_WAKE"   /*!< Identifier */

/* PTB6 (number AF5), DDR_SW_EN# */
/* Routed pin properties */
#define BOARD_DDR_SW_EN_PERIPHERAL                                           PTB   /*!< Peripheral name */
#define BOARD_DDR_SW_EN_SIGNAL                                              port   /*!< Signal name */
#define BOARD_DDR_SW_EN_CHANNEL                                                6   /*!< Signal channel */
#define BOARD_DDR_SW_EN_PIN_NAME                                            PTB6   /*!< Routed pin name */
#define BOARD_DDR_SW_EN_PIN_FUNCTION_ID                         IOMUXC_PTB6_PTB6   /*!< Pin function id */
#define BOARD_DDR_SW_EN_LABEL                                       "DDR_SW_EN#"   /*!< Label */
#define BOARD_DDR_SW_EN_NAME                                         "DDR_SW_EN"   /*!< Identifier */

/* PTB7 (number AF6), BT_HOST_WAKE */
/* Routed pin properties */
#define BOARD_BT_HOST_WAKE_PERIPHERAL                                        PTB   /*!< Peripheral name */
#define BOARD_BT_HOST_WAKE_SIGNAL                                           port   /*!< Signal name */
#define BOARD_BT_HOST_WAKE_CHANNEL                                             7   /*!< Signal channel */
#define BOARD_BT_HOST_WAKE_PIN_NAME                                         PTB7   /*!< Routed pin name */
#define BOARD_BT_HOST_WAKE_PIN_FUNCTION_ID                      IOMUXC_PTB7_PTB7   /*!< Pin function id */
#define BOARD_BT_HOST_WAKE_LABEL                                  "BT_HOST_WAKE"   /*!< Label */
#define BOARD_BT_HOST_WAKE_NAME                                   "BT_HOST_WAKE"   /*!< Identifier */

/* PTA16 (number AG18), I2C0_SCL */
/* Routed pin properties */
#define BOARD_I2C0_SCL_PERIPHERAL                                         LPI2C0   /*!< Peripheral name */
#define BOARD_I2C0_SCL_SIGNAL                                          lpi2c_scl   /*!< Signal name */
#define BOARD_I2C0_SCL_PIN_NAME                                            PTA16   /*!< Routed pin name */
#define BOARD_I2C0_SCL_PIN_FUNCTION_ID                   IOMUXC_PTA16_LPI2C0_SCL   /*!< Pin function id */
#define BOARD_I2C0_SCL_LABEL                                          "I2C0_SCL"   /*!< Label */
#define BOARD_I2C0_SCL_NAME                                           "I2C0_SCL"   /*!< Identifier */

/* PTA17 (number AD19), I2C0_SDA */
/* Routed pin properties */
#define BOARD_I2C0_SDA_PERIPHERAL                                         LPI2C0   /*!< Peripheral name */
#define BOARD_I2C0_SDA_SIGNAL                                          lpi2c_sda   /*!< Signal name */
#define BOARD_I2C0_SDA_PIN_NAME                                            PTA17   /*!< Routed pin name */
#define BOARD_I2C0_SDA_PIN_FUNCTION_ID                   IOMUXC_PTA17_LPI2C0_SDA   /*!< Pin function id */
#define BOARD_I2C0_SDA_LABEL                                          "I2C0_SDA"   /*!< Label */
#define BOARD_I2C0_SDA_NAME                                           "I2C0_SDA"   /*!< Identifier */

/* PTB12 (number AC8), I2C3_SCL */
/* Routed pin properties */
#define BOARD_I2C3_SCL_PERIPHERAL                                         LPI2C3   /*!< Peripheral name */
#define BOARD_I2C3_SCL_SIGNAL                                          lpi2c_scl   /*!< Signal name */
#define BOARD_I2C3_SCL_PIN_NAME                                            PTB12   /*!< Routed pin name */
#define BOARD_I2C3_SCL_PIN_FUNCTION_ID                   IOMUXC_PTB12_LPI2C3_SCL   /*!< Pin function id */
#define BOARD_I2C3_SCL_LABEL                                          "I2C3_SCL"   /*!< Label */
#define BOARD_I2C3_SCL_NAME                                           "I2C3_SCL"   /*!< Identifier */

/* PTB13 (number AD8), I2C3_SDA */
/* Routed pin properties */
#define BOARD_I2C3_SDA_PERIPHERAL                                         LPI2C3   /*!< Peripheral name */
#define BOARD_I2C3_SDA_SIGNAL                                          lpi2c_sda   /*!< Signal name */
#define BOARD_I2C3_SDA_PIN_NAME                                            PTB13   /*!< Routed pin name */
#define BOARD_I2C3_SDA_PIN_FUNCTION_ID                   IOMUXC_PTB13_LPI2C3_SDA   /*!< Pin function id */
#define BOARD_I2C3_SDA_LABEL                                          "I2C3_SDA"   /*!< Label */
#define BOARD_I2C3_SDA_NAME                                           "I2C3_SDA"   /*!< Identifier */

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

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);                                 /*!< Function assigned for the core: Cortex-M4[cm4] */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_I2C_ConfigurePins(void);                        /*!< Function assigned for the core: Cortex-M4[cm4] */

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
