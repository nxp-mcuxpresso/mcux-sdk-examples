/*! *********************************************************************************
 * \defgroup Device Information Service
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DEVICE_INFO_INTERFACE_H_
#define _DEVICE_INFO_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

typedef struct utf8s_tag
{
    uint16_t stringLength;
    char *pUtf8s;
} utf8s_t;

typedef struct systemId_tag
{
    uint8_t oui[3];
    uint8_t manufacturerId[5];
} systemId_t;

typedef struct regCertDataList_tag
{
    uint16_t length;
    void *pList;
} regCertDataList_t;

typedef struct pnpId_tag
{
    uint8_t vendorIdSource;
    uint16_t vendorId;
    uint16_t productId;
    uint16_t productVersion;
} pnpId_t;

/*! Device Info - Configuration */
typedef struct disConfig_tag
{
    uint16_t serviceHandle;
    utf8s_t manufacturerName;
    utf8s_t modelNumber;
    utf8s_t serialNumber;
    utf8s_t hwRevision;
    utf8s_t fwRevision;
    utf8s_t swRevision;
    systemId_t *pSystemId;
    regCertDataList_t rcdl;
    pnpId_t *pPnpId;
} disConfig_t;

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*!**********************************************************************************
 * \brief        Starts Device Info service functionality
 *
 * \param[in]    pServiceConfig  Pointer to structure that contains server
 *                               configuration information.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Dis_Start(disConfig_t *pServiceConfig);

/*!**********************************************************************************
 * \brief        Stops Device Info service functionality
 *
 * \param[in]    pServiceConfig  Pointer to structure that contains server
 *                               configuration information.
 *
 * \return       gBleSuccess_c or error.
 ************************************************************************************/
bleResult_t Dis_Stop(disConfig_t *pServiceConfig);

#ifdef __cplusplus
}
#endif

#endif /* _BATTERY_INTERFACE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
