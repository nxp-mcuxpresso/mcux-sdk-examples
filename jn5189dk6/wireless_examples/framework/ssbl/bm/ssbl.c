/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 ** SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */

#include "fsl_device_registers.h"
#include "rom_api.h"
#include "rom_secure.h"
#include "flash_header.h"
#include "fsl_flash.h"
#include "fsl_spifi.h"
#include "fsl_sha.h"
#include <setjmp.h>
#include "rom_isp.h"
#include "rom_aes.h"
#include "rom_psector.h"
#include "OtaUtils.h"

/*******************************************************************************
 * Private Macros
 ******************************************************************************/
#define KB(x) ((x)<<10)
#define DBGOUT PRINTF

#define SET_SCB_VTOR(x)  *(volatile uint32_t *)0xE000ED08 = (uint32_t)(x)
#define SET_MPU_CTRL(x) (*(volatile uint32_t *)0xe000ed94 = (uint32_t)(x))

#define UPPER_TEXT_LIMIT 0x96000 /* Some applications may reach higher addresses than 0x90000 */


extern jmp_buf  *exception_buf;

#define TRY do {jmp_buf exc_buf;                  \
                jmp_buf *old_buf = exception_buf; \
                exception_buf = &exc_buf;         \
                switch (setjmp(exc_buf)) { case 0:

#define CATCH(x) break; case x:
#define YRT } exception_buf = old_buf; } while(0);

#define RAISE_ERROR(x, val)   { x = val; break; }

#ifdef SSBL_BLOB_LOAD
#define SOTA_GET_BLOB_MAJOR_VERSION(blobVersion) (blobVersion >> 16)
#define SOTA_GET_BLOB_MINOR_VERSION(blobVersion) (blobVersion & 0x0000FFFF)
#endif

#undef MIN
#define MIN(a,b) ((a) <= (b) ? (a) : (b))

#define SSBL_SIZE 0x2000

#ifndef SSBL_VERSION
#define SSBL_VERSION 0
#endif

#define THUMB_ENTRY(x)                 (void*)((x) | 1)

#define ROM_API_aesDeinit              THUMB_ENTRY(0x03001460)
#define ROM_API_efuse_AESKeyPresent    THUMB_ENTRY(0x030016ec)
#define ROM_API_psector_Init           THUMB_ENTRY(0x03004e94)
#define ROM_API_BOOT_SetImageAddress   THUMB_ENTRY(0x03000eb4)
#if 0
#define ROM_API_BOOT_SetCfgWord        THUMB_ENTRY(0x03000ec0)
#define ROM_API_BOOT_ResumeGetCfgWord  THUMB_ENTRY(0x03000ecc)
#endif

#define SHA256_SIZE_BYTES 32U

/* Avoid using Fro_ClkSel_t that is being moved from fsl_clock.c to fsl_clock.h*/
typedef enum
{
    FRO12M_ENA_SHIFT,
    FRO32M_ENA_SHIFT,
    FRO48M_ENA_SHIFT,
    FRO64M_ENA_SHIFT,
    FRO96M_ENA_SHIFT,
} FroClkSelBit_t;

/* If OTA went wrong, prevent device to go into ISP and simply reset */
#ifndef DISABLE_ISP
#define DISABLE_ISP                    0
#endif

#ifndef USE_WATCHDOG
#define USE_WATCHDOG                   0
#endif

#ifndef EXTERNAL_FLASH_DATA_OTA
#define EXTERNAL_FLASH_DATA_OTA        0
#endif

#if EXTERNAL_FLASH_DATA_OTA
#define EXTRA_SECTION_MAGIC            0x73676745
#define EXTRA_SECTION_MAGIC_APP_ONLY   0x65706F4E
#define COPY_BUFFER_LENGTH             256U
#define EXTRA_BUFFER_SHA_LENGTH        16U

/*!
 * @brief Extra data header.
 *
 * Be very cautious when modifying the EXTRA_DATA_HEADER_T structure (alignment) as
 * these structures are used in the image_tool.py (which does not take care of alignment).
 */
typedef struct
{
    uint32_t extra_data_magic;                  /*!< Identifier for extra section descriptor */
    uint32_t extra_data_lma;                    /*!< Extra data load machine address - typically External flash address*/
    uint32_t extra_data_size;                   /*!< Extra data size in Bytes */
    uint8_t extra_data_hash[SHA256_SIZE_BYTES]; /*!< Computed Hash of the extra data - 256 Bits (32 Bytes)*/
} EXTRA_DATA_HEADER_T;

#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
#define QSPI_EARLY_START    /* In case both EXTERNAL_FLASH_DATA_OTA & gOTAUseCustomOtaEntry are enabled,  QSPI_EARLY_START must be enabled to fit in 8K */
#endif /* gOTAUseCustomOtaEntry */
#endif /* EXTERNAL_FLASH_DATA_OTA */

#define GET_PAGE_MULTIPLE_UP(X, PAGE_SIZE) ((X + PAGE_SIZE-1)/PAGE_SIZE)


#define ENABLE_FROCLK(x) PMC->FRO192M |= (1 << (PMC_FRO192M_DIVSEL_SHIFT+(x)))

/*!
 * @brief Simple hash verification flag (disabled by default).
 *
 * When secure boot is not used, this flag can be set to append a hash at the end of the image.
 * The hash will be used in the simple hash verification mechanism, which ensures data integrity
 * of the OTA image.The image hash will be placed right after the boot block data.
 *
 * The simple hash verification mechanism has the following responsabilities:
 *   - check integrity of the OTA image (it can be app or SSBL) before copying.
 *   - redundant check of the internal flash after the image was copied.
 */
#ifndef gSimpleHashVerification
#define gSimpleHashVerification 0
#endif

#if USE_WATCHDOG
#include "fsl_wwdt.h"
#define WWDT_TIMEOUT_S 5 /* Set Watchdog timeout value to 5s */
#endif
/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

typedef enum {
    StatusOta_simple_hash_error = -11,
    StatusOta_incorrect_extra_data = -10,
    StatusOta_fatal_error = -9,
    StatusOta_downgrade_error = -8,
    StatusOta_compatibility_error = -7,
    StatusOta_ciphering_error = -6,
    StatusOta_security_error = -5,
    StatusOta_incorrect_image_struct = -4,
    StatusOta_bus_fault = -3,
    StatusOta_operation_error = -2,
    StatusOtaFlash_entry_error = -1,
    StatusOta_ok = 0,
    StatusOta_ok_so_far = 115,

} StatusOta_t;

typedef enum {
    BUS_EXCEPTION = 1,
} EXCEPTION_ERROR_T;

typedef uint32_t (*aesDeinit_t)(void);
typedef bool (*efuse_AESKeyPresent_t)(void);
typedef psector_page_state_t (*psector_Init_t)(psector_partition_id_t part_index, psector_page_data_t *page);
typedef int (*BOOT_SetImageAddress_t)(uint32_t img_addr);
#if 0
typedef void (*BOOT_SetCfgWord_t)(uint32_t cfg_word);
typedef uint32_t (*BOOT_ResumeGetCfgWord_t)(void);
#endif

typedef enum {
    NO_CIPHER,
    AES_EFUSE_CIPHER,
    AES_PASSWORD_CIPHER,
} CipherMethod_t;

#ifdef SSBL_BLOB_LOAD
typedef struct {
    uint16_t blob_id;
    uint32_t blob_version;
} ImageCompatibilityListElem_t;
#endif

/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/
extern int SPIFI_Flash_Init(void);
extern uint8_t SPIFI_eraseArea(uint32_t Addr, int32_t size);
extern void SPIFI_writeData(uint32_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf);
extern uint32_t SPIFI_getSize(void);
extern uint32_t SPIFI_getSectorSize(void);
/************************************************************************************
*************************************************************************************
* Variables
*************************************************************************************
************************************************************************************/

jmp_buf  *exception_buf;

const static aesDeinit_t aesDeinit                         = (aesDeinit_t)ROM_API_aesDeinit;
const static efuse_AESKeyPresent_t efuse_AESKeyPresent     = (efuse_AESKeyPresent_t)ROM_API_efuse_AESKeyPresent;
const static psector_Init_t psector_Init                   = (psector_Init_t)ROM_API_psector_Init;
const static BOOT_SetImageAddress_t BOOT_SetImageAddress   = (BOOT_SetImageAddress_t)ROM_API_BOOT_SetImageAddress;
#if 0
const static BOOT_SetCfgWord_t BOOT_SetCfgWord             = (BOOT_SetCfgWord_t)ROM_API_BOOT_SetCfgWord;
const static BOOT_ResumeGetCfgWord_t BOOT_ResumeGetCfgWord = (BOOT_ResumeGetCfgWord_t)ROM_API_BOOT_ResumeGetCfgWord;
#endif

/* The SSBL version will be embedded in the binary, in .ro_version section. */
volatile uint32_t ssblVersion __attribute__((section(".ro_version"))) = SSBL_VERSION;

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/* Cannot reuse version from boot ROM because static function with a non ABI compliant parameter passing */
static psector_page_state_t ProtectedSectorRead(psector_partition_id_t part_index, psector_page_data_t *page)
{

    psector_page_state_t protected_sector_state;

    /* Read pFlash whether an update occurred or not */
    protected_sector_state = psector_Init(part_index, page);

    return protected_sector_state;
}

static int ReconstructRootCert(IMAGE_CERT_T *cert,
                               psector_page_data_t* pPage0,
                               psector_page_data_t* pFlash_page)
{
    int st = -1;
    do
    {
        if (aesInit() != 0)
            break;
        if (OtaUtils_ReconstructRootCert(cert, pPage0, pFlash_page) != gOtaUtilsSuccess_c)
            break;
        aesAbort(1);
        aesDeinit();
        st = 0;
    } while (0);
    return st;
}

static void SysReset(void)
{
#if 0
    RESET_SystemReset();
#else
    PMC->CTRL |= PMC_CTRL_SYSTEMRESETENABLE(1);
    NVIC_SystemReset();
#endif
    while(1);
}

static void EnableBusFault(void)
{
    /* In the absence of a proper setting in SHCSR, all faults go to a HardFault
     * Since we want a finer discrimination for BusFaults in order to catch Faults when
     * accessing the Flash on pages in error, lets set  BUSFAULTENA.
     */
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk ;

}

static void FaultRecovery(void)
{
    if (exception_buf != NULL)
    {
        longjmp(*exception_buf, BUS_EXCEPTION);
    }
    while(1);
}

__attribute__((section(".after_vectors")))  void BusFault_Handler(void)
{
    /* Retrieve the stack pointer */
    /* At this point LR contains the EXC_RETURN whose value tells
     * which stack pointer to restore : MSP Main Stack pointer or
     * PSP Process Stack pointer*/
    asm volatile (
        "TST   lr, #0x4\n"
        "ITE   EQ\n"
        "MRSEQ r0, MSP\n"
        "MRSNE r0, PSP\n"
        :
        :
        : "r0"
    );

    /* Set the exception return address to recovery function
     * Force execution of the Fault Recovery function and patch the LR to be reloaded
     * in the stack frame.
     * As the exception fired SP got decreased by 8 32 bit words as it stored:
     * PSR, PC, LR, R13, R3-R0
     * From the stack bottom LR is to be found at offset 6 * 4 i.e. 24, still here we have
     * gone deeper in the stack by 1 more word because r7 gets  pushed on the stack too
     *  thence the 28 offset
     * */
    asm volatile (
        "MOV   r1, %0\n"
        "STR   r1, [r0, #28]\n"
        :
        : "r"(FaultRecovery)
        : "r0", "r1"
    );

}

/* In case of wrong ImgType, IMG_TYPE_NB is returned  */
static uint8_t ssbl_ExtractImgType(const IMG_HEADER_T *imageHeader)
{
    uint8_t imgType = IMG_DIRECTORY_MAX_SIZE;
    if (imageHeader && imageHeader->imageSignature >= IMAGE_SIGNATURE
            && imageHeader->imageSignature < IMAGE_SIGNATURE + IMG_DIRECTORY_MAX_SIZE)
    {
        imgType = (imageHeader->imageSignature - IMAGE_SIGNATURE);
    }
    return imgType;
}

#ifdef SSBL_BLOB_LOAD
static uint16_t ssbl_ExtractBlobIdVersion(uint32_t blob_start_address, uint32_t *blob_version)
{
    uint32_t u32BootBlockOffset;
    IMG_HEADER_T *psImgHdr;
    BOOT_BLOCK_T *psBootBlock;

    psImgHdr = (IMG_HEADER_T *)blob_start_address;
    u32BootBlockOffset = psImgHdr->bootBlockOffset;
    psBootBlock = (BOOT_BLOCK_T *)(u32BootBlockOffset + blob_start_address);
    *blob_version = psBootBlock->version;
    return (uint16_t) psBootBlock->img_type;
}

static bool ssbl_CheckBlobCompatibility(const image_directory_entry_t *img_directory, const uint32_t *compatibilityList, uint16_t blob_id)
{
    bool status = false;
    const image_directory_entry_t *dest_dir_entry = NULL;
    ImageCompatibilityListElem_t *compatibilityListElem = (ImageCompatibilityListElem_t *)((uint32_t) compatibilityList + sizeof(uint32_t));
    uint32_t compatibilityListLength = *compatibilityList;
    int i;
    uint32_t imgDirBlobId = 0;
    uint32_t currentBlobVersion = 0;

    do {
        /* Get the compatibility list element in the OTA archive that matches the blob ID */
        while (compatibilityListLength) {
            if (compatibilityListElem->blob_id == blob_id) break;
            compatibilityListLength--;
            compatibilityListElem++;
        }
        if (0 == compatibilityListLength)
            break;

        /* Get the version corresponding to the blob ID using the image directory and verify it */
        for (i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
        {
            dest_dir_entry = img_directory+i;
            /* Ship empty entries */
            if (dest_dir_entry->img_base_addr != 0)
            {
                imgDirBlobId = ssbl_ExtractBlobIdVersion(dest_dir_entry->img_base_addr, &currentBlobVersion);
                if (compatibilityListElem->blob_id == imgDirBlobId
                        && compatibilityListElem->blob_version > currentBlobVersion)
                {
                    break;
                }
            }
        }

        if (IMG_DIRECTORY_MAX_SIZE == i)
        {
            status = true;
            break;
        }

    } while (0);

    return status;
}

static bool ssbl_CheckCompatibility(const image_directory_entry_t *img_directory, const IMG_HEADER_T *imageHeader, const IMG_HEADER_T *otaImageHeader)
{
    bool status = false;
    const BOOT_BLOCK_T *bootBlock = (BOOT_BLOCK_T *) ((uint32_t) imageHeader + imageHeader->bootBlockOffset);
    const BOOT_BLOCK_T *otaBootBlock = (BOOT_BLOCK_T *) ((uint32_t) otaImageHeader + otaImageHeader->bootBlockOffset);

    uint32_t compatibilityOffset = otaBootBlock->compatibility_offset;
    const uint32_t *compatibilityList = (uint32_t *) ((uint32_t) otaImageHeader + otaBootBlock->compatibility_offset);
    uint32_t compatibilityListLength = *compatibilityList;
    ImageCompatibilityListElem_t *compatibilityListElem = NULL;

    bool isCompatible = true;

    do
    {
        if (compatibilityOffset % sizeof(uint32_t))
            break;

        if (compatibilityOffset + sizeof(ImageCompatibilityListElem_t) * compatibilityListLength +  sizeof(uint32_t) >= UPPER_TEXT_LIMIT)
            break;

        /* The compatibility lists in the OTA archive and in the corresponding blob must have the same length */
        if (compatibilityListLength != *((uint32_t *) ((uint32_t) imageHeader + bootBlock->compatibility_offset)))
            break;

        /* Get the first element of the compatibility list of the current version of blob being OTA updated */
        compatibilityListElem = (ImageCompatibilityListElem_t *)((uint32_t) imageHeader + bootBlock->compatibility_offset + sizeof(uint32_t));

        /* Check the compatibility of all blobs present in the compatibility list of the current version of blob being OTA updated */
        while (compatibilityListLength) {
            if (!(isCompatible = ssbl_CheckBlobCompatibility(img_directory, compatibilityList, compatibilityListElem->blob_id))) break;
            compatibilityListLength--;
            compatibilityListElem++;
        }
        if (!isCompatible)
            break;

        status = true;
    } while (0);

    return status;
}
#endif
static void ImgDirectoryEntryClear(image_directory_entry_t * dir_entry)
{
    dir_entry->img_nb_pages = 0;
    dir_entry->img_base_addr = 0;
    dir_entry->img_type = 0;
    dir_entry->flags = 0;
}

#ifdef SSBL_BLOB_LOAD
static  __attribute__ ((noinline))  const BOOT_BLOCK_T* GetBootBlockPtr(const IMG_HEADER_T * img_hdr)
{
    /* The noinline is there so as to prevent -Os to perform an optimization that causes a HardFault */
    uint32_t hdr = (uint32_t)img_hdr + img_hdr->bootBlockOffset;
    return (const BOOT_BLOCK_T*)hdr;
}
#endif

static const image_directory_entry_t * ssbl_GetImgDirEntryBasedOnImgType(
        const image_directory_entry_t * img_directory,
        uint8_t imgType
)
{
    const image_directory_entry_t * entry_found = NULL;
    const image_directory_entry_t *dir_entry_iterator = NULL;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        dir_entry_iterator = img_directory+i;
        if (dir_entry_iterator->img_type == imgType && dir_entry_iterator->img_nb_pages != 0)
        {
            entry_found = dir_entry_iterator;
            break;
        }
    }
    return entry_found;
}

#if EXTERNAL_FLASH_DATA_OTA
static bool ssbl_CheckExtFlashDesc(const image_directory_entry_t *img_directory, uint32_t ext_flash_text_addr, uint32_t ext_flash_text_size)
{
    bool res = false;
    if((ext_flash_text_size == 0)
            || (ext_flash_text_addr < FSL_FEATURE_SPIFI_START_ADDR)
            || (ext_flash_text_addr > (FSL_FEATURE_SPIFI_START_ADDR + SPIFI_getSize())))
    {
        return res;
    }
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        if(img_directory[i].img_type == OTA_UTILS_PSECT_EXT_FLASH_TEXT_PARTITION_IMAGE_TYPE)
        {
            if(ext_flash_text_addr == img_directory[i].img_base_addr || ext_flash_text_size <= FLASH_PAGE_SIZE * img_directory[i].img_nb_pages)
            {
                res = true;
                break;
            }
        }
    }
    return res;
}

