/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "clock_config.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "ISI_QMC_DGC"

/*! @Brief Debug console configuration */
#ifndef DEBUG_CONSOLE_UART_INDEX
#define DEBUG_CONSOLE_UART_INDEX 8 /* EVK or EVB, if ISI_QMC  used this must not be used, We expect LPUART8 to be used with DEBUG CONSOLE */
#endif

/* The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_CLK_FREQ 80000000

#if DEBUG_CONSOLE_UART_INDEX == 1
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART1
#define BOARD_DEBUG_UART_INSTANCE 1U
#define BOARD_UART_IRQ            LPUART1_IRQn
#define BOARD_UART_IRQ_HANDLER    LPUART1_IRQHandler
#elif DEBUG_CONSOLE_UART_INDEX == 6
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART6
#define BOARD_DEBUG_UART_INSTANCE 6U
#define BOARD_UART_IRQ            LPUART6_IRQn
#define BOARD_UART_IRQ_HANDLER    LPUART6_IRQHandler
#elif DEBUG_CONSOLE_UART_INDEX == 8
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART8
#define BOARD_DEBUG_UART_INSTANCE 8U
#define BOARD_UART_IRQ            LPUART8_IRQn
#define BOARD_UART_IRQ_HANDLER    LPUART8_IRQHandler
#else
#error "Unsupported UART"
#endif

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE (115200U)
#endif /* BOARD_DEBUG_UART_BAUDRATE */

/*! @Brief Freemaster serial comm config*/
#ifndef FMSTR_UART_INDEX
#define FMSTR_UART_INDEX 6 /* EVK or EVB, if ISI_QMC  used this must not be used, We expect LPUART6 to be used with FMSTR */
#endif

/* The UART to use for freemaster. */
#define BOARD_FMSTR_UART_TYPE     kSerialPort_Uart
#define BOARD_FMSTR_UART_CLK_FREQ 80000000

#if FMSTR_UART_INDEX == 1
#define BOARD_FMSTR_UART_BASEADDR (uint32_t) LPUART1
#define BOARD_FMSTR_UART_INSTANCE 1U
#define BOARD_FMSTR_UART_IRQ            LPUART1_IRQn
#define BOARD_FMSTR_UART_IRQ_HANDLER    LPUART1_IRQHandler
#elif FMSTR_UART_INDEX == 6
#define BOARD_FMSTR_UART_BASEADDR (uint32_t) LPUART6
#define BOARD_FMSTR_UART_INSTANCE 6U
#define BOARD_FMSTR_UART_IRQ            LPUART6_IRQn
#define BOARD_FMSTR_UART_IRQ_HANDLER    LPUART6_IRQHandler
#elif FMSTR_UART_INDEX == 8
#define BOARD_FMSTR_UART_BASEADDR (uint32_t) LPUART8
#define BOARD_FMSTR_UART_INSTANCE 8U
#define BOARD_FMSTR_UART_IRQ            LPUART8_IRQn
#define BOARD_FMSTR_UART_IRQ_HANDLER    LPUART8_IRQHandler
#else
#error "FMSTR - Unsupported UART"
#endif

#ifndef BOARD_FMSTR_UART_BAUDRATE
#define BOARD_FMSTR_UART_BAUDRATE (115200U)
#endif /* BOARD_FMSTR_UART_BAUDRATE */

/* Definitions for eRPC MU transport layer */
#if defined(FSL_FEATURE_MU_SIDE_A)
#define MU_BASE        MUA
#define MU_IRQ         MUA_IRQn
#define MU_IRQ_HANDLER MUA_IRQHandler
#endif
#if defined(FSL_FEATURE_MU_SIDE_B)
#define MU_BASE        MUB
#define MU_IRQ         MUB_IRQn
#define MU_IRQ_HANDLER MUB_IRQHandler
#endif
#define MU_IRQ_PRIORITY (2)

/*! @brief The USER_LEDs used for board */
#define LOGIC_LED_ON  (0U)
#define LOGIC_LED_OFF (1U)

