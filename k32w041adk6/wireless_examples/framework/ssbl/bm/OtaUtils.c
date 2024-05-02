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

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "OtaUtils.h"

/* Driver includes */
#include "fsl_flash.h"
#include "fsl_sha.h"

#include "flash_header.h"
#include "rom_psector.h"
#include "rom_secure.h"
#include "rom_api.h"

/* This flag can be set to 1 to redirect OTA AES usage through SecLib,
 * ensuring mutex protection against concurrent access to AES hardware. */
#ifndef gOTA_UseSecLibAes
#define gOTA_UseSecLibAes 0
#endif

#if gOTA_UseSecLibAes
#include "SecLib.h"
#else
#include "rom_aes.h"
#endif

/************************************************************************************
*************************************************************************************
* Private Macros
*************************************************************************************
************************************************************************************/

#define THUMB_ENTRY(x)                 (void*)((x) | 1)
#define CRC_FINALIZE(x)                ((x) ^ ~0UL)

#ifdef PDM_EXT_FLASH
#define DEFAULT_PDM_SIZE                0x00
#else
#define DEFAULT_PDM_SIZE                0x7e00
#endif
#define INTERNAL_FLASH_MAX_SAFE_VALUE   0x9de00

#define SIGNATURE_WRD_LEN               (SIGNATURE_LEN / 4)

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
#define OTA_ENTRY_TAG                    0x41544F45
#define ROM_API_HASH_SHA256              THUMB_ENTRY(0x03006210)
#endif /* gOTAUseCustomOtaEntry */

#define ROM_API_efuse_LoadUniqueKey      THUMB_ENTRY(0x030016f4)
#define ROM_API_aesLoadKeyFromOTP        THUMB_ENTRY(0x0300146c)
#define ROM_API_crc_update               THUMB_ENTRY(0x0300229c)
#define ROM_API_boot_CheckVectorSum      THUMB_ENTRY(0x03000648)
#define ROM_API_flash_GetDmaccStatus     THUMB_ENTRY(0x03001f64)

#define BUFFER_SHA_LENGTH                16
#define OTA_UTILS_DEBUG(...)

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

typedef struct {
    uint16_t blob_id;
    uint32_t blob_version;
} ImageCompatibilityListElem_t;

typedef union
{
    IMG_HEADER_T       imgHeader;
    BOOT_BLOCK_T       imgBootBlock;
    ImageCertificate_t imgCertificate; /* will contains only the img signature if no certificate is given */
} ImageParserUnion;

typedef int (*efuse_LoadUniqueKey_t)(void);
typedef uint32_t (*aesLoadKeyFromOTP_t)(AES_KEY_SIZE_T keySize);
typedef uint32_t (*crc_update_t)(uint32_t crc, const void* data, size_t data_len);
typedef uint32_t (*boot_CheckVectorSum_t)(const IMG_HEADER_T *image);
typedef uint32_t (*flash_GetDmaccStatus_t)(uint8_t *address);

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)

typedef uint32_t (*hash_sha256_t)(uint32_t *addr, uint32_t length, uint32_t *hash);

typedef struct {
    CustomOtaEntries_t *pEntry;
    uint32_t short_hash;
    uint32_t length;
    uint32_t magic;
} OtaCustomStorage_t;

#if (gOTACustomOtaEntryMemory == OTACustomStorage_Ram)
#if defined(__CC_ARM)
extern uint32_t Image$$__base_RAM$$Base[];
extern uint32_t Image$$__top_RAM$$Length[];
extern uint32_t Image$$__base_RAM1$$End[];
extern uint32_t Image$$__top_RAM1$$Length[];
#else /* defined(__CC_ARM) */
extern uint32_t __base_RAM[];
extern uint32_t __top_RAM[];
extern uint32_t __base_RAM1[];
extern uint32_t __top_RAM1[];
#endif
#endif /* gOTACustomOtaEntryMemory */


#endif /* gOTAUseCustomOtaEntry */
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if !defined(gOTA_UseSecLibAes) || (gOTA_UseSecLibAes == 0)
static const efuse_LoadUniqueKey_t efuse_LoadUniqueKey     = (efuse_LoadUniqueKey_t) ROM_API_efuse_LoadUniqueKey;
static const aesLoadKeyFromOTP_t aesLoadKeyFromOTP         = (aesLoadKeyFromOTP_t) ROM_API_aesLoadKeyFromOTP;
#endif
static const crc_update_t crc_update                       = (crc_update_t)ROM_API_crc_update;
static const boot_CheckVectorSum_t boot_CheckVectorSum     = (boot_CheckVectorSum_t)ROM_API_boot_CheckVectorSum;
static const flash_GetDmaccStatus_t flash_GetDmaccStatus   = (flash_GetDmaccStatus_t) ROM_API_flash_GetDmaccStatus;

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
static const hash_sha256_t hash_sha256     = (hash_sha256_t) ROM_API_HASH_SHA256;
#endif /* gOTAUseCustomOtaEntry */

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

