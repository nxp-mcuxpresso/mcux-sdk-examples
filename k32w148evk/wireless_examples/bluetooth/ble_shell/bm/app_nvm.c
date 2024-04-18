/*! *********************************************************************************
* Copyright 2021-2023 NXP
*
* \file
*
* This is a source file for the common application NVM code.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#if (defined gAppUseNvm_d) && (gAppUseNvm_d != 0)
#include "NVM_Interface.h"
#endif /* gAppUseNvm_d */

#include "ble_config.h"
#include "ble_general.h"
#include "app_conn.h"

/*! *********************************************************************************
*************************************************************************************
* Public macros
*************************************************************************************
********************************************************************************** */
/*!
 * Enable/disable use NV flash procedures for operations triggered by the host stack
 * Do not modify directly. Redefine it in the app_preinclude.h file
 */
#ifndef gAppUseNvm_d
#define gAppUseNvm_d                    (FALSE)
#endif /* gAppUseNvm_d */

/*!
 * When Advanced Secure Mode is not used, write into NVM only the 16 bytes of the
 * plaintext IRK.
 */
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d == FALSE))
#define gIdentityHeaderOverhead_c      24U
#else
#define gIdentityHeaderOverhead_c       0U
#endif /* gAppSecureMode_d */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
/* NVM Dataset identifiers */
#if gAppUseNvm_d
#define nvmId_BondingHeaderId_c          0x4011
#define nvmId_BondingDataDynamicId_c     0x4012
#define nvmId_BondingDataStaticId_c      0x4013
#define nvmId_BondingDataLegacyId_c      0x4014
#define nvmId_BondingDataDeviceInfoId_c  0x4015
#define nvmId_BondingDataDescriptorId_c  0x4016
#define nvmId_BleLocalKeysId_c           0x4017
#endif /* gAppUseNvm_d */

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if gAppUseNvm_d
#if gUnmirroredFeatureSet_d == TRUE
static bleBondIdentityHeaderBlob_t*  aBondingHeader[gMaxBondedDevices_c];
static bleBondDataDynamicBlob_t*     aBondingDataDynamic[gMaxBondedDevices_c];
static bleBondDataStaticBlob_t*      aBondingDataStatic[gMaxBondedDevices_c];
static bleBondDataLegacyBlob_t*      aBondingDataLegacy[gMaxBondedDevices_c];
static bleBondDataDeviceInfoBlob_t*  aBondingDataDeviceInfo[gMaxBondedDevices_c];
static bleBondDataDescriptorBlob_t*  aBondingDataDescriptor[gMaxBondedDevices_c *
                                        gcGapMaximumSavedCccds_c];
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
static bleLocalKeysBlob_t*           aBleLocalKeys[gcSecureModeSavedLocalKeysNo_c];
#endif
NVM_RegisterDataSet(aBondingHeader,
                    gMaxBondedDevices_c,
                    (gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c),
                    nvmId_BondingHeaderId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
NVM_RegisterDataSet(aBondingDataDynamic,
                    gMaxBondedDevices_c,
                    gBleBondDataDynamicSize_c,
                    nvmId_BondingDataDynamicId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
NVM_RegisterDataSet(aBondingDataStatic,
                    gMaxBondedDevices_c,
                    gBleBondDataStaticSize_c,
                    nvmId_BondingDataStaticId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
NVM_RegisterDataSet(aBondingDataLegacy,
                    gMaxBondedDevices_c,
                    gBleBondDataLegacySize_c,
                    nvmId_BondingDataLegacyId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
NVM_RegisterDataSet(aBondingDataDeviceInfo,
                    gMaxBondedDevices_c,
                    gBleBondDataDeviceInfoSize_c,
                    nvmId_BondingDataDeviceInfoId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
NVM_RegisterDataSet(aBondingDataDescriptor,
                    gMaxBondedDevices_c * gcGapMaximumSavedCccds_c,
                    gBleBondDataDescriptorSize_c,
                    nvmId_BondingDataDescriptorId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
NVM_RegisterDataSet(aBleLocalKeys,
                    gcSecureModeSavedLocalKeysNo_c,
                    (uint16_t)sizeof(bleLocalKeysBlob_t) ,
                    nvmId_BleLocalKeysId_c,
                    (uint16_t)gNVM_NotMirroredInRamAutoRestore_c);
#endif /* defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U) */
#else /* gUnmirroredFeatureSet_d */
static bleBondIdentityHeaderBlob_t  aBondingHeader[gMaxBondedDevices_c];
static bleBondDataDynamicBlob_t     aBondingDataDynamic[gMaxBondedDevices_c];
static bleBondDataStaticBlob_t      aBondingDataStatic[gMaxBondedDevices_c];
static bleBondDataLegacyBlob_t      aBondingDataLegacy[gMaxBondedDevices_c];
static bleBondDataDeviceInfoBlob_t  aBondingDataDeviceInfo[gMaxBondedDevices_c];
static bleBondDataDescriptorBlob_t  aBondingDataDescriptor[gMaxBondedDevices_c *
                                        gcGapMaximumSavedCccds_c];
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
static bleLocalKeysBlob_t           aBleLocalKeys[gcSecureModeSavedLocalKeysNo_c];
#endif
/* register datasets */
NVM_RegisterDataSet(aBondingHeader,
                    gMaxBondedDevices_c,
                    (gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c),
                    nvmId_BondingHeaderId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
NVM_RegisterDataSet(aBondingDataDynamic,
                    gMaxBondedDevices_c,
                    gBleBondDataDynamicSize_c,
                    nvmId_BondingDataDynamicId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
NVM_RegisterDataSet(aBondingDataStatic,
                    gMaxBondedDevices_c,
                    gBleBondDataStaticSize_c,
                    nvmId_BondingDataStaticId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
NVM_RegisterDataSet(aBondingDataLegacy,
                    gMaxBondedDevices_c,
                    gBleBondDataLegacySize_c,
                    nvmId_BondingDataLegacyId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
NVM_RegisterDataSet(aBondingDataDeviceInfo,
                    gMaxBondedDevices_c,
                    gBleBondDataDeviceInfoSize_c,
                    nvmId_BondingDataDeviceInfoId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
NVM_RegisterDataSet(aBondingDataDescriptor,
                    gMaxBondedDevices_c * gcGapMaximumSavedCccds_c,
                    gBleBondDataDescriptorSize_c,
                    nvmId_BondingDataDescriptorId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
NVM_RegisterDataSet(aBleLocalKeys,
                    gcSecureModeSavedLocalKeysNo_c,
                    (uint16_t)sizeof(bleLocalKeysBlob_t) ,
                    nvmId_BleLocalKeysId_c,
                    (uint16_t)gNVM_MirroredInRam_c);
#endif /* defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U) */
#endif /* gUnmirroredFeatureSet_d */
#else /* gAppUseNvm_d */
static bleBondDataBlob_t          maBondDataBlobs[gMaxBondedDevices_c] = {{{{0}}}};
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
static bleLocalKeysBlob_t         aBleLocalKeys[gcSecureModeSavedLocalKeysNo_c]; 
#endif
#endif /* gAppUseNvm_d */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
*\fn           bleResult_t App_NvmErase(uint8_t mEntryIdx)
*\brief        This function erases the data corresponding to an entry.
*
*\param  [in]  mEntryIdx  Index of the entry that should be deleted.
*
* \return  bleResult_t
********************************************************************************** */
bleResult_t App_NvmErase
(
    uint8_t mEntryIdx
)
{
    bleResult_t status = gBleSuccess_c;
#if gAppUseNvm_d
    NVM_Status_t nvmStatus = gNVM_OK_c;
    uint32_t mDescIdx = 0U;
#endif
    
    if(mEntryIdx >= (uint8_t)gMaxBondedDevices_c)
    {
          status = gBleInvalidParameter_c;
    }
    else
    {
#if gAppUseNvm_d
#if gUnmirroredFeatureSet_d == TRUE
        nvmStatus = NvErase((void**)&aBondingHeader[mEntryIdx]);
        
        if (nvmStatus == gNVM_OK_c)
        {
            nvmStatus = NvErase((void**)&aBondingDataDynamic[mEntryIdx]);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            nvmStatus = NvErase((void**)&aBondingDataStatic[mEntryIdx]);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            nvmStatus = NvErase((void**)&aBondingDataLegacy[mEntryIdx]);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            nvmStatus = NvErase((void**)&aBondingDataDeviceInfo[mEntryIdx]);
        }
        
        for(mDescIdx = ((uint32_t)mEntryIdx * gcGapMaximumSavedCccds_c);
            mDescIdx < ((uint32_t)mEntryIdx + 1U) * gcGapMaximumSavedCccds_c; mDescIdx++)
        {
            nvmStatus = NvErase((void**)&aBondingDataDescriptor[mDescIdx]);
            
            if (nvmStatus != gNVM_OK_c)
            {
                break;
            }
        }
#else // mirrored
        FLib_MemSet(&aBondingHeader[mEntryIdx], 0, gBleBondIdentityHeaderSize_c);
        nvmStatus = NvSaveOnIdle((void*)&aBondingHeader[mEntryIdx], FALSE);
        
        if (nvmStatus == gNVM_OK_c)
        {
            FLib_MemSet(&aBondingDataDynamic[mEntryIdx], 0, gBleBondDataDynamicSize_c);
            nvmStatus = NvSaveOnIdle((void*)&aBondingDataDynamic[mEntryIdx], FALSE);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            FLib_MemSet(&aBondingDataStatic[mEntryIdx], 0, gBleBondDataStaticSize_c);
            nvmStatus = NvSaveOnIdle((void*)&aBondingDataStatic[mEntryIdx], FALSE);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            FLib_MemSet(&aBondingDataLegacy[mEntryIdx], 0, gBleBondDataLegacySize_c);
            nvmStatus = NvSaveOnIdle((void*)&aBondingDataLegacy[mEntryIdx], FALSE);
        }
        
        if (nvmStatus == gNVM_OK_c)
        {
            FLib_MemSet(&aBondingDataDeviceInfo[mEntryIdx], 0, gBleBondDataDeviceInfoSize_c);
            nvmStatus = NvSaveOnIdle((void*)&aBondingDataDeviceInfo[mEntryIdx], FALSE);
        }
        
        for(mDescIdx = ((uint32_t)mEntryIdx * gcGapMaximumSavedCccds_c);
            mDescIdx < ((uint32_t)mEntryIdx + 1U) * gcGapMaximumSavedCccds_c; mDescIdx++)
        {
            FLib_MemSet(&aBondingDataDescriptor[mDescIdx], 0, gBleBondDataDescriptorSize_c);
            nvmStatus = NvSaveOnIdle((void*)&aBondingDataDescriptor[mDescIdx], FALSE);
            
            if (nvmStatus == gNVM_OK_c)
            {
                break;
            }
        }
#endif
        if (nvmStatus != gNVM_OK_c)
        {
            status = gBleNVMError_c;
        }
#else /* gAppUseNvm_d */
        FLib_MemSet(&maBondDataBlobs[mEntryIdx], 0, sizeof(bleBondDataBlob_t));
#endif
    }
    
    return status;
}

/*! *********************************************************************************
*\fn           bleResult_t App_NvmWrite(uint8_t  mEntryIdx,
*                                void*    pBondHeader,
*                                void*    pBondDataDynamic,
*                                void*    pBondDataStatic,
*                                void*    pBondDataLegacy,
*                                void*    pBondDataDeviceInfo,
*                                void*    pBondDataDescriptor,
*                                uint8_t  mDescriptorIndex)
*\brief        Write data to NVM.
*
*\param  [in]  mEntryIdx              NVM entry index.
*\param  [in]  pBondHeader            Pointer to the bonding header. Can be NULL.
*\param  [in]  pBondDataDynamic       Pointer to the dynamic bonding data structure.
*                                     Can be NULL.
*\param  [in]   pBondDataStatic       Pointer to the static bonding data structure.
*                                     Can be NULL.
*\param  [in]   pBondDataLegacy       Pointer to the legacy bonding data structure.
*                                     Can be NULL.
*\param  [in]  pBondDataDeviceInfo    Pointer to the bonding data device info
*                                     structure. Can be NULL.
*\param  [in]  pBondDataDescriptor    Pointer to the bonding data descriptor.
*                                     Can be NULL.
*\param  [in]  mDescriptorIndex       Bonding data descriptor index.
*
* \return    bleResult_t
*
********************************************************************************** */
bleResult_t App_NvmWrite
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataLegacy,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
    bleResult_t status = gBleSuccess_c;
#if gAppUseNvm_d
    NVM_Status_t nvmStatus = gNVM_OK_c;
#endif
    if(mEntryIdx >= (uint8_t)gMaxBondedDevices_c)
    {
          status = gBleInvalidParameter_c;
    }
    else
    {
#if gAppUseNvm_d
        uint8_t  idx   = 0;
        
#if gUnmirroredFeatureSet_d == TRUE
        uint32_t mSize = 0;
        void**   ppNvmData = NULL;
        void*    pRamData = NULL;
#endif
        
#if gUnmirroredFeatureSet_d == TRUE
        
        for(idx = 0U; idx < 6U; idx++)
        {
            ppNvmData = NULL;
            switch(*(uint8_t*)&idx)
            {
                case 0:
                {
                    if(pBondHeader != NULL)
                    {
                        ppNvmData = (void**)&aBondingHeader[mEntryIdx];
                        pRamData  = pBondHeader;
                        mSize     = gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c;
                    }
                }
                break;
                case 1:
                {
                    if(pBondDataDynamic != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataDynamic[mEntryIdx];
                        pRamData  = pBondDataDynamic;
                        mSize     = gBleBondDataDynamicSize_c;
                    }
                }
                break;
                case 2:
                {
                    if(pBondDataStatic != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataStatic[mEntryIdx];
                        pRamData  = pBondDataStatic;
                        mSize     = gBleBondDataStaticSize_c;
                    }
                }
                break;
                case 3:
                {
                    if(pBondDataLegacy != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataLegacy[mEntryIdx];
                        pRamData  = pBondDataLegacy;
                        mSize     = gBleBondDataLegacySize_c;
                    }
                }
                break;
                case 4:
                {
                    if(pBondDataDeviceInfo != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataDeviceInfo[mEntryIdx];
                        pRamData  = pBondDataDeviceInfo;
                        mSize     = gBleBondDataDeviceInfoSize_c;
                    }
                }
                break;
                case 5:
                {
                    if(pBondDataDescriptor != NULL)
                    {
                        if(mDescriptorIndex < gcGapMaximumSavedCccds_c)
                        {
                            ppNvmData = (void**)&aBondingDataDescriptor[mEntryIdx *
                                gcGapMaximumSavedCccds_c +
                                    mDescriptorIndex];
                            pRamData  = pBondDataDescriptor;
                            mSize     = gBleBondDataDescriptorSize_c;
                        }
                    }
                }
                break;
                default:
                ; /* No action required */
                break;
            }
            
            if(ppNvmData != NULL)
            {
                if(gNVM_OK_c == NvMoveToRam(ppNvmData))
                {
                    FLib_MemCpy(*ppNvmData, pRamData, mSize);
                    nvmStatus = NvSaveOnIdle(ppNvmData, FALSE);
                }
                else
                {
                    *ppNvmData = pRamData;
                    nvmStatus = NvSyncSave(ppNvmData, FALSE);
                }
            }
            
            if (nvmStatus != gNVM_OK_c)
            {
                /* An error occured, return error status. */
                status = gBleNVMError_c;
                break;
            }
        }
#else // gMirroredFeatureSet_d
        
        for(idx = 0U; idx < 6U; idx++)
        {
            switch(idx)
            {
                case 0:
                {
                    if(pBondHeader != NULL)
                    {
                        FLib_MemCpy((void*)&aBondingHeader[mEntryIdx], pBondHeader, gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c);
                        nvmStatus = NvSaveOnIdle((void*)&aBondingHeader[mEntryIdx], FALSE);
                    }
                }
                break;
                case 1:
                {
                    if(pBondDataDynamic != NULL)
                    {
                        FLib_MemCpy((void*)&aBondingDataDynamic[mEntryIdx], pBondDataDynamic,
                                    gBleBondDataDynamicSize_c);
                        nvmStatus = NvSaveOnIdle((void*)&aBondingDataDynamic[mEntryIdx], FALSE);
                    }
                }
                break;
                case 2:
                {
                    if(pBondDataStatic != NULL)
                    {
                        FLib_MemCpy((void*)&aBondingDataStatic[mEntryIdx], pBondDataStatic, gBleBondDataStaticSize_c);
                        nvmStatus = NvSaveOnIdle((void*)&aBondingDataStatic[mEntryIdx], FALSE);
                    }
                }
                break;
                case 3:
                {
                    if(pBondDataLegacy != NULL)
                    {
                        FLib_MemCpy((void*)&aBondingDataLegacy[mEntryIdx], pBondDataLegacy, gBleBondDataLegacySize_c);
                        nvmStatus = NvSaveOnIdle((void*)&aBondingDataLegacy[mEntryIdx], FALSE);
                    }
                }
                break;
                case 4:
                {
                    if(pBondDataDeviceInfo != NULL)
                    {
                        FLib_MemCpy((void*)&aBondingDataDeviceInfo[mEntryIdx], pBondDataDeviceInfo, gBleBondDataDeviceInfoSize_c);
                        nvmStatus = NvSaveOnIdle((void*)&aBondingDataDeviceInfo[mEntryIdx], FALSE);
                    }
                }
                break;
                case 5:
                {
                    if(pBondDataDescriptor != NULL)
                    {
                        if(mDescriptorIndex < gcGapMaximumSavedCccds_c)
                        {
                            FLib_MemCpy((void*)&aBondingDataDescriptor[mEntryIdx * gcGapMaximumSavedCccds_c + 
                                        mDescriptorIndex], pBondDataDescriptor, gBleBondDataDescriptorSize_c);
                            nvmStatus = NvSaveOnIdle((void*)&aBondingDataDescriptor[mEntryIdx *
                                                     gcGapMaximumSavedCccds_c + mDescriptorIndex], FALSE);
                        }
                    }
                }
                break;
                default:
                {
                    ; /* No action required */
                }
                break;
            }
            
            if (nvmStatus != gNVM_OK_c)
            {
                /* An error occured, return error status.*/
                status = gBleNVMError_c;
                break;
            }
        }
        
#endif //gUnmirroredFeatureSet_d
        
#else
        
        if(pBondHeader != NULL)
        {
            FLib_MemCpy(&maBondDataBlobs[mEntryIdx].bondHeader, pBondHeader, gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c);
        }
        
        if(pBondDataDynamic != NULL)
        {
            FLib_MemCpy((uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobDynamic,
                        pBondDataDynamic,
                        gBleBondDataDynamicSize_c
                            );
        }
        
        if(pBondDataStatic != NULL)
        {
            FLib_MemCpy((uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobStatic,
                        pBondDataStatic,
                        gBleBondDataStaticSize_c
                            );
        }
        
        if(pBondDataLegacy != NULL)
        {
            FLib_MemCpy((uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobLegacy,
                        pBondDataLegacy,
                        gBleBondDataLegacySize_c
                            );
        }
        
        if(pBondDataDeviceInfo != NULL)
        {
            FLib_MemCpy((uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobDeviceInfo,
                        pBondDataDeviceInfo,
                        gBleBondDataDeviceInfoSize_c
                            );
        }
        
        if(pBondDataDescriptor != NULL && mDescriptorIndex != gcGapMaximumSavedCccds_c)
        {
            FLib_MemCpy((uint8_t*)&(maBondDataBlobs[mEntryIdx].bondDataDescriptors[mDescriptorIndex]),
                        pBondDataDescriptor,
                        gBleBondDataDescriptorSize_c
                            );
        }
        
#endif
    }
    
    return status;
}

/*! *********************************************************************************
*\fn        bleResult_t App_NvmRead(uint8_t  mEntryIdx,
*                             void*    pBondHeader,
*                             void*    pBondDataDynamic,
*                             void*    pBondDataStatic,
*                             void*    pBondDataLegacy,
*                             void*    pBondDataDeviceInfo,
*                             void*    pBondDataDescriptor,
*                             uint8_t  mDescriptorIndex)
*\brief      Read data from NVM.
*
*\param[in]  mEntryIdx              NVM entry index.
*\param[in]  pBondHeader            Pointer to the place where the the bonding header
*                                   will be read.. Can be NULL.
*\param[in]  pBondDataDynamic       Pointer to the place where the the dynamic bonding
*                                   data structure will be read. Can be NULL.
*\param[in]  pBondDataStatic        Pointer to the place where the static bonding data
*                                   structure will be read. Can be NULL.
*\param[in]  pBondDataLegacy        Pointer to the place where the legacy bonding data
*                                   structure will be read. Can be NULL.
*\param[in]  pBondDataDeviceInfo    Pointer to the place where the bonding data device
*                                   info structure will be read. Can be NULL.
*\param[in]  pBondDataDescriptor    Pointer to the place where the bonding data descriptor
*                                    will be read. Can be NULL.
*\param[in]  mDescriptorIndex       Bonding data descriptor index.
*
* \return  bleResult_t
********************************************************************************** */
bleResult_t App_NvmRead
(
    uint8_t  mEntryIdx,
    void*    pBondHeader,
    void*    pBondDataDynamic,
    void*    pBondDataStatic,
    void*    pBondDataLegacy,
    void*    pBondDataDeviceInfo,
    void*    pBondDataDescriptor,
    uint8_t  mDescriptorIndex
)
{
    bleResult_t status = gBleSuccess_c;

    if(mEntryIdx >= (uint8_t)gMaxBondedDevices_c)
    {
          status = gBleInvalidParameter_c;
    }
    else
    {
#if gAppUseNvm_d
        uint8_t  idx = 0;
#if gUnmirroredFeatureSet_d == TRUE
        uint32_t mSize = 0;
        void**   ppNvmData = NULL;
        void*    pRamData = NULL;
#endif
        
#if gUnmirroredFeatureSet_d == TRUE
        for(idx = 0U; idx < 6U; idx++)
        {
            ppNvmData = NULL;
            switch(*(uint8_t*)&idx)
            {
                case 0:
                {
                    if(pBondHeader != NULL)
                    {
                        ppNvmData = (void**)&aBondingHeader[mEntryIdx];
                        pRamData  = pBondHeader;
                        mSize     = gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c;
                    }
                }
                break;
                case 1:
                {
                    if(pBondDataDynamic != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataDynamic[mEntryIdx];
                        pRamData  = pBondDataDynamic;
                        mSize     = gBleBondDataDynamicSize_c;
                    }
                }
                break;
                case 2:
                {
                    if(pBondDataStatic != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataStatic[mEntryIdx];
                        pRamData  = pBondDataStatic;
                        mSize     = gBleBondDataStaticSize_c;
                    }
                }
                break;
                case 3:
                {
                    if(pBondDataLegacy != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataLegacy[mEntryIdx];
                        pRamData  = pBondDataLegacy;
                        mSize     = gBleBondDataLegacySize_c;
                    }
                }
                break;
                case 4:
                {
                    if(pBondDataDeviceInfo != NULL)
                    {
                        ppNvmData = (void**)&aBondingDataDeviceInfo[mEntryIdx];
                        pRamData  = pBondDataDeviceInfo;
                        mSize     = gBleBondDataDeviceInfoSize_c;
                    }
                }
                break;
                case 5:
                {
                    if(pBondDataDescriptor != NULL)
                    {
                        if(mDescriptorIndex < gcGapMaximumSavedCccds_c)
                        {
                            ppNvmData = (void**)&aBondingDataDescriptor[mEntryIdx *
                                gcGapMaximumSavedCccds_c +
                                    mDescriptorIndex];
                            pRamData  = pBondDataDescriptor;
                            mSize     = gBleBondDataDescriptorSize_c;
                        }
                    }
                }
                break;
                default:
                {
                    ; /* No action required */
                }
                break;
            }
            
            /* if ppNvmData is not NULL the same holds for pRamData */
            if((NULL != ppNvmData) && (NULL != *ppNvmData))
            {
                FLib_MemCpy(pRamData, *ppNvmData, mSize);
            }
            if((NULL != ppNvmData) && (NULL == *ppNvmData))
            {
                status = gBleUnavailable_c;
                break;
            }
        }
#else // gMirroredFeatureSet_d
        for(idx = 0U; idx < 6U; idx++)
        {
            switch(idx)
            {
                case 0:
                {
                    if(pBondHeader != NULL)
                    {
                        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingHeader[mEntryIdx], FALSE))
                        {
                            FLib_MemCpy(pBondHeader, (void*)&aBondingHeader[mEntryIdx], gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c);
                        }
                        else
                        {
                            status = gBleNVMError_c;
                        }
                    }
                }
                break;
                case 1:
                {
                    if(pBondDataDynamic != NULL)
                    {
                        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingDataDynamic[mEntryIdx], FALSE))
                        {
                            FLib_MemCpy(pBondDataDynamic, (void*)&aBondingDataDynamic[mEntryIdx],
                                        gBleBondDataDynamicSize_c);
                        }
                        else
                        {
                            status = gBleNVMError_c;
                        }
                    }
                }
                break;
                case 2:
                {
                    if(pBondDataStatic != NULL)
                    {
                        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingDataStatic[mEntryIdx], FALSE))
                        {
                            FLib_MemCpy(pBondDataStatic, (void*)&aBondingDataStatic[mEntryIdx],
                                        gBleBondDataStaticSize_c);
                        }
                        else
                        {
                            status = gBleNVMError_c;
                        }
                    }
                }
                break;
                case 3:
                {
                    if(pBondDataLegacy != NULL)
                    {
                        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingDataLegacy[mEntryIdx], FALSE))
                        {
                            FLib_MemCpy(pBondDataLegacy, (void*)&aBondingDataLegacy[mEntryIdx],
                                        gBleBondDataLegacySize_c);
                        }
                        else
                        {
                            status = gBleNVMError_c;
                        }
                    }
                }
                break;
                case 4:
                {
                    if(pBondDataDeviceInfo != NULL)
                    {
                        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingDataDeviceInfo[mEntryIdx], FALSE))
                        {
                            FLib_MemCpy(pBondDataDeviceInfo, (void*)&aBondingDataDeviceInfo[mEntryIdx],
                                        gBleBondDataDeviceInfoSize_c);
                        }
                        else
                        {
                            status = gBleNVMError_c;
                        }
                    }
                }
                break;
                case 5:
                {
                    if(pBondDataDescriptor != NULL)
                    {
                        if(mDescriptorIndex < gcGapMaximumSavedCccds_c)
                        {
                            if(gNVM_OK_c == NvRestoreDataSet((void*)&aBondingDataDescriptor[mEntryIdx *
                                                             gcGapMaximumSavedCccds_c + mDescriptorIndex], FALSE))
                            {
                                FLib_MemCpy(pBondDataDescriptor, (void*)&aBondingDataDescriptor[mEntryIdx *
                                            gcGapMaximumSavedCccds_c + mDescriptorIndex], gBleBondDataDescriptorSize_c);
                            }
                            else
                            {
                                status = gBleNVMError_c;
                            }
                        }
                    }
                }
                break;
                default:
                {
                    ; /* No action required */
                }
                break;
            }
            
            if (status != gBleSuccess_c)
            {
                /* An error occured, return error status.*/
                break;
            }
        }
#endif
        
#else
        
        if(pBondHeader != NULL)
        {
            FLib_MemCpy(pBondHeader, &maBondDataBlobs[mEntryIdx].bondHeader, gBleBondIdentityHeaderSize_c - gIdentityHeaderOverhead_c);
        }
        
        if(pBondDataDynamic != NULL)
        {
            FLib_MemCpy(pBondDataDynamic,
                        (uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobDynamic,
                        gBleBondDataDynamicSize_c
                            );
        }
        
        if(pBondDataStatic != NULL)
        {
            FLib_MemCpy(pBondDataStatic,
                        (uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobStatic,
                        gBleBondDataStaticSize_c
                            );
        }
        
        if(pBondDataLegacy != NULL)
        {
            FLib_MemCpy(pBondDataLegacy,
                        (uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobLegacy,
                        gBleBondDataLegacySize_c
                            );
        }
        
        if(pBondDataDeviceInfo != NULL)
        {
            FLib_MemCpy(pBondDataDeviceInfo,
                        (uint8_t*)&maBondDataBlobs[mEntryIdx].bondDataBlobDeviceInfo,
                        gBleBondDataDeviceInfoSize_c
                            );
        }
        
        if(pBondDataDescriptor != NULL && mDescriptorIndex < gcGapMaximumSavedCccds_c)
        {
            FLib_MemCpy(pBondDataDescriptor,
                        (uint8_t*)&(maBondDataBlobs[mEntryIdx].bondDataDescriptors[mDescriptorIndex]),
                        gBleBondDataDescriptorSize_c
                            );
        }
        
#endif
    }
    
    return status;
}
#if (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U))
/*! *********************************************************************************
*\fn           bleResult_t App_NvmWriteLocalKeys(uint8_t  mEntryIdx,
*                                                 void*   pLocalKey )
*\brief        Write local key to NVM.
*
*\param  [in]  mEntryIdx              NVM entry index 0/1 - local IRK/CSRK.
*\param  [in]  pLocalKey              Pointer to a structure of type bleLocalKeysBlob_t containing the key blob .
* \return    bleResult_t
*
********************************************************************************** */
bleResult_t App_NvmWriteLocalKeys
(
uint8_t  mEntryIdx,
void*    pLocalKey
)
{
    bleResult_t status = gBleSuccess_c;
#if gAppUseNvm_d
    NVM_Status_t nvmStatus = gNVM_OK_c;
#endif /* gAppUseNvm_d */
    if(mEntryIdx >= (uint8_t)gcSecureModeSavedLocalKeysNo_c)
    {
        status = gBleInvalidParameter_c;
    }
    else
    {
#if gAppUseNvm_d
        
#if gUnmirroredFeatureSet_d == TRUE
        uint32_t mSize = 0;
        void**   ppNvmData = (void**)&aBleLocalKeys[mEntryIdx];
        mSize =  sizeof(bleLocalKeysBlob_t);
        
        if(gNVM_OK_c == NvMoveToRam(ppNvmData))
        {
            FLib_MemCpy(*ppNvmData, pLocalKey, mSize);
            nvmStatus = NvSaveOnIdle(ppNvmData, FALSE);
        }
        else
        {
            *ppNvmData = pLocalKey;
            nvmStatus = NvSyncSave(ppNvmData, FALSE);
        }
        
        if (nvmStatus != gNVM_OK_c)
        {
            /* An error occured, return error status. */
            status = gBleNVMError_c;
        }
        
#else /* gUnmirroredFeatureSet_d */
        FLib_MemCpy((void*)&aBleLocalKeys[mEntryIdx], pLocalKey, sizeof(bleLocalKeysBlob_t));
        nvmStatus = NvSaveOnIdle((void*)&aBleLocalKeys[mEntryIdx], FALSE);
        
        if (nvmStatus != gNVM_OK_c)
        {
            /* An error occured, return error status.*/
            status = gBleNVMError_c;
        }
#endif /* gUnmirroredFeatureSet_d */
        
#else /* gAppUseNvm_d */
        FLib_MemCpy(&aBleLocalKeys[mEntryIdx], pLocalKey, sizeof(bleLocalKeysBlob_t));
#endif /* gAppUseNvm_d */
    }
    return status;
}

/*! *********************************************************************************
*\fn        bleResult_t App_NvmReadLocalKeys(uint8_t  mEntryIdx,
*                                            void*    pLocalKey)
*\brief      Read local key from NVM.
*
*\param[in]  mEntryIdx              NVM entry index 0/1 - local IRK/CSRK.
*\param[in]  pLocalKey              Pointer to a structure of type bleLocalKeysBlob_t 
*                                   where the key blob will be read.
*
* \return  bleResult_t
********************************************************************************** */
bleResult_t App_NvmReadLocalKeys
(
uint8_t  mEntryIdx,
void*    pLocalKey
)
{
    bleResult_t status = gBleSuccess_c;
    
    if(mEntryIdx >= (uint8_t)gcSecureModeSavedLocalKeysNo_c)
    {
        status = gBleInvalidParameter_c;
    }
    else
    {
#if gAppUseNvm_d
        
#if gUnmirroredFeatureSet_d == TRUE
        uint32_t mSize = 0;
        void**   ppNvmData = (void**)&aBleLocalKeys[mEntryIdx];
        mSize     = sizeof(bleLocalKeysBlob_t);
        if(NULL != *ppNvmData)
        {
            FLib_MemCpy(pLocalKey, *ppNvmData, mSize);
        }
        else
        {
            status = gBleUnavailable_c;
        }
        
#else /* gUnmirroredFeatureSet_d */
        if(gNVM_OK_c == NvRestoreDataSet((void*)&aBleLocalKeys[mEntryIdx], FALSE))
        {
            FLib_MemCpy(pLocalKey, (void*)&aBleLocalKeys[mEntryIdx], sizeof(bleLocalKeysBlob_t));
        }
        else
        {
            status = gBleNVMError_c;
        }                  
#endif /* gUnmirroredFeatureSet_d */
        
#else /* gAppUseNvm_d */
        FLib_MemCpy(pLocalKey, &aBleLocalKeys[mEntryIdx], sizeof(bleLocalKeysBlob_t));
#endif /* gAppUseNvm_d */
    }
    
    return status;
}
#endif /* (defined(gAppSecureMode_d) && (gAppSecureMode_d > 0U)) */