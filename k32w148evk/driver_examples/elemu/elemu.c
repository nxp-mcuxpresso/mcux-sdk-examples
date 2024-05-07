/*
 * Copyright 2021,2023 NXP
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

#include "fsl_elemu.h"

#if (defined(KW45_A0_SUPPORT) && KW45_A0_SUPPORT)
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define ELEMU_OPEN_SESSION                     (0x13040017u)
#define ELEMU_OPEN_SESSION_SIZE                (0x5u)
#define ELEMU_OPEN_SESSION_RESPONSE            (0x13013c3c)
#define ELEMU_KEY_STORE_CONTEXT_INIT           (0x49020017u)
#define ELEMU_KEY_STORE_CONTEXT_INIT_SIZE      (0x3u)
#define ELEMU_KEY_STORE_CONTEXT_INIT_RESPONSE  (0x49013c3cu)
#define ELEMU_OPEN_KEY_STORE_ALLOCATE          (0x48010017u)
#define ELEMU_OPEN_KEY_STORE_ALLOCATE_SIZE     (0x2u)
#define ELEMU_OPEN_KEY_STORE_ALLOCATE_RESPONSE (0x48013c3cu)
#define ELEMU_KEY_OBJECT_INIT                  (0x41010017u)
#define ELEMU_KEY_OBJECT_INIT_SIZE             (0x2u)
#define ELEMU_KEY_OBJECT_INIT_RESPONSE         (0x41013c3cu)
#define ELEMU_KEY_OBJECT_ALLOCATE              (0x42050017u)
#define ELEMU_KEY_OBJECT_ALLOCATE_SIZE         (0x6u)
#define ELEMU_KEY_OBJECT_ALLOCATE_RESPONSE     (0x42003c3cu)
#define ELEMU_KEY_STORE_SET_KEY                (0x4c050017u)
#define ELEMU_KEY_STORE_SET_KEY_SIZE           (0x6u)
#define ELEMU_KEY_STORE_SET_KEY_RESPONSE       (0x4c003c3cu)
#define ELEMU_SYMMETRIC_CONTEXT_INIT           (0x25040017u)
#define ELEMU_SYMMETRIC_CONTEXT_INIT_SIZE      (0x5u)
#define ELEMU_SYMMETRIC_CONTEXT_INIT_RESPONSE  (0x25013c3cu)
#define ELEMU_CIPHER_ONE_GO                    (0x23060017u)
#define ELEMU_CIPHER_ONE_GO_SIZE               (0x7u)
#define ELEMU_CIPHER_ONE_GO_RESPONSE           (0x23003c3cu)
#define ELEMU_CONTEXT_FREE                     (0x15010017u)
#define ELEMU_CONTEXT_FREE_SIZE                (0x2u)
#define ELEMU_CONTEXT_FREE_RESPONSE            (0x15003c3c)
#define ELEMU_KEY_OBJECT_FREE                  (0x47010017u)
#define ELEMU_KEY_OBJECT_FREE_SIZE             (0x2u)
#define ELEMU_KEY_OBJECT_FREE_RESPONSE         (0x47003c3cu)
#define ELEMU_CLOSE_SESSION                    (0x14010017u)
#define ELEMU_CLOSE_SESSION_SIZE               (0x2u)
#define ELEMU_CLOSE_SESSION_RESPONSE           (0x14003c3cu)
/*******************************************************************************
 * Prototypes
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
     * This code example demonstrates EdgeLock usage for AES CBC operation. The AES CBC operation is performed in
     * following steps:
     * 1. Open EdgeLock session
     * 2. Create and allocate key store
     * 3. Create and allocate key object
     * 4. Set the key
     * 5. Initialize AES CBC operation context
     * 6. Perform AES CBC operation
     * 7. Close all opened contexts and created objects
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */
    /* KEY = 1f8e4973953f3fb0bd6b16662e9a3c17 */
    uint8_t symKeyData[16] = {0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0,
                              0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17};
    /* IV = 2fe2b333ceda8f98f4a99b40d2cd34a8 */
    uint8_t ivData[16] = {0x2f, 0xe2, 0xb3, 0x33, 0xce, 0xda, 0x8f, 0x98,
                          0xf4, 0xa9, 0x9b, 0x40, 0xd2, 0xcd, 0x34, 0xa8};
    /* PLAINTEXT = 45cf12964fc824ab76616ae2f4bf0822 */
    uint8_t plainData[16] = {0x45, 0xcf, 0x12, 0x96, 0x4f, 0xc8, 0x24, 0xab,
                             0x76, 0x61, 0x6a, 0xe2, 0xf4, 0xbf, 0x08, 0x22};
    /* CIPHERTEXT = 0f61c4d44c5147c03c195ad7e2cc12b2 */
    uint8_t cipherDataRef[16] = {0x0f, 0x61, 0xc4, 0xd4, 0x4c, 0x51, 0x47, 0xc0,
                                 0x3c, 0x19, 0x5a, 0xd7, 0xe2, 0xcc, 0x12, 0xb2};

    uint8_t cipherData[16] = {0};
    status_t result        = kStatus_Fail;
    uint32_t sessionId;
    uint32_t keyStoreId;
    uint32_t keyObjectId;
    uint32_t aesOperationId;
    /* Allocate message buffer with maximal message unit trasfer register count */
    uint32_t tmsg[ELEMU_TR_COUNT];
    /* Allocate message buffer with maximal message unit receive register count */
    uint32_t rmsg[ELEMU_RR_COUNT];

    do
    {
        /* HW init */
    BOARD_InitPins();
    BOARD_InitDebugConsole();
        PRINTF("ELEMU Peripheral Driver Example\r\n");
        /* Lock MU ownership to current thread */
        if (ELEMU_mu_get_ownership(ELEMUA) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Open EdgeLock session */
        tmsg[0] = ELEMU_OPEN_SESSION; // OPEN_SESSION Command Header
        tmsg[1] = 0x2u;               // EdgeLock ID
        tmsg[2] = 0x0u;               // Reserved
        tmsg[3] = 0x0u;               // Reserved
        tmsg[4] = 0x0u;               // Reserved
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_OPEN_SESSION_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_OPEN_SESSION_RESPONSE)
        {
            /* read response data */
            sessionId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Initialize key store context */
        tmsg[0] = ELEMU_KEY_STORE_CONTEXT_INIT; // KEY_STORE_CONTEXT_INIT Command Header
        tmsg[1] = sessionId;
        tmsg[2] = 0x0u; // User ID=0
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_STORE_CONTEXT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_KEY_STORE_CONTEXT_INIT_RESPONSE)
        {
            /* read response data */
            keyStoreId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* Allocate key store */
        tmsg[0] = ELEMU_OPEN_KEY_STORE_ALLOCATE; // KEY_STORE_ALLOCATE Command Header
        tmsg[1] = keyStoreId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_OPEN_KEY_STORE_ALLOCATE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_OPEN_KEY_STORE_ALLOCATE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }

        /* Key object init */
        tmsg[0] = ELEMU_KEY_OBJECT_INIT; // KEY_OBJECT_INIT Command Header
        tmsg[1] = keyStoreId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_KEY_OBJECT_INIT_RESPONSE)
        {
            /* read response data */
            keyObjectId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Key object allocate slot in key store */
        tmsg[0] = ELEMU_KEY_OBJECT_ALLOCATE; // KEY_OBJECT_ALLOCATE Command Header
        tmsg[1] = keyObjectId;
        tmsg[2] = 0x0u; // key user ID=0
        tmsg[3] = 0x0u; // symmetric key will be used
        tmsg[4] = 0x0u; // reserved
        tmsg[5] = 0x0u; // cannot read key, can be set only or export
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_ALLOCATE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_OBJECT_ALLOCATE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Set key */
        tmsg[0] = ELEMU_KEY_STORE_SET_KEY; // KEY_STORE_SET_KEY Command Header
        tmsg[1] = keyStoreId;
        tmsg[2] = keyObjectId;
        tmsg[3] = (uint32_t)symKeyData;
        tmsg[4] = 128u; // key length in bits
        tmsg[5] = 0x0u; // data is plain key
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_STORE_SET_KEY_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_STORE_SET_KEY_RESPONSE)
        {
            result = kStatus_Fail;
            break;
            ;
        }
        /* Initialize AES operation */
        tmsg[0] = ELEMU_SYMMETRIC_CONTEXT_INIT; // SYMMETRIC_CONTEXT_INIT Command Header
        tmsg[1] = sessionId;
        tmsg[2] = keyObjectId;
        tmsg[3] = 1; // AES CBC selected
        tmsg[4] = 0; // encryption mode
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_SYMMETRIC_CONTEXT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_SYMMETRIC_CONTEXT_INIT_RESPONSE)
        {
            /* read response data */
            aesOperationId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Perform AES CBC operation */
        tmsg[0] = ELEMU_CIPHER_ONE_GO; // CIPHER_ONE_GO Command Header
        tmsg[1] = aesOperationId;
        tmsg[2] = (uint32_t)ivData;     // IV data
        tmsg[3] = 16u;                  // 16 bytes
        tmsg[4] = (uint32_t)plainData;  // data to be encrypted
        tmsg[5] = (uint32_t)cipherData; // encrypted data
        tmsg[6] = 16u;                  // Data length in bytes
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CIPHER_ONE_GO_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CIPHER_ONE_GO_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free operation context */
        tmsg[0] = ELEMU_CONTEXT_FREE; // CONTEXT_FREE Command Header
        tmsg[1] = aesOperationId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CONTEXT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CONTEXT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free key object */
        tmsg[0] = ELEMU_KEY_OBJECT_FREE; // KEY_OBJECT_FREE Command Header
        tmsg[1] = keyObjectId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_OBJECT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free key store context */
        tmsg[0] = ELEMU_CONTEXT_FREE;
        tmsg[1] = keyStoreId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CONTEXT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CONTEXT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Close session context */
        tmsg[0] = ELEMU_CLOSE_SESSION; // CLOSE_SESSION Command Header
        tmsg[1] = sessionId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CLOSE_SESSION_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CLOSE_SESSION_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Release MU ownership */
        if (ELEMU_mu_release_ownership(ELEMUA) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        result = kStatus_Success;
    } while (0);

    if (result == kStatus_Success)
    {
        if (memcmp((void *)cipherDataRef, (void *)cipherData, sizeof(cipherDataRef)))
        {
            PRINTF(
                "ERROR: expected result of AES CBC encrypted data is different from value provided by Security "
                "Sub-System!\r\n");
        }
        else
        {
            PRINTF(
                "SUCCESS: expected result of AES CBC encrypted data is equal to value provided by Security "
                "Sub-System!!\r\n");
        }
    }
    else
    {
        PRINTF("ERROR: execution of commands on Security Sub-System failed!\r\n");
    }
    while (1)
    {
    }
}
#else
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ELEMU_OPEN_SESSION                    (0x13020017u)
#define ELEMU_OPEN_SESSION_SIZE               (0x3u)
#define ELEMU_OPEN_SESSION_RESPONSE           (0x13013c3cu)
#define ELEMU_KEY_STORE_CONTEXT_INIT          (0x49020017u)
#define ELEMU_KEY_STORE_CONTEXT_INIT_SIZE     (0x3u)
#define ELEMU_KEY_STORE_CONTEXT_INIT_RESPONSE (0x49013c3cu)
#define ELEMU_KEY_OBJECT_INIT                 (0x41010017u)
#define ELEMU_KEY_OBJECT_INIT_SIZE            (0x2u)
#define ELEMU_KEY_OBJECT_INIT_RESPONSE        (0x41013c3cu)
#define ELEMU_KEY_OBJECT_ALLOCATE             (0x42060017u)
#define ELEMU_KEY_OBJECT_ALLOCATE_SIZE        (0x7u)
#define ELEMU_KEY_OBJECT_ALLOCATE_RESPONSE    (0x42003c3cu)
#define ELEMU_KEY_STORE_SET_KEY               (0x4c060017u)
#define ELEMU_KEY_STORE_SET_KEY_SIZE          (0x7u)
#define ELEMU_KEY_STORE_SET_KEY_RESPONSE      (0x4c003c3cu)
#define ELEMU_SYMMETRIC_CONTEXT_INIT          (0x25040017u)
#define ELEMU_SYMMETRIC_CONTEXT_INIT_SIZE     (0x5u)
#define ELEMU_SYMMETRIC_CONTEXT_INIT_RESPONSE (0x25013c3cu)
#define ELEMU_CIPHER_ONE_GO                   (0x23060017u)
#define ELEMU_CIPHER_ONE_GO_SIZE              (0x7u)
#define ELEMU_CIPHER_ONE_GO_RESPONSE          (0x23003c3cu)
#define ELEMU_CONTEXT_FREE                    (0x15010017u)
#define ELEMU_CONTEXT_FREE_SIZE               (0x2u)
#define ELEMU_CONTEXT_FREE_RESPONSE           (0x15003c3cu)
#define ELEMU_KEY_OBJECT_FREE                 (0x47020017u)
#define ELEMU_KEY_OBJECT_FREE_SIZE            (0x3u)
#define ELEMU_KEY_OBJECT_FREE_RESPONSE        (0x47003c3cu)
#define ELEMU_KEY_STORE_CONTEXT_FREE          (0x76010017u)
#define ELEMU_KEY_STORE_CONTEXT_FREE_SIZE     (0x2u)
#define ELEMU_KEY_STORE_CONTEXT_FREE_RESPONSE (0x76003c3cu)
#define ELEMU_CLOSE_SESSION                   (0x14010017u)
#define ELEMU_CLOSE_SESSION_SIZE              (0x2u)
#define ELEMU_CLOSE_SESSION_RESPONSE          (0x14003c3cu)
/*******************************************************************************
 * Prototypes
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
     * This code example demonstrates EdgeLock usage for AES CBC operation. The AES CBC operation is performed in
     * following steps:
     * 1. Open EdgeLock session
     * 2. Create key store
     * 3. Create and allocate key object
     * 4. Set the key
     * 5. Initialize AES CBC operation context
     * 6. Perform AES CBC operation
     * 7. Close all opened contexts and created objects
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */
    /* KEY = 1f8e4973953f3fb0bd6b16662e9a3c17 */
    uint8_t symKeyData[16] = {0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0,
                              0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17};
    /* IV = 2fe2b333ceda8f98f4a99b40d2cd34a8 */
    uint8_t ivData[16] = {0x2f, 0xe2, 0xb3, 0x33, 0xce, 0xda, 0x8f, 0x98,
                          0xf4, 0xa9, 0x9b, 0x40, 0xd2, 0xcd, 0x34, 0xa8};
    /* PLAINTEXT = 45cf12964fc824ab76616ae2f4bf0822 */
    uint8_t plainData[16] = {0x45, 0xcf, 0x12, 0x96, 0x4f, 0xc8, 0x24, 0xab,
                             0x76, 0x61, 0x6a, 0xe2, 0xf4, 0xbf, 0x08, 0x22};
    /* CIPHERTEXT = 0f61c4d44c5147c03c195ad7e2cc12b2 */
    uint8_t cipherDataRef[16] = {0x0f, 0x61, 0xc4, 0xd4, 0x4c, 0x51, 0x47, 0xc0,
                                 0x3c, 0x19, 0x5a, 0xd7, 0xe2, 0xcc, 0x12, 0xb2};

    uint8_t cipherData[16] = {0};
    status_t result        = kStatus_Fail;
    uint32_t sessionId;
    uint32_t keyStoreId;
    uint32_t keyObjectId;
    uint32_t aesOperationId;
    /* Allocate message buffer with maximal message unit trasfer register count */
    uint32_t tmsg[ELEMU_TR_COUNT];
    /* Allocate message buffer with maximal message unit receive register count */
    uint32_t rmsg[ELEMU_RR_COUNT];

    do
    {
        /* HW init */
    BOARD_InitPins();
    BOARD_InitDebugConsole();
        PRINTF("ELEMU Peripheral Driver Example\r\n");
        /* Lock MU ownership to current thread */
        if (ELEMU_mu_get_ownership(ELEMUA) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Open EdgeLock session */
        tmsg[0] = ELEMU_OPEN_SESSION; /* OPEN_SESSION Command Header */
        tmsg[1] = 0x2u;               /* EdgeLock ID */
        tmsg[2] = 0x0u;               /* Session User ID */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_OPEN_SESSION_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_OPEN_SESSION_RESPONSE)
        {
            /* read response data */
            sessionId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Initialize key store */
        tmsg[0] = ELEMU_KEY_STORE_CONTEXT_INIT; /* KEY_STORE_CONTEXT_INIT Command Header */
        tmsg[1] = sessionId;
        tmsg[2] = 0x0u; /* User ID=0 */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_STORE_CONTEXT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_KEY_STORE_CONTEXT_INIT_RESPONSE)
        {
            /* read response data */
            keyStoreId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* Key object init */
        tmsg[0] = ELEMU_KEY_OBJECT_INIT; /* KEY_OBJECT_INIT Command Header */
        tmsg[1] = keyStoreId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_KEY_OBJECT_INIT_RESPONSE)
        {
            /* read response data */
            keyObjectId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Key object allocate slot in key store */
        tmsg[0] = ELEMU_KEY_OBJECT_ALLOCATE; /* KEY_OBJECT_ALLOCATE Command Header */
        tmsg[1] = keyObjectId;
        tmsg[2] = 0x0u;  /* key user ID */
        tmsg[3] = 0x1u;  /* Default key part - symetric key */
        tmsg[4] = 0x10u; /* Symetric key cipher type */
        tmsg[5] = 16u;   /* 16 bytes max key slot size */
        tmsg[6] = 0x01u; /* Allow only AES operation for key */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_ALLOCATE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_OBJECT_ALLOCATE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Set key */
        tmsg[0] = ELEMU_KEY_STORE_SET_KEY; /* KEY_STORE_SET_KEY Command Header */
        tmsg[1] = keyStoreId;
        tmsg[2] = keyObjectId;
        tmsg[3] = (uint32_t)symKeyData;
        tmsg[4] = 16u;  /* key data buffer size */
        tmsg[5] = 128u; /* key length in bits */
        tmsg[6] = 0x1u; /* Default key part - symetric key */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_STORE_SET_KEY_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_STORE_SET_KEY_RESPONSE)
        {
            result = kStatus_Fail;
            break;
            ;
        }
        /* Initialize AES operation */
        tmsg[0] = ELEMU_SYMMETRIC_CONTEXT_INIT; /* SYMMETRIC_CONTEXT_INIT Command Header */
        tmsg[1] = sessionId;
        tmsg[2] = keyObjectId;
        tmsg[3] = 1; /* AES CBC selected */
        tmsg[4] = 0; /* encryption mode */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_SYMMETRIC_CONTEXT_INIT_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] == ELEMU_SYMMETRIC_CONTEXT_INIT_RESPONSE)
        {
            /* read response data */
            aesOperationId = rmsg[1];
        }
        else
        {
            result = kStatus_Fail;
            break;
        }
        /* Perform AES CBC operation */
        tmsg[0] = ELEMU_CIPHER_ONE_GO; /* CIPHER_ONE_GO Command Header */
        tmsg[1] = aesOperationId;
        tmsg[2] = (uint32_t)ivData;     /* IV data */
        tmsg[3] = 16u;                  /* IV data size in bytes */
        tmsg[4] = (uint32_t)plainData;  /* data to be encrypted */
        tmsg[5] = (uint32_t)cipherData; /* encrypted data */
        tmsg[6] = 16u;                  /* data length in bytes */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CIPHER_ONE_GO_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CIPHER_ONE_GO_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free operation context */
        tmsg[0] = ELEMU_CONTEXT_FREE; /* CONTEXT_FREE Command Header */
        tmsg[1] = aesOperationId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CONTEXT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CONTEXT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free key object */
        tmsg[0] = ELEMU_KEY_OBJECT_FREE; /* KEY_OBJECT_FREE Command Header */
        tmsg[1] = keyObjectId;
        tmsg[2] = 0x0u; /* Remove key, but not defragmented key store after key removal. */
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_OBJECT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_OBJECT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Free key store context */
        tmsg[0] = ELEMU_KEY_STORE_CONTEXT_FREE;
        tmsg[1] = keyStoreId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_KEY_STORE_CONTEXT_FREE_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_KEY_STORE_CONTEXT_FREE_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Close session context */
        tmsg[0] = ELEMU_CLOSE_SESSION; /* CLOSE_SESSION Command Header */
        tmsg[1] = sessionId;
        /* Send message Security Sub-System */
        if (ELEMU_mu_send_message(ELEMUA, tmsg, ELEMU_CLOSE_SESSION_SIZE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Wait for response from Security Sub-System */
        if (ELEMU_mu_get_response(ELEMUA, rmsg, ELEMU_RR_COUNT) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        /* Check that response corresponds to the sent command */
        if (rmsg[0] != ELEMU_CLOSE_SESSION_RESPONSE)
        {
            result = kStatus_Fail;
            break;
        }
        /* Release MU ownership */
        if (ELEMU_mu_release_ownership(ELEMUA) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        result = kStatus_Success;
    } while (0);

    if (result == kStatus_Success)
    {
        if (memcmp((void *)cipherDataRef, (void *)cipherData, sizeof(cipherDataRef)))
        {
            PRINTF(
                "ERROR: expected result of AES CBC encrypted data is different from value returned by Security "
                "Sub-System!\r\n");
        }
        else
        {
            PRINTF(
                "SUCCESS: expected result of AES CBC encrypted data is equal to value returned by Security "
                "Sub-System!!\r\n");
        }
    }
    else
    {
        PRINTF("ERROR: execution of commands on Security Sub-System failed!\r\n");
    }
    while (1)
    {
        char ch = GETCHAR();
        PUTCHAR(ch);
    }
}
#endif /* KW45_A0_SUPPORT */
