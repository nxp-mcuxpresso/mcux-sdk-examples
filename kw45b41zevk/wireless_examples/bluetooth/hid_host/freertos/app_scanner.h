/*! *********************************************************************************
* Copyright 2024 NXP
*
*
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef APP_SCANNER_H
#define APP_SCANNER_H

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */
typedef struct appScanningParams_tag
{
    gapScanningParameters_t *pHostScanParams;         /*!< Pointer to host scan structure */
    gapFilterDuplicates_t enableDuplicateFiltering;   /*!< Duplicate filtering mode */
    uint16_t duration;                                /*!< scan duration  */
    uint16_t period;                                  /*!< scan period  */
} appScanningParams_t;

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapScanningCallback_t pfScanCallback;

/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*\fn           bleResult_t BluetoothLEHost_StartScanning(
*                  appScanningParams_t   *pAppScanParams,
*                  gapScanningCallback_t pfCallback
*              )
*\brief        Start the Bluetooth LE scanning using the parameters specified.
*
*\param  [in]  pScanningParameters    Pointer to the structure containing the scanning
*                                     parameters.
*\param  [in]  pfCallback             The scanning callback.

*\return       bleResult_t            Result of the oeration.
********************************************************************************** */
bleResult_t BluetoothLEHost_StartScanning
(
    appScanningParams_t   *pAppScanParams,
    gapScanningCallback_t pfCallback
);

/*! *********************************************************************************
*\fn           bool_t BluetoothLEHost_MatchDataInAdvElementList(
*                  gapAdStructure_t *pElement,
*                  void             *pData,
*                  uint8_t          iDataLen
*              )
*\brief        Search if the contents from pData can be found in an advertising
*              element.
*
*\param  [in]  pElement    Pointer to the structure containing the ad
*                          structure element.
*\param  [in]  pData       Pointer to the data to be searched for.
*\param  [in]  iDataLen    The length of the data.

*\retval       TRUE        Data was found in this element.
*\retval       FALSE       Data was not found in this element.
********************************************************************************** */
bool_t BluetoothLEHost_MatchDataInAdvElementList
(
    gapAdStructure_t *pElement,
    void             *pData,
    uint8_t          iDataLen
);

#endif /* APP_SCANNER_H */