#ifndef BOARD_USER_LED1_GPIO
#define BOARD_USER_LED1_GPIO GPIO5
#endif
#ifndef BOARD_USER_LED1_GPIO_PIN
#define BOARD_USER_LED1_GPIO_PIN (5U)
#endif

#ifndef BOARD_USER_LED2_GPIO
#define BOARD_USER_LED2_GPIO GPIO5
#endif
#ifndef BOARD_USER_LED2_GPIO_PIN
#define BOARD_USER_LED2_GPIO_PIN (6U)
#endif

#ifndef BOARD_USER_LED3_GPIO
#define BOARD_USER_LED3_GPIO GPIO5
#endif
#ifndef BOARD_USER_LED3_GPIO_PIN
#define BOARD_USER_LED3_GPIO_PIN (7U)
#endif

#ifndef BOARD_USER_LED4_GPIO
#define BOARD_USER_LED4_GPIO GPIO5
#endif
#ifndef BOARD_USER_LED4_GPIO_PIN
#define BOARD_USER_LED4_GPIO_PIN (8U)
#endif

#define USER_LED1_OFF()\
    GPIO_PortClear(BOARD_USER_LED1_GPIO, 1U << BOARD_USER_LED1_GPIO_PIN) 	/*!< Turn off target USER_LED */
#define USER_LED1_ON()\
	GPIO_PortSet(BOARD_USER_LED1_GPIO, 1U << BOARD_USER_LED1_GPIO_PIN) 		/*!<Turn on target USER_LED*/
#define USER_LED1_TOGGLE()\
	GPIO_PortToggle(BOARD_USER_LED1_GPIO, 1U << BOARD_USER_LED1_GPIO_PIN) 	/*!< Toggle target USER_LED */

#define USER_LED2_OFF() \
    GPIO_PortClear(BOARD_USER_LED2_GPIO, 1U << BOARD_USER_LED2_GPIO_PIN)	/*!< Turn off target USER_LED */
#define USER_LED2_ON()\
	GPIO_PortSet(BOARD_USER_LED2_GPIO, 1U << BOARD_USER_LED2_GPIO_PIN) 		/*!<Turn on target USER_LED*/
#define USER_LED2_TOGGLE()\
	GPIO_PortToggle(BOARD_USER_LED2_GPIO, 1U << BOARD_USER_LED2_GPIO_PIN) 	/*!< Toggle target USER_LED */

#define USER_LED3_OFF()\
    GPIO_PortClear(BOARD_USER_LED3_GPIO, 1U << BOARD_USER_LED3_GPIO_PIN)	/*!< Turn off target USER_LED */
#define USER_LED3_ON()\
	GPIO_PortSet(BOARD_USER_LED3_GPIO, 1U << BOARD_USER_LED3_GPIO_PIN) 		/*!<Turn on target USER_LED*/
#define USER_LED3_TOGGLE()\
	GPIO_PortToggle(BOARD_USER_LED3_GPIO, 1U << BOARD_USER_LED3_GPIO_PIN) 	/*!< Toggle target USER_LED */

#define USER_LED4_OFF()\
    GPIO_PortClear(BOARD_USER_LED4_GPIO, 1U << BOARD_USER_LED4_GPIO_PIN)	/*!< Turn off target USER_LED */
#define USER_LED4_ON()\
	GPIO_PortSet(BOARD_USER_LED4_GPIO, 1U << BOARD_USER_LED4_GPIO_PIN) 		/*!<Turn on target USER_LED*/
#define USER_LED4_TOGGLE()\
	GPIO_PortToggle(BOARD_USER_LED4_GPIO, 1U << BOARD_USER_LED4_GPIO_PIN) 	/*!< Toggle target USER_LED */

