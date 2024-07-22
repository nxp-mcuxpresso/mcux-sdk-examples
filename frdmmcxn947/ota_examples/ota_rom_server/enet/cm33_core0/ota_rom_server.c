/*
 * Copyright 2024 NXP
 * All rights reserved.
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
#include "board.h"

#include "httpsrv.h"
#include "httpsrv_port.h"

#include "mflash_drv.h"
#include "timers.h"
#include "fsl_debug_console.h"

#include "rom_iap.h"


#include "fsl_enet.h"
#include "fsl_phylan8741.h"
#define EXAMPLE_ENET_BASE ENET0
/*******************************************************************************
 * Definitions
 ******************************************************************************/


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

static int ssi_ota_status(HTTPSRV_SSI_PARAM_STRUCT *param);
static int ssi_ota_image_upload(HTTPSRV_SSI_PARAM_STRUCT *param);
static int ssi_ota_image_info(HTTPSRV_SSI_PARAM_STRUCT *param);


/*******************************************************************************
 * Local Types
 ******************************************************************************/



/*******************************************************************************
 * Variables
 ******************************************************************************/
phy_lan8741_resource_t g_phy_resource;

extern const HTTPSRV_FS_DIR_ENTRY httpsrv_fs_data[];

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"upload", cgi_ota_upload},
    {"reboot", cgi_ota_reboot},
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

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET_BASE, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET_BASE, phyAddr, regAddr, pData);
}


enum ota_status_t
{
    OTA_STATUS_NONE = 0,
    OTA_STATUS_UPLOAD_FAILED,
    OTA_STATUS_UPLOAD_OK,
    OTA_STATUS_SETUP_ERROR,
} ota_status = OTA_STATUS_NONE;


char *ota_status_strings[] = {
    "Select firmware files to be updated and click <b>Upload</b>.",
    "<b>Failed</b> to upload image, see console log for details.",
    "Update images successfully uploaded.",
    "<b>Error</b> - device misconfiguration, see console log for details.",
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
 
    html = "<tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));

    snprintf(buf, sizeof(buf),
             "<td><b>Image %s</b></td>"
             "<td><input type='file' id='upload_image_%u' name='upload_image_%u'></td>",
             "SB3", 0, 0);
    HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));

    html = "</tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));

    return 0;
}


/* generates current image information */

static int ssi_ota_image_info(HTTPSRV_SSI_PARAM_STRUCT *param)
{
    char buf[128];
    char *html;
    const char* invalid_header_msg = "<td colspan=100>Invalid image header</td>";
    
    struct mbi_image_info imginfo;
    
    uint32_t slot0_log_addr;
    uint32_t slot1_log_addr;
    
    
    if (is_remap_active())
    {
        slot0_log_addr = 0x100000;
        slot1_log_addr = 0;
    }
    else
    {
        slot0_log_addr = 0;
        slot1_log_addr = 0x100000;
    }

    /* Slot 0 info */
    
    html = "<table border='1'><tr><th>Bank</th><th>Active</th><th>ImgVersion</th><th>FwVersion</th><th>Size</th><th>Type</th><th>ExecAddr</th></tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    html = "<tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    if (is_sb3_header((void *) slot0_log_addr))
    {
        parse_mbi_image_info((uint32_t *) slot0_log_addr, &imginfo);            
    
        snprintf(buf, sizeof(buf), "<td>%d</td><td>%s</td><td>%u</td><td>%u</td><td>%lu</td><td>%u</td><td>0x%x</td>",
                 0, is_remap_active()?" ":"X",
                 imginfo.img_version, imginfo.fw_version, imginfo.length, imginfo.type, imginfo.execaddr);
    }
    else
    {
        strcpy(buf, invalid_header_msg);
    }
    
    HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));
    
    html = "</tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    /* Slot 1 info */
       
    html = "<tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    if (is_sb3_header((void *) slot1_log_addr))
    {    
        parse_mbi_image_info((uint32_t *) slot1_log_addr, &imginfo);
        
        snprintf(buf, sizeof(buf), "<td>%d</td><td>%s</td><td>%u</td><td>%u</td><td>%lu</td><td>%u</td><td>0x%x</td>",
                 1, is_remap_active()?"X":" ",
                 imginfo.img_version, imginfo.fw_version, imginfo.length, imginfo.type, imginfo.execaddr);
    }
    else
    {
        strcpy(buf, invalid_header_msg);
    }
    
    HTTPSRV_ssi_write(param->ses_handle, buf, strlen(buf));
    
    html = "</tr>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    html = "</table>";
    HTTPSRV_ssi_write(param->ses_handle, html, strlen(html));
    
    return 0;
}


/* function that keeps receiveing uploaded payload and hands it over to ROM */