static bool_t OtaUtils_IsInternalFlashAddr(uint32_t image_addr)
{
    uint32_t internalFlashAddrStart = 0;
    uint32_t internalFlashSize = 0;
    ROM_GetFlash(&internalFlashAddrStart, &internalFlashSize);
    return ((image_addr >= internalFlashAddrStart)
            && image_addr < (internalFlashAddrStart+internalFlashSize));
}

static bool_t OtaUtils_IsExternalFlashAddr(uint32_t image_addr, uint32_t ext_flash_size)
{
    return (image_addr >= (uint32_t) FSL_FEATURE_SPIFI_START_ADDR
                && image_addr < (FSL_FEATURE_SPIFI_START_ADDR + ext_flash_size));
}

static const psector_page_data_t * OtaUtils_GetPage0ValidSubpage(void)
{
#define SECT_STATE_PAGE0_POS 0
#define SECT_STATE_PAGE0_WIDTH 16

    const psector_page_data_t * ret = NULL;
    const uint32_t * pBI_psect_state = (uint32_t*)0x04015fe4;
    uint32_t  val = * pBI_psect_state;
    uint8_t subpage_state;
    val = (val >> SECT_STATE_PAGE0_POS) & ((1<<SECT_STATE_PAGE0_WIDTH) -1);
    const psector_page_data_t * page_addr[2] = { (psector_page_data_t *)0x9e800, (psector_page_data_t *)0x9ea00 };
    for (int i = 0; i < 2; i++)
    {
        subpage_state = (val & 0xff);
        if (subpage_state != PAGE_STATE_ERROR)
        {
            ret =  page_addr[i];
            break;
        }
        val >>=8;
    }
    return ret;
}

/* In case of wrong ImgType, IMG_TYPE_NB is returned  */
static uint8_t OtaUtils_CheckImageTypeFromImgHeader(const IMG_HEADER_T *pImageHeader)
{
    uint8_t imgType = IMG_DIRECTORY_MAX_SIZE;
    if (pImageHeader && pImageHeader->imageSignature >= IMAGE_SIGNATURE
            && pImageHeader->imageSignature < IMAGE_SIGNATURE + IMG_DIRECTORY_MAX_SIZE)
    {
        imgType = (pImageHeader->imageSignature - IMAGE_SIGNATURE);
    }
    return imgType;
}

static otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlash(uint16_t nbBytesToRead,
                                                           uint32_t address,
                                                           uint8_t *pOutbuf,
                                                           OtaUtils_EEPROM_ReadData pFunctionEepromRead,
                                                           eEncryptionKeyType eType,
                                                           void *pParam)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    otaUtilsResult_t readResult = gOtaUtilsSuccess_c;
    uint8_t alignedBufferStart[16];
    uint8_t alignedBufferEnd[16];
    uint16_t nbByteToRead = nbBytesToRead;
    uint8_t *pBuf = pOutbuf;

    uint32_t lastAddrToRead = address+nbBytesToRead-1;

    /* Encrypted reads require to have an addr aligned on 16 bytes */
    uint16_t nbByteToAlignStart = address % 16;
    uint16_t nbByteToAlignEnd = (16*(lastAddrToRead/16) + 15) - lastAddrToRead;
    uint16_t nbByteToMoveInAlignedBufferStart = 0;
    uint16_t nbByteToMoveInAlignedBufferEnd = 0;
    uint16_t nbByteToReadBeforeEndAlignBuffer = 0;

    address -= nbByteToAlignStart;

    do {
#ifdef DEBUG
        if ((nbByteToRead + nbByteToAlignStart + nbByteToAlignEnd)%16 != 0)
            break;
#endif
        /* Get the number of block that we will need to read */
        int nb_blocks =  (nbByteToRead + nbByteToAlignStart + nbByteToAlignEnd)/16;

        if (nbByteToAlignStart)
        {
            if ((readResult=pFunctionEepromRead(sizeof(alignedBufferStart),  address, &alignedBufferStart[0])) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
            else
            {
                address+= sizeof(alignedBufferStart);
            }
        }

        /* Check if we need to read more bytes */
        if (address < lastAddrToRead)
        {
            if (nbByteToAlignStart)
            {
                nbByteToMoveInAlignedBufferStart = sizeof(alignedBufferStart) - nbByteToAlignStart;
                pBuf += nbByteToMoveInAlignedBufferStart;
            }

            if (nbByteToAlignEnd)
            {
                nbByteToMoveInAlignedBufferEnd = sizeof(alignedBufferEnd) - nbByteToAlignEnd;
            }
            nbByteToReadBeforeEndAlignBuffer = nbByteToRead - nbByteToMoveInAlignedBufferStart - nbByteToMoveInAlignedBufferEnd;
            if (nbByteToReadBeforeEndAlignBuffer%16 != 0)
                break;
            if ((readResult=pFunctionEepromRead(nbByteToReadBeforeEndAlignBuffer,  address, pBuf)) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
            address += nbByteToReadBeforeEndAlignBuffer;
            if (nbByteToAlignEnd && (readResult=pFunctionEepromRead(sizeof(alignedBufferEnd),  address, alignedBufferEnd)) != gOtaUtilsSuccess_c)
            {
                result = readResult;
                break;
            }
        }
        else
        {
            /* The asked buffer is too small and can fit in alignedBufferStart */
            nbByteToAlignEnd = 0;
        }

        aesContext_t aesContext;
        if (eType == eEfuseKey)
        {
            //aesInit();
            OtaUtils_AesLoadKeyFromOTP(&aesContext, AES_KEY_128BITS);
        }
        else if (eType == eSoftwareKey && pParam != NULL)
        {
            sOtaUtilsSoftwareKey * pSoftKey = (sOtaUtilsSoftwareKey *) pParam;
            OtaUtils_AesLoadKeyFromSW(&aesContext, AES_KEY_128BITS, (uint32_t*)pSoftKey->pSoftKeyAes);
            break;
        }

        OtaUtils_AesSetMode(&aesContext, AES_MODE_ECB_DECRYPT, AES_INT_BSWAP | AES_OUTT_BSWAP);
        if (nbByteToAlignStart)
        {
            OtaUtils_AesProcessBlocks(&aesContext, (void*)alignedBufferStart, (void*)alignedBufferStart,  1);
            nb_blocks -=1;
        }
        if (nbByteToAlignEnd)
        {
            OtaUtils_AesProcessBlocks(&aesContext, (void*)pBuf, (void*)pBuf,  nb_blocks-1);
            OtaUtils_AesProcessBlocks(&aesContext, (void*)alignedBufferEnd, (void*)alignedBufferEnd,  1);
        }
        else
        {
            OtaUtils_AesProcessBlocks(&aesContext, (void*)pBuf, (void*)pBuf,  nb_blocks);
        }

        /* Fill missing pBuf bytes */
        pBuf-=nbByteToMoveInAlignedBufferStart;

        if (nbByteToAlignStart)
        {
            uint16_t i;
            uint16_t t=0;
            for (i=nbByteToAlignStart; i<sizeof(alignedBufferStart); i++)
            {
                pBuf[t++] = alignedBufferStart[i];
            }
        }

        if (nbByteToAlignEnd)
        {
            uint16_t i;
            for (i=0; i<nbByteToMoveInAlignedBufferEnd; i++)
            {
                *(pBuf+nbByteToMoveInAlignedBufferStart+nbByteToReadBeforeEndAlignBuffer+i) = alignedBufferEnd[i];
            }
        }
        result = gOtaUtilsSuccess_c;
    } while (0);

    return result;
}

static bool_t OtaUtils_VerifySignature(uint32_t address,
                                       uint32_t nbBytesToRead,
                                       const uint32_t *pPublicKey,
                                       const uint8_t *pSignatureToVerify,
                                       OtaUtils_ReadBytes pFunctionRead,
                                       void * pReadFunctionParam,
                                       OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    bool_t result = FALSE;
    uint32_t nbPageToRead = nbBytesToRead/BUFFER_SHA_LENGTH;
    uint32_t lastPageNbByteNumber = nbBytesToRead - (nbPageToRead*BUFFER_SHA_LENGTH);
    uint32_t i = 0;
    do {
        uint8_t pageContent[BUFFER_SHA_LENGTH];
        uint8_t digest[32];
        sha_ctx_t hash_ctx;
        size_t sha_sz = 32;
        /* Initialise SHA clock do not call SHA_ClkInit(SHA0) because the HAL pulls in too much code  */
        SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_HASH_MASK;
        if (SHA_Init(SHA0, &hash_ctx, kSHA_Sha256) != kStatus_Success)
            break;
        for (i=0; i<nbPageToRead; i++)
        {
            if (pFunctionRead(sizeof(pageContent),  address+(i*BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                break;
            if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, BUFFER_SHA_LENGTH) != kStatus_Success)
                break;
        }
        /* Read bytes located on the last page */
        if (pFunctionRead(lastPageNbByteNumber,  address+(i*BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;
        if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, lastPageNbByteNumber) != kStatus_Success)
                break;
        if (SHA_Finish(SHA0,  &hash_ctx, &digest[0], &sha_sz) != kStatus_Success)
            break;
        if (!secure_VerifySignature(digest, pSignatureToVerify, pPublicKey))
            break;
        result = TRUE;
    } while (0);

    SYSCON->AHBCLKCTRLCLR[1] =  SYSCON_AHBCLKCTRL1_HASH_MASK; /* equivalent to SHA_ClkDeinit(SHA0) */
    return result;
}

static bool_t OtaUtils_FindBlankPage(uint32_t startAddr, uint16_t size)
{
    bool_t result = FALSE;
    uint32_t addrIterator = startAddr;

    addrIterator &= ~(FLASH_PAGE_SIZE -1); /* Align address iterator with begining of the flash page.
                                              This allows to check the exact number of page needed in a single while loop
                                              and not checking an extra page when startAddr+size is a multiple of FLASH_PAGE_SIZE */

    while (addrIterator < startAddr+size)
    {
        if (flash_GetDmaccStatus((uint8_t *)addrIterator) == 0)
        {
            result = TRUE;
            break;
        }
        addrIterator += FLASH_PAGE_SIZE;
    }

    return result;
}

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

uint32_t OtaUtils_AesLoadKeyFromOTP(aesContext_t* pContext,
                                    uint32_t keySize)
{
#if gOTA_UseSecLibAes
    pContext->keySize = keySize;
    pContext->pSoftwareKey = NULL;
    return 0;
#else
    efuse_LoadUniqueKey();
    return aesLoadKeyFromOTP((AES_KEY_SIZE_T)keySize);
#endif
}

uint32_t OtaUtils_AesLoadKeyFromSW(aesContext_t* pContext,
                                   uint32_t  keySize,
                                   uint32_t* pKey)
{
#if gOTA_UseSecLibAes
    pContext->keySize = keySize;
    pContext->pSoftwareKey = pKey;
    return 0;
#else
    return aesLoadKeyFromSW((AES_KEY_SIZE_T)keySize, pKey);
#endif
}

uint32_t OtaUtils_AesSetMode(aesContext_t* pContext,
                          uint32_t modeVal,
                          uint32_t flags)
{
#if gOTA_UseSecLibAes
    pContext->mode = modeVal;
    pContext->flags = flags;
    return 0;
#else
    return aesMode((AES_MODE_T)modeVal, flags);
#endif
}

uint32_t OtaUtils_AesProcessBlocks(const aesContext_t* pContext,
                                   uint32_t* pBlockIn,
                                   uint32_t* pBlockOut,
                                   uint32_t  numBlocks)
{
#if gOTA_UseSecLibAes
    return AES_128_ProcessBlocks((const void*)pContext, pBlockIn, pBlockOut, numBlocks);
#else
    return aesProcess(pBlockIn, pBlockOut, numBlocks);
#endif
}

otaUtilsResult_t OtaUtils_ReadFromInternalFlash(uint16_t nbBytesToRead,
                                                uint32_t address,
                                                uint8_t *pOutbuf,
                                                void *pParam,
                                                OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t result = gOtaUtilsError_c;

    do {
        if (!OtaUtils_IsInternalFlashAddr(address))
            break;
        /* If one blank page is found return error */
        if (OtaUtils_FindBlankPage(address, nbBytesToRead))
            break;
        if (pFunctionEepromRead == NULL || pOutbuf == NULL)
            break;
        pFunctionEepromRead(nbBytesToRead, address, pOutbuf);
        result = gOtaUtilsSuccess_c;
    } while (0);

    return result;
}


otaUtilsResult_t OtaUtils_ReadFromUnencryptedExtFlash(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    if (pFunctionEepromRead != NULL)
    {
        result = pFunctionEepromRead(nbBytesToRead, address, pOutbuf);
    }
    return result;
}

otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashEfuseKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    return OtaUtils_ReadFromEncryptedExtFlash(nbBytesToRead, address, pOutbuf, pFunctionEepromRead, eEfuseKey, NULL);
}

otaUtilsResult_t OtaUtils_ReadFromEncryptedExtFlashSoftwareKey(uint16_t nbBytesToRead,
                                                            uint32_t address,
                                                            uint8_t *pOutbuf,
                                                            void *pParam,
                                                            OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    return OtaUtils_ReadFromEncryptedExtFlash(nbBytesToRead, address, pOutbuf, pFunctionEepromRead, eSoftwareKey, pParam);
}

uint32_t OtaUtils_GetModifiableInternalFlashTopAddress(void)
{
    uint32_t int_reserved_or_pdm_size = 0;
    bool is_pdm_found = false;

    /* Access PSECT from flash directly without initiate full structure in RAM (only pointer) */
    const psector_page_data_t *  page = OtaUtils_GetPage0ValidSubpage();
    assert(page != NULL);

    /* Go through PSECT and update int_flash_top_addr if the NVM or Reserved partition is found in internal flash. Else, use the default value */
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        if(page->page0_v3.img_directory[i].img_type == OTA_UTILS_PSECT_NVM_PARTITION_IMAGE_TYPE)
        {
            is_pdm_found = true;
            if(OtaUtils_IsInternalFlashAddr(page->page0_v3.img_directory[i].img_base_addr))
            {
                int_reserved_or_pdm_size += page->page0_v3.img_directory[i].img_nb_pages*FLASH_PAGE_SIZE;
                /* Continuing looping on the image directory in case other reserved or DPM partitions are found */
            }
        }
        else if((page->page0_v3.img_directory[i].img_type == OTA_UTILS_PSECT_RESERVED_PARTITION_IMAGE_TYPE && OtaUtils_IsInternalFlashAddr(page->page0_v3.img_directory[i].img_base_addr)))
        {
            int_reserved_or_pdm_size += page->page0_v3.img_directory[i].img_nb_pages*FLASH_PAGE_SIZE;
            /* Continuing looping on the image directory in case other reserved or DPM partitions are found */
        }
    }
    if(!is_pdm_found)
        int_reserved_or_pdm_size += DEFAULT_PDM_SIZE; /* No specific PDM found - use the default PDM size */

    if(int_reserved_or_pdm_size > INTERNAL_FLASH_MAX_SAFE_VALUE)
        int_reserved_or_pdm_size = INTERNAL_FLASH_MAX_SAFE_VALUE; /* Should never enter here ! */

    return (INTERNAL_FLASH_MAX_SAFE_VALUE - int_reserved_or_pdm_size); /* reduce the accessible flash size by the reserved/PDM size */
}

bool OtaUtils_ImgDirectorySanityCheck(psector_page_data_t * page0, uint32_t ext_flash_size)
{
    const image_directory_entry_t * img_directory = &page0->page0_v3.img_directory[0];
    bool res = false;
    uint32_t img_base, img_end;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        if(img_directory[i].img_nb_pages != 0)
        {
            img_base = img_directory[i].img_base_addr;
            img_end = img_base + FLASH_PAGE_SIZE * img_directory[i].img_nb_pages - 1;
            bool overlap = false;
            if(OtaUtils_IsInternalFlashAddr(img_base))
            {
                if(img_directory[i].img_type == OTA_UTILS_PSECT_EXT_FLASH_TEXT_PARTITION_IMAGE_TYPE)
                {
                    /* Wrong address for this partition type */
                    res = false;
                    break;
                }
                if(OtaUtils_IsInternalFlashAddr(img_end) && img_end < INTERNAL_FLASH_MAX_SAFE_VALUE)
                {
                    /* Partition boundaries are good for internal flash - Check with other partitions */
                    for (int j = i+1; j < IMG_DIRECTORY_MAX_SIZE; j++ )
                    {
                        if(img_directory[j].img_nb_pages != 0)
                        {
                            uint32_t other_entry_base = img_directory[j].img_base_addr;
                            uint32_t other_entry_end = other_entry_base + FLASH_PAGE_SIZE * img_directory[j].img_nb_pages - 1;
                            if(OtaUtils_IsInternalFlashAddr(other_entry_base))
                            {
                                if (((img_base <= other_entry_base) && (img_end > other_entry_base)) || /* Partition ends in the middle of another one */
                                        ((img_base >= other_entry_base) && (img_base < other_entry_end))) /* Partition starts in the middle on another one */
                                {
                                    overlap = true;
                                    break;
                                }
                            }
                        }
                    }
                    res =  !overlap;
                }
                else
                {
                    res = false;
                    break;
                }
            }
            else
            {
                if (OtaUtils_IsExternalFlashAddr(img_base, ext_flash_size) && OtaUtils_IsExternalFlashAddr(img_end, ext_flash_size))
                {
                    for (int j = i+1; j < IMG_DIRECTORY_MAX_SIZE; j++ )
                    {
                        uint32_t other_entry_base = img_directory[j].img_base_addr;
                        uint32_t other_entry_end = other_entry_base + FLASH_PAGE_SIZE * img_directory[j].img_nb_pages;
                        if(OtaUtils_IsExternalFlashAddr(other_entry_base, ext_flash_size))
                        {
                            if (((img_base <= other_entry_base) && (img_end > other_entry_base)) ||
                                    ((img_base >= other_entry_base) && (img_base < other_entry_end)))
                            {
                                overlap = true;
                                break;
                            }
                        }
                    }
                    res =  !overlap;
                }
                else
                {
                    res = false;
                    break;
                }
            }
        }
    }
    return res;
}

