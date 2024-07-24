/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_sss_mgmt.h"
#include "fsl_sss_sscp.h"
#include "fsl_sscp_mu.h"

#include "ele_200_fw.h" /* ELE FW */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t get_ele_fw_version(ELEMU_Type *mu, uint8_t *ele_fw_version)
{
    sscp_context_t sscpContext    = {0u};
    sss_mgmt_t mgmtContext        = {0u};
    sss_sscp_session_t sssSession = {0u};
    status_t status               = kStatus_Fail;

    size_t datalen = 8u;

    /* PropertyId of Edgelock Firmware version */
    uint32_t propertyId = 0x51u;

    do
    {
        if (sscp_mu_init(&sscpContext, mu) != kStatus_SSCP_Success)
        {
            break;
        }
        /* OPEN SESSION */
        if (sss_sscp_open_session(&sssSession, 0u, kType_SSS_Ele200, &sscpContext) != kStatus_SSS_Success)
        {
            break;
        }

        if (sss_mgmt_context_init(&mgmtContext, &sssSession) != kStatus_SSS_Success)
        {
            break;
        }

        /* READ FUSE */
        if (sss_mgmt_get_property(&mgmtContext, propertyId, ele_fw_version, &datalen) != kStatus_SSS_Success)
        {
            break;
        }

        /* If all steps before passes without break, then consider it as success*/
        status = kStatus_Success;

    } while (false);

    /* FREE TUNNEL CONTEXT */
    sss_mgmt_context_free(&mgmtContext);
    /* CLOSE SESSION */
    sss_sscp_close_session(&sssSession);

    return status;
}

/*!
 * @brief Main function
 */
int main(void)
{
    char ch;
    status_t status = kStatus_Fail;
    uint32_t ele_version[2];

    /* ELE will respond with 0xFFFFFFFFFFFFFFFF if no FW is loaded*/
    static const uint32_t no_fw_loaded[2] = {0xFFFFFFFF, 0xFFFFFFFF};

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("ELE Common example\r\n");

    PRINTF("Get ELE FW version\r\n");

    status = get_ele_fw_version(ELEMUA, (uint8_t *)ele_version);
    if (status != kStatus_Success)
    {
        PRINTF("ERROR: Reading ELE FW version failed!\r\n");
    }

    PRINTF("FW version[0]: %X, version[1]: %X\r\n", ele_version[0], ele_version[1]);

    /* Check if FW is loaded*/
    if (memcmp(no_fw_loaded, ele_version, sizeof(ele_version)))
    {
        PRINTF("FW loaded in ELE\r\n");
    }
    else
    {
        PRINTF("FW not loaded in ELE\r\n");
    }

    PRINTF("Load ELE FW\r\n");
    status = ELEMU_loadFw(ELEMUA, (uint32_t *)fw);
    if (status != kStatus_Success)
    {
        PRINTF("ERROR: EdgeLock FW loading failed!\r\n");
    }

    status = get_ele_fw_version(ELEMUA, (uint8_t *)ele_version);
    PRINTF("FW version[0]: %X, version[1]: %X\r\n", ele_version[0], ele_version[1]);

    /* Check if FW is loaded*/
    if (memcmp(no_fw_loaded, ele_version, sizeof(ele_version)))
    {
        PRINTF("FW loaded in ELE\r\n");
    }
    else
    {
        PRINTF("FW not loaded in ELE\r\n");
    }

    PRINTF("Example end\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
