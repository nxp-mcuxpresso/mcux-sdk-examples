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

#define SET_SCB_VTOR(x)  *(uint32_t *)0xE000ED08 = (uint32_t)(x)
#define SET_MPU_CTRL(x) (*(uint32_t*)0xe000ed94 = (uint32_t)(x))

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

#define THUMB_ENTRY(x)                 (void*)((x) | 1)

#define ROM_API_aesDeinit              THUMB_ENTRY(0x03001460)
#define ROM_API_efuse_AESKeyPresent    THUMB_ENTRY(0x030016ec)
#define ROM_API_psector_Init           THUMB_ENTRY(0x03004e94)
#define ROM_API_BOOT_SetImageAddress   THUMB_ENTRY(0x03000eb4)
#if 0
#define ROM_API_BOOT_SetCfgWord        THUMB_ENTRY(0x03000ec0)
#define ROM_API_BOOT_ResumeGetCfgWord  THUMB_ENTRY(0x03000ecc)
#endif


/* Avoid using Fro_ClkSel_t that is being moved from fsl_clock.c to fsl_clock.h*/
typedef enum
{
    FRO12M_ENA_SHIFT,
    FRO32M_ENA_SHIFT,
    FRO48M_ENA_SHIFT,
    FRO64M_ENA_SHIFT,
    FRO96M_ENA_SHIFT,
} FroClkSelBit_t;

#ifndef EXTERNAL_FLASH_DATA_OTA
#define EXTERNAL_FLASH_DATA_OTA        0
#endif

#if EXTERNAL_FLASH_DATA_OTA
#define EXTRA_SECTION_MAGIC     0x73676745
#define COPY_BUFFER_LENGTH        256U
#define MAX_EXTRA_DATA_SIZE        0x7000U /* Should be tuned according use case */
#define EXTRA_BUFFER_SHA_LENGTH        16U

/*!
 * @brief Extra data header.
 *
 * Be very cautious when modifying the EXTRA_DATA_HEADER_T structure (alignment) as
 * these structures are used in the image_tool.py (which does not take care of alignment).
 */
typedef struct
{
    uint32_t extra_data_magic;        /*!< Identifier for extra section descriptor */
    uint32_t extra_data_lma;        /*!< Extra data load machine address - typically External flash address*/
    uint32_t extra_data_size;       /*!< Extra data size in Bytes */
    uint8_t extra_data_hash[32];    /*!< Computed Hash of the extra data - 256 Bits (32 Bytes)*/
} EXTRA_DATA_HEADER_T;

#define GET_PAGE_MULTIPLE_UP(X, PAGE_SIZE) ((X + PAGE_SIZE-1)/PAGE_SIZE)

#endif

#define ENABLE_FROCLK(x) PMC->FRO192M |= (1 << (PMC_FRO192M_DIVSEL_SHIFT+(x)))
/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