int32_t process_sb3_upload(struct multipart_read_ctx *ctx)
{
    int32_t ret = 0;
    int32_t chunk_len;
    
    sb3_iap_ctx_t iap_ctx;

    /* Page buffers */
    size_t prog_size;
    uint8_t *prog_buf;

    /* Received/processed data counter */
    uint32_t total_processed = 0;

    /* buffer for sb processing */
    prog_size = 0x1000;
    prog_buf  = httpsrv_mem_alloc(prog_size);
    if (prog_buf == NULL)
    {
        PRINTF("%s: prog buffer allocation error\n", __func__);
        return -1;
    }
    
    ret = sb3_iap_init(&iap_ctx);
    if (ret != kStatus_Success)
    {
        PRINTF("%s: sb3_iap_init() failed\n", __func__);
        ret = -1;
        goto cleanup;
    }
    
    do
    {
        /* The data is received in requested chunk size, except for the last one */
        chunk_len = multipart_read_data(ctx, (uint8_t *)prog_buf, prog_size);
        
        /* end of stream */
        if (chunk_len < 1) break;
        
        if (total_processed == 0)
        {
            /* notify if this isn't sb3 stream */
            if (!is_sb3_header(prog_buf))
            {
                PRINTF("WARNING: Uploaded stream doesn't start with SB3 signature!\n");
            }
        }
        
        /* Hand over for sb3 processing */
        ret = sb3_iap_pump(&iap_ctx, prog_buf, chunk_len);
        if (ret != kStatus_Success && ret != kStatusRomLdrDataUnderrun)
        {
            PRINTF("%s: sb3_iap_pump() failed with %d\n", __func__, ret);
            ret = -1;
            goto cleanup;
        }

        total_processed += chunk_len;

        PRINTF("%s: processed %i bytes\n", __func__, total_processed);
        
    } while (chunk_len == prog_size);

    /* If there was error reading multipart content, report failure */
    if ((chunk_len < 0))
    {
        PRINTF("\n%s: error reading data\n", __func__);
        ret = -1;
    }

    if (ret == 0)
    {
        sb3_iap_finalize(&iap_ctx);
        ret = total_processed;
        PRINTF("\n%s: upload complete (%u bytes)\n", __func__, total_processed);
    }
    
cleanup:
    
    httpsrv_mem_free(prog_buf);
    sb3_iap_free(&iap_ctx);

    return ret;
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
             * is not present at all
             */
            filename_ok = !mpr_ctx->form_data_filename_present || strlen(mpr_ctx->form_data_filename);

            if (prefix_match && filename_ok)
            {
                int image = atoi(&mpr_ctx->form_data_name[strlen(prefix)]);
                int32_t stored;

                images_processed++;                
     
                stored = process_sb3_upload(mpr_ctx);
                if (stored < 0)
                {
                    /* Error during upload */
                    PRINTF("Error during upload\n");
                    response.status_code = HTTPSRV_CODE_INTERNAL_ERROR;
                    continue;
                }
                
                PRINTF("Image upload completed\n");
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


#if HTTPSRV_CFG_MBEDTLS_ENABLE
static HTTPSRV_TLS_PARAM_STRUCT tls_params;
#endif


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
        LWIP_PLATFORM_DIAG(("HTTPSRV_init() failed"));
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
    int cmpa_ok;
    int remap_size;
    
    LWIP_UNUSED_ARG(arg);

    initNetwork();
    
    
    /* Test device is setup - CMPA header is present and flash remapping enabled */
    
    cmpa_ok = *((uint16_t*)0x1004002) == 0x5963;
    remap_size = ((*((uint8_t *)0x1004004) & 0x1F)+1)*32;
    
    PRINTF("CMPA header %s; REMAP size %u kB\n", cmpa_ok ? "OK" : "MISMATCH!", remap_size);
    
    if (!cmpa_ok || remap_size == 0)
    {
        PRINTF("WARNING! Device doesn't seem to be configured properly! Check instructions in readme file.\n");
        ota_status = OTA_STATUS_SETUP_ERROR;
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
    CLOCK_EnableClock(kCLOCK_InputMux);
    /* Attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_EnableClock(kCLOCK_LPUart4);

    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    CLOCK_EnableClock(kCLOCK_Trng);

    /* Select RMII type PHY */
    SYSCON0->ENET_PHY_INTF_SEL = SYSCON_ENET_PHY_INTF_SEL_PHY_SEL(1);
#if 0
    /* ENET is clocked from PLL0 (150 MHz) */
    CLOCK_AttachClk(kPLL0_to_ENETRMII);
    CLOCK_SetClkDiv(kCLOCK_DivEnetrmiiClk, 3);
#else
    /* ENET is clocked from TXCLK PIN
        R274 0-Ohm (originally DNP) needs to be added to the board
    */
    CLOCK_AttachClk(kNONE_to_ENETRMII);        /* No internal clock */
    CLOCK_SetClkDiv(kCLOCK_DivEnetrmiiClk, 0); /* Halt */
#endif
    //    CLOCK_AttachClk(kPLL0_to_ENETPTPREF);
    /* Enable ENET bus clocks */
    CLOCK_EnableClock(kCLOCK_Enet);
    /* Set MII speed - must be done after ENET is clocked */
    ENET_SetSMI(EXAMPLE_ENET_BASE, CLOCK_GetPll0OutFreq());
    /* Reset ENET */
    SYSCON0->PRESETCTRL2 = SYSCON_PRESETCTRL2_ENET_RST_MASK;
    SYSCON0->PRESETCTRL2 &= ~SYSCON_PRESETCTRL2_ENET_RST_MASK;

    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
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