static uint32_t ssbl_GetMaxExtFlashTextSize(const image_directory_entry_t *img_directory)
{
    uint32_t size = 0;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        if(img_directory[i].img_type == OTA_UTILS_PSECT_EXT_FLASH_TEXT_PARTITION_IMAGE_TYPE)
        {
            size = FLASH_PAGE_SIZE * img_directory[i].img_nb_pages;
            break;
        }
    }
    return size;
}
#endif

static bool ssbl_CheckOtaEntryVsImgDir(const image_directory_entry_t *img_directory, uint32_t ota_img_start, uint32_t ota_img_pages)
{
    bool res = true;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        if(img_directory[i].img_type == OTA_UTILS_PSECT_OTA_PARTITION_IMAGE_TYPE)
        {
            if(ota_img_start < img_directory[i].img_base_addr || (ota_img_start + FLASH_PAGE_SIZE*ota_img_pages) > (img_directory[i].img_base_addr + FLASH_PAGE_SIZE*img_directory[i].img_nb_pages))
            {
                res = false;
                break;
            }
        }
    }
    return res;
}

static otaUtilsResult_t ssbl_EEPROM_ReadData(uint16_t nbBytes, uint32_t address, uint8_t *pInbuf)
{
    memcpy(pInbuf, (void*)(address), nbBytes);
    return gOtaUtilsSuccess_c;
}

