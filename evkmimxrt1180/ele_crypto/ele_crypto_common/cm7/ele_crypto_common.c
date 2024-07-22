/*
 * Copyright 2024 NXP
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

#include "ele_crypto.h" /* ELE Crypto SW */
#include "ele_fw.h"     /* ELE FW, to be placed in bootable container in real world app */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define S3MU MU_APPS_S3MUA

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
    /*
     * This code example demonstrates EdgeLock usage in following steps:
     * 1.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 2.  Compute hash one go and verify result with expected value
     * 3.  Compute hash streaming and verify result with expected value
     * 4.  Start the RNG
     * 5.  Get RNG random data
     */

    status_t result = kStatus_Fail;

    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave Sub-System crypto example:\r\n\r\n");

        /****************** Load EdgeLock FW message ***************************/
        PRINTF("****************** Load EdgeLock FW ***********************\r\n");
        if (ELE_LoadFw(S3MU, ele_fw) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock FW loaded and authenticated successfully.\r\n\r\n");
        }

        /****************** Hash SHA256  ***********************/
        PRINTF("****************** Compute Hash (SHA256) of massage *******\r\n");
        /* Message to be hashed */
        SDK_ALIGN(uint8_t message[], 8u) =
            "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
            "Get thee back into the tempest and the Nights Plutonian shore!"
            "Leave no black plume as a token of that lie thy soul hath spoken!"
            "Leave my loneliness unbroken! quit the bust above my door!"
            "Take thy beak from out my heart, and take thy form from off my door!"
            "Quoth the raven, Nevermore.  ";
        /* Output for HASH */
        SDK_ALIGN(static uint8_t output[32], 8u);
        /* Expected SHA-256 for the message. */
        static const uint8_t sha256[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                         0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                         0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};
        uint32_t length               = sizeof(message) - 1; /* no null string terminating */
        uint32_t outLength            = 0;

        /* Compute HASH ONE Go */
        if (ELE_Hash(S3MU, (const uint8_t *)message, length, output, sizeof(output), &outLength, kELE_Sha256) ==
            kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, sha256, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HASH matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* Compute HASH Init */
        ele_hash_ctx_t hashCtx;

        if (ELE_Hash_Init(S3MU, &hashCtx, kELE_Sha256) == kStatus_Success)
        {
            PRINTF("*SUCCESS* HASH Init done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        if (ELE_Hash_Update(S3MU, &hashCtx, kELE_Sha256, (const uint8_t *)message, length) == kStatus_Success)
        {
            PRINTF("*SUCCESS* HASH Update done.\r\n\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        memset(output, 0u, sizeof(output));

        if (ELE_Hash_Finish(S3MU, &hashCtx, kELE_Sha256, output, sizeof(output), &outLength, NULL, 0u) ==
            kStatus_Success)
        {
            /* Check output Hash digest data */
            if (memcmp(output, sha256, outLength) == 0)
            {
                PRINTF("*SUCCESS* Computed HASH (Finish) matches the expected value.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** Start RNG ***********************/
        PRINTF("****************** Start RNG ******************************\r\n");
        if (ELE_StartRng(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock RNG Start success.\r\n\r\n");
        }

        uint32_t trng_state = 0u;
        do
        {
            result = ELE_GetTrngState(S3MU, &trng_state);
        } while (((trng_state & 0xFFu) != kELE_TRNG_ready) &&
                 ((trng_state & 0xFF00u) != kELE_TRNG_CSAL_success << 8u ) &&
                   result == kStatus_Success);

        PRINTF("EdgeLock RNG ready to use.\r\n\r\n");

        /****************** Get RNG Random ***********************/
        PRINTF("****************** Get RNG Random **********************\r\n");
        uint32_t random[8] = {0u};

        if (ELE_RngGetRandom(S3MU, random, sizeof(random), kNoReseed) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get RNG random data successfully. First word: 0x%x\r\n\r\n", *random);
        }

        /****************** END of Example *************************************/
        result = kStatus_Success;

    } while (0);

    if (result == kStatus_Success)
    {
        PRINTF("End of Example with SUCCESS!!\r\n\r\n");
    }
    else
    {
        PRINTF("ERROR: execution of commands on Security Sub-System failed!\r\n\r\n");
    }
    while (1)
    {
    }
}
