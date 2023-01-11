/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "httpsrv_multipart.h"

#include "lwip/api.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "ksdk_mbedtls.h"
#include "httpsrv.h"
#include "httpsrv_port.h"

#include "mbedtls/certs.h"

#include "mflash_drv.h"
#include "timers.h"
#include "fsl_debug_console.h"

#include "sysflash/sysflash.h"
#include "flash_map.h"
#include "mcuboot_app_support.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FlexSPI_AMBA_BASE FlexSPI0_AMBA_BASE
#define APP_DEBUG_UART_TYPE     kSerialPort_Uart
#define APP_DEBUG_UART_INSTANCE 12U
#define APP_DEBUG_UART_CLK_FREQ CLOCK_GetFlexcommClkFreq(12)
#define APP_DEBUG_UART_FRG_CLK \
    (&(const clock_frg_clk_config_t){12U, kCLOCK_FrgPllDiv, 255U, 0U}) /*!< Select FRG0 mux as frg_pll */
#define APP_DEBUG_UART_CLK_ATTACH kFRG_to_FLEXCOMM12
#define APP_DEBUG_UART_BAUDRATE   115200

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG LWIP_DBG_ON
#endif
#ifndef HTTPD_STACKSIZE
#define HTTPD_STACKSIZE 1000
#endif
#ifndef HTTPD_PRIORITY
#define HTTPD_PRIORITY DEFAULT_THREAD_PRIO
#endif
#ifndef DEBUG_WS
#define DEBUG_WS 0
#endif

#define OTA_UPDATE_PART     (BL_FEATURE_SECONDARY_IMG_START - FlexSPI_AMBA_BASE)
#define OTA_MAX_IMAGE_SIZE  (BL_FEATURE_PRIMARY_IMG_PARTITION_SIZE - BL_IMG_HEADER_SIZE)
#define OTA_IMAGE_LOAD_ADDR (BL_FEATURE_PRIMARY_IMG_START + BL_IMG_HEADER_SIZE)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static int cgi_ota_upload(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_ota_reboot(HTTPSRV_CGI_REQ_STRUCT *param);
static int cgi_ota_accept(HTTPSRV_CGI_REQ_STRUCT *param);

static int ssi_ota_status(HTTPSRV_SSI_PARAM_STRUCT *param);
static int ssi_ota_image_upload(HTTPSRV_SSI_PARAM_STRUCT *param);
static int ssi_ota_image_info(HTTPSRV_SSI_PARAM_STRUCT *param);

static int32_t mcuboot_image_size_sanity_check(const uint8_t *data, uint32_t size);

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern const HTTPSRV_FS_DIR_ENTRY httpsrv_fs_data[];

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"upload", cgi_ota_upload},
    {"reboot", cgi_ota_reboot},
    {"accept", cgi_ota_accept},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

const HTTPSRV_SSI_LINK_STRUCT ssi_lnk_tbl[] = {
    {"ota_status", ssi_ota_status},
    {"ota_image_upload", ssi_ota_image_upload},
    {"ota_image_info", ssi_ota_image_info},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void APP_InitAppDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    /* attach FRG0 clock to FLEXCOMM12 (debug console) */
    CLOCK_SetFRGClock(APP_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(APP_DEBUG_UART_CLK_ATTACH);

    uartClkSrcFreq = APP_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(APP_DEBUG_UART_INSTANCE, APP_DEBUG_UART_BAUDRATE, APP_DEBUG_UART_TYPE, uartClkSrcFreq);
}


enum ota_status_t
{
    OTA_STATUS_NONE = 0,
    OTA_STATUS_UPLOAD_FAILED,
    OTA_STATUS_UPLOAD_OK,
    OTA_STATUS_TESTING,
    OTA_STATUS_BOOTLOADER_ERROR,
} ota_status = OTA_STATUS_NONE;

char *ota_status_strings[] = {
    "Select firmware files to be updated and click <b>Upload</b>.",
    "<b>Failed</b> to upload all requested images, see console log for details.",
    "Update images successfully uploaded, click <b>Reboot</b> to start in test mode.",
    "Running in test mode, click <b>accept</b> to make image permanent or <br>Reboot</b> for rollback.",
    "<b>Error</b> - check bootloader installation.",
};

/* Server Side Include callback for OTA status. */
static int ssi_ota_status(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    char *status_string = ota_status_strings[ota_status];
    HTTPSRV_ssi_write(param->ses_handle, status_string, strlen(status_string));
    return (0);
}

/* generates image upload form */

static int ssi_ota_image_upload(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    char buf[128];
    char *html;

    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++)
    {
        const char *name = boot_image_names[image];

        html = "<tr>";
        HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));

        snprintf(buf, sizeof(buf),
                 "<td><b>Image %s</b></td>"
                 "<td><input type='file' id='upload_image_%u' name='upload_image_%u'></td>",
                 name, image, image);
        HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));

        html = "</tr>";
        HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    }

    return 0;
}