/*! @brief Define the port interrupt number for the board switches */
#ifndef BOARD_USER_BUTTON1_GPIO
#define BOARD_USER_BUTTON1_GPIO GPIO6
#endif
#ifndef BOARD_USER_BUTTON1_GPIO_PIN
#define BOARD_USER_BUTTON1_GPIO_PIN (6U)
#endif
#define BOARD_USER_BUTTON1_IRQ         GPIO6_Combined_0_15_IRQn
#define BOARD_USER_BUTTON1_IRQ_HANDLER GPIO6_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON1_NAME        "SW1"

#ifndef BOARD_USER_BUTTON2_GPIO
#define BOARD_USER_BUTTON2_GPIO GPIO6
#endif
#ifndef BOARD_USER_BUTTON2_GPIO_PIN
#define BOARD_USER_BUTTON2_GPIO_PIN (7U)
#endif
#define BOARD_USER_BUTTON2_IRQ         GPIO6_Combined_0_15_IRQn
#define BOARD_USER_BUTTON2_IRQ_HANDLER GPIO6_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON2_NAME        "SW2"

#ifndef BOARD_USER_BUTTON3_GPIO
#define BOARD_USER_BUTTON3_GPIO GPIO6
#endif
#ifndef BOARD_USER_BUTTON3_GPIO_PIN
#define BOARD_USER_BUTTON3_GPIO_PIN (8U)
#endif
#define BOARD_USER_BUTTON3_IRQ         GPIO6_Combined_0_15_IRQn
#define BOARD_USER_BUTTON3_IRQ_HANDLER GPIO6_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON3_NAME        "SW3"

#ifndef BOARD_USER_BUTTON4_GPIO
#define BOARD_USER_BUTTON4_GPIO GPIO6
#endif
#ifndef BOARD_USER_BUTTON4_GPIO_PIN
#define BOARD_USER_BUTTON4_GPIO_PIN (9U)
#endif
#define BOARD_USER_BUTTON4_IRQ         GPIO6_Combined_0_15_IRQn
#define BOARD_USER_BUTTON4_IRQ_HANDLER GPIO6_Combined_0_15_IRQHandler
#define BOARD_USER_BUTTON4_NAME        "SW4"

/*! @brief The DIGITAL OUTPUTs used for board */
#define LOGIC_DIG_OUT_ON  (0U)
#define LOGIC_DIG_OUT_OFF (1U)

#ifndef BOARD_DIG_OUT_0_GPIO
#define BOARD_DIG_OUT_0_GPIO GPIO5
#endif
#ifndef BOARD_DIG_OUT_0_GPIO_PIN
#define BOARD_DIG_OUT_0_GPIO_PIN (1U)
#endif
#ifndef BOARD_DIG_OUT_1_GPIO
#define BOARD_DIG_OUT_1_GPIO GPIO5
#endif
#ifndef BOARD_DIG_OUT_1_GPIO_PIN
#define BOARD_DIG_OUT_1_GPIO_PIN (2U)
#endif
#ifndef BOARD_DIG_OUT_2_GPIO
#define BOARD_DIG_OUT_2_GPIO GPIO5
#endif
#ifndef BOARD_DIG_OUT_2_GPIO_PIN
#define BOARD_DIG_OUT_2_GPIO_PIN (3U)
#endif
#ifndef BOARD_DIG_OUT_3_GPIO
#define BOARD_DIG_OUT_3_GPIO GPIO5
#endif
#ifndef BOARD_DIG_OUT_3_GPIO_PIN
#define BOARD_DIG_OUT_3_GPIO_PIN (4U)
#endif
#ifndef BOARD_SLOW_DIG_OUT_4_GPIO
#define BOARD_SLOW_DIG_OUT_4_GPIO GPIO13
#endif
#ifndef BOARD_SLOW_DIG_OUT_4_GPIO_PIN
#define BOARD_SLOW_DIG_OUT_4_GPIO_PIN (3U)
#endif
#ifndef BOARD_SLOW_DIG_OUT_5_GPIO
#define BOARD_SLOW_DIG_OUT_5_GPIO GPIO13
#endif
#ifndef BOARD_SLOW_DIG_OUT_5_GPIO_PIN
#define BOARD_SLOW_DIG_OUT_5_GPIO_PIN (4U)
#endif
#ifndef BOARD_SLOW_DIG_OUT_6_GPIO
#define BOARD_SLOW_DIG_OUT_6_GPIO GPIO13
#endif
#ifndef BOARD_SLOW_DIG_OUT_6_GPIO_PIN
#define BOARD_SLOW_DIG_OUT_6_GPIO_PIN (5U)
#endif
#ifndef BOARD_SLOW_DIG_OUT_7_GPIO
#define BOARD_SLOW_DIG_OUT_7_GPIO GPIO13
#endif
#ifndef BOARD_SLOW_DIG_OUT_7_GPIO_PIN
#define BOARD_SLOW_DIG_OUT_7_GPIO_PIN (6U)
#endif

