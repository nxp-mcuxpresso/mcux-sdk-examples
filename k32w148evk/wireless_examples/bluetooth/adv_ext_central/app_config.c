/*! *********************************************************************************
* \addtogroup App Config
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file contains configuration data for the application and stack
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "gap_interface.h"
#include "ble_constants.h"
#include "gatt_db_handles.h"
#include "ble_conn_manager.h"
#include "adv_ext_central.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define smpEdiv                 0x1F99
#define mcEncryptionKeySize_c   16

/*! Scan interval (4.5 s) */
#define gAEScanInterval_d        (7200U)

/*! Scan window (2 s) */
#define gAEScanWindow_d          (3200U)

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

gapScanningParameters_t gScanParams =
{
    /* type */              gScanTypeActive_c,
    /* interval */          gAEScanInterval_d,
    /* window */            gAEScanWindow_d,
    /* ownAddressType */    gBleAddrTypePublic_c,
    /* filterPolicy */      (uint8_t)gScanAll_c,
    /* scanning PHY */      (uint8_t)(gLePhy1MFlag_c | gLePhyCodedFlag_c)
};

/* Default Connection Request Parameters */
gapConnectionRequestParameters_t gConnReqParams =
{
    .scanInterval = 36,
    .scanWindow = 36,
    .filterPolicy = (uint8_t)gUseDeviceAddress_c,
    .ownAddressType = gBleAddrTypePublic_c,
    .connIntervalMin = gcConnectionIntervalMinDefault_c,
    .connIntervalMax = gcConnectionIntervalMinDefault_c,
    .connLatency = 0,
    .supervisionTimeout = 0x03E8,
    .connEventLengthMin = 0,
    .connEventLengthMax = 0xFFFF,
    .initiatingPHYs = (uint8_t)gLePhy1MFlag_c,
};
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
gapDecisionInstructionsData_t gDbafDecisionInstructions =
{
    .testGroup = gDITG_NewTestGroup_c,
    .passCriteria = gDITPC_CheckPasses_c,
    .relevantField = gDIRF_ResolvableTag_c,
    .testParameters.resolvableTagKey = {0x44, 0x65, 0x63, 0x69, 0x73, 0x69, 0x6F, 0x6E,
    0x42, 0x61, 0x73, 0x65, 0x64, 0x41, 0x64, 0x76},
};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/* SMP Data */
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
gapPairingParameters_t gPairingParameters = {
    .withBonding = (bool_t)gAppUseBonding_d,
    .securityModeAndLevel = gSecurityMode_1_Level_3_c,
    .maxEncryptionKeySize = mcEncryptionKeySize_c,
    .localIoCapabilities = gIoKeyboardDisplay_c,
    .oobAvailable = FALSE,
    .centralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .peripheralKeys = (gapSmpKeyFlags_t) (gLtk_c | gIrk_c),
    .leSecureConnectionSupported = TRUE,
    .useKeypressNotifications = FALSE,
};
#endif /* gAppUsePairing_d */

/* LTK */
static uint8_t smpLtk[gcSmpMaxLtkSize_c] =
    {0xD6, 0x93, 0xE8, 0xA4, 0x23, 0x55, 0x48, 0x99,
     0x1D, 0x77, 0x61, 0xE6, 0x63, 0x2B, 0x10, 0x8E};

/* RAND*/
static uint8_t smpRand[gcSmpMaxRandSize_c] =
    {0x26, 0x1E, 0xF6, 0x09, 0x97, 0x2E, 0xAD, 0x7E};

/* IRK */
static uint8_t smpIrk[gcSmpIrkSize_c] =
    {0x0A, 0x2D, 0xF4, 0x65, 0xE3, 0xBD, 0x7B, 0x49,
     0x1E, 0xB4, 0xC0, 0x95, 0x95, 0x13, 0x46, 0x73};

/* CSRK */
static uint8_t smpCsrk[gcSmpCsrkSize_c] =
    {0x90, 0xD5, 0x06, 0x95, 0x92, 0xED, 0x91, 0xD7,
     0xA8, 0x9E, 0x2C, 0xDC, 0x4A, 0x93, 0x5B, 0xF9};

gapSmpKeys_t gSmpKeys = {
    .cLtkSize = mcEncryptionKeySize_c,
    .aLtk = (void *)smpLtk,
    .aIrk = (void *)smpIrk,
    .aCsrk = (void *)smpCsrk,
    .aRand = (void *)smpRand,
    .cRandSize = gcSmpMaxRandSize_c,
    .ediv = smpEdiv,
};

/* Device Security Requirements */
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
static gapSecurityRequirements_t        deviceSecurity = gGapDefaultSecurityRequirements_d;

gapDeviceSecurityRequirements_t deviceSecurityRequirements = {
    .pSecurityRequirements          = (void*)&deviceSecurity,
    .cNumServices                   = 0,
    .aServiceSecurityRequirements   = NULL
};
#endif /* gAppUsePairing_d */
