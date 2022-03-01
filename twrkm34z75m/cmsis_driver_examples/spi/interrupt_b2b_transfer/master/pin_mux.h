/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*! @brief Direction type  */
typedef enum _pin_mux_direction
{
  kPIN_MUX_DirectionInput = 0U,         /* Input direction */
  kPIN_MUX_DirectionOutput = 1U,        /* Output direction */
  kPIN_MUX_DirectionInputOrOutput = 2U  /* Input or output direction */
} pin_mux_direction_t;

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

/* PORTI6 (number 6), UART2_RX */
#define BOARD_DEBUG_UART_RX_PERIPHERAL                                     UART2   /*!< Device name: UART2 */
#define BOARD_DEBUG_UART_RX_SIGNAL                                            RX   /*!< UART2 signal: RX */
#define BOARD_DEBUG_UART_RX_PIN_NAME                                    UART2_RX   /*!< Pin name */
#define BOARD_DEBUG_UART_RX_LABEL                                     "UART2_RX"   /*!< Label */
#define BOARD_DEBUG_UART_RX_NAME                                 "DEBUG_UART_RX"   /*!< Identifier name */

/* PORTI7 (number 7), UART2_TX */
#define BOARD_DEBUG_UART_TX_PERIPHERAL                                     UART2   /*!< Device name: UART2 */
#define BOARD_DEBUG_UART_TX_SIGNAL                                            TX   /*!< UART2 signal: TX */
#define BOARD_DEBUG_UART_TX_PIN_NAME                                    UART2_TX   /*!< Pin name */
#define BOARD_DEBUG_UART_TX_LABEL                                     "UART2_TX"   /*!< Label */
#define BOARD_DEBUG_UART_TX_NAME                                 "DEBUG_UART_TX"   /*!< Identifier name */

/*!
 * @brief Calls initialization functions.
 *
 */
void BOARD_InitBootPins(void);

/*!
 * @brief
 * UART2 IrDA Select: Pad RX input PTI[6] or PTE[6] selected for RX input of UART2 and UART2 TX signal is not used
 * for modulation.
 */
 #define MISC_CTL_UART2IRSEL_NONE 0x00u
 
/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void BOARD_InitPins(void);

/* PORTG2 (number 112), J32[B9]/J9[2]/U4[1]/SPI0_PCS0 */
#define SPI0_INITPINS_FLASH_CS_PERIPHERAL                                   SPI0   /*!< Device name: SPI0 */
#define SPI0_INITPINS_FLASH_CS_SIGNAL                                       PCS0   /*!< SPI0 signal: PCS0 */
#define SPI0_INITPINS_FLASH_CS_PIN_NAME                                SPI0_PCS0   /*!< Pin name */
#define SPI0_INITPINS_FLASH_CS_LABEL             "J32[B9]/J9[2]/U4[1]/SPI0_PCS0"   /*!< Label */
#define SPI0_INITPINS_FLASH_CS_NAME                                   "FLASH_CS"   /*!< Identifier name */

/* PORTG3 (number 113), J32[B7]/J12[1]/U4[6]/SPI0_SCK */
#define SPI0_INITPINS_FLASH_CLK_PERIPHERAL                                  SPI0   /*!< Device name: SPI0 */
#define SPI0_INITPINS_FLASH_CLK_SIGNAL                                       SCK   /*!< SPI0 signal: SCK */
#define SPI0_INITPINS_FLASH_CLK_PIN_NAME                                SPI0_SCK   /*!< Pin name */
#define SPI0_INITPINS_FLASH_CLK_LABEL            "J32[B7]/J12[1]/U4[6]/SPI0_SCK"   /*!< Label */
#define SPI0_INITPINS_FLASH_CLK_NAME                                 "FLASH_CLK"   /*!< Identifier name */

/* PORTG4 (number 114), J32[B10]/J15[1]/U4[5]/SPI0_MOSI */
#define SPI0_INITPINS_FLASH_SI_PERIPHERAL                                   SPI0   /*!< Device name: SPI0 */
#define SPI0_INITPINS_FLASH_SI_SIGNAL                                       MOSI   /*!< SPI0 signal: MOSI */
#define SPI0_INITPINS_FLASH_SI_PIN_NAME                                SPI0_MOSI   /*!< Pin name */
#define SPI0_INITPINS_FLASH_SI_LABEL           "J32[B10]/J15[1]/U4[5]/SPI0_MOSI"   /*!< Label */
#define SPI0_INITPINS_FLASH_SI_NAME                                   "FLASH_SI"   /*!< Identifier name */

