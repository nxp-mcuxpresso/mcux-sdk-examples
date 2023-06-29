/*! *********************************************************************************
* Copyright 2020 NXP
* All rights reserved.
*
* \file
*
* This is the header file for the OTA Programming Support.
*
** SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#ifndef _OTA_UTILS_H_
#define _OTA_UTILS_H_

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "EmbeddedTypes.h"
#include "psector_api.h"
#include "rom_secure.h"

/*! *********************************************************************************
*************************************************************************************
* Public MACRO
*************************************************************************************
********************************************************************************** */

#define OTA_UTILS_IMAGE_INVALID_ADDR              (0xFFFFFFFF)

#define OTA_UTILS_PSECT_SSBL_PARTITION_IMAGE_TYPE               (0x00)
#define OTA_UTILS_PSECT_OTA_PARTITION_IMAGE_TYPE                (0xFC)
#define OTA_UTILS_PSECT_NVM_PARTITION_IMAGE_TYPE                (0xFD)
#define OTA_UTILS_PSECT_EXT_FLASH_TEXT_PARTITION_IMAGE_TYPE     (0xFE)
#define OTA_UTILS_PSECT_RESERVED_PARTITION_IMAGE_TYPE           (0xFF) /* For reserving some pages for data storage outside PDM */


#ifndef BIT
#define BIT(x) (1<<(x))
#endif

#define OTA_BOOTABLE_IMAGE_FLAG      BIT(0)
#define OTA_CUSTOM_ENTRY_FLAG        BIT(2)
#define OTA_IMAGE_VALIDATED_FLAG     BIT(3)
#define OTA_IMAGE_APPLIED_FLAG       BIT(4)
#define OTA_AES_SW_CIPHERING_FLAG    BIT(6)
#define OTA_AES_FUSED_CIPHERING_FLAG BIT(7)

#if defined (gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
#define OTACustomStorage_Ram         0
#define OTACustomStorage_ExtFlash    1

#ifndef gOTACustomOtaEntryMemory
#define gOTACustomOtaEntryMemory     OTACustomStorage_Ram
#endif

#define OTAMaxCustomEntryNumber     8
#define OTAMaxCustomDataWords       16 /* 16 words = 64 Bytes */

#endif /* gOTAUseCustomOtaEntry */
/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */

typedef struct {
    IMAGE_CERT_T certificate;
    uint8_t signature[SIGNATURE_LEN];
} ImageCertificate_t;

typedef struct {
    uint8_t signature[SIGNATURE_LEN];
} ImageSignature_t;

typedef enum {
  gOtaUtilsSuccess_c = 0,
  gOtaUtilsReadError_c,
  gOtaUtilsInvalidKey_c,
  gOtaUtilsError_c,
  gOtaUtilsHwAcceleratorNotReady_c,
} otaUtilsResult_t;

typedef struct {
    const uint8_t *pSoftKeyAes;
} sOtaUtilsSoftwareKey;

typedef enum {
    eCipherKeyNone,
    eEfuseKey,
    eSoftwareKey,
} eEncryptionKeyType;

typedef struct {
    uint32_t mode;          /* Setup mode: ECB, etc. */
    uint32_t keySize;       /* Size of the AES key. */
    uint32_t* pSoftwareKey; /* Address to save the software key. */
    uint32_t flags;         /* Extra flags, normally 0. */
} aesContext_t;

#if defined (gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)

typedef enum {
    otaNoImage,
    otaNewImage,
    otaValidated,
    otaStarted,
    otaApplied,
    otaNotApplied
}otaCustomState_t;

typedef struct {
    image_directory_entry_t entries[OTAMaxCustomEntryNumber];   /* All entries */
    uint32_t custom_data[OTAMaxCustomDataWords];                /* Allow support for custom data */
    uint16_t custom_data_length;                                /* Size of custom data if needed */
    otaCustomState_t ota_state;                                 /* State of the OTA procedure - Used by SSBL to specify whether or not the OTA has been applied */
    uint8_t number_of_entry;                                    /* Number of entry filled */
} CustomOtaEntries_t;
#endif /* gOTAUseCustomOtaEntry */

/*
 * Function pointer to read from the flash driver
 */