/* generates current image information */

static int ssi_ota_image_info(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    status_t status;
    uint32_t imgstate;
    char buf[128];
    char *html;

    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++)
    {
        status = bl_get_image_state(image, &imgstate);
        if (status != kStatus_Success)
        {
            PRINTF("Failed to get state of image %u (ret %d)", image, status);
            return 0;
        }

        for (int slot = 0; slot < 2; slot++)
        {
            int faid                = image * 2 + slot;
            struct flash_area *fa   = &boot_flash_map[faid];
            uint32_t slotaddr       = fa->fa_off + BOOT_FLASH_BASE;
            struct image_header *ih = (void *)slotaddr;
            char versionstr[40];

            int slotused = ih->ih_magic == IMAGE_MAGIC;

            if (slotused)
            {
                struct image_version *iv = &ih->ih_ver;
                snprintf(versionstr, sizeof(versionstr), "%u.%u.%u-%lu", iv->iv_major, iv->iv_minor, iv->iv_revision,
                         iv->iv_build_num);
            }

            html = "<tr>";
            HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));

            /* slot; name; version; size */

            snprintf(buf, sizeof(buf), "<td>%d</td><td>%s</td><td>%s</td><td>%lu</td>", faid, fa->fa_name,
                     slotused ? versionstr : "-", slotused ? ih->ih_img_size : 0);
            HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));

            /* Primary slots only */

            if (faid % 2 == 0)
            {
                /* swap type */

                snprintf(buf, sizeof(buf), "<td rowspan='2'>%s</td>", bl_imgstate_to_str(imgstate));
                HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));

                html = "<td rowspan=2>";
                HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));

                /* activate accept button when imgstate is Testing */
                snprintf(buf, sizeof(buf), "<button type='submit' name='image_accept' value='%u' %s>accept</button>",
                         image, imgstate != kSwapType_Testing ? "disabled" : "");
                HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));

                html = "</td>";
                HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
            }

            html = "</tr>";
            HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
        }
    }

    return 0;
}

static bool cgi_get_varval(char *src, char *var_name, char *dst, uint32_t length)
{
    char *name;
    bool result;
    uint32_t index;
    uint32_t n_length;

    result = false;
    dst[0] = 0;
    name   = src;

    n_length = strlen(var_name);

    while ((name != NULL) && ((name = strstr(name, var_name)) != NULL))
    {
        if (name[n_length] == '=')
        {
            name += n_length + 1;

            index = strcspn(name, "&");
            if (index >= length)
            {
                index = length - 1;
            }
            strncpy(dst, name, index);
            dst[index] = '\0';
            result     = true;
            break;
        }
        else
        {
            name = strchr(name, '&');
        }
    }

    return (result);
}