#define DIG_OUT_0_OFF()\
    GPIO_PortClear(BOARD_DIG_OUT_0_GPIO, 1U << BOARD_DIG_OUT_0_GPIO_PIN)
#define DIG_OUT_0_ON()\
	GPIO_PortSet(BOARD_DIG_OUT_0_GPIO, 1U << BOARD_DIG_OUT_0_GPIO_PIN)
#define DIG_OUT_0_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_DIG_OUT_0_GPIO, 1U << BOARD_DIG_OUT_0_GPIO_PIN)

#define DIG_OUT_1_OFF()\
    GPIO_PortClear(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)
#define DIG_OUT_1_ON()\
	GPIO_PortSet(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)
#define DIG_OUT_1_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_DIG_OUT_1_GPIO, 1U << BOARD_DIG_OUT_1_GPIO_PIN)

#define DIG_OUT_2_OFF()\
    GPIO_PortClear(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)
#define DIG_OUT_2_ON()\
	GPIO_PortSet(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)
#define DIG_OUT_2_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_DIG_OUT_2_GPIO, 1U << BOARD_DIG_OUT_2_GPIO_PIN)

#define DIG_OUT_3_OFF() \
    GPIO_PortClear(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)
#define DIG_OUT_3_ON()\
	GPIO_PortSet(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)
#define DIG_OUT_3_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_DIG_OUT_3_GPIO, 1U << BOARD_DIG_OUT_3_GPIO_PIN)

#define SLOW_DIG_OUT_4_OFF()\
    GPIO_PortClear(BOARD_SLOW_DIG_OUT_4_GPIO, 1U << BOARD_SLOW_DIG_OUT_4_GPIO_PIN)
#define SLOW_DIG_OUT_4_ON()\
	GPIO_PortSet(BOARD_SLOW_DIG_OUT_4_GPIO, 1U << BOARD_SLOW_DIG_OUT_4_GPIO_PIN)
#define SLOW_DIG_OUT_4_TOGGLE()\
    GPIO_PortToggle(BOARD_SLOW_DIG_OUT_4_GPIO, 1U << BOARD_SLOW_DIG_OUT_4_GPIO_PIN)

#define SLOW_DIG_OUT_5_OFF()\
    GPIO_PortClear(BOARD_SLOW_DIG_OUT_5_GPIO, 1U << BOARD_SLOW_DIG_OUT_5_GPIO_PIN)
#define SLOW_DIG_OUT_5_ON()\
	GPIO_PortSet(BOARD_SLOW_DIG_OUT_5_GPIO, 1U << BOARD_SLOW_DIG_OUT_5_GPIO_PIN)
#define SLOW_DIG_OUT_5_TOGGLE()\
    GPIO_PortToggle(BOARD_SLOW_DIG_OUT_5_GPIO, 1U << BOARD_SLOW_DIG_OUT_5_GPIO_PIN)

#define SLOW_DIG_OUT_6_OFF()\
    GPIO_PortClear(BOARD_SLOW_DIG_OUT_6_GPIO, 1U << BOARD_SLOW_DIG_OUT_6_GPIO_PIN)