typedef otaUtilsResult_t (*OtaUtils_EEPROM_ReadData)(uint16_t nbBytes,
                                                     uint32_t address,
                                                     uint8_t *pInbuf);

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
/*
 * Function pointer to write to the flash driver
 */
typedef otaUtilsResult_t (*OtaUtils_EEPROM_WriteData)(uint16_t nbBytes,
                                                     uint32_t address,
                                                     uint8_t *pInbuf);
#endif

/*
 * Function allowing to read data
 * Note: a OtaUtils_EEPROM_ReadData function is required to access flash drivers
 */
typedef otaUtilsResult_t (*OtaUtils_ReadBytes)(uint16_t nbBytes,
                                               uint32_t address,
                                               uint8_t *pInbuf,
                                               void *pParam,
                                               OtaUtils_EEPROM_ReadData eepromFunction);


/*! *********************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
********************************************************************************** */

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
* \brief  Allows hardware AES to use the efuse OTP secret key for operations.
*
* \param[in] pContext: software context that stores key/mode information.
* \param[in] keySize: size of the AES key
*
* \return    uint32_t
*
********************************************************************************** */
uint32_t OtaUtils_AesLoadKeyFromOTP(aesContext_t* pContext,
                                    uint32_t keySize);

/*! *********************************************************************************
* \brief  Allows hardware AES to use a software key for operations.
*
* \param[in] pContext: software context that stores key/mode information.
* \param[in] keySize: size of the AES key
* \param[in/out] pKey: address to save the software key
*
* \return    uint32_t
*
********************************************************************************** */
uint32_t OtaUtils_AesLoadKeyFromSW(aesContext_t* pContext,
                                   uint32_t  keySize,
                                   uint32_t* pKey);

/*! *********************************************************************************
* \brief  Allows hardware AES to select an operation mode.
*
* \param[in] pContext: software context that stores key/mode information.
* \param[in] modeVal: setup mode
* \param[in] flags: extra flags, normally 0
*
* \return    uint32_t
*
********************************************************************************** */
uint32_t OtaUtils_AesSetMode(aesContext_t* pContext,
                             uint32_t modeVal,
                             uint32_t flags);

/*! *********************************************************************************
* \brief  Allows hardware AES to start processing using selected mode and key.
*
* \param[in] pContext: software context that stores key/mode information.
* \param[in] pBlockIn: pointer to the location of the input
* \param[in/out] pBlockOut: pointer to the location of the output
* \param[in] numBlocks: number of 16 bytes blocks to be processed
*
* \return    uint32_t
*
********************************************************************************** */
uint32_t OtaUtils_AesProcessBlocks(const aesContext_t* pContext,
                                   uint32_t* pBlockIn,
                                   uint32_t* pBlockOut,
                                   uint32_t  numBlocks);

/*! *********************************************************************************
* \brief  Allows to read bytes from the Internal Flash
*
* \param[in] nbBytesToRead: number of bytes to read
* \param[in] address: offset/address to read from the internal flash
* \param[in/out] pOutbuf: buffer to fill
* \param[in] pParam: optional param
* \param[in] pFunctionEepromRead: Eeprom function
*
* \return    otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_ReadFromInternalFlash(uint16_t nbBytesToRead,
                                                uint32_t address,
                                                uint8_t *pOutbuf,
                                                void * pParam,
                                                OtaUtils_EEPROM_ReadData pFunctionEepromRead);

/*! *********************************************************************************
* \brief  Allows to read bytes from an encrypted flash using Efuse key
*
* \param[in] nbBytesToRead: number of bytes to read
* \param[in] address: offset/address to read from the internal flash
* \param[in/out] pOutbuf: buffer to fill
* \param[in] pParam: optional param
* \param[in] pFunctionEepromRead: Eeprom function
*
* \return    otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashEfuseKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead);

/*! *********************************************************************************
* \brief  Allows to read bytes from an encrypted flash using Software key
*
* \param[in] nbBytesToRead: number of bytes to read
* \param[in] address: offset/address to read from the internal flash
* \param[in/out] pOutbuf: buffer to fill
* \param[in] pParam: the software key (format: sOtaUtilsSoftwareKey)
* \param[in] pFunctionEepromRead: Eeprom function
*
* \return    otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashSoftwareKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead);

/*! *********************************************************************************
* \brief  Allows to read bytes from an unencrypted external flash
*
* \param[in] nbBytesToRead: number of bytes to read
* \param[in] address: offset/address to read from the internal flash
* \param[in/out] pOutbuf: buffer to fill
* \param[in] pParam: the software key (format: sOtaUtilsSoftwareKey)
* \param[in] pFunctionEepromRead: Eeprom function
*
* \return    otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_ReadFromUnencryptedExtFlash(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead);

/*! *********************************************************************************
* \brief  Get the internal flash limit address that can be erased/written according to partitions
*
* \param[in] -
*
* \return  maximum flash address that can be erased/written
*
********************************************************************************** */
uint32_t OtaUtils_GetModifiableInternalFlashTopAddress(void);

