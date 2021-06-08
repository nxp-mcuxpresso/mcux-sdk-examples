/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_types.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

uint16_t fsciBleGetUuidBufferSize(bleUuidType_t uuidType)
{
    uint16_t dataSize;

    /* Get the variable size for the needed buffer (depending on UUID type) */
    switch (uuidType)
    {
        case gBleUuidType16_c:
        {
            /* UUID 16 bits */
            dataSize = sizeof(uint16_t);
        }
        break;

        case gBleUuidType32_c:
        {
            /* UUID 32 bits */
            dataSize = sizeof(uint32_t);
        }
        break;

        case gBleUuidType128_c:
        {
            /* UUID 128 bits */
            dataSize = 16U * sizeof(uint8_t);
        }
        break;

        default:
            dataSize = 0;
            break;
    }

    /* Return the needed buffer size */
    return dataSize;
}

void fsciBleGetUuidFromBuffer(bleUuid_t *pUuid, uint8_t **ppBuffer, bleUuidType_t uuidType)
{
    /* Read UUID from buffer (depending on UUID type) */
    switch (uuidType)
    {
        case gBleUuidType16_c:
        {
            /* UUID 16 bits */
            fsciBleGetUint16ValueFromBuffer(pUuid->uuid16, *ppBuffer);
        }
        break;

        case gBleUuidType32_c:
        {
            /* UUID 32 bits */
            fsciBleGetUint32ValueFromBuffer(pUuid->uuid32, *ppBuffer);
        }
        break;

        case gBleUuidType128_c:
        {
            /* UUID 128 bits */
            fsciBleGetUint128ValueFromBuffer(pUuid->uuid128, *ppBuffer);
        }
        break;

        default:
            break;
    }
}

void fsciBleGetBufferFromUuid(bleUuid_t *pUuid, uint8_t **ppBuffer, bleUuidType_t uuidType)
{
    /* Write UUID in buffer (depending on UUID type) */
    switch (uuidType)
    {
        case gBleUuidType16_c:
        {
            /* UUID 16 bits */
            fsciBleGetBufferFromUint16Value(pUuid->uuid16, *ppBuffer);
        }
        break;

        case gBleUuidType32_c:
        {
            /* UUID 32 bits */
            fsciBleGetBufferFromUint32Value(pUuid->uuid32, *ppBuffer);
        }
        break;

        case gBleUuidType128_c:
        {
            /* UUID 128 bits */
            fsciBleGetBufferFromUint128Value(&pUuid->uuid128, *ppBuffer);
        }
        break;

        default:
            break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