typedef enum {
#if EXTERNAL_FLASH_DATA_OTA
    StatusOta_incorrect_extra_data = -9,
#endif
    StatusOTA_downgrade_error = -8,
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
extern uint32_t INT_STORAGE_START[];
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

static bool AddressWithinInternalFlashSafeRange(uint32_t addr)
{
    return (addr < (uint32_t) INT_STORAGE_START);
}

static  bool ssbl_ImgDirectorySanityCheck(psector_page_data_t * page0)

{
    const image_directory_entry_t * img_directory = &page0->page0_v3.img_directory[0];
    bool res = false;
    uint32_t img_base, img_end;
    for (int i = 0; i < IMG_DIRECTORY_MAX_SIZE; i++)
    {
        img_base = img_directory[i].img_base_addr;
        img_end = img_base + FLASH_PAGE_SIZE * img_directory[i].img_nb_pages;
        bool overlap = false;
        if (AddressWithinInternalFlashSafeRange(img_end))
        {
            for (int j = i+1; j < IMG_DIRECTORY_MAX_SIZE; j++ )
            {
                uint32_t other_entry_base = img_directory[j].img_base_addr;
                uint32_t other_entry_end = other_entry_base + FLASH_PAGE_SIZE * img_directory[j].img_nb_pages;
                if (((img_base < other_entry_base) && (img_end > other_entry_base)) ||
                        ((img_base >= other_entry_base) && (img_base < other_entry_end)))
                {
                    overlap = true;
                    break;
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
    return res;
}

static otaUtilsResult_t ssbl_EEPROM_ReadData(uint16_t nbBytes, uint32_t address, uint8_t *pInbuf)
{
    memcpy(pInbuf, (void*)(address), nbBytes);
    return gOtaUtilsSuccess_c;
}

#if EXTERNAL_FLASH_DATA_OTA
static otaUtilsResult_t ssbl_ComputeExtraDataHash(uint32_t p_extra_data_addr,
                                                  uint32_t extra_data_size,
                                                  uint8_t *p_computed_hash,
                                                  OtaUtils_ReadBytes pFunctionRead,
                                                  void * pReadFunctionParam,
                                                  OtaUtils_EEPROM_ReadData pFunctionEepromRead)
{
    otaUtilsResult_t res = gOtaUtilsError_c;
    uint32_t nbPageToRead = extra_data_size/EXTRA_BUFFER_SHA_LENGTH;
    uint32_t lastPageNbByteNumber = extra_data_size - (nbPageToRead*EXTRA_BUFFER_SHA_LENGTH);
    uint32_t i = 0;

    do {
        uint8_t pageContent[EXTRA_BUFFER_SHA_LENGTH];
        uint8_t digest[32];
        sha_ctx_t hash_ctx;
        size_t sha_sz = 32;
        /* Initialise SHA clock do not call SHA_ClkInit(SHA0) because the HAL pulls in too much code  */
        SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_HASH_MASK;
        if (SHA_Init(SHA0, &hash_ctx, kSHA_Sha256) != kStatus_Success)
            break;
        for (i=0; i<nbPageToRead; i++)
        {
            if (pFunctionRead(sizeof(pageContent),  p_extra_data_addr+(i*EXTRA_BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
                break;
            if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, EXTRA_BUFFER_SHA_LENGTH) != kStatus_Success)
                break;
        }
        /* Read bytes located on the last page */
        if (pFunctionRead(lastPageNbByteNumber,  p_extra_data_addr+(i*EXTRA_BUFFER_SHA_LENGTH), &pageContent[0], pReadFunctionParam, pFunctionEepromRead) != gOtaUtilsSuccess_c)
            break;
        if (SHA_Update(SHA0, &hash_ctx, (const uint8_t*)pageContent, lastPageNbByteNumber) != kStatus_Success)
                break;
        if (SHA_Finish(SHA0,  &hash_ctx, &digest[0], &sha_sz) != kStatus_Success)
            break;
        /* Compare with the computed Hash */
        res = gOtaUtilsSuccess_c;
        void *p_hash = &digest[0];
        while (sha_sz)
        {
            if ( *((uint8_t *)p_hash) != *((uint8_t *)p_computed_hash))
            {
                res = gOtaUtilsError_c;
                break;
            }
            p_computed_hash = (uint8_t* )p_computed_hash+1;
            p_hash = (uint8_t* )p_hash+1;
            sha_sz--;
        }
    } while (0);
    SYSCON->AHBCLKCTRLCLR[1] =  SYSCON_AHBCLKCTRL1_HASH_MASK; /* equivalent to SHA_ClkDeinit(SHA0) */
    return res;
}
#endif

static StatusOta_t ssbl_GetOtaFile(const image_directory_entry_t * ota_entry,
                              const image_directory_entry_t * img_directory,
                              const uint8_t *zigbee_password,
                              uint32_t *selected_img_addr,
                              const IMAGE_CERT_T * root_cert, uint32_t min_valid_addr)
{
    StatusOta_t err = StatusOta_ok_so_far;
    bool_t external_flash_ota = (ota_entry->img_base_addr >= FSL_FEATURE_SPIFI_START_ADDR);
    uint32_t img_addr_ota = ota_entry->img_base_addr;
    uint32_t img_addr_targeted;
    uint32_t img_total_len = 0;
    CipherMethod_t cipher_method = NO_CIPHER;
    OtaUtils_ReadBytes pFunction_read = OtaUtils_ReadFromInternalFlash;
    OtaUtils_EEPROM_ReadData pFunction_eeprom_read = ssbl_EEPROM_ReadData;
    uint8_t *p_param = NULL;
    sOtaUtilsSoftwareKey softKey;
    const image_directory_entry_t * dest_dir_entry = NULL;
    IMG_HEADER_T img_header_ota;
    BOOT_BLOCK_T boot_block_ota;
    uint8_t img_type_ota;
    uint16_t img_nb_pages = ota_entry->img_nb_pages;
    uint8_t page_content_ota[FLASH_PAGE_SIZE];
    bool_t ssblUpdate = FALSE;

    do
    {
        if (external_flash_ota)
        {
#ifndef QSPI_EARLY_START
            if (SPIFI_Flash_Init() < 0)
            {
                RAISE_ERROR(err, StatusOta_operation_error);
            }
#endif
            if (ota_entry->flags & 0x80)
            {
                if (efuse_AESKeyPresent())
                {
                    cipher_method = AES_EFUSE_CIPHER;
                }
                pFunction_read = OtaUtils_ReadFromEncryptedExtFlashEfuseKey;
            }
            else if (ota_entry->flags & 0x40)
            {
                cipher_method = AES_PASSWORD_CIPHER;
                pFunction_read = OtaUtils_ReadFromEncryptedExtFlashSoftwareKey;
                softKey.pSoftKeyAes = zigbee_password;
                p_param = (uint8_t *) &softKey;
            }
            if (cipher_method != NO_CIPHER)
            {
                if (aesInit() != 0)
                    RAISE_ERROR(err, StatusOta_operation_error);
            }
            else
            {
                pFunction_read = OtaUtils_ReadFromUnencryptedExtFlash;
            }
        }

        TRY
        {
            img_addr_targeted = OtaUtils_ValidateImage(pFunction_read,
                                          p_param,
                                          pFunction_eeprom_read,
                                          img_addr_ota,
                                          min_valid_addr,
                                          root_cert,
                                          TRUE,
                                          FALSE);
        }
        CATCH(BUS_EXCEPTION)
        {
            RAISE_ERROR(err, StatusOta_bus_fault);
        }
        YRT

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
        int img_nb_pages_ota = ((boot_block_ota.img_len + FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE);
        int dst_section_size = (dest_dir_entry->img_nb_pages*FLASH_PAGE_SIZE);
        img_total_len = boot_block_ota.img_len;
        if(root_cert != NULL)
        {
            img_total_len += sizeof(ImageSignature_t);
        }
        if (img_nb_pages_ota > dest_dir_entry->img_nb_pages
                || img_total_len > dst_section_size)
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
                        RAISE_ERROR(err, StatusOTA_downgrade_error);
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
            if((extra_data_header.extra_data_magic == EXTRA_SECTION_MAGIC) && (img_nb_pages*FLASH_PAGE_SIZE >= img_total_len + extra_data_header.extra_data_size))
            {
                /* Sanity check on extracted descriptor */
                if((extra_data_header.extra_data_size == 0) ||
                        (extra_data_header.extra_data_lma < FSL_FEATURE_SPIFI_START_ADDR)
                        || (extra_data_header.extra_data_lma > FSL_FEATURE_SPIFI_END_ADDR)
                        || extra_data_header.extra_data_size > MAX_EXTRA_DATA_SIZE)
                {
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);
                }

                extra_data_src_addr = img_addr_ota + img_total_len;
                if(extra_data_src_addr + extra_data_header.extra_data_size >= FSL_FEATURE_SPIFI_END_ADDR)
                {
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);
                }

                if(gOtaUtilsSuccess_c != ssbl_ComputeExtraDataHash((uint32_t *)extra_data_src_addr, extra_data_header.extra_data_size, extra_data_header.extra_data_hash, pFunction_read, p_param, pFunction_eeprom_read))
                {
                    /* At this stage nothing as been done - simply exit and keep the current image */
                    RAISE_ERROR(err, StatusOta_incorrect_extra_data);
                }

                /* Checks passed - Start erasing the needed area - must be multiple of 4K */

                if(0 != SPIFI_eraseArea(extra_data_header.extra_data_lma - FSL_FEATURE_SPIFI_START_ADDR, (int32_t) GET_PAGE_MULTIPLE_UP(MAX_EXTRA_DATA_SIZE, 4096U)))
                {
                    /* Exit on failure - TODO: if data already erased, only ISP mode can recover */
                    RAISE_ERROR(err, StatusOta_operation_error);
                }
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
                    dst_section_size = dst_section_size - COPY_BUFFER_LENGTH;
                }
                if(err != StatusOta_ok_so_far)
                {
                    /* Exit on failure - TODO: if data already erased, only ISP mode can recover */
                    RAISE_ERROR(err, StatusOta_operation_error);
                }
                /* Remove the extra pages from the total image length */
                img_nb_pages = GET_PAGE_MULTIPLE_UP(img_nb_pages*FLASH_PAGE_SIZE - extra_data_header.extra_data_size, FLASH_PAGE_SIZE);
            }
        }
#endif /* EXTERNAL_FLASH_DATA_OTA */

        /* Erase flash section needed before copying */
        uint8_t * end_addr = (uint8_t*)img_addr_targeted + dst_section_size -1;
        if ((uint32_t *)end_addr > INT_STORAGE_START)
        {
            if(img_addr_targeted + ota_entry->img_nb_pages*FLASH_PAGE_SIZE < (uint32_t) INT_STORAGE_START)
            {
                end_addr = (uint8_t*)img_addr_targeted + ota_entry->img_nb_pages*FLASH_PAGE_SIZE -1;
            }
            else
            {
                RAISE_ERROR(err, StatusOta_incorrect_image_struct);
            }
        }
        if (FLASH_Erase(FLASH, (uint8_t*)img_addr_targeted,
                (uint8_t*) end_addr) != FLASH_DONE)
        {
            RAISE_ERROR(err, StatusOta_operation_error);
        }

        for (int i= 0 ; i < img_nb_pages; i++)
        {
            TRY
            {
                pFunction_read(sizeof(page_content_ota),  img_addr_ota+(i*FLASH_PAGE_SIZE), &page_content_ota[0], p_param, pFunction_eeprom_read);
            }
            CATCH(BUS_EXCEPTION)
            {
                RAISE_ERROR(err, StatusOta_bus_fault);
            }
            YRT
#if EXTERNAL_FLASH_DATA_OTA
            if(i==(img_nb_pages-1) && (img_total_len%FLASH_PAGE_SIZE != 0))
            {
                /* For last page, we may need to clear some bytes (end of the page) */
                for (uint32_t j=(img_total_len%FLASH_PAGE_SIZE); j<FLASH_PAGE_SIZE; j++)
                {
                    page_content_ota[j] = 0xFF;
                }
            }
#endif
            if (FLASH_Program(FLASH, (uint32_t*) (img_addr_targeted+(i*FLASH_PAGE_SIZE)),
                    (uint32_t*) &page_content_ota[0], FLASH_PAGE_SIZE) != FLASH_DONE)
                RAISE_ERROR(err, StatusOta_operation_error);
        }

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
    do {
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
        if (!ssbl_ImgDirectorySanityCheck(&page[0]) )
        {
            /* status is -1 so ends up in ISP */
            break;
        }

        /* Check whether an OTA has been pushed */
        if (page[0].page0_v3.ota_entry.img_nb_pages != 0)
        {
            ssbl_GetOtaFile(&page[0].page0_v3.ota_entry, &page[0].page0_v3.img_directory[0], &page[0].page0_v3.zigbee_password[0], &page[0].page0_v3.SelectedImageAddress, root_cert, 0);
            /* Will require an update anyway to remove the OTA entry */
            /* If any OTA has necessarily been consumed whether correct or not */
            ImgDirectoryEntryClear(&page[0].page0_v3.ota_entry);
            update_page0_required = true;

        }

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

            /* Provoke Reset now to force update of image address */
            SysReset();
        }

        if (image_addr == OTA_UTILS_IMAGE_INVALID_ADDR)
            break;

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
    }
    return 0;
}