uint32_t OtaUtils_ValidateImage(OtaUtils_ReadBytes pFunctionRead,
                                void *pReadFunctionParam,
                                OtaUtils_EEPROM_ReadData pFunctionEepromRead,
                                uint32_t imgAddr,
                                uint32_t minValidAddr,
                                const IMAGE_CERT_T * pRootCert,
                                bool_t inOtaCheck, bool_t isRemappable)
{
    uint32_t result_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
    ImageParserUnion uImgParser;
    uint32_t headerBootBlockMarker = 0;
    uint32_t runAddr = 0;
    uint8_t imgType = IMG_DIRECTORY_MAX_SIZE;
    uint32_t bootBlockOffsetFound = 0;
    uint32_t imgSizeFound = 0;

    do {
        /* Try to extract the imageHeader */
        if (pFunctionRead(sizeof(IMG_HEADER_T),  imgAddr, (uint8_t *)&uImgParser.imgHeader, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;

        imgType = OtaUtils_CheckImageTypeFromImgHeader(&uImgParser.imgHeader);
        if (imgType == IMG_DIRECTORY_MAX_SIZE)
        {
            break;
        }

        if (isRemappable)
        {
            /* Check that entry point is within tested archive */
            runAddr = (uImgParser.imgHeader.vectors[1] & ~0xfffUL);
        }
        else
        {
            runAddr = imgAddr;
        }

        if (!inOtaCheck)
        {
            if (runAddr != imgAddr) break;
        }

        if (uImgParser.imgHeader.bootBlockOffset % sizeof(uint32_t) ) break;

        if (uImgParser.imgHeader.bootBlockOffset + sizeof(BOOT_BLOCK_T) >= OtaUtils_GetModifiableInternalFlashTopAddress()) break;

        /* compute CRC of the header */
        uint32_t crc = ~0UL;
        crc = crc_update(crc, &uImgParser.imgHeader, sizeof(IMG_HEADER_T)-sizeof(uImgParser.imgHeader.header_crc));
        crc = CRC_FINALIZE(crc);

        if (uImgParser.imgHeader.header_crc != crc) break;

        if (boot_CheckVectorSum(&uImgParser.imgHeader) != 0) break;

        /* Save data before parsing the bootblock */
        bootBlockOffsetFound = uImgParser.imgHeader.bootBlockOffset;

        /* Try to extract the bootblock */
        if (pFunctionRead(sizeof(BOOT_BLOCK_T),  bootBlockOffsetFound + imgAddr, (uint8_t *)&uImgParser.imgBootBlock, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;

        headerBootBlockMarker = uImgParser.imgBootBlock.header_marker;

        if (!(  (headerBootBlockMarker >= BOOT_BLOCK_HDR_MARKER) &&
                (headerBootBlockMarker <= BOOT_BLOCK_HDR_MARKER+2))) break;
        if (!inOtaCheck)
        {
            if (uImgParser.imgBootBlock.target_addr != runAddr) break;
        }
        else
        {
            runAddr = uImgParser.imgBootBlock.target_addr;
        }
        if (runAddr < minValidAddr) break;
        if (uImgParser.imgBootBlock.stated_size < (bootBlockOffsetFound + sizeof(BOOT_BLOCK_T))) break;
        if (uImgParser.imgBootBlock.img_len > uImgParser.imgBootBlock.stated_size) break;

        /* Refuse OTA FW update if MinVersion has a version lower than the registered minimum version */
        uint32_t min_version = psector_Read_MinVersion();
        if (uImgParser.imgBootBlock.version < min_version) break;

        if (uImgParser.imgBootBlock.compatibility_offset != 0)
        {
            uint32_t compatibility_list_sz = 0;
            OTA_UTILS_DEBUG("Compatibility list found\n");
            if (uImgParser.imgBootBlock.compatibility_offset < (bootBlockOffsetFound - sizeof(uint32_t)))
            {
                /* Try to read the compatibility list size */
                if (pFunctionRead(sizeof(uint32_t),  imgAddr + uImgParser.imgBootBlock.compatibility_offset, (uint8_t *)&compatibility_list_sz, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                    break;
                if (uImgParser.imgBootBlock.compatibility_offset != bootBlockOffsetFound -
                        (sizeof(uint32_t) + compatibility_list_sz *sizeof(ImageCompatibilityListElem_t)) )
                    break;
            }
            else
                break;
        }

        /* Save bootblock data */
        imgSizeFound = uImgParser.imgBootBlock.img_len;

        /* Security check */
        if (pRootCert != NULL)
        {
            uint32_t imgSignatureOffset = bootBlockOffsetFound + sizeof(BOOT_BLOCK_T);
            const uint32_t *pKey =  (const uint32_t *)&pRootCert->public_key[0];
            OTA_UTILS_DEBUG("==> Img authentication is enabled\n");
            if (uImgParser.imgBootBlock.certificate_offset != 0)
            {
                OTA_UTILS_DEBUG("Certificate found\n");
                /* Check that the certificate is inside the img */
                if ((uImgParser.imgBootBlock.certificate_offset + sizeof(ImageCertificate_t)) != imgSizeFound)
                {
                    break;
                }
                /* If there is a certificate is must comply with the expectations */
                /* There must be a trailing ImageAuthTrailer_t appended to boot block */
                if (pFunctionRead(sizeof(ImageCertificate_t), imgAddr + uImgParser.imgBootBlock.certificate_offset, (uint8_t *)&uImgParser.imgCertificate, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                {
                    break;
                }
                if (uImgParser.imgCertificate.certificate.certificate_marker != CERTIFICATE_MARKER)
                {
                    break;
                }
                /* Check the signature of the certificate */
                if (!secure_VerifyCertificate(&uImgParser.imgCertificate.certificate, pKey, &uImgParser.imgCertificate.signature[0]))
                {
                    break;
                }
                pKey =  (const uint32_t *)&uImgParser.imgCertificate.certificate.public_key[0];
                imgSignatureOffset += sizeof(ImageCertificate_t);
            }
            else if (imgSignatureOffset != imgSizeFound)
            {
                break;
            }
            OTA_UTILS_DEBUG("Img signature found\n");
            /* Read the img signature */
            if (pFunctionRead(sizeof(ImageSignature_t), imgAddr + imgSignatureOffset, (uint8_t *)&uImgParser.imgCertificate.signature, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            {
                break;
            }

            const uint8_t * img_sign = (uint8_t*)&uImgParser.imgCertificate.signature[0];
            if (!OtaUtils_VerifySignature(imgAddr, imgSignatureOffset, pKey, img_sign, pFunctionRead,
                                            pReadFunctionParam, pFunctionEepromRead))
                break;
        }
        result_addr = runAddr;

    } while (0);
    OTA_UTILS_DEBUG("OTA_Utils => OtaUtils_ValidateImage result addr = 0x%x\n", result_addr);
    return result_addr;
}

otaUtilsResult_t OtaUtils_ReconstructRootCert(IMAGE_CERT_T *pCert, const psector_page_data_t* pPage0, const psector_page_data_t* pFlashPage)
{
    otaUtilsResult_t result = gOtaUtilsError_c;
    uint32_t keyValid;
    aesContext_t aesContext;
    do
    {
        if (pCert == NULL || pPage0 == NULL)
            break;

        keyValid = pPage0->page0_v3.img_pk_valid;
        if (keyValid < 2)
        {
            result = gOtaUtilsInvalidKey_c;
            break;
        }
        /* Decrypt the public key using the efuse key */
        OtaUtils_AesLoadKeyFromOTP(&aesContext, AES_KEY_128BITS);
        OtaUtils_AesSetMode(&aesContext, AES_MODE_ECB_DECRYPT, AES_INT_BSWAP | AES_OUTT_BSWAP);
        OtaUtils_AesProcessBlocks(&aesContext, (uint32_t*)&pPage0->page0_v3.image_pubkey[0], &pCert->public_key[0],  SIGNATURE_LEN/16);

        if (pFlashPage != NULL)
        {
            pCert->customer_id   = pFlashPage->pFlash.customer_id;
            pCert->min_device_id = pFlashPage->pFlash.min_device_id;
            pCert->max_device_id = pFlashPage->pFlash.max_device_id;
        }
        else
        {
            pCert->customer_id = psector_Read_CustomerId();
            pCert->min_device_id = psector_Read_MinDeviceId();
            pCert->max_device_id = psector_Read_MaxDeviceId();
        }

        pCert->certificate_marker = CERTIFICATE_MARKER;
        pCert->certificate_id = 0UL;
        pCert->usage_flags = 0UL;
        result = gOtaUtilsSuccess_c;
    } while (0);
    return result;
}

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
otaUtilsResult_t OtaUtils_StoreCustomOtaEntry(CustomOtaEntries_t *pEntry, OtaUtils_EEPROM_WriteData pFunctionWrite, uint32_t storageTopAddr)
{
    otaUtilsResult_t res = gOtaUtilsError_c;
    uint32_t entry_size = sizeof(CustomOtaEntries_t) - (OTAMaxCustomEntryNumber - pEntry->number_of_entry)*sizeof(image_directory_entry_t) - (OTAMaxCustomDataWords*sizeof(uint32_t) - pEntry->custom_data_length);
    uint32_t total_size = sizeof(OtaCustomStorage_t) - 4 + entry_size; /* -4 because first word is already contained in the "entry" pointer */
    uint8_t pCustomStorage[sizeof(OtaCustomStorage_t) + sizeof(CustomOtaEntries_t) - 4] = {0}; /* -4 because first word is already contained in the "entry" pointer */
    uint32_t offset = 0;

    do {
        if(storageTopAddr < total_size)
            break;

        uint32_t storage_addr = storageTopAddr - total_size;

        /* Copy the OTA entries */
        for(uint32_t i=0; i < pEntry->number_of_entry*sizeof(image_directory_entry_t); i++)
        {
            pCustomStorage[offset] = *((uint8_t *)((uint32_t)pEntry->entries + i));
            offset += sizeof(uint8_t);
        }

        /* Copy custom data */
         for(uint16_t i=0; i<pEntry->custom_data_length; i++)
        {
            pCustomStorage[offset] = *((uint8_t *)((uint32_t)pEntry->custom_data + i));
            offset += sizeof(uint8_t);
        }

         /* Copy custom data length */
         *((uint16_t *)((uint32_t)pCustomStorage + offset)) = pEntry->custom_data_length;
         offset += sizeof(uint16_t);

         /* Copy OTA status */
        pCustomStorage[offset] = pEntry->ota_state;
         offset += sizeof(otaCustomState_t);

        /* Copy the image entries number */
        pCustomStorage[offset] = pEntry->number_of_entry;
        offset += sizeof(uint8_t);

        /* Compute HASH of entry */
        uint32_t hash[8] = {0};
        if ( 0 != hash_sha256((uint32_t *)pCustomStorage, entry_size, (uint32_t *)&hash))
            break;

        /*  Fill the Custom storage header */
         *((uint32_t *)((uint32_t)pCustomStorage + offset)) = hash[0];
         offset += sizeof(uint32_t);
         *((uint32_t *)((uint32_t)pCustomStorage + offset)) = entry_size;
         offset += sizeof(uint32_t);
         *((uint32_t *)((uint32_t)pCustomStorage + offset)) = OTA_ENTRY_TAG;
         offset += sizeof(uint32_t);

        /* Copy data at expected location
         * - Application must ensure this region was cleared before
         * - Application must ensure the selected storage address is valid
         */
        res = pFunctionWrite(total_size, (uint32_t)(storage_addr), pCustomStorage);

    } while (0);
    return res;
}

otaUtilsResult_t OtaUtils_GetCustomOtaEntry(CustomOtaEntries_t *pEntry, uint16_t *pLenghtBytes, OtaUtils_EEPROM_ReadData pFunctionRead, uint32_t storageTopAddr)
{
    otaUtilsResult_t res = gOtaUtilsError_c;
    uint8_t pCustomEntry[sizeof(OtaCustomStorage_t) + sizeof(CustomOtaEntries_t) - 4] = {0};
    do{
        if(pEntry == NULL || pLenghtBytes == NULL)
            break;

        if(storageTopAddr < sizeof(OtaCustomStorage_t))
            break;

        /* Get the Custom storage header */
        uint32_t storage_addr = storageTopAddr - sizeof(OtaCustomStorage_t);
        OtaCustomStorage_t extra_storage_header;
        if( gOtaUtilsSuccess_c != pFunctionRead(sizeof(OtaCustomStorage_t), storage_addr, (uint8_t *)&extra_storage_header))
            break;
        if ( extra_storage_header.magic != OTA_ENTRY_TAG)
            break;
        *pLenghtBytes = (uint16_t)extra_storage_header.length;

        /* Get the entries */
        if(storage_addr < (*pLenghtBytes - 4))
            break;
        storage_addr = ((uint32_t)storage_addr - *pLenghtBytes + 4);
        if(gOtaUtilsSuccess_c != pFunctionRead(*pLenghtBytes, (uint32_t)(storage_addr), (uint8_t *)pCustomEntry))
            break;

        /* Compute HASH of entry */
        uint32_t hash[8] = {0};
        if ( 0 != hash_sha256((uint32_t *)pCustomEntry, (uint32_t )*pLenghtBytes, hash))
            break;
        if(extra_storage_header.short_hash != hash[0])
            break;

        /* All data verified - fill the user structure */
        uint32_t offset = *pLenghtBytes;
        offset -= sizeof(uint8_t);
        pEntry->number_of_entry = *((uint8_t *)((uint32_t)pCustomEntry + offset));
        offset -= sizeof(otaCustomState_t);
        pEntry->ota_state = *((otaCustomState_t *)((uint32_t)pCustomEntry + offset));
        offset -= sizeof(uint16_t);
        pEntry->custom_data_length = *((uint16_t *)((uint32_t)pCustomEntry + offset));
        offset -= pEntry->custom_data_length;
        memcpy(pEntry->custom_data, (uint32_t *)((uint32_t)pCustomEntry + offset), pEntry->custom_data_length);
        memcpy(pEntry->entries, pCustomEntry, pEntry->number_of_entry*sizeof(image_directory_entry_t));

        res = gOtaUtilsSuccess_c;

    } while(0);
    return res;
}

#endif /* gOTAUseCustomOtaEntry */