#if defined(gOTACustomOtaEntryMemory) && (gOTACustomOtaEntryMemory == OTACustomStorage_Ram)
static otaUtilsResult_t ssbl_RAM_WriteData(uint16_t nbBytes, uint32_t address, uint8_t *pInbuf)
{
    memcpy((void*)(address), pInbuf, nbBytes);
    return gOtaUtilsSuccess_c;
}
#endif

#if EXTERNAL_FLASH_DATA_OTA || gSimpleHashVerification
static otaUtilsResult_t ssbl_ComputeHash(uint32_t start_address,
                                         uint32_t total_size,
                                         uint8_t * cache_buffer,
                                         uint16_t cache_size,
                                         uint8_t * out_hash,
                                         OtaUtils_ReadBytes pFunctionRead,
                                         void * pReadFunctionParam,
                                         OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t res = gOtaUtilsError_c;
    uint32_t nbPageToRead = total_size/cache_size;
    uint32_t lastPageNbByteNumber = total_size - (nbPageToRead*cache_size);
    uint32_t i = 0;

    do
    {
        sha_ctx_t hash_ctx;
        size_t sha_sz = SHA256_SIZE_BYTES;
        /* Initialise SHA clock do not call SHA_ClkInit(SHA0) because the HAL pulls in too much code  */
        SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_HASH_MASK;
        if (SHA_Init(SHA0, &hash_ctx, kSHA_Sha256) != kStatus_Success)
        {
            break;
        }

        for (i = 0; i < nbPageToRead; i++)
        {
            if (pFunctionRead(cache_size, start_address + (i*cache_size), cache_buffer, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            {
                break;
            }
            if (SHA_Update(SHA0, &hash_ctx, cache_buffer, cache_size) != kStatus_Success)
            {
                break;
            }
        }

        /* Read bytes located on the last page */
        if (pFunctionRead(lastPageNbByteNumber, start_address + (i*cache_size), cache_buffer, pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
        {
            break;
        }
        if (SHA_Update(SHA0, &hash_ctx, cache_buffer, lastPageNbByteNumber) != kStatus_Success)
        {
            break;
        }
        if (SHA_Finish(SHA0,  &hash_ctx, out_hash, &sha_sz) != kStatus_Success)
        {
            break;
        }
        /* Compare with the computed Hash */
        res = gOtaUtilsSuccess_c;
    } while (0);
    SYSCON->AHBCLKCTRLCLR[1] =  SYSCON_AHBCLKCTRL1_HASH_MASK; /* equivalent to SHA_ClkDeinit(SHA0) */

    return res;
}

static otaUtilsResult_t ssbl_VerifyHash(uint8_t *hash, uint8_t *expected_hash)
{
    otaUtilsResult_t res = gOtaUtilsSuccess_c;

    size_t sha_sz = SHA256_SIZE_BYTES;
    while (sha_sz)
    {
        if (*hash != *expected_hash)
        {
            res = gOtaUtilsError_c;
            break;
        }
        expected_hash++;
        hash++;
        sha_sz--;
    }

    return res;
}
#endif // EXTERNAL_FLASH_DATA_OTA || gSimpleHashVerification

#if EXTERNAL_FLASH_DATA_OTA
static otaUtilsResult_t ssbl_ComputeExtraDataHash(uint32_t p_extra_data_addr,
                                                  uint32_t extra_data_size,
                                                  uint8_t *p_computed_hash,
                                                  OtaUtils_ReadBytes pFunctionRead,
                                                  void * pReadFunctionParam,
                                                  OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t res = gOtaUtilsError_c;

    do
    {
        uint8_t pageContent[EXTRA_BUFFER_SHA_LENGTH];
        uint8_t digest[SHA256_SIZE_BYTES];

        res = ssbl_ComputeHash(p_extra_data_addr,
                               extra_data_size,
                               &pageContent[0],
                               (uint16_t)EXTRA_BUFFER_SHA_LENGTH,
                               &digest[0],
                               pFunctionRead,
                               pReadFunctionParam,
                               pFunctionEepromRead);
        if (gOtaUtilsSuccess_c != res)
        {
            break;
        }

        res = ssbl_VerifyHash(&digest[0], p_computed_hash);
    } while (0);

    return res;
}
#endif

#if gSimpleHashVerification
static StatusOta_t ssbl_VerifySimpleHash(uint32_t img_addr_ota,
                                         uint32_t img_size_ota,
                                         uint8_t *p_expected_hash,
                                         OtaUtils_ReadBytes pFunctionRead)
{
    StatusOta_t err = StatusOta_ok;
    otaUtilsResult_t result = gOtaUtilsError_c;
    uint8_t page_content_ota[FLASH_PAGE_SIZE];
    uint8_t digest[SHA256_SIZE_BYTES];

    /* Compute hash of image */
    do
    {
        TRY
        {
            result = ssbl_ComputeHash(img_addr_ota,
                                img_size_ota,
                                &page_content_ota[0],
                                (uint16_t)FLASH_PAGE_SIZE,
                                &digest[0],
                                pFunctionRead,
                                NULL,
                                ssbl_EEPROM_ReadData);
            if (gOtaUtilsSuccess_c != result)
            {
                RAISE_ERROR(err, StatusOta_simple_hash_error);
            }
        }
        CATCH(BUS_EXCEPTION)
        {
            RAISE_ERROR(err, StatusOta_bus_fault);
        }
        YRT

        result = ssbl_VerifyHash(&digest[0], p_expected_hash);
        if (gOtaUtilsSuccess_c != result)
        {
            RAISE_ERROR(err, StatusOta_simple_hash_error);
        }
    } while (0);

    return err;
}

static bool_t ssbl_ValidateImageWithSimpleHash(uint32_t img_addr_ota, OtaUtils_ReadBytes pFunction_read, uint8_t *p_param)
{
    StatusOta_t err = StatusOta_ok;
    IMG_HEADER_T img_header_ota;
    BOOT_BLOCK_T boot_block_ota;
    uint8_t expected_hash[SHA256_SIZE_BYTES];

    do
    {
        TRY
        {
            pFunction_read(sizeof(IMG_HEADER_T), img_addr_ota, (uint8_t *)&img_header_ota, p_param, ssbl_EEPROM_ReadData);
            pFunction_read(sizeof(BOOT_BLOCK_T), img_addr_ota + img_header_ota.bootBlockOffset, (uint8_t *)&boot_block_ota, p_param, ssbl_EEPROM_ReadData);
            pFunction_read(SHA256_SIZE_BYTES, img_addr_ota + img_header_ota.bootBlockOffset + sizeof(BOOT_BLOCK_T), expected_hash, p_param, ssbl_EEPROM_ReadData);
        }
        CATCH(BUS_EXCEPTION)
        {
            RAISE_ERROR(err, StatusOta_bus_fault);
        }
        YRT

        err = ssbl_VerifySimpleHash(img_addr_ota, boot_block_ota.img_len, expected_hash, pFunction_read);
    } while (false);

    return err == StatusOta_ok;
}
#endif

/*
 * Validate the image and get its targeted address and the related function/parameters to access it
 */
static uint32_t ssbl_GetValidImageAndFunctions(const image_directory_entry_t *p_ota_entry, const image_directory_entry_t * img_directory, const uint8_t *zigbee_password, const IMAGE_CERT_T * root_cert, CipherMethod_t *p_cipher_method, OtaUtils_ReadBytes *pFunction_read, uint8_t *p_param, sOtaUtilsSoftwareKey *p_softKey)
{
    uint32_t img_addr_targeted = OTA_UTILS_IMAGE_INVALID_ADDR;
    *pFunction_read = OtaUtils_ReadFromInternalFlash;
    *p_cipher_method = NO_CIPHER;
    do
    {
        if ((p_ota_entry->img_base_addr >= FSL_FEATURE_SPIFI_START_ADDR))
        {
#ifndef QSPI_EARLY_START
            if(SPIFI_Flash_Init() < 0)
            {
                break;
            }
#endif
            if (p_ota_entry->flags & 0x80)
            {
                if (efuse_AESKeyPresent())
                {
                    *p_cipher_method = AES_EFUSE_CIPHER;
                }
                *pFunction_read = OtaUtils_ReadFromEncryptedExtFlashEfuseKey;
            }
            else if (p_ota_entry->flags & 0x40)
            {
                *p_cipher_method = AES_PASSWORD_CIPHER;
                *pFunction_read = OtaUtils_ReadFromEncryptedExtFlashSoftwareKey;
                p_softKey->pSoftKeyAes = zigbee_password;
                p_param = (uint8_t *) p_softKey;
            }
            if (*p_cipher_method != NO_CIPHER)
            {
                if (aesInit() != 0)
                {
                    break;
                }
            }
            else
            {
                *pFunction_read = OtaUtils_ReadFromUnencryptedExtFlash;
            }

        }
        /* If OTA partition was not provisioned in PSECT, continue the OTA for backward compatibility */
        if(!ssbl_CheckOtaEntryVsImgDir(img_directory, p_ota_entry->img_base_addr, p_ota_entry->img_nb_pages))
        {
            break;
        }

        TRY
        {
#if gSimpleHashVerification
            /* If simple hash verification fails, stop the OTA. This prevents possible issues
             * in multi-image context, when one hash verification passes, but another one fails.
             * The images should only be applied if all the hash verifications pass. */
            if (!ssbl_ValidateImageWithSimpleHash(p_ota_entry->img_base_addr, *pFunction_read, p_param))
            {
                break;
            }
#endif
            img_addr_targeted = OtaUtils_ValidateImage(*pFunction_read,
                                              p_param,
                                              ssbl_EEPROM_ReadData,
                                              p_ota_entry->img_base_addr,
                                              0,
                                              root_cert,
                                              TRUE,
                                              FALSE);
        }
        CATCH(BUS_EXCEPTION)
        {
            break;
        }
        YRT

    } while(false);
    return img_addr_targeted;
}




static StatusOta_t ssbl_GetOtaFile(const image_directory_entry_t * ota_entry,
                              const image_directory_entry_t * img_directory,
                              const uint8_t *zigbee_password,
                              uint32_t *selected_img_addr,
                              const IMAGE_CERT_T * root_cert, uint32_t min_valid_addr)
{
    StatusOta_t err = StatusOta_ok_so_far;
    uint32_t img_addr_ota = ota_entry->img_base_addr;
    uint32_t img_addr_targeted;
    uint32_t internal_img_size = 0;
    CipherMethod_t cipher_method = NO_CIPHER;
    OtaUtils_ReadBytes pFunction_read = OtaUtils_ReadFromInternalFlash;
    OtaUtils_EEPROM_ReadData pFunction_eeprom_read = ssbl_EEPROM_ReadData;
    uint8_t *p_param = NULL;
    sOtaUtilsSoftwareKey softKey;
    const image_directory_entry_t * dest_dir_entry = NULL;
    IMG_HEADER_T img_header_ota;
    BOOT_BLOCK_T boot_block_ota;
    uint8_t img_type_ota;
    uint16_t ota_img_nb_pages = ota_entry->img_nb_pages;
    uint32_t dest_partition_size;
    uint8_t page_content_ota[FLASH_PAGE_SIZE];
    bool_t ssblUpdate = FALSE;
#if defined(SSBL_BLOB_LOAD) || EXTERNAL_FLASH_DATA_OTA
    bool_t external_flash_ota = (ota_entry->img_base_addr >= FSL_FEATURE_SPIFI_START_ADDR);
#endif

#if USE_WATCHDOG
    /* refresh watchdog before applying OTA entry */
    WWDT_Refresh(WWDT);
#endif /* USE_WATCHDOG */

    do
    {
        img_addr_targeted = ssbl_GetValidImageAndFunctions(ota_entry, img_directory, zigbee_password, root_cert, &cipher_method, &pFunction_read, p_param, &softKey);
        if (img_addr_targeted == OTA_UTILS_IMAGE_INVALID_ADDR)
            RAISE_ERROR(err, StatusOta_incorrect_image_struct);

        /* Try to find the matching image directory entry */
        TRY
        {
            pFunction_read(sizeof(IMG_HEADER_T),  img_addr_ota, (uint8_t *)&img_header_ota, p_param, pFunction_eeprom_read);
            pFunction_read(sizeof(BOOT_BLOCK_T),  img_addr_ota + img_header_ota.bootBlockOffset, (uint8_t *)&boot_block_ota, p_param, pFunction_eeprom_read);
        }
        CATCH(BUS_EXCEPTION)
        {
            RAISE_ERROR(err, StatusOta_bus_fault);
        }
        YRT

        img_type_ota = ssbl_ExtractImgType(&img_header_ota);

        /* Check if it is an SSBL update */
        if (img_type_ota == 0 && img_addr_targeted == 0)
        {
            ssblUpdate = TRUE;
            img_addr_targeted += SSBL_SIZE;
        }

        dest_dir_entry = ssbl_GetImgDirEntryBasedOnImgType(img_directory, img_type_ota);

        if (dest_dir_entry == NULL)
            RAISE_ERROR(err, StatusOtaFlash_entry_error);

        /* Test to check that the image type is the expected one */
        if (dest_dir_entry->img_type != img_type_ota)
            RAISE_ERROR(err, StatusOta_incorrect_image_struct);

        /* Check the targeted address value */
        if (dest_dir_entry->img_base_addr != boot_block_ota.target_addr)
            RAISE_ERROR(err, StatusOta_incorrect_image_struct);

        /* Check that the size of the new update will fit in the final destination */
        dest_partition_size = dest_dir_entry->img_nb_pages*FLASH_PAGE_SIZE;
        internal_img_size = boot_block_ota.img_len;
        if(root_cert != NULL)
        {
            internal_img_size += sizeof(ImageSignature_t);
        }

#if gSimpleHashVerification
        internal_img_size += SHA256_SIZE_BYTES;
#endif

        if (GET_PAGE_MULTIPLE_UP(internal_img_size, FLASH_PAGE_SIZE) > dest_dir_entry->img_nb_pages
                || internal_img_size > dest_partition_size)
            RAISE_ERROR(err, StatusOta_incorrect_image_struct);

#ifdef SSBL_BLOB_LOAD
        /* Compatibility list is not check when OTA comes from external flash */
        if (!external_flash_ota)
        {
            IMG_HEADER_T img_header_current_img;
            BOOT_BLOCK_T boot_block_current_img;
            memset(&img_header_current_img, 0x0, sizeof(IMG_HEADER_T));
            memset(&boot_block_current_img, 0x0, sizeof(BOOT_BLOCK_T));
            if (OtaUtils_ReadFromInternalFlash(sizeof(IMG_HEADER_T), dest_dir_entry->img_base_addr, (uint8_t *) &img_header_current_img, NULL, ssbl_EEPROM_ReadData) 
                == gOtaUtilsSuccess_c)
            {
                if (OtaUtils_ReadFromInternalFlash(sizeof(BOOT_BLOCK_T), dest_dir_entry->img_base_addr + img_header_current_img.bootBlockOffset, (uint8_t *) &boot_block_current_img, NULL, ssbl_EEPROM_ReadData) 
                    == gOtaUtilsSuccess_c && boot_block_current_img.compatibility_offset != 0)
                {
                    /* Check major and minor version
                    * Reject update if major are not equal
                    * Otherwise check the minor version and do not allow the downgrade
                    */
                    if (SOTA_GET_BLOB_MAJOR_VERSION(boot_block_current_img.version) != SOTA_GET_BLOB_MAJOR_VERSION(boot_block_ota.version)
                            || SOTA_GET_BLOB_MINOR_VERSION(boot_block_ota.version) <= SOTA_GET_BLOB_MINOR_VERSION(boot_block_current_img.version))
                    {
                        RAISE_ERROR(err, StatusOta_downgrade_error);
                    }

                    if (!ssbl_CheckCompatibility(img_directory, (IMG_HEADER_T *) dest_dir_entry->img_base_addr, (IMG_HEADER_T *) img_addr_ota))
                    {
                        RAISE_ERROR(err, StatusOta_compatibility_error);
                    }
                }
            }
        }
#endif

#if EXTERNAL_FLASH_DATA_OTA
        if (!ssblUpdate && external_flash_ota)
        {
            /* Check the extra data integrity before doing anything else */
            EXTRA_DATA_HEADER_T extra_data_header;
            uint32_t extra_data_src_addr = 0;
            uint8_t temp_buff[COPY_BUFFER_LENGTH] = {0};
            memset(&extra_data_header, 0x0, sizeof(EXTRA_DATA_HEADER_T));

            /* Try to extract Extra Section Descriptor */
            TRY
            {
                pFunction_read(sizeof(EXTRA_DATA_HEADER_T),  img_addr_ota + img_header_ota.bootBlockOffset - sizeof(EXTRA_DATA_HEADER_T), (uint8_t *)&extra_data_header, p_param, ssbl_EEPROM_ReadData);
            }
            CATCH(BUS_EXCEPTION)
            {
                RAISE_ERROR(err, StatusOta_bus_fault);
            }
            YRT

            /* Check if descriptor has been found */
            if((extra_data_header.extra_data_magic == EXTRA_SECTION_MAGIC) && (ota_img_nb_pages*FLASH_PAGE_SIZE >= internal_img_size + extra_data_header.extra_data_size))
            {
                /* Sanity check on extracted descriptor */
                if(!ssbl_CheckExtFlashDesc(img_directory, extra_data_header.extra_data_lma, extra_data_header.extra_data_size))
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);

                extra_data_src_addr = img_addr_ota + internal_img_size;
                if(extra_data_src_addr + extra_data_header.extra_data_size >= (FSL_FEATURE_SPIFI_START_ADDR + SPIFI_getSize()))
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);

                if(gOtaUtilsSuccess_c != ssbl_ComputeExtraDataHash(extra_data_src_addr, extra_data_header.extra_data_size, extra_data_header.extra_data_hash, pFunction_read, p_param, pFunction_eeprom_read))
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);

                /* Checks passed - Start erasing the needed area - must be multiple of 4K */
                uint32_t ext_flash_text_partition = ssbl_GetMaxExtFlashTextSize(img_directory);
                if(0 != SPIFI_eraseArea(extra_data_header.extra_data_lma - FSL_FEATURE_SPIFI_START_ADDR, (int32_t) GET_PAGE_MULTIPLE_UP(ext_flash_text_partition, SPIFI_getSectorSize())*SPIFI_getSectorSize()))
                    /* Exit on failure - If data already erased, only ISP mode can recover */
                    RAISE_ERROR(err, StatusOta_fatal_error);

                /* Copy extra data buffers */
                for (int i= 0 ; i < GET_PAGE_MULTIPLE_UP(extra_data_header.extra_data_size, COPY_BUFFER_LENGTH); i++)
                {
                    TRY
                    {
                        pFunction_read(COPY_BUFFER_LENGTH,  extra_data_src_addr + i*COPY_BUFFER_LENGTH, (uint8_t *)&temp_buff, p_param, pFunction_eeprom_read);
                    }
                    CATCH(BUS_EXCEPTION)
                    {
                        RAISE_ERROR(err, StatusOta_bus_fault);
                    }
                    YRT
                    SPIFI_writeData(COPY_BUFFER_LENGTH, extra_data_header.extra_data_lma + i*COPY_BUFFER_LENGTH - FSL_FEATURE_SPIFI_START_ADDR, (uint8_t *)&temp_buff);
                }

                if(err != StatusOta_ok_so_far)
                    /* Exit on failure - If data already erased, only ISP mode can recover */
                    RAISE_ERROR(err, StatusOta_fatal_error);

                /* Remove the extra pages from the total image length */
                ota_img_nb_pages = GET_PAGE_MULTIPLE_UP(ota_img_nb_pages*FLASH_PAGE_SIZE - extra_data_header.extra_data_size, FLASH_PAGE_SIZE);
            }
        }
#endif /* EXTERNAL_FLASH_DATA_OTA */

        /* Erase flash section needed before copying  - take the image partition size from PSECT
         * If image partition size exceed internal flash safe limit, use the size of the received image instead (if fits into the internal flash safe range) */
        uint8_t * erase_end_addr = (uint8_t*)img_addr_targeted + dest_partition_size - 1;
        uint32_t max_modifiable_int_addr = OtaUtils_GetModifiableInternalFlashTopAddress();
        if ((uint32_t)erase_end_addr > max_modifiable_int_addr)
        {
            if(img_addr_targeted + ota_entry->img_nb_pages*FLASH_PAGE_SIZE < max_modifiable_int_addr)
            {
                erase_end_addr = (uint8_t*)img_addr_targeted + ota_entry->img_nb_pages*FLASH_PAGE_SIZE - 1;
            }
            else
            {
                RAISE_ERROR(err, StatusOta_incorrect_image_struct);
            }
        }
        if (FLASH_Erase(FLASH, (uint8_t*)img_addr_targeted,
                (uint8_t*) erase_end_addr) != FLASH_DONE)
        {
            /* Exit on failure - Considering issues during flash erase as fatal error - Directly go to ISP without clearing OTA entry */
            RAISE_ERROR(err, StatusOta_fatal_error);
        }

        /* Before copying the image, check the iterator limit value. If inconsistency is detected, reduce it to the actual application size */
        uint32_t ota_img_max_size = ota_img_nb_pages*FLASH_PAGE_SIZE;
        if(ota_img_max_size > dest_partition_size || ota_img_max_size > internal_img_size)
        {
            ota_img_nb_pages = GET_PAGE_MULTIPLE_UP(internal_img_size, FLASH_PAGE_SIZE);
        }

        /* Copying the image */
        for (int i = 0; i < ota_img_nb_pages; i++)
        {
            TRY
            {
                pFunction_read(sizeof(page_content_ota), img_addr_ota + (i*FLASH_PAGE_SIZE), &page_content_ota[0], p_param, pFunction_eeprom_read);
            }
            CATCH(BUS_EXCEPTION)
            {
                RAISE_ERROR(err, StatusOta_bus_fault);
            }
            YRT
#if EXTERNAL_FLASH_DATA_OTA
            uint32_t unaligned_bytes = internal_img_size%FLASH_PAGE_SIZE;
            if(i==(ota_img_nb_pages-1) && (unaligned_bytes != 0))
            {
                /* In case application image end address is not FLASH_PAGE_SIZE,
                 * we may need to clear some bytes in the end of last page, to not copy unwanted data into flash */
                for (uint32_t j=(unaligned_bytes); j<FLASH_PAGE_SIZE; j++)
                {
                    page_content_ota[j] = 0xFF;
                }
            }
#endif
            if (FLASH_Program(FLASH, (uint32_t*) (img_addr_targeted+(i*FLASH_PAGE_SIZE)),
                    (uint32_t*) &page_content_ota[0], FLASH_PAGE_SIZE) != FLASH_DONE)
                RAISE_ERROR(err, StatusOta_operation_error);
        }

        if(err != StatusOta_ok_so_far)
            /* Exit on failure - Data already erased and issue occurs during programming - Directly go to ISP without clearing OTA entry */
            RAISE_ERROR(err, StatusOta_fatal_error);

#if gSimpleHashVerification
        uint8_t expected_hash[SHA256_SIZE_BYTES];

        TRY
        {
            uint32_t expected_hash_addr = img_addr_ota + img_header_ota.bootBlockOffset + sizeof(BOOT_BLOCK_T);
            pFunction_read(SHA256_SIZE_BYTES, expected_hash_addr, expected_hash, p_param, ssbl_EEPROM_ReadData);
        }
        CATCH(BUS_EXCEPTION)
        {
            RAISE_ERROR(err, StatusOta_bus_fault);
        }
        YRT

        /* Do a new simple hash verification to ensure the data was copied correctly in internal flash.
         * If this verification fails, the device should reset and retry copying the OTA image. */
        err = ssbl_VerifySimpleHash(img_addr_targeted, internal_img_size - SHA256_SIZE_BYTES, expected_hash, OtaUtils_ReadFromInternalFlash);
        if (StatusOta_ok != err)
        {
            SysReset();
        }

        err = StatusOta_ok_so_far;
#endif

        if (ssblUpdate)
        {
            *selected_img_addr = BOOT_RemapAddress(img_addr_targeted);
        }
        err ^= StatusOta_ok_so_far;
    } while (0);

    if (cipher_method != NO_CIPHER)
    {
        aesAbort(1);
        aesDeinit();
        SYSCON->AHBCLKCTRLCLR[1] =  SYSCON_AHBCLKCTRL1_HASH_MASK; /* equivalent to SHA_ClkDeinit(SHA0) */
    }

    return err;
}

#ifdef SSBL_BLOB_LOAD
static const image_directory_entry_t * ssbl_GetImgDirEntryBasedOnBlobId(
        const image_directory_entry_t * img_directory,
        uint16_t blob_id
)
{
    const image_directory_entry_t * entry_found = NULL;
    const image_directory_entry_t *dir_entry_iterator = NULL;
    uint32_t blobIdFound = 0;
    uint32_t blobVersion = 0;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        dir_entry_iterator = img_directory+i;
        blobIdFound = ssbl_ExtractBlobIdVersion(dir_entry_iterator->img_base_addr, &blobVersion);
        if (blob_id == blobIdFound)
        {
            entry_found = dir_entry_iterator;
            break;
        }
    }
    return entry_found;
}
#endif
static uint32_t ssbl_SearchForValidBootableImg(
        const image_directory_entry_t * img_directory,
        const uint32_t min_valid_addr,
        const IMAGE_CERT_T *root_cert,
        uint32_t * preferred_app
)
{
    const image_directory_entry_t *dir_entry_iterator = NULL;
    const image_directory_entry_t *dir_entry_bootable = NULL;
    uint32_t image_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
    bool isBootable = false;

    /* loop on all image directory entries */
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        dir_entry_iterator = img_directory+i;
        isBootable = dir_entry_iterator->flags & IMG_FLAG_BOOTABLE;
        if (isBootable)
        {
            if (OtaUtils_ValidateImage(OtaUtils_ReadFromInternalFlash,
                                          NULL,
                                          ssbl_EEPROM_ReadData,
                                          dir_entry_iterator->img_base_addr,
                                          min_valid_addr,
                                          root_cert,
                                          FALSE,
                                          FALSE) != OTA_UTILS_IMAGE_INVALID_ADDR)
            {
                dir_entry_bootable = dir_entry_iterator;
                break;
            }
        }

    }
    /* At least one bootable img is expected */
    if (dir_entry_bootable != NULL)
    {
        image_addr = dir_entry_bootable->img_base_addr;
        *preferred_app = 0x000000ff & dir_entry_bootable->img_type;
    }
    return image_addr;
}