/* receive update image and store it in the flash partition */
int32_t store_update_image(struct multipart_read_ctx *ctx, uint32_t partition_phys_addr, uint32_t partition_size)
{
    int32_t retval = 0;

    uint32_t chunk_flash_addr;
    int32_t chunk_len;

    uint32_t next_erase_addr;

    /* Page buffers */
    uint32_t page_size;
    uint32_t *page_buffer;

    /* Received/processed data counter */
    uint32_t total_processed = 0;

    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;

    /* Check partition alignment */
    if (!mflash_drv_is_sector_aligned(partition_phys_addr) || !mflash_drv_is_sector_aligned(partition_size))
    {
        PRINTF("%s: partition not aligned\n", __func__);
        return -1;
    }

    /* Allocate page buffer(s) is one go */
    page_size   = MFLASH_PAGE_SIZE;
    page_buffer = httpsrv_mem_alloc(page_size);
    if (page_buffer == NULL)
    {
        PRINTF("%s: page buffer allocation error\n", __func__);
        return -1;
    }

    /* Pre-set address of area not erased so far */
    next_erase_addr  = partition_phys_addr;
    chunk_flash_addr = partition_phys_addr;
    total_processed  = 0;

    do
    {
        /* The data is epxected for be received by page sized chunks (except for the last one) */
        chunk_len = multipart_read_data(ctx, (uint8_t *)page_buffer, page_size);

        if (chunk_len > 0)
        {
            if (chunk_flash_addr >= partition_phys_addr + partition_size)
            {
                /* Partition boundary exceeded */
                PRINTF("\n%s: partition boundary exceedded\n", __func__);
                retval = -1;
                break;
            }

            /* Perform erase when encountering next sector */
            if (chunk_flash_addr >= next_erase_addr)
            {
                mflash_result = mflash_drv_sector_erase(next_erase_addr);
                if (mflash_result != 0)
                {
                    break;
                }
                next_erase_addr += MFLASH_SECTOR_SIZE;
            }

            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (chunk_len < page_size)
            {
                memset((uint8_t *)page_buffer + chunk_len, 0xff, page_size - chunk_len);
            }

            /* Program the page */
            mflash_result = mflash_drv_page_program(chunk_flash_addr, page_buffer);
            if (mflash_result != 0)
            {
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;

            PRINTF("\r%s: processed %i bytes", __func__, total_processed);
        }

    } while (chunk_len == page_size);

    /* If there was error reading multipart content, report failure */
    if ((chunk_len < 0))
    {
        PRINTF("\n%s: error reading data\n", __func__);
        retval = -1;
    }

    /* Check result of the last flash operation */
    if (mflash_result != 0)
    {
        /* Error during flash operation */
        PRINTF("\n%s: FLASH ERROR AT offset 0x%x\n", __func__, chunk_flash_addr);
        retval = -1;
    }

    /* Unless retval is already set (to an error code) */
    if (retval == 0)
    {
        PRINTF("\n%s: upload complete (%u bytes)\n", __func__, total_processed);
        retval = total_processed;
    }

    /* Clean up */
    httpsrv_mem_free(page_buffer);

    return retval;
}

/* Common Gateway Interface callback for OTA update. */
static int cgi_ota_upload(HTTPSRV_CGI_REQ_STRUCT *param)
{
    struct multipart_read_ctx *mpr_ctx;
    int images_processed = 0;

    HTTPSRV_CGI_RES_STRUCT response = {0};
    response.ses_handle             = param->ses_handle;
    response.status_code            = HTTPSRV_CODE_OK;

    const char *prefix = "upload_image_";

    if (param->request_method == HTTPSRV_REQ_POST)
    {
        mpr_ctx = (struct multipart_read_ctx *)httpsrv_mem_alloc(sizeof(struct multipart_read_ctx));
        if (mpr_ctx == NULL)
        {
            response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
            return (response.content_length);
        }

        multipart_read_init(mpr_ctx, param->ses_handle);

        while ((mpr_ctx->state == MULTIPART_EXPECT_HEADERS) && (response.status_code == HTTPSRV_CODE_OK))
        {
            int headers;
            int prefix_match, filename_ok;

            headers = multipart_read_headers(mpr_ctx);
            if (headers <= 0)
            {
                response.status_code = HTTPSRV_CODE_BAD_REQ;
                break;
            }

            prefix_match = !strncmp(mpr_ctx->form_data_name, prefix, strlen(prefix));

            /* Continue to data download only when filename is not empty or when the 'filename' attribute
             * is not present at all (to remain compatible with previous implementation)
             */
            filename_ok = !mpr_ctx->form_data_filename_present || strlen(mpr_ctx->form_data_filename);

            if (prefix_match && filename_ok)
            {
                int image = atoi(&mpr_ctx->form_data_name[strlen(prefix)]);
                uint32_t dst_addr, dst_size;
                int32_t stored;
                struct flash_area *fa;
                status_t status;

                images_processed++;

                if (image < 0 || image >= MCUBOOT_IMAGE_NUMBER)
                {
                    PRINTF("Unexpected image number %d\n", image);
                    response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
                    continue;
                }
                /* Secondary image slot parameters */

                fa       = &boot_flash_map[FLASH_AREA_IMAGE_SECONDARY(image)];
                dst_addr = fa->fa_off;
                dst_size = fa->fa_size;

                stored = store_update_image(mpr_ctx, dst_addr, dst_size);
                if (stored < 0)
                {
                    /* Error during upload */
                    PRINTF("Error during upload\n");
                    response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
                    continue;
                }

                if (bl_verify_image(dst_addr, stored) <= 0)
                {
                    /* Image validation failed */
                    PRINTF("Image validation failed\n");
                    response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
                    continue;
                }

                /* Mark image swap type as Testing */
                status = bl_update_image_state(image, kSwapType_ReadyForTest);
                if (status != kStatus_Success)
                {
                    PRINTF("FAILED to mark image as ReadyForTest (ret=%d)\n", status);
                    response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
                    continue;
                }
            }
            else
            {
                PRINTF("SKIPPING form data of '%s'\n", mpr_ctx->form_data_name);
                /* Discard unknown multipart data block */
                multipart_read_data(mpr_ctx, NULL, -1);
            }
        }

        httpsrv_mem_free(mpr_ctx);

        /* Write the response using chunked transmission coding. */
        response.content_type = HTTPSRV_CONTENT_TYPE_HTML;
        /* Set content length to -1 to indicate unknown content length. */
        response.content_length = -1;
        response.data           = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">";
        response.data_length    = strlen(response.data);
        HTTPSRV_cgi_write(&response);

        if (images_processed > 0)
        {
            if (response.status_code == HTTPSRV_CODE_OK)
            {
                response.data        = "<html><head><title>File upload successfull!</title>";
                response.data_length = strlen(response.data);

                ota_status = OTA_STATUS_UPLOAD_OK;
            }
            else
            {
                response.data        = "<html><head><title>File upload failed!</title>";
                response.data_length = strlen(response.data);

                ota_status = OTA_STATUS_UPLOAD_FAILED;
            }

            HTTPSRV_cgi_write(&response);
        }

        response.data        = "<meta http-equiv=\"refresh\" content=\"0; url=ota.shtml\"></head><body></body></html>";
        response.data_length = strlen(response.data);
        HTTPSRV_cgi_write(&response);
        response.data_length = 0;
        HTTPSRV_cgi_write(&response);
    }

    return (response.content_length);
}

void reboot_timer_callback(TimerHandle_t timer)
{
    PRINTF("System reset...\n");
    NVIC_SystemReset();
}

/* Common Gateway Interface callback for device reboot. */
static int cgi_ota_reboot(HTTPSRV_CGI_REQ_STRUCT *param)
{
    /* Static variable is used to avoid repetitive creation of the timer as that it expected to happen just once (reboot
     * follows) */
    static TimerHandle_t reboot_timer = NULL;

    HTTPSRV_CGI_RES_STRUCT response = {0};
    response.ses_handle             = param->ses_handle;
    response.status_code            = HTTPSRV_CODE_OK;

    /* Write the response using chunked transmission coding. */
    response.content_type = HTTPSRV_CONTENT_TYPE_HTML;
    /* Set content length to -1 to indicate unknown content length. */
    response.content_length = -1;
    response.data           = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">";
    response.data_length    = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data        = "<html><head><title>Going down for reboot</title>";
    response.data_length = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data = "<meta http-equiv=\"refresh\" content=\"0; url=ota_reboot.html\"></head><body></body></html>";
    response.data_length = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data_length = 0;
    HTTPSRV_cgi_write(&response);

    if (reboot_timer == NULL)
    {
        /* Actual reboot is delayed to give the HTTP server a chance to send the content generated by CGI and gracefully
         * close the connection */
        reboot_timer = xTimerCreate("reboot_timer", pdMS_TO_TICKS(3000), pdFALSE, NULL, reboot_timer_callback);
        xTimerStart(reboot_timer, 0);
    }

    return (response.content_length);
}

/* Common Gateway Interface callback for accepting the update. */
static int cgi_ota_accept(HTTPSRV_CGI_REQ_STRUCT *param)
{
    int ret;
    int image;
    char buffer[128];
    status_t status;

    PRINTF("Accept query: %s\n", param->query_string);

    ret = cgi_get_varval(param->query_string, "image_accept", buffer, sizeof(buffer));
    if (!ret)
    {
        PRINTF("Failed to parse 'image_accept' attribute in GET request string\n");
        return 0;
    }

    image = atoi(buffer);
    if (image < 0 || image > MCUBOOT_IMAGE_NUMBER)
    {
        PRINTF("Unexpected image number %d\n", image);
        return 0;
    }

    /* Set image as confirmed */

    status = bl_update_image_state(image, kSwapType_Permanent);
    if (status != kStatus_Success)
    {
        PRINTF("FAILED to accept image (ret=%d)\n", status);
        return 0;
    }

    PRINTF("boot_set_confirmed_multi(%d): %d\n", image, ret);
    ota_status = OTA_STATUS_NONE;

    HTTPSRV_CGI_RES_STRUCT response = {0};
    response.ses_handle             = param->ses_handle;
    response.status_code            = HTTPSRV_CODE_OK;

    /* Write the response using chunked transmission coding. */
    response.content_type = HTTPSRV_CONTENT_TYPE_HTML;
    /* Set content length to -1 to indicate unknown content length. */
    response.content_length = -1;
    response.data           = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">";
    response.data_length    = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data        = "<html><head><title>Accept update</title>";
    response.data_length = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data        = "<meta http-equiv=\"refresh\" content=\"0; url=ota.shtml\"></head><body></body></html>";
    response.data_length = strlen(response.data);
    HTTPSRV_cgi_write(&response);
    response.data_length = 0;
    HTTPSRV_cgi_write(&response);

    return (response.content_length);
}

#if HTTPSRV_CFG_MBEDTLS_ENABLE
static HTTPSRV_TLS_PARAM_STRUCT tls_params;
#endif

static int32_t mcuboot_image_size_sanity_check(const uint8_t *data, uint32_t size)
{
    struct image_header *ih;
    struct image_tlv_info *it;
    uint32_t decl_size;
    uint32_t tlv_size;

    ih = (struct image_header *)data;

    /* do we have at least the header */
    if (size < sizeof(struct image_header))
    {
        return 0;
    }

    /* check magic number */
    if (ih->ih_magic != IMAGE_MAGIC)
    {
        return 0;
    }

    /* check that we have at least the amount of data declared by the header */
    decl_size = ih->ih_img_size + ih->ih_hdr_size + ih->ih_protect_tlv_size;
    if (size < decl_size)
    {
        return 0;
    }

    /* check protected TLVs if any */
    if (ih->ih_protect_tlv_size > 0)
    {
        if (ih->ih_protect_tlv_size < sizeof(struct image_tlv_info))
        {
            return 0;
        }
        it = (struct image_tlv_info *)(data + ih->ih_img_size + ih->ih_hdr_size);
        if ((it->it_magic != IMAGE_TLV_PROT_INFO_MAGIC) || (it->it_tlv_tot != ih->ih_protect_tlv_size))
        {
            return 0;
        }
    }

    /* check for optional TLVs following the image as declared by the header */
    tlv_size = size - decl_size;
    if (tlv_size > 0)
    {
        if (tlv_size < sizeof(struct image_tlv_info))
        {
            return 0;
        }
        it = (struct image_tlv_info *)(data + decl_size);
        if ((it->it_magic != IMAGE_TLV_INFO_MAGIC) || (it->it_tlv_tot != tlv_size))
        {
            return 0;
        }
    }

    return 1;
}

/*!
 * @brief Initializes server.
 */
static void http_server_socket_init(void)
{
    HTTPSRV_PARAM_STRUCT params;
    uint32_t httpsrv_handle;

    /* Init Fs*/
    HTTPSRV_FS_init(httpsrv_fs_data);

    /* Init HTTPSRV parameters.*/
    memset(&params, 0, sizeof(params));
    params.root_dir   = "";
    params.index_page = "/index.html";
    /* params.auth_table  = auth_realms; */
    params.cgi_lnk_tbl = cgi_lnk_tbl;
    params.ssi_lnk_tbl = ssi_lnk_tbl;
#if HTTPSRV_CFG_MBEDTLS_ENABLE
    tls_params.certificate_buffer      = (const unsigned char *)mbedtls_test_srv_crt;
    tls_params.certificate_buffer_size = mbedtls_test_srv_crt_len;
    tls_params.private_key_buffer      = (const unsigned char *)mbedtls_test_srv_key;
    tls_params.private_key_buffer_size = mbedtls_test_srv_key_len;

    params.tls_param = &tls_params;
#endif
    /* Init HTTP Server.*/
    httpsrv_handle = HTTPSRV_init(&params);
    if (httpsrv_handle == 0)
    {
        LWIP_PLATFORM_DIAG(("HTTPSRV_init() is Failed"));
    }
}

/*!
 * @brief Initializes lwIP stack.
 */
int initNetwork(void);

/*!
 * @brief The main function containing server thread.
 */
static void main_thread(void *arg)
{
    LWIP_UNUSED_ARG(arg);

    initNetwork();

    /* determine if there is any image in TEST state */
    for (int image = 0; image < MCUBOOT_IMAGE_NUMBER; image++)
    {
        status_t status;
        uint32_t imgstate;

        status = bl_get_image_state(image, &imgstate);
        if (status != kStatus_Success)
        {
            PRINTF("Failed to get state of image %u (ret %d)", image, status);
        }

        if (imgstate == kSwapType_Testing)
        {
            ota_status = OTA_STATUS_TESTING;
            break;
        }
    }

    http_server_socket_init();

    vTaskDelete(NULL);
}

/*!
 * @brief Main function.
 */

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    APP_InitAppDebugConsole();

    /* Define the init structure for the OSPI reset pin*/
    gpio_pin_config_t reset_config = {
        kGPIO_DigitalOutput,
        1,
    };

    /* Init output OSPI reset pin. */
    GPIO_PortInit(GPIO, BOARD_FLASH_RESET_GPIO_PORT);
    GPIO_PinInit(GPIO, BOARD_FLASH_RESET_GPIO_PORT, BOARD_FLASH_RESET_GPIO_PIN, &reset_config);

    /* Make sure casper ram buffer has power up */
    POWER_DisablePD(kPDRUNCFG_PPD_CASPER_SRAM);
    POWER_ApplyPD();
    CRYPTO_InitHardware();

    mflash_drv_init();

    /* create server thread in RTOS */
    if (sys_thread_new("main", main_thread, NULL, HTTPD_STACKSIZE, HTTPD_PRIORITY) == NULL)
        LWIP_ASSERT("main(): Task creation failed.", 0);

    /* run RTOS */
    vTaskStartScheduler();

    /* should not reach this statement */
    for (;;)
        ;
}
