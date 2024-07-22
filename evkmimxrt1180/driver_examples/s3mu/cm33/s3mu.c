/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_s3mu.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define S3MU MU_RT_S3MUA
#define APP_MAX_TR_COUNT (0x1u)
#define APP_MAX_RR_COUNT (0x2u)

#define S3MU_PING_ELE              (0x17010106u)
#define S3MU_PING_SIZE             (0x1u)
#define S3MU_PING_RESPONSE_HDR     (0xe1010206u)
#define S3MU_PING_RESPONSE_SUCCESS (0xd6u)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    status_t result = kStatus_Fail;
    /* Allocate message buffer with maximal message unit trasfer count */
    uint32_t tmsg[APP_MAX_TR_COUNT];
    /* Allocate message buffer with maximal message unit receive count */
    uint32_t rmsg[APP_MAX_RR_COUNT];

    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
        PRINTF("S3MU Driver Example:\r\n\r\n");

        /****************** Ping EdgeLock **************************************/
        PRINTF("****************** Ping EdgeLock **************************\r\n");
        tmsg[0] = S3MU_PING_ELE; /* S3MU_PING_ELE Command Header */
        /* Send message to ELE via MU driver */
        if (S3MU_SendMessage(S3MU, tmsg, S3MU_PING_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (S3MU_GetResponse(S3MU, rmsg) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == S3MU_PING_RESPONSE_HDR && rmsg[1] == S3MU_PING_RESPONSE_SUCCESS)
        {
            PRINTF("S3MU Driver Ping command sucessfully.\r\n\r\n");
        }
        else
        {
            PRINTF("Ping FAIL!!!!\r\n\r\n");
            result = kStatus_Fail;
            break;
        }

        result = kStatus_Success;
    } while (0);

    if (result == kStatus_Success)
    {
        PRINTF("End of Example with SUCCESS!!\r\n\r\n");
    }
    else
    {
        PRINTF("ERROR: execution of commands failed!\r\n\r\n");
    }
    while (1)
    {
    }
}