#ifdef SSBL_BLOB_LOAD
/*
 * The function is in charge of checking if the img is a blob
 * and if yes, the function oversees the validation of each blob composing
 * the application using the compatibility list.
 * If no compatibility list is found in the img this means that it is not a blob
 * and therefore the function do nothing.
 *
 * Prerequisite: this function must be called once img_addr given as a parameter has been validated,
 * otherwise its value should be equal to OTA_UTILS_IMAGE_INVALID_ADDR
 */
static void ssbl_CheckBlobsLinkToBootableImg(
        const image_directory_entry_t * img_directory,
        uint32_t *image_addr,
        const uint32_t min_valid_addr,
        const IMAGE_CERT_T *root_cert
)
{
    const BOOT_BLOCK_T * bootBlock = NULL;
    const uint32_t *compatibilityList = NULL;
    const ImageCompatibilityListElem_t * compatibilityListElem = NULL;
    const image_directory_entry_t *blobEntry = NULL;
    uint32_t compatibilityListLen = 0;

    if (image_addr && *image_addr != OTA_UTILS_IMAGE_INVALID_ADDR)
    {
        bootBlock = GetBootBlockPtr((IMG_HEADER_T *) *image_addr);
        if (bootBlock && bootBlock->compatibility_offset != 0)
        {
            compatibilityList = (uint32_t *) ((uint32_t) *image_addr + bootBlock->compatibility_offset);
            compatibilityListLen = *compatibilityList;
            compatibilityList++;
            compatibilityListElem = (ImageCompatibilityListElem_t *)(compatibilityList);
            while (compatibilityListLen){
                blobEntry = ssbl_GetImgDirEntryBasedOnBlobId(img_directory, compatibilityListElem->blob_id);
                if (blobEntry == NULL)
                {
                    *image_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
                    break;
                }
                /* Validate the blob */
                if (OtaUtils_ValidateImage(OtaUtils_ReadFromInternalFlash,
                                          NULL,
                                          ssbl_EEPROM_ReadData,
                                          blobEntry->img_base_addr,
                                          min_valid_addr,
                                          root_cert,
                                          FALSE,
                                          FALSE)
                        == OTA_UTILS_IMAGE_INVALID_ADDR)
                {
                    *image_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
                    break;
                }
                compatibilityListElem++;
                compatibilityListLen--;
            }

        }
    }
}
#endif
static uint32_t ssbl_SearchBootableImage(
        uint32_t *preferred_app,
        bool *update_page0_required,
        const image_directory_entry_t * img_directory,
        const uint32_t min_valid_addr,
        const IMAGE_CERT_T *root_cert
)
{
    const image_directory_entry_t *dir_entry = NULL;
    uint32_t image_addr = OTA_UTILS_IMAGE_INVALID_ADDR;

    /* Try to find an entry in the img dir with an img id equals to the first byte of the preferred_app */
    dir_entry = ssbl_GetImgDirEntryBasedOnImgType(img_directory, (uint8_t) *preferred_app);
    /* Check if a bootable img has been found */
    if (dir_entry != NULL && dir_entry->flags & IMG_FLAG_BOOTABLE)
    {
        image_addr = OtaUtils_ValidateImage(OtaUtils_ReadFromInternalFlash,
                                          NULL,
                                          ssbl_EEPROM_ReadData,
                                          dir_entry->img_base_addr,
                                          min_valid_addr,
                                          root_cert,
                                          FALSE,
                                          FALSE);
#ifdef SSBL_BLOB_LOAD
        if (image_addr != OTA_UTILS_IMAGE_INVALID_ADDR)
            ssbl_CheckBlobsLinkToBootableImg(img_directory, &image_addr, min_valid_addr, root_cert);
#endif
    }
    else
    {
        /* there is no valid preferred app yet, try to find one bootable image */
        image_addr = ssbl_SearchForValidBootableImg(img_directory, min_valid_addr, root_cert, preferred_app);
#ifdef SSBL_BLOB_LOAD
        if (image_addr != OTA_UTILS_IMAGE_INVALID_ADDR)
            ssbl_CheckBlobsLinkToBootableImg(img_directory, &image_addr, min_valid_addr, root_cert);
#endif
        if (image_addr != OTA_UTILS_IMAGE_INVALID_ADDR)
        {
            *update_page0_required = true;
        }
    }

    return image_addr;
}

