/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/


#include "modelrunner.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_clock.h"
#include "app.h"
#ifdef MODELRUNNER_HTTP
#include "fsl_phy.h"
#include "fsl_phylan8741.h"
#include "fsl_enet.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef MODELRUNNER_HTTP
phy_lan8741_resource_t g_phy_resource;
#else

#define SYSTICK_PRESCALE 1U
#define TICK_PRIORITY 1U

volatile uint32_t msTicks;
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef MODELRUNNER_HTTP
static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET_BASE)]);
    ENET_SetSMI(EXAMPLE_ENET_BASE, CLOCK_GetCoreSysClkFreq());
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET_BASE, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET_BASE, phyAddr, regAddr, pData);
}
#endif

int64_t os_clock_now(){
    int64_t us = ((SystemCoreClock / 1000) - SysTick->VAL) / (SystemCoreClock / 1000000);
    us += (int64_t)(sys_now()*1e3);
    return us;
}


#ifdef __cplusplus
extern "C" {
#endif

int64_t getTime() { return os_clock_now(); }

int gethostname(char *name)
{
    const char* host = "mcxn";
    memcpy(name, host, strlen(host));
    return 0;
}

#ifdef __cplusplus
}
#endif

/*!
 * @brief Main function.
 */
int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable caching of flash memory */
    SYSCON->LPCAC_CTRL = SYSCON->LPCAC_CTRL & ~SYSCON_LPCAC_CTRL_DIS_LPCAC_MASK;
    SYSCON->NVM_CTRL = SYSCON->NVM_CTRL & ~SYSCON_NVM_CTRL_DIS_FLASH_DATA_MASK;

#ifdef MODELRUNNER_HTTP
    CLOCK_AttachClk(kNONE_to_ENETRMII);
    CLOCK_EnableClock(kCLOCK_Enet);
    SYSCON0->PRESETCTRL2 = SYSCON_PRESETCTRL2_ENET_RST_MASK;
    SYSCON0->PRESETCTRL2 &= ~SYSCON_PRESETCTRL2_ENET_RST_MASK;

    MDIO_Init();

    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
#endif


    PRINTF("\r\n*************************************************");
    PRINTF("\r\n               TFLite Modelrunner");
    PRINTF("\r\n*************************************************\r\n");

    modelrunner();

}
