/*****************************************************************************
 *
 * MODULE: JN-AN-1241
 *
 * COMPONENT: perpheral.h
 *
 * DESCRIPTION: Common functions
 *
 *****************************************************************************
 *
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5168, JN5179].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright 2017 NXP
 *
 ***************************************************************************/
#ifndef PERIPHERAL_H_
#define PERIPHERAL_H_

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#if JENNIC_CHIP_FAMILY == JN516x
/* For REG_GPIO_DIN */
#include "PeripheralRegs_JN516x.h"
#endif
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions
 *                        ***/
/****************************************************************************/


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
#ifdef ZSE_BUILD
PUBLIC void vApp_ISR_SystemController(void);
PUBLIC void vApp_ETSIHighPowerModuleEnable(void);

PUBLIC void vAppCopyEepromToFlash(void);
PUBLIC void vAppCopyFlashToEeprom(void);
PUBLIC void vAppSetFlashStatusWord(uint32 u32Status);
PUBLIC uint32 u32AppGetFlashStatusWord(void);
PUBLIC void vAppEraseFullEeprom(void);

PUBLIC void vApp_WatchdogRestart(void);
PUBLIC uint16 u16AppGetTemp(void);

//app_sleep_functions
PUBLIC bool bTestDisableSleep(void);
//PUBLIC uint8 u8App_ReadUartStatusFlag(void);
PUBLIC void vAppWaitUARTTx(void);
//MDI_ReadFirmVer
PUBLIC void vAppDisableUart(uint8 uart);

PUBLIC void App_vSoftwareReset( void);

PUBLIC void vAppDeepSleep(void);
PUBLIC void vApp_InitEeprom(void);
#ifdef RADIO_RECALIBRATION
PUBLIC uint8 App_u8CalibrateRadio( void);
#endif
#endif

PUBLIC uint32 vMDI_GpioIsOutPinSet(uint32 u32Pin);
#if JENNIC_CHIP_FAMILY == JN518x
PUBLIC void vAHI_DioSetPullup(uint32 u32On, uint32 u32Off);
PUBLIC void vAHI_WatchdogRestart(void);
#endif

PUBLIC void vAppSwDelayOneUs();
PUBLIC void vAppSwDelayMs(uint32 ms);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/
#if JENNIC_CHIP_FAMILY == JN518x
static inline uint32 vMDI_GpioIsInPinSet(uint32 u32Pin)
{
	/*
	 * As opposed to 6x, on 8x we don't have a reg where all the GPIO statuses
	 * can be read; rather it's an array of u8s (or u32s) that contains the
	 * status for each GPIO. As such, just do a quick calculation on which
	 * array offset is needed and return directly
	 */
	return GPIO->W[0][__builtin_ctz(u32Pin)];
}

static inline void vAHI_DioSetOutput(uint32 u32On, uint32 u32Off)
{
	if (u32On)
		GPIO->SET[0] = u32On;
	if (u32Off)
		GPIO->CLR[0] = u32Off;
}

static inline void vAHI_DioSetDirection(uint32 u32Inputs, uint32 u32Outputs)
{
	if (u32Inputs)
		GPIO->DIRCLR[0] = u32Inputs;
	if (u32Outputs)
		GPIO->DIRSET[0] = u32Outputs;
}

#else
static inline uint32 vMDI_GpioIsInPinSet(uint32 u32Pin)
{
	return u32REG_GpioRead(REG_GPIO_DIN) & u32Pin;
}

#endif /* JENNIC_CHIP_FAMILY == JN518x */
#if defined __cplusplus
}
#endif

#endif /*PERIPHERAL_H_*/
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