#if USE_WATCHDOG

static void ssbl_InitWatchdog(void)
{
    wwdt_config_t wwdt_config;
    uint32_t wdtFreq;

    /* Enable FRO 32 KHz output */
    PMC->PDRUNCFG |= PMC_PDRUNCFG_ENA_FRO32K_MASK;
    SYSCON->OSC32CLKSEL &= ~SYSCON_OSC32CLKSEL_SEL32KHZ_MASK;

    /* Configure WWDT clock */
    SYSCON->PRESETCTRLSET[0] = SYSCON_PRESETCTRL0_RTC_RST_SHIFT; /* Hold WWDT under reset */
    SYSCON->WDTCLKDIV |= SYSCON_WDTCLKDIV_HALT_MASK;
    SYSCON->WDTCLKSEL = kCLOCK_WdtOsc32kClk;
    SYSCON->WDTCLKDIV |= 0U;
    SYSCON->WDTCLKDIV &= ~SYSCON_WDTCLKDIV_DIV_MASK;
    SYSCON->WDTCLKDIV &= ~SYSCON_WDTCLKDIV_HALT_MASK;
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_WWDT_CLK_SET_MASK;
    SYSCON->PRESETCTRLCLR[0] = SYSCON_PRESETCTRL0_WWDT_RST_SHIFT; /* Release WWDT reset */

    /* enable WDTRESETENABLE bit in PMC CTRL */
    PMC->CTRL |= PMC_CTRL_WDTRESETENABLE_MASK;

    /* Catch previous WWDT reset */
    if (WWDT_GetStatusFlags(WWDT) & kWWDT_TimeoutFlag)
    {
        WWDT_ClearStatusFlags(WWDT, kWWDT_TimeoutFlag);
#ifdef WAIT_DEBUG
        /* Loop here in order to wait for a debugger to be connected
         * before running the SSBL */
        volatile int vole = 0;
        while (!vole);
#endif
    }

    /* The WDT divides the input frequency by 4 - Input clock is FRO32M*/
    wdtFreq = 32768UL/4;

    /*
     * Set watchdog feed time constant to WWDT_TIMEOUT_S (5s)
     * Set no watchdog warning
     * Set no watchdog window
     */
    WWDT_GetDefaultConfig(&wwdt_config);
    wwdt_config.timeoutValue = wdtFreq * WWDT_TIMEOUT_S;
    wwdt_config.warningValue = 0;
    wwdt_config.windowValue  = 0xFFFFFF;
    /* Configure WWDT to reset on timeout */
    wwdt_config.enableWatchdogReset = true;
    /* Setup watchdog clock frequency(Hz). */
    wwdt_config.clockFreq_Hz = 32768UL;
    WWDT_Init(WWDT, &wwdt_config);
    NVIC_EnableIRQ(WDT_BOD_IRQn);

    /* First refresh */
    WWDT_Refresh(WWDT);
}