#define SLOW_DIG_OUT_6_ON()\
	GPIO_PortSet(BOARD_SLOW_DIG_OUT_6_GPIO, 1U << BOARD_SLOW_DIG_OUT_6_GPIO_PIN)
#define SLOW_DIG_OUT_6_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_SLOW_DIG_OUT_6_GPIO, 1U << BOARD_SLOW_DIG_OUT_6_GPIO_PIN)

#define SLOW_DIG_OUT_7_OFF()\
    GPIO_PortClear(BOARD_SLOW_DIG_OUT_7_GPIO, 1U << BOARD_SLOW_DIG_OUT_7_GPIO_PIN)
#define SLOW_DIG_OUT_7_ON()\
	GPIO_PortSet(BOARD_SLOW_DIG_OUT_7_GPIO, 1U << BOARD_SLOW_DIG_OUT_7_GPIO_PIN)
#define SLOW_DIG_OUT_7_TOGGLE()                                       \
    GPIO_PortToggle(BOARD_SLOW_DIG_OUT_7_GPIO, 1U << BOARD_SLOW_DIG_OUT_7_GPIO_PIN)

/*! @brief The DIGITAL INPUTs used for board */
#ifndef BOARD_DIG_IN0_GPIO
#define BOARD_DIG_IN0_GPIO 				GPIO2
#endif
#ifndef BOARD_DIG_IN0_GPIO_PIN
#define BOARD_DIG_IN0_GPIO_PIN 			(6U)
#endif
#define BOARD_DIG_IN0_IRQ         		GPIO2_Combined_0_15_IRQn
#define BOARD_DIG_IN0_IRQ_HANDLER 		GPIO2_Combined_0_15_IRQHandler
#define BOARD_DIG_IN0_NAME        		"DIG_IN0"

#ifndef BOARD_DIG_IN1_GPIO
#define BOARD_DIG_IN1_GPIO 				GPIO2
#endif
#ifndef BOARD_DIG_IN1_GPIO_PIN
#define BOARD_DIG_IN1_GPIO_PIN 			(7U)
#endif
#define BOARD_DIG_IN1_IRQ         		GPIO2_Combined_0_15_IRQn
#define BOARD_DIG_IN1_IRQ_HANDLER 		GPIO2_Combined_0_15_IRQHandler
#define BOARD_DIG_IN1_NAME        		"DIG_IN1"

#ifndef BOARD_DIG_IN2_GPIO
#define BOARD_DIG_IN2_GPIO 				GPIO2
#endif
#ifndef BOARD_DIG_IN2_GPIO_PIN
#define BOARD_DIG_IN2_GPIO_PIN 			(27U)
#endif
#define BOARD_DIG_IN2_IRQ         		GPIO2_Combined_16_31_IRQn
#define BOARD_DIG_IN2_IRQ_HANDLER 		GPIO2_Combined_16_31_IRQHandler
#define BOARD_DIG_IN2_NAME        		"DIG_IN2"

#ifndef BOARD_DIG_IN3_GPIO
#define BOARD_DIG_IN3_GPIO 				GPIO4
#endif
#ifndef BOARD_DIG_IN3_GPIO_PIN
#define BOARD_DIG_IN3_GPIO_PIN 			(0U)
#endif
#define BOARD_DIG_IN3_IRQ         		GPIO4_Combined_0_15_IRQn
#define BOARD_DIG_IN3_IRQ_HANDLER 		GPIO4_Combined_0_15_IRQHandler
#define BOARD_DIG_IN3_NAME        		"DIG_IN3"

#ifndef BOARD_SLOW_DIG_IN4_GPIO
#define BOARD_SLOW_DIG_IN4_GPIO 			GPIO13
#endif
#ifndef BOARD_SLOW_DIG_IN4_GPIO_PIN
#define BOARD_SLOW_DIG_IN4_GPIO_PIN 		(7U)
#endif
#define BOARD_SLOW_DIG_IN4_IRQ         		GPIO13_Combined_0_31_IRQn
#define BOARD_SLOW_DIG_IN4_IRQ_HANDLER 		GPIO13_Combined_0_31_IRQHandler
#define BOARD_SLOW_DIG_IN4_NAME        		"DIG_IN4"

