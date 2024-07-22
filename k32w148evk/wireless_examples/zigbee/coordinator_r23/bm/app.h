/*
 * Copyright 2019-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _APP_H_
#define _APP_H_

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c >= 1)
#include "fsl_component_serial_manager.h"
#endif
#if defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)
#include "fsl_component_led.h"
#endif
#if defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)
#include "fsl_component_button.h"
#endif
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*${macro:start}*/
/* UART instance and clock */
#define DEMO_UART          LPUART0
#define DEMO_UART_CLKSRC   LPUART0_CLK_SRC
#define DEMO_UART_CLK_FREQ (12000000)

#if (defined(gBoardLedCnt_c) && defined(gAppLedCnt_c) && (gBoardLedCnt_c < gAppLedCnt_c))
#undef gAppLedCnt_c
#define gAppLedCnt_c gBoardLedCnt_c
#endif

#if (defined(gBoardButtonCnt_c) && defined(gAppButtonCnt_c) && (gBoardButtonCnt_c < gAppButtonCnt_c))
#undef gAppButtonCnt_c
#define gAppButtonCnt_c gBoardButtonCnt_c
#endif
#define LED_FLASH_CONFIG_ARRAY_DEFINE(name, count) led_flash_config_t name[count]

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
#define LedOn(ledNo)  (void)LED_TurnOnOff((led_handle_t)g_ledHandle[ledNo], 1U)
#define LedOff(ledNo) (void)LED_TurnOnOff((led_handle_t)g_ledHandle[ledNo], 0U)
#define LedFlashing(ledNo) \
    (void)LED_Flash((led_handle_t)g_ledHandle[ledNo], (led_flash_config_t *)&g_ledFlashConfig[ledNo])
#define LedSetColor(ledNo, ledColor) (void)LED_SetColor((led_handle_t)g_ledHandle[ledNo], (uint32_t)ledColor)
#define Led1On()                     LedOn(0U)
#define Led1Off()                    LedOff(0U)
#define Led1Flashing()               LedFlashing(0U)

#if gAppLedCnt_c > 1
#define Led2On()       LedOn(1U)
#define Led2Off()      LedOff(1U)
#define Led2Flashing() LedFlashing(1U)
#else
#define Led2On()
#define Led2Off()
#define Led2Flashing()
#endif /* gAppLedCnt_c > 1 */
#else
#define LedOn(ledNo)
#define LedOff(ledNo)
#define LedSetColor(ledNo, ledColor)
#define Led1On()
#define Led1Off()
#define Led1Flashing()
#define Led2On()
#define Led2Off()
#define Led2Flashing()
#define LedFlashing(ledNo)
#define LedStopFlashingAllLeds()
#define LedStartFlashingAllLeds()
#endif /* (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0)) */

/*${macro:end}*/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
/*serial manager handle*/
extern SERIAL_MANAGER_HANDLE_DEFINE(gSerMgrIf);

#if (gAppUseSerialManager_c > 1)
/*serial manager handle*/
extern SERIAL_MANAGER_HANDLE_DEFINE(gSerMgrIf2);
#endif

/*Define fsci serial manager handle*/
#if defined(gFsciIncluded_c) && (gFsciIncluded_c > 0)
extern serial_handle_t g_fsciHandleList[gFsciIncluded_c];
#endif /*gFsciIncluded_c > 0*/
#endif

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
/*Define led handle*/
extern LED_HANDLE_ARRAY_DEFINE(g_ledHandle, gAppLedCnt_c);
#endif /*gAppLedCnt_c > 0*/

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
/*Define led flash configuration*/
extern LED_FLASH_CONFIG_ARRAY_DEFINE(g_ledFlashConfig, gAppLedCnt_c);
#endif /*gAppLedCnt_c > 0*/

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
/*Define button handle*/
extern BUTTON_HANDLE_ARRAY_DEFINE(g_buttonHandle, gAppButtonCnt_c);
#endif /*gAppButtonCnt_c > 0*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
/*!
 * \brief Initialize all services required by application (only), timer manager, serial
 *      manager, led, button, low power. this is optional if application does not use any of these
 *
 */
void APP_InitServices(void);

/*!
 * \brief Initialize all services required for BLE connectivity : clocks, link layer, HCI transport, etc..
 *      This is mandatory for BLE connectivity support.
 *      Shall be called before host stack initialization
 *
 * \return int 0 if success, negative value if error.
 */
int APP_InitBle(void);

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
void LedStartFlashingAllLeds(void);
void LedStopFlashingAllLeds(void);
#endif /*gAppLedCnt_c > 0*/
/*${prototype:end}*/

#endif /* _APP_H_ */