static void ssbl_DeinitWatchdog(void)
{
    SYSCON->AHBCLKCTRLCLRS[0] = SYSCON_AHBCLKCTRLCLR0_WWDT_CLK_CLR_MASK;
    SYSCON->WDTCLKSEL = kCLOCK_WdtNoClock;
}
#endif /* USE_WATCHDOG */

/******************************************************************************
*******************************************************************************
* Main function
*******************************************************************************
******************************************************************************/

int main(void)
{
    int status = -1;
    psector_page_data_t page[2];
    psector_page_state_t page_state[2];

//#define WAIT_DEBUG
#ifdef WAIT_DEBUG
    /* Loop here in order to wait for a debugger to be connected
     * before running the SSBL */
    volatile int vole = 0;
    while (!vole);
#endif

    ENABLE_FROCLK(FRO32M_ENA_SHIFT); /* Enable FRO32M clock */
    SYSCON->MAINCLKSEL = SYSCON_MAINCLKSEL_SEL(3); /* MAINCLK 3: FRO32M 2: XTAL32M */

    EnableBusFault();
    SYSCON->MEMORYREMAP = (SYSCON->MEMORYREMAP & 0xfffff) | 0xe400000;

#if USE_WATCHDOG
    ssbl_InitWatchdog();
#endif /* USE_WATCHDOG */

    do
    {
        bool update_page0_required = false;
        uint32_t image_addr = OTA_UTILS_IMAGE_INVALID_ADDR;
        uint32_t end_of_ssbl;

        /* Read protected sectors */
        page_state[0] = ProtectedSectorRead(PSECTOR_PAGE0_PART, &page[0]);
        if (page_state[0] < PAGE_STATE_DEGRADED)  break;
        page_state[1] = ProtectedSectorRead(PSECTOR_PFLASH_PART, &page[1]);
        if (page_state[1] < PAGE_STATE_DEGRADED)  break;

        IMAGE_CERT_T root_cert_inst;
        IMAGE_CERT_T * root_cert = NULL;

        if (page[1].pFlash.image_authentication_level !=  AUTH_NONE)
        {
            root_cert = &root_cert_inst;
            if (ReconstructRootCert(root_cert, &page[0], &page[1]) != 0) break;
        }

        end_of_ssbl = 0x00 + (uint32_t)SSBL_SIZE;

        /* Sanity check of image directory */
        if (!OtaUtils_ImgDirectorySanityCheck(&page[0], SPIFI_getSize()))
        {
            /* status is -1 so ends up in ISP */
            break;
        }

#ifdef QSPI_EARLY_START
        if(SPIFI_Flash_Init() < 0)
        {
            break;
        }
#endif
        /* Check whether an OTA has been pushed */
#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)

        CustomOtaEntries_t ota_entry = {0};
        uint16_t ota_entry_size = 0;
        ota_entry.ota_state = otaNoImage;
        StatusOta_t procedure_status = StatusOta_ok;
        uint32_t ota_entry_addr = page[0].page0_v3.ota_entry.img_base_addr;
        uint8_t ota_entry_flags = page[0].page0_v3.ota_entry.flags;

        if ((ota_entry_flags & OTA_CUSTOM_ENTRY_FLAG) && (ota_entry_addr != 0))
        {
            do
            {
#ifndef QSPI_EARLY_START
                if(page[0].page0_v3.ota_entry.img_base_addr > FSL_FEATURE_SPIFI_START_ADDR && (SPIFI_Flash_Init() < 0))
                {
                    procedure_status = StatusOta_operation_error;
                    break;
                }
#endif
                /* ssbl_EEPROM_ReadData can be used for either for Ram or Ext flash storage as it use memcpy */
                if(gOtaUtilsSuccess_c != OtaUtils_GetCustomOtaEntry(&ota_entry, &ota_entry_size, ssbl_EEPROM_ReadData, ota_entry_addr))
                {
                    procedure_status = StatusOta_operation_error;
                    break;
                }

                if(ota_entry.number_of_entry > 0)
                {
                    /* Is image(s) not yet validated */
                    if(!(ota_entry_flags & OTA_IMAGE_VALIDATED_FLAG))
                    {
                        CipherMethod_t cipher_method = NO_CIPHER;
                        OtaUtils_ReadBytes pFunction_read = OtaUtils_ReadFromInternalFlash;
                        uint8_t *p_param = NULL;
                        sOtaUtilsSoftwareKey softkey;
                        for(uint8_t i = 0; i < ota_entry.number_of_entry; i++)
                        {
                            if(OTA_UTILS_IMAGE_INVALID_ADDR == ssbl_GetValidImageAndFunctions(&ota_entry.entries[i], &page[0].page0_v3.img_directory[0], &page[0].page0_v3.zigbee_password[0], root_cert, &cipher_method, &pFunction_read, p_param, &softkey))
                            {
                                procedure_status = StatusOta_operation_error;
                                break;
                            }
                        }
                        /* Mark the image(s) as validated */
                        ota_entry_flags = ota_entry_flags | OTA_IMAGE_VALIDATED_FLAG;
                    }

                    if(procedure_status != StatusOta_ok)
                    {
                        break;
                    }

                    /* Apply first OTA entry */
                    procedure_status = ssbl_GetOtaFile(&ota_entry.entries[0], &page[0].page0_v3.img_directory[0], &page[0].page0_v3.zigbee_password[0], &page[0].page0_v3.SelectedImageAddress, root_cert, 0);

                    if(procedure_status != StatusOta_ok)
                    {
                        break;
                    }

                    /* Mark the OTA process as started and image(s) as applied */
                    ota_entry.ota_state = otaStarted;
                    ota_entry_flags = ota_entry_flags | OTA_IMAGE_APPLIED_FLAG;

                    /* Shift all entries */
                    ota_entry.number_of_entry --;
                    if(ota_entry.number_of_entry != 0)
                    {
                        memcpy(&ota_entry.entries[0], &ota_entry.entries[1], ota_entry.number_of_entry*sizeof(image_directory_entry_t));
                    }
                    else
                    {
                        /* Latest image was applied */
                        ota_entry.ota_state = otaApplied;
                    }
                    /* Clear last entry */
                    memset(&ota_entry.entries[ota_entry.number_of_entry], 0, sizeof(image_directory_entry_t));

                    /* Update ota entry flags */
                    page[0].page0_v3.ota_entry.flags = ota_entry_flags;
                }
                else
                {
                    ImgDirectoryEntryClear(&page[0].page0_v3.ota_entry);
                }
            } while(false);

#ifdef SSBL_VERSION
            /* Copy the SSBL version in custom data */
            ota_entry.custom_data[0] = SSBL_VERSION;
            ota_entry.custom_data_length = 4;
#endif
            /* Trigger the PSECT update */
            update_page0_required = true;

            if(procedure_status != StatusOta_ok)
            {
                ota_entry.number_of_entry = 0;
                ota_entry.ota_state = otaNotApplied;
                ImgDirectoryEntryClear(&page[0].page0_v3.ota_entry);
                if(ota_entry_flags & OTA_IMAGE_APPLIED_FLAG || procedure_status == StatusOta_fatal_error)
                {
                    /* Either a part of the OTA already applied or a fatal error occurred during flash copy - go to ISP */
                    break;
                }
            }

            /* In any case, try to store the updated entry table - On error, clear the ota_entry in PSECT */
#if defined(gOTACustomOtaEntryMemory)
#if (gOTACustomOtaEntryMemory == OTACustomStorage_ExtFlash)
#ifndef QSPI_EARLY_START
            if(SPIFI_Flash_Init() < 0)
            {
                break;
            }
#endif
            if(SPIFI_eraseArea((ota_entry_addr - ota_entry_size - FSL_FEATURE_SPIFI_START_ADDR) & ~ (SPIFI_getSectorSize() - 1), SPIFI_getSectorSize()))
            {
                ImgDirectoryEntryClear(&page[0].page0_v3.ota_entry);
                if(ota_entry_flags & OTA_IMAGE_APPLIED_FLAG)
                {
                    /* OTA already applied - go to ISP */
                    break;
                }
            }
            else
            {
                OtaUtils_StoreCustomOtaEntry(&ota_entry, (OtaUtils_EEPROM_WriteData) SPIFI_writeData, ota_entry_addr - FSL_FEATURE_SPIFI_START_ADDR);
            }
#else
            OtaUtils_StoreCustomOtaEntry(&ota_entry, ssbl_RAM_WriteData, ota_entry_addr);
#endif /* gOTACustomOtaEntryMemory */
#endif
        }
        else
        {
#endif /* gOTAUseCustomOtaEntry */
        if (page[0].page0_v3.ota_entry.img_nb_pages != 0)
        {
            if(StatusOta_fatal_error == ssbl_GetOtaFile(&page[0].page0_v3.ota_entry, &page[0].page0_v3.img_directory[0], &page[0].page0_v3.zigbee_password[0], &page[0].page0_v3.SelectedImageAddress, root_cert, 0))
            {
                break; /* Go to ISP */
            }

            /* Will require an update anyway to remove the OTA entry */
            /* If any OTA has necessarily been consumed whether correct or not */
            ImgDirectoryEntryClear(&page[0].page0_v3.ota_entry);
            update_page0_required = true;
        }
#if defined(gOTAUseCustomOtaEntry) && (gOTAUseCustomOtaEntry == 1)
        }
#endif
        if (page[1].pFlash.image_authentication_level ==  AUTH_ON_FW_UPDATE)
        {
            /* Reset the root_cert to do authentication only on fw update and not at each boot */
            root_cert = NULL;
        }

        /* Check for valid image data in protected sector */
        image_addr = ssbl_SearchBootableImage(
                &page[0].page0_v3.preferred_app_index,
                &update_page0_required,
                page[0].page0_v3.img_directory,
                end_of_ssbl,
                root_cert
        );

        if (update_page0_required)
        {
            page[0].hdr.version++;
            page[0].hdr.magic    = PSECTOR_PAGE0_MAGIC;
            page[0].hdr.checksum = psector_CalculateChecksum((psector_page_t *)&page[0]);

            psector_WriteUpdatePage(PSECTOR_PAGE0_PART, (psector_page_t *)&page[0]);

#if USE_WATCHDOG
            /* Disable the WWDT before reset */
            ssbl_DeinitWatchdog();
#endif /* USE_WATCHDOG */

            /* Provoke Reset now to force update of image address */
            SysReset();
        }

        if (image_addr == OTA_UTILS_IMAGE_INVALID_ADDR)
        {
            break;
        }

#if EXTERNAL_FLASH_DATA_OTA
        /* If here, bootable image is found. Find the extra data descriptor in image if any */
        IMG_HEADER_T img_header_ota;
        BOOT_BLOCK_T boot_block_ota;
        EXTRA_DATA_HEADER_T extra_data_header;

        OtaUtils_ReadFromInternalFlash(sizeof(IMG_HEADER_T),  image_addr, (uint8_t *)&img_header_ota, NULL, ssbl_EEPROM_ReadData);
        OtaUtils_ReadFromInternalFlash(sizeof(BOOT_BLOCK_T),  image_addr + img_header_ota.bootBlockOffset, (uint8_t *)&boot_block_ota, NULL, ssbl_EEPROM_ReadData);
        OtaUtils_ReadFromInternalFlash(sizeof(EXTRA_DATA_HEADER_T),  image_addr + img_header_ota.bootBlockOffset - sizeof(EXTRA_DATA_HEADER_T), (uint8_t *)&extra_data_header, NULL, ssbl_EEPROM_ReadData);

        /* If any extra data linked to the image is expected in external flash, compute the Hash */
        if(((extra_data_header.extra_data_magic == EXTRA_SECTION_MAGIC) || (extra_data_header.extra_data_magic == EXTRA_SECTION_MAGIC_APP_ONLY)) && (extra_data_header.extra_data_size != 0))
        {
#ifndef QSPI_EARLY_START
            if (SPIFI_Flash_Init() < 0)
            {
                /* Ends up in ISP */
                break;
            }
#endif
            if(gOtaUtilsSuccess_c != ssbl_ComputeExtraDataHash(extra_data_header.extra_data_lma, extra_data_header.extra_data_size, extra_data_header.extra_data_hash, OtaUtils_ReadFromUnencryptedExtFlash, NULL, ssbl_EEPROM_ReadData))
            {
                /* Hash comparison failed - Ends up in ISP */
                break;
            }
        }
#endif /* EXTERNAL_FLASH_DATA_OTA */
        /*
         * Clear pending status into flash for ATE test.
         * During ATE test after reset the flash controller is waiting for a cleared
         * status on register pFLASH->INT_STATUS
         *
         */
        FLASH->INT_CLR_STATUS = FLASH_STAT_ALL;


        uint32_t u32AppStackPointer = ((IMG_HEADER_T *)image_addr)->vectors[0];
        uint32_t u32AppResetVector  = ((IMG_HEADER_T *)image_addr)->vectors[1];

        /* Remember which address was picked for Resume acceleration */
        BOOT_SetImageAddress(image_addr);

#if 0
        /* Remember which address was picked for Resume acceleration. Using
         * previously unused bits of cfg_word in the persistent data area:
         * bits 31:20, leaving bit 19 still unused and untouched. Assumptions
         * required to get value to fit into available space:
         *  - image_addr is exact multiple of 512 bytes
         *  - image_addr is within 640kB flash space (not QSPI)
         *
         *  If assumptions cannot be met, value of 0 is set so can be detected
         *  by SSBL during warm start.
         */
        uint32_t u32ImageAddrPage;
        u32ImageAddrPage = image_addr / 512U;
        if (   (0 != (image_addr & 511U))
            || (u32ImageAddrPage > 0xFFFU)
           )
        {
            u32ImageAddrPage = 0U;
        }
        BOOT_SetCfgWord((BOOT_ResumeGetCfgWord() & 0x000FFFFFUL)
                        | (u32ImageAddrPage << 20UL));

        /* To read value back out later: */
        image_addr = ((BOOT_ResumeGetCfgWord() >> 20UL) & 0xFFFU) * 512U;
#endif

        SET_MPU_CTRL(0); /* Turn off the MPU in case it got set by some ROM function */
        /* Reset the vector table back to default */
        SET_SCB_VTOR(image_addr);

#if USE_WATCHDOG
        /* Disable the WWDT before reset */
        ssbl_DeinitWatchdog();
#endif /* USE_WATCHDOG */

        /* Load application stack pointer and jump to reset vector */
        asm volatile (
                "mov sp, %0\n"
                "bx  %1\n"
                :
                : "r" (u32AppStackPointer),
                  "r" (u32AppResetVector));
    } while (0);

    if (status < 0)
    {
#if DISABLE_ISP
        SysReset();
#else
        int status = ISP_Entry(ISP_INVALID_EXTENSION);
        if (!status)
        {
            //DBGOUT("No valid Image & ISP enabled - > enter ISP\n");
        }
        else
        {
            //DBGOUT("No valid Image & ISP disabled - > dead state\n");
            while(1);
        }
#endif /* DISABLE_ISP */
    }
    return 0;
}