#ifndef BOARD_SLOW_DIG_IN5_GPIO
#define BOARD_SLOW_DIG_IN5_GPIO 			GPIO13
#endif
#ifndef BOARD_SLOW_DIG_IN5_GPIO_PIN
#define BOARD_SLOW_DIG_IN5_GPIO_PIN 		(8U)
#endif
#define BOARD_SLOW_DIG_IN5_IRQ         		GPIO13_Combined_0_31_IRQn
#define BOARD_SLOW_DIG_IN5_IRQ_HANDLER 		GPIO13_Combined_0_31_IRQHandler
#define BOARD_SLOW_DIG_IN5_NAME        		"DIG_IN5"

#ifndef BOARD_SLOW_DIG_IN6_GPIO
#define BOARD_SLOW_DIG_IN6_GPIO 			GPIO13
#endif
#ifndef BOARD_SLOW_DIG_IN6_GPIO_PIN
#define BOARD_SLOW_DIG_IN6_GPIO_PIN 		(9U)
#endif
#define BOARD_SLOW_DIG_IN6_IRQ         		GPIO13_Combined_0_31_IRQn
#define BOARD_SLOW_DIG_IN6_IRQ_HANDLER 		GPIO13_Combined_0_31_IRQHandler
#define BOARD_SLOW_DIG_IN6_NAME        		"DIG_IN6"

#ifndef BOARD_SLOW_DIG_IN7_GPIO
#define BOARD_SLOW_DIG_IN7_GPIO 			GPIO13
#endif
#ifndef BOARD_SLOW_DIG_IN7_GPIO_PIN
#define BOARD_SLOW_DIG_IN7_GPIO_PIN 		(10U)
#endif
#define BOARD_SLOW_DIG_IN7_IRQ         		GPIO13_Combined_0_31_IRQn
#define BOARD_SLOW_DIG_IN7_IRQ_HANDLER 		GPIO13_Combined_0_31_IRQHandler
#define BOARD_SLOW_DIG_IN7_NAME        		"DIG_IN7"

/*! @brief The board flash size */
#define BOARD_FLASH_SIZE (0x1000000U) /*TODO: change is different flash memory used*/

/*! @brief The ENET0 PHY address. */
#define BOARD_ENET0_PHY_ADDRESS (0x02U) /* Phy address of enet port 0. */

/*! @brief The ENET1 PHY address. */
#define BOARD_ENET1_PHY_ADDRESS (0x01U) /* Phy address of enet port 1. */

/* @Brief USB PHY condfiguration */
#define BOARD_USB_PHY_D_CAL     (0x07U)
#define BOARD_USB_PHY_TXCAL45DP (0x06U)
#define BOARD_USB_PHY_TXCAL45DM (0x06U)

/*! @Brief SE050 I2C and reset */
/* TODO: point of change with GENERAL I2C*/
#define BOARD_SE050_I2C_BASEADDR             	LPI2C5
#define BOARD_SE050_I2C_INSTANCE             	5U
#define BOARD_SE050_I2C_INDEX 					(5)

#define BOARD_SE050_I2C_CLOCK_SOURCE_SELECT  	(5U)
#define BOARD_SE050_I2C_CLOCK_SOURCE_DIVIDER 	(8U)
#define BOARD_SE050_I2C_CLOCK_ROOT				kCLOCK_Root_Lpi2c5
#define BOARD_SE050_I2C_CLOCK_FREQ           	(CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c5))

#define BOARD_SE050_I2C_IRQ         			LPI2C5_IRQn
#define BOARD_SE050_I2C_IRQ_HANDLER 			LPI2C5_IRQHandler