/*! *********************************************************************************
* \brief  Allows to check the partitions from PSECT Image Directory
*
* \param[in] page0: pointer to the PSECT (Page0)
* \param[in] ext_flash_size: Size of external flash
*
* \return  TRUE if partitions are good or FALSE if not valid
*
********************************************************************************** */
bool OtaUtils_ImgDirectorySanityCheck(psector_page_data_t * page0, uint32_t ext_flash_size);

/*! *********************************************************************************
* \brief  Allows validate an image
*
* \param[in] pFunctionRead: function pointer to read data
*            can be OtaUtils_ReadFromInternalFlash/OtaUtils_ReadFromEncryptedExtFlashEfuseKey
*            /OtaUtils_ReadFromEncryptedExtFlashSoftwareKey or others
* \param[in] pFunctionEepromRead: function pointer to data from a memory driver
* \param[in] imgAddr: address/offset from where to validate the image
* \param[in] minValidAddr: min valid address/offset
* \param[in] pRootCert: root certificate uses to authenticate the image
* \param[in] inOtaCheck: the image to check is coming from an OTA
* \param[in] isRemappable: image support remap or not
*
* \return  target addr of the image or IMAGE_INVALID_ADDR if not valid
*
********************************************************************************** */
uint32_t OtaUtils_ValidateImage(OtaUtils_ReadBytes pFunctionRead,
                                void *pReadFunctionParam,
                                OtaUtils_EEPROM_ReadData pFunctionEepromRead,
                                uint32_t imgAddr,
                                uint32_t minValidAddr,
                                const IMAGE_CERT_T * pRootCert,
                                bool_t inOtaCheck, bool_t isRemappable);

/*! *********************************************************************************
* \brief  Allows to reconstruct the device root certificate
*
* \param[in/out] pCert: certificate to be filled
* \param[in] pPage0 content
* \param[in] pFlashPage content or NULL
*
* \return otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_ReconstructRootCert(IMAGE_CERT_T *pCert, const psector_page_data_t* pPage0, const psector_page_data_t* pFlashPage);

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
/*! *********************************************************************************
* \brief  Allows to store custom OTA entry structure in Ram/Flash
*
* \param[in] pEntry: OTA entry fields to be stored in RAM/Flash
* \param[in] pFunctionWrite: function pointer to write data from a memory driver
* \param[in] storageTopAddr: Top address of the storage
*
* \return otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_StoreCustomOtaEntry(CustomOtaEntries_t *pEntry, OtaUtils_EEPROM_WriteData pFunctionWrite, uint32_t storageTopAddr);

/*! *********************************************************************************
* \brief  Access custom OTA entry structure in Ram/Flash
*
* \param[in/out] data: structure to receive the OTA entry fields from RAM/Flash
* \param[in/out] lenght_bytes: size in bytes of the data
* \param[in] pFunctionRead: function pointer to read data from a memory driver
* \param[in] storageTopAddr: Top address of the storage
*
* \return otaUtilsResult_t
*
********************************************************************************** */
otaUtilsResult_t OtaUtils_GetCustomOtaEntry(CustomOtaEntries_t *pEntry, uint16_t *pLenghtBytes, OtaUtils_EEPROM_ReadData pFunctionRead, uint32_t storageTopAddr);
#endif /* gOTAUseCustomOtaEntry */

#ifdef __cplusplus
}
#endif

#endif /* _OTA_UTILS_H_ */
