/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ROM_IAP_H_
#define _ROM_IAP_H_

#include "fsl_flash.h"
#include "fsl_flexspi_nor_flash.h"


enum {kStatusRomLdrDataUnderrun = 10109};


typedef struct _nboot_context
{
    uint32_t buffer[0x2e8/sizeof(uint32_t)];
} nboot_context_t;


typedef union StandardVersion
{
    struct
    {
        uint8_t bugfix; /*!< bugfix version [7:0] */
        uint8_t minor;  /*!< minor version [15:8] */
        uint8_t major;  /*!< major version [23:16] */
        char name;      /*!< name [31:24] */
    };
    uint32_t version;   /*!< combined version numbers */
} standard_version_t;


typedef struct kb_api_parameter_struct
{
    uint32_t allocStart;
    uint32_t allocSize;
}kp_api_init_param_t ;


// As described by SRM

typedef struct api_core_context
{
 uint32_t reserved0[10];
 uint32_t reserved1[3];
 flash_config_t flashConfig;
 flexspi_nor_config_t flexspinorCfg;
 uint32_t reserved2[2];
 uint32_t reserved3[1];
 nboot_context_t *nbootCtx;
 uint8_t *sharedBuf;
 uint32_t reserved4[6];
} api_core_context_t;


typedef struct sb3_iap_ctx
{
    api_core_context_t core_ctx;
    int __pad__[4];
    kp_api_init_param_t init_param;
    
} sb3_iap_ctx_t;


typedef struct mbi_image_info
{
    uint32_t length;
    uint8_t  type;
    uint16_t img_version;
    uint32_t execaddr;    
    uint32_t cert_offset;
    uint32_t cert_size;
    uint32_t fw_version;
} mci_image_info_t;

/********************************************************************************
 * IAP API
 *******************************************************************************/

//!@brief return IAP API version
standard_version_t iap_api_version(void);

//!@brief Initialize the IAP API runtime environment
status_t iap_api_init(api_core_context_t *coreCtx, const kp_api_init_param_t *param);

//!@brief Deinitialize the IAP API runtime environment
status_t iap_api_deinit(api_core_context_t *coreCtx);

//!@brief Intialize the memory interface of the IAP API
status_t iap_mem_init(api_core_context_t *coreCtx);
//!@brief Perform the memory write operation
status_t iap_mem_write(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, const uint8_t *buf, uint32_t memoryId);
//!@brief Perform the Memory read operation
status_t iap_mem_read(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint8_t *buf, uint32_t memoryId);
//!@brief Perform the Fill operation
status_t iap_mem_fill(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint32_t pattern, uint32_t memoryId);
//!@brief Perform the Memory erase operation
status_t iap_mem_erase(api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint32_t memoryId);
//!@brief Perform the full Memory erase operation
status_t iap_mem_erase_all(api_core_context_t *coreCtx, uint32_t memoryId);
//!@brief Perform the Memory configuration operation
status_t iap_mem_config(api_core_context_t *coreCtx, uint32_t *config, uint32_t memoryId);
//!@brief Perform the Memory Flush operation
status_t iap_mem_flush(api_core_context_t *coreCtx);

//!@brief Perform the Sbloader runtime environment initialization
status_t api_sbloader_init(api_core_context_t *ctx);
//!@brief Handle the SB data stream
status_t api_sbloader_pump(api_core_context_t *ctx, uint8_t *data, uint32_t length);
//!@brief Finish the sbloader handling
status_t api_sbloader_finalize(api_core_context_t *ctx);


/********************************************************************************
 * IAP Helpes Functions
 *******************************************************************************/

/* tests if flah remapping is active */
int is_remap_active(void);

/* tests presence of SB3 file signature */
int is_sb3_header(const void *header);

/* parses basic information about MBI image */
void parse_mbi_image_info(const uint32_t *image, struct mbi_image_info *info);

/* ROM IAP - initialization call */
status_t sb3_iap_init(sb3_iap_ctx_t *ctx);

/* ROM IAP - pump function used to sequentially pass SB3 data to ROM for processing */
status_t sb3_iap_pump(sb3_iap_ctx_t *ctx, uint8_t *data, size_t len);

/* ROM IAP - finall call after succesfull processing by pump function */
void sb3_iap_finalize(sb3_iap_ctx_t *ctx);

/* ROM IAP - used to free allocated memory*/
void sb3_iap_free(sb3_iap_ctx_t *ctx);

#endif // _ROM_IAP_H_