#ifndef BOARD_SE050_RST_GPIO
#define BOARD_SE050_RST_GPIO GPIO6
#endif
#ifndef BOARD_SE050_RST_GPIO_PIN
#define BOARD_SE050_RST_GPIO_PIN (3U)
#endif
#define SE050_RST_INIT(output)                                            \
    GPIO_PinWrite(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, output); \
    BOARD_USER_LED_GPIO->GDIR |= (1U << BOARD_USER_LED_GPIO_PIN) /*!< Enable target SE050_RST */
#define SE050_RST_ASSERT() \
    GPIO_PortClear(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN)                 /*!< Turn off target SE050_RST */
#define SE050_RST_RELEASE() GPIO_PortSet(BOARD_USER_LED_GPIO, 1U << BOARD_USER_LED_GPIO_PIN) /*!<Turn on target SE050_RST*/

/*! @Brief General I2C interface -> LCD touch or temp sensor or FRDM interfaces */
#define BOARD_GENERAL_I2C_BASEADDR             	LPI2C3
#define BOARD_GENERAL_I2C_INSTANCE             	3U
#define BOARD_GENERAL_I2C_INDEX 				(3)

#define BOARD_GENERAL_I2C_CLOCK_SOURCE_SELECT  	(6U)
#define BOARD_GENERAL_I2C_CLOCK_SOURCE_DIVIDER 	(8U)
#define BOARD_GENERAL_I2C_CLOCK_ROOT			kCLOCK_Root_Lpi2c3
#define BOARD_GENERAL_I2C_CLOCK_FREQ           	(CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c3))

#define BOARD_GENERAL_I2C_IRQ         			LPI2C3_IRQn
#define BOARD_GENERAL_I2C_IRQ_HANDLER 			LPI2C3_IRQHandler

/*! @brief SPI (FLEXIO SPI) pins. */
#define BOARD_SPI_DEVICE_SEL0_GPIO   			GPIO13
#define BOARD_SPI_DEVICE_SEL0_PIN    			11
#define BOARD_SPI_DEVICE_SEL1_GPIO 				GPIO13
#define BOARD_SPI_DEVICE_SEL1_PIN  				12
/* Second option
#define BOARD_SPI_DEVICE_SEL0_GPIO   			GPIO12
#define BOARD_SPI_DEVICE_SEL0_PIN    			10
#define BOARD_SPI_DEVICE_SEL1_GPIO 				GPIO12
#define BOARD_SPI_DEVICE_SEL1_PIN  				11
*/
#define SPI_DEVICE_SEL0_INIT(output)                                            \
    GPIO_PinWrite(BOARD_SPI_DEVICE_SEL0_GPIO, BOARD_SPI_DEVICE_SEL0_PIN, output); \
    BOARD_SPI_DEVICE_SEL0_GPIO->GDIR |= (1U << BOARD_SPI_DEVICE_SEL0_PIN)
#define SPI_DEVICE_SEL0_LOW() \
    GPIO_PortClear(BOARD_SPI_DEVICE_SEL0_GPIO, 1U << BOARD_SPI_DEVICE_SEL0_PIN)
#define SPI_DEVICE_SEL0_HIGH() GPIO_PortSet(BOARD_SPI_DEVICE_SEL0_GPIO, 1U << BOARD_SPI_DEVICE_SEL0_PIN)
#define SPI_DEVICE_SEL1_INIT(output)                                            \
    GPIO_PinWrite(BOARD_SPI_DEVICE_SEL1_GPIO, BOARD_SPI_DEVICE_SEL1_PIN, output); \
    BOARD_SPI_DEVICE_SEL1_GPIO->GDIR |= (1U << BOARD_SPI_DEVICE_SEL1_PIN)
#define SPI_DEVICE_SEL1_LOW() \
    GPIO_PortClear(BOARD_SPI_DEVICE_SEL1_GPIO, 1U << BOARD_SPI_DEVICE_SEL1_PIN)
#define SPI_DEVICE_SEL1_HIGH() GPIO_PortSet(BOARD_SPI_DEVICE_SEL1_GPIO, 1U << BOARD_SPI_DEVICE_SEL1_PIN)

