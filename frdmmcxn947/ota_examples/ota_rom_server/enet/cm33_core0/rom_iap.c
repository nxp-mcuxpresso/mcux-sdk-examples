/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "rom_iap.h"
#include "fsl_flash.h"
#include "fsl_debug_console.h"

/*
ROM In Application Programming (IAP) API for MCX N10 series MCU's
*/

#define ROM_IAP_PTR ((iap_api_interface_t **)0x1303FC34)


typedef struct iap_api_interface_struct
{
    standard_version_t version; //!< IAP API version number.
    status_t (*api_init)(api_core_context_t *coreCtx, const kp_api_init_param_t *param);
    status_t (*api_deinit)(api_core_context_t *coreCtx);
    status_t (*mem_init)(api_core_context_t *ctx);
    status_t (*mem_read)(api_core_context_t *ctx, uint32_t addr, uint32_t len, uint8_t *buf, uint32_t memoryId);
    status_t (*mem_write)(api_core_context_t *ctx, uint32_t addr, uint32_t len, const uint8_t *buf, uint32_t memoryId);
    status_t (*mem_fill)(api_core_context_t *ctx, uint32_t addr, uint32_t len, uint32_t pattern, uint32_t memoryId);
    status_t (*mem_flush)(api_core_context_t *ctx);
    status_t (*mem_erase)(api_core_context_t *ctx, uint32_t addr, uint32_t len, uint32_t memoryId);
    status_t (*mem_config)(api_core_context_t *ctx, uint32_t *buf, uint32_t memoryId);
    status_t (*mem_erase_all)(api_core_context_t *ctx, uint32_t memoryId);
    status_t (*sbloader_init)(api_core_context_t *ctx);
    status_t (*sbloader_pump)(api_core_context_t *ctx, uint8_t *data, uint32_t length);
    status_t (*sbloader_finalize)(api_core_context_t *ctx);
} iap_api_interface_t;



/********************************************************************************
 * IAP API
 *******************************************************************************/

standard_version_t iap_api_version(void)
{
    return (*ROM_IAP_PTR)->version;
}

status_t iap_api_init(api_core_context_t *coreCtx, const kp_api_init_param_t *param)
{
    return (*ROM_IAP_PTR)->api_init(coreCtx, param);
}

status_t iap_api_deinit(api_core_context_t *coreCtx)
{
    return (*ROM_IAP_PTR)->api_deinit(coreCtx);
}

status_t iap_mem_init(api_core_context_t *coreCtx)
{
    return (*ROM_IAP_PTR)->mem_init(coreCtx);
}

status_t iap_mem_read(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint8_t *buf, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_read(coreCtx, start, lengthInBytes, buf, memoryId);
}

status_t iap_mem_write(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, const uint8_t *buf, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_write(coreCtx, start, lengthInBytes, buf, memoryId);
}

status_t iap_mem_fill(
    api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint32_t pattern, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_fill(coreCtx, start, lengthInBytes, pattern, memoryId);
}

status_t iap_mem_erase(api_core_context_t *coreCtx, uint32_t start, uint32_t lengthInBytes, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_erase(coreCtx, start, lengthInBytes, memoryId);
}

status_t iap_mem_erase_all(api_core_context_t *coreCtx, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_erase_all(coreCtx, memoryId);
}

status_t iap_mem_config(api_core_context_t *coreCtx, uint32_t *config, uint32_t memoryId)
{
    return (*ROM_IAP_PTR)->mem_config(coreCtx, config, memoryId);
}

status_t iap_mem_flush(api_core_context_t *coreCtx)
{
    return (*ROM_IAP_PTR)->mem_flush(coreCtx);
}

status_t api_sbloader_init(api_core_context_t *ctx)
{
    return (*ROM_IAP_PTR)->sbloader_init(ctx);
}

status_t api_sbloader_pump(api_core_context_t *ctx, uint8_t *data, uint32_t length)
{
    return (*ROM_IAP_PTR)->sbloader_pump(ctx, data, length);
}

status_t api_sbloader_finalize(api_core_context_t *ctx)
{
    return (*ROM_IAP_PTR)->sbloader_finalize(ctx);
}


/********************************************************************************
 * IAP Helpes Functions
 *******************************************************************************/

int is_remap_active(void)
{
    return NPX0->REMAP ? 1 : 0;
}


int is_sb3_header(const void *header)
{
    return !memcmp("sbv3", header, 4);
}


void parse_mbi_image_info(const uint32_t *image, struct mbi_image_info *info)
{
    info->length       = image[0x20/4];
    info->type         = image[0x24/4] & 0xff;
    info->img_version  = (image[0x24/4] & (1<<10)) ? (image[0x24/4] >> 16) : 0;
    info->execaddr     = image[0x34/4];
    
    info->cert_offset = image[0x28/4];
    info->cert_size   = image[info->cert_offset/4 + 2];
    info->fw_version  = image[(info->cert_offset+info->cert_size)/4 + 2];    
}


status_t sb3_iap_init(sb3_iap_ctx_t *ctx)
{
    status_t status;
    
    const size_t iapWorkBufSize = 0x1000;
    
    //standard_version_t iapVersion = iap_api_version();
    //PRINTF("IAP API version=%d.%d.%d\n", iapVersion.major, iapVersion.minor, iapVersion.bugfix);
    
    memset(&ctx->core_ctx, 0x0, sizeof(ctx->core_ctx));
    
    ctx->core_ctx.flashConfig.modeConfig.sysFreqInMHz = SystemCoreClock / 1000000;
    
    memset(&ctx->init_param, 0x0, sizeof(ctx->init_param));
    
    ctx->init_param.allocSize = iapWorkBufSize;
    ctx->init_param.allocStart = (uint32_t) pvPortMalloc(iapWorkBufSize);
    
    if (ctx->init_param.allocStart == NULL)
    {
        PRINTF("%s: Failed to allocate memory for IAP work buffer\n", __func__);
        return kStatus_Fail;
    }
    
    status = iap_api_init(&(ctx->core_ctx), &(ctx->init_param));
    if (status != kStatus_Success)
    {
        PRINTF("%s: iap_api_init() failed with %d\n", __func__, status);
        goto cleanup;
    }
        
    status = api_sbloader_init(&ctx->core_ctx);
    if (status != kStatus_Success)
    {
        PRINTF("%s: api_sbloader_init() failed with %d\n", __func__, status);
        goto cleanup;
    }
    
    return status;
    
cleanup:
    
    vPortFree((void *) ctx->init_param.allocStart);
    
    return status;
}

status_t sb3_iap_pump(sb3_iap_ctx_t *ctx, uint8_t *data, size_t len)
{
    status_t status;
    
    status = api_sbloader_pump(&ctx->core_ctx, data, len);
    
    return status;
}

    
void sb3_iap_finalize(sb3_iap_ctx_t *ctx)
{
    api_sbloader_finalize(&ctx->core_ctx);
}


void sb3_iap_free(sb3_iap_ctx_t *ctx)
{
    vPortFree((void *)ctx->init_param.allocStart);
}