/* PORTG5 (number 115), J32[B11]/J13[2]/U4[2]/SPI0_MISO */
#define SPI0_INITPINS_FLASH_SO_PERIPHERAL                                   SPI0   /*!< Device name: SPI0 */
#define SPI0_INITPINS_FLASH_SO_SIGNAL                                       MISO   /*!< SPI0 signal: MISO */
#define SPI0_INITPINS_FLASH_SO_PIN_NAME                                SPI0_MISO   /*!< Pin name */
#define SPI0_INITPINS_FLASH_SO_LABEL           "J32[B11]/J13[2]/U4[2]/SPI0_MISO"   /*!< Label */
#define SPI0_INITPINS_FLASH_SO_NAME                                   "FLASH_SO"   /*!< Identifier name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI0_InitPins(void);

/* PORTG2 (number 112), J32[B9]/J9[2]/U4[1]/SPI0_PCS0 */
#define SPI0_DEINITPINS_FLASH_CS_PERIPHERAL                                 ADC0   /*!< Device name: ADC0 */
#define SPI0_DEINITPINS_FLASH_CS_SIGNAL                                       SE   /*!< ADC0 signal: SE */
#define SPI0_DEINITPINS_FLASH_CS_CHANNEL                                      11   /*!< ADC0 SE channel: 11 */
#define SPI0_DEINITPINS_FLASH_CS_PIN_NAME                              ADC0_SE11   /*!< Pin name */
#define SPI0_DEINITPINS_FLASH_CS_LABEL           "J32[B9]/J9[2]/U4[1]/SPI0_PCS0"   /*!< Label */
#define SPI0_DEINITPINS_FLASH_CS_NAME                                 "FLASH_CS"   /*!< Identifier name */

/* PORTG3 (number 113), J32[B7]/J12[1]/U4[6]/SPI0_SCK */
#define SPI0_DEINITPINS_FLASH_CLK_PERIPHERAL                                 LCD   /*!< Device name: LCD */
#define SPI0_DEINITPINS_FLASH_CLK_SIGNAL                                       P   /*!< LCD signal: P */
#define SPI0_DEINITPINS_FLASH_CLK_CHANNEL                                     10   /*!< LCD P channel: 10 */
#define SPI0_DEINITPINS_FLASH_CLK_PIN_NAME                               LCD_P10   /*!< Pin name */
#define SPI0_DEINITPINS_FLASH_CLK_LABEL          "J32[B7]/J12[1]/U4[6]/SPI0_SCK"   /*!< Label */
#define SPI0_DEINITPINS_FLASH_CLK_NAME                               "FLASH_CLK"   /*!< Identifier name */

/* PORTG4 (number 114), J32[B10]/J15[1]/U4[5]/SPI0_MOSI */
#define SPI0_DEINITPINS_FLASH_SI_PERIPHERAL                                  LCD   /*!< Device name: LCD */
#define SPI0_DEINITPINS_FLASH_SI_SIGNAL                                        P   /*!< LCD signal: P */
#define SPI0_DEINITPINS_FLASH_SI_CHANNEL                                      11   /*!< LCD P channel: 11 */
#define SPI0_DEINITPINS_FLASH_SI_PIN_NAME                                LCD_P11   /*!< Pin name */
#define SPI0_DEINITPINS_FLASH_SI_LABEL         "J32[B10]/J15[1]/U4[5]/SPI0_MOSI"   /*!< Label */
#define SPI0_DEINITPINS_FLASH_SI_NAME                                 "FLASH_SI"   /*!< Identifier name */

/* PORTG5 (number 115), J32[B11]/J13[2]/U4[2]/SPI0_MISO */
#define SPI0_DEINITPINS_FLASH_SO_PERIPHERAL                                  LCD   /*!< Device name: LCD */
#define SPI0_DEINITPINS_FLASH_SO_SIGNAL                                        P   /*!< LCD signal: P */
#define SPI0_DEINITPINS_FLASH_SO_CHANNEL                                      12   /*!< LCD P channel: 12 */
#define SPI0_DEINITPINS_FLASH_SO_PIN_NAME                                LCD_P12   /*!< Pin name */
#define SPI0_DEINITPINS_FLASH_SO_LABEL         "J32[B11]/J13[2]/U4[2]/SPI0_MISO"   /*!< Label */
#define SPI0_DEINITPINS_FLASH_SO_NAME                                 "FLASH_SO"   /*!< Identifier name */

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI0_DeinitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI1_InitPins(void);

/*!
 * @brief Configures pin routing and optionally pin electrical features.
 *
 */
void SPI1_DeinitPins(void);

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
