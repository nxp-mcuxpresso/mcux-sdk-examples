/*! *********************************************************************************
 * \addtogroup BLE
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef BLE_INIT_H
#define BLE_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "board.h"

#if (defined(gUseHciTransportDownward_d) && (gUseHciTransportDownward_d == 1)) || \
    (defined(gUseHciTransportUpward_d) && (gUseHciTransportUpward_d == 1))
#include "hci_transport.h"
#endif /* gUseHciTransportDownward_d || gUseHciTransportUpward_d */

#if !defined(gUseHciTransportDownward_d) || (!gUseHciTransportDownward_d)
#include "controller_interface.h"

#if defined(K32W232H_SERIES)
#include "nxp2p4_xcvr.h"
#endif

#include "fsl_adapter_flash.h"
#endif /* gUseHciTransportDownward_d */

/************************************************************************************
*************************************************************************************
* Public macros - Do not modify directly! Override in app_preinclude.h if needed.
*************************************************************************************
************************************************************************************/
#ifndef gBleXcvrInitRetryCount_c
#define gBleXcvrInitRetryCount_c (10U)
#endif /* gBleXcvrInitRetryCount_c */

/*
 * This enables/disables the HCI Reset command sent by the Host at init sequence
 * Default value is disabled, only for gUseHciTransportDownward_d is required
 */
#ifndef gHostInitResetController_c
    #if defined(gUseHciTransportDownward_d) && gUseHciTransportDownward_d
        #define gHostInitResetController_c TRUE
    #else /* gUseHciTransportDownward_d */
        #define gHostInitResetController_c FALSE
    #endif /* gUseHciTransportDownward_d */
#endif /* gHostInitResetController_c */

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
#if !defined(gUseHciTransportUpward_d) || (!gUseHciTransportUpward_d)
extern bool_t gHostInitResetController;
#endif /* gUseHciTransportUpward_d */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn           bleResult_t Ble_Initialize(gapGenericCallback_t gapGenericCallback)
*\brief        Performs full initialization of the BLE stack.
*
*\param  [in]  genericCallback    Callback used by the Host Stack to propagate GAP
*                                 generic events to the application.
*
*\return       bleResult_t        Result of the operation.
*
*\remarks      The gInitializationComplete_c generic GAP event is triggered on
*              completion.
********************************************************************************** */
bleResult_t Ble_Initialize
(
    gapGenericCallback_t gapGenericCallback
);

/*! *********************************************************************************
*\fn           bleResult_t Ble_DeInitialize()
*\brief        Frees the resources not allocated by the BLE stack.
*
*\param  [in]  none
*
*\return       bleResult_t        Result of the operation.
*
*\remarks      The function should be called upon receiving gDeInitializationComplete_c
*              event in the generic callback.
********************************************************************************** */
bleResult_t Ble_DeInitialize(void);
/*! *********************************************************************************
*\fn           void Ble_SetBDAddr(void)
*\brief        Set Bluetooth Device Address in Controller. .
*
*\param  [in]  void
*
*\return       void
*
*\remarks      Must be done after HCI init and before Host init
********************************************************************************** */
#if defined(gUseHciTransportDownward_d) && gUseHciTransportDownward_d
#if defined(KW45B41Z83_SERIES) || \
    defined(KW45B41Z82_SERIES) || \
    defined(K32W1480_SERIES)   || \
    defined(CPU_MCXW345CHNA) || defined(CPU_MCXW345CUKA) || \
    defined(KW47B42ZB7_cm33_core0_SERIES) || defined(KW47B42ZB6_cm33_core0_SERIES) || defined(KW47B42ZB3_cm33_core0_SERIES) || \
    defined(KW47B42ZB2_cm33_core0_SERIES) || defined(KW47B42Z97_cm33_core0_SERIES) || defined(KW47B42Z96_cm33_core0_SERIES) || \
    defined(KW47B42Z83_cm33_core0_SERIES)

void Ble_SetBDAddr(void);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* BLE_INIT_H */

/*! *********************************************************************************
* @}
********************************************************************************** */