/*! @brief The MIPI panel pins. */
/*define BOARD_MIPI_PANEL_RST_GPIO   		GPIO5 // potential conflict with LPUART8_RX (Freemaster)
#define BOARD_MIPI_PANEL_RST_PIN    		10 */
#define BOARD_MIPI_PANEL_POWER_GPIO 		GPIO6 // potential conflict with BOARD_SPI_DEVICE_SEL0_GPIO (2nd option)
#define BOARD_MIPI_PANEL_POWER_PIN  		10
/* Back light pin. */
#define BOARD_MIPI_PANEL_BL_GPIO 			BOARD_MIPI_PANEL_POWER_GPIO
#define BOARD_MIPI_PANEL_BL_PIN  			BOARD_MIPI_PANEL_POWER_PIN

/* Touch panel. */
#define BOARD_MIPI_PANEL_TOUCH_I2C_BASEADDR      BOARD_GENERAL_I2C_BASEADDR
#define BOARD_MIPI_PANEL_TOUCH_I2C_CLOCK_ROOT    BOARD_GENERAL_I2C_CLOCK_ROOT
#define BOARD_MIPI_PANEL_TOUCH_I2C_CLOCK_SOURCE  BOARD_GENERAL_I2C_CLOCK_SOURCE_SELECT
#define BOARD_MIPI_PANEL_TOUCH_I2C_CLOCK_DIVIDER BOARD_GENERAL_I2C_CLOCK_SOURCE_DIVIDER
#define BOARD_MIPI_PANEL_TOUCH_I2C_CLOCK_FREQ    BOARD_GENERAL_I2C_CLOCK_FREQ
/*#define BOARD_MIPI_PANEL_TOUCH_RST_GPIO          GPIO5 // potential conflict with LPUART8_TX (Freemaster)
#define BOARD_MIPI_PANEL_TOUCH_RST_PIN           9 */
#define BOARD_MIPI_PANEL_TOUCH_INT_GPIO          GPIO6 // potential conflict with BOARD_SPI_DEVICE_SEL1_GPIO (2nd option)
#define BOARD_MIPI_PANEL_TOUCH_INT_PIN           11

/* SD card */
#define BOARD_HAS_SDCARD (1U)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/
uint32_t BOARD_DebugConsoleSrcFreq(void);

void BOARD_InitDebugConsole(void);

void BOARD_ConfigMPU(void);

#if defined(SDK_I2C_BASED_COMPONENT_USED) && SDK_I2C_BASED_COMPONENT_USED
void BOARD_LPI2C_Init(LPI2C_Type *base, uint32_t clkSrc_Hz);
status_t BOARD_LPI2C_Send(LPI2C_Type *base,
                          uint8_t deviceAddress,
                          uint32_t subAddress,
                          uint8_t subaddressSize,
                          uint8_t *txBuff,
                          uint8_t txBuffSize);
status_t BOARD_LPI2C_Receive(LPI2C_Type *base,
                             uint8_t deviceAddress,
                             uint32_t subAddress,
                             uint8_t subaddressSize,
                             uint8_t *rxBuff,
                             uint8_t rxBuffSize);
status_t BOARD_LPI2C_SendSCCB(LPI2C_Type *base,
                              uint8_t deviceAddress,
                              uint32_t subAddress,
                              uint8_t subaddressSize,
                              uint8_t *txBuff,
                              uint8_t txBuffSize);
status_t BOARD_LPI2C_ReceiveSCCB(LPI2C_Type *base,
                                 uint8_t deviceAddress,
                                 uint32_t subAddress,
                                 uint8_t subaddressSize,
                                 uint8_t *rxBuff,
                                 uint8_t rxBuffSize);
void BOARD_MIPIPanelTouch_I2C_Init(void);
status_t BOARD_MIPIPanelTouch_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize);
status_t BOARD_MIPIPanelTouch_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize);
#endif /* SDK_I2C_BASED_COMPONENT_USED */

void BOARD_SD_Pin_Config(uint32_t speed, uint32_t strength);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_H_ */
