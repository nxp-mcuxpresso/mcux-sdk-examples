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
#define S3MU MU_RT_S3MUA

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

status_t ELE_APP_Cleanup(
    uint32_t dataStorageID, uint32_t keyHandleID, uint32_t keyStoreHandleID, uint32_t nvmStorageID, uint32_t sessionID)
{
    /****************** Close Data storage session *************************/
    if (ELE_CloseDataStorage(S3MU, dataStorageID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close data storage session successfully.\r\n\r\n");
    }

    /****************** Close Key Management Service **********************/
    PRINTF("****************** Close Key Management Service ***********\r\n");
    if (ELE_CloseKeyService(S3MU, keyHandleID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close Key Management Service successfully.\r\n\r\n");
    }

    /****************** Close Key Store  ***********************************/
    PRINTF("****************** Close Key Store ************************\r\n");
    if (ELE_CloseKeystore(S3MU, keyStoreHandleID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close Key Store successfully.\r\n\r\n");
    }

    /****************** Close NVM storage session **************************/
    if (ELE_CloseNvmStorageService(S3MU, nvmStorageID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close NVM storage session successfully.\r\n\r\n");
    }

    /****************** Close EdgeLock session *****************************/
    PRINTF("****************** Close EdgeLock session *****************\r\n");
    if (ELE_CloseSession(S3MU, sessionID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close session successfully.\r\n\r\n");
    }

    return kStatus_Success;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /*
     * This code example demonstrates EdgeLock usage in following steps:
     * 0.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 1.  Start RNG
     * 2.  Initialize ELE services
     * 3.  Open EdgeLock session
     * 4.  Create Keystore (Note: this keystore is empty and holds only internal derived key for generic data services)
     * 5.  Open NVM Storage service
     * 6.  Open Key Management
     * 7.  Export Master storage and Keystore chunks
     * 8.  Open data storage service
     * 9.  Store encrypted generic data and get encrypted chunk
     * 10. Close all services and session
     * 11. In real world application, Power Off or reboot.
     * 12. Open new session and NVM Storage service.
     * 13. Import Master Storage chunk (exported in step 7.)
     * 14. Open Keystore using chunk (exported in step 7.)
     * 15. Retrieve generic data by decrypting encrypted chunk (from step 9.)
     * 16. Close keystore, session and services
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */

    status_t result = kStatus_Fail;
    uint32_t sessionID, keyStoreHandleID, nvmStorageID, keyHandleID, dataStorageID;
    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave secure Sub-System Driver Example:\r\n\r\n");

        /****************** Load EdgeLock FW message ***********************/
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

        /****************** Start RNG ***********************/
        PRINTF("****************** Start RNG ******************************\r\n");
        if (ELE_StartRng(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock RNG Start success.\r\n");
        }

        uint32_t trng_state = 0u;
        do
        {
            result = ELE_GetTrngState(S3MU, &trng_state);
        } while (((trng_state & 0xFFu) != kELE_TRNG_ready) &&
                 ((trng_state & 0xFF00u) != kELE_TRNG_CSAL_success << 8u ) &&
                   result == kStatus_Success);

        PRINTF("EdgeLock RNG ready to use.\r\n\r\n");

        /****************** Initialize ELE services ************/
        PRINTF("****************** Initialize ELE services ****************\r\n");
        if (ELE_InitServices(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("ELE services initialized successfully.\r\n\r\n");
        }

        /****************** Open EdgeLock session ***********************/
        PRINTF("****************** Open EdgeLock session ******************\r\n");
        if (ELE_OpenSession(S3MU, &sessionID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open session successfully. Session ID: 0x%x\r\n\r\n", sessionID);
        }

        /****************** Create Key Store  *********************************/
        PRINTF("****************** Create Key Store ***********************\r\n");
        ele_keystore_t keystoreParam = {0u};

        keystoreParam.id            = 0x12345678u;
        keystoreParam.nonce         = 0xabcdef12u;
        keystoreParam.max_updates   = 0xff;
        keystoreParam.min_mac_check = false;
        keystoreParam.min_mac_len   = 0u;

        if (ELE_CreateKeystore(S3MU, sessionID, &keystoreParam, &keyStoreHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open service and create Key Store successfully. Key Store ID: 0x%x\r\n\r\n", keyStoreHandleID);
        }

        /****************** Open NVM Storage service **************************/
        PRINTF("****************** Open NVM Storage service ****************\r\n");
        if (ELE_OpenNvmStorageService(S3MU, sessionID, &nvmStorageID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open NVM Storage service successfully. Handle ID: 0x%x\r\n\r\n", nvmStorageID);
        }

        /****************** Key Management Open *******************************/
        PRINTF("****************** Key Management Open ********************\r\n");
        if (ELE_OpenKeyService(S3MU, keyStoreHandleID, &keyHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Key management service successfully. Key Handle ID: 0x%x\r\n\r\n", keyHandleID);
        }

        /****************** Open Data Storage service **************************/
        PRINTF("****************** Open Storage service ********************\r\n");
        if (ELE_OpenDataStorage(S3MU, keyStoreHandleID, &dataStorageID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Storage service successfully. Handle ID: 0x%x\r\n\r\n", dataStorageID);
        }

        /****************** Data Storage Store *********************************/
        PRINTF("****************** Data Storage Store **********************\r\n");
        /* Prepare data to be stored */
        SDK_ALIGN(uint8_t plain[100u], 8u) = {0u};
        for (int i = 0u; i < sizeof(plain); i++)
        {
            plain[i] = i;
        }

        ele_data_storage_t dataStorageParam = {0u}; /* Make sure all config is cleared before using! */
        dataStorageParam.dataID             = 1u;
        dataStorageParam.data               = (uint32_t *)plain;
        dataStorageParam.data_size          = sizeof(plain);
        dataStorageParam.option             = kStandardOption;
        // uint8_t chunk[sizeof(plain) + CHUNK_META_SIZE] = {0u};
        // dataStorageParam.chunk_size = sizeof(plain) + CHUNK_META_SIZE;
        // dataStorageParam.chunk_addr = (uint32_t*)chunk;
        /* If chunk_addr is NULL, output data buffer is dynamically allocated and address returned */
        /* in chunk_addr struct member. Output chunk size equals data size + 36B for ELE meta data */
        if (ELE_StoreDataStorage(S3MU, dataStorageID, &dataStorageParam) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Data Storage successfully. Exported Chunk at address 0x%x\r\n\r\n", dataStorageParam.chunk_addr);
        }

        /****************** Key Management Export Chunks **********************/
        PRINTF("****************** Key Management Export Chunks ***********\r\n");

        ele_chunks_t chunks = {0u};
        // Master chunk is stored in chunks.MasterChunk[MASTER_CHUNK_SIZE] buffer with fixed size 100 Bytes
        chunks.KeyStoreChunk = NULL; // If NULL, heap is automatically used and size passed in KeyStoreSize

        if (ELE_ExportChunks(S3MU, keyHandleID, false, 0u, false, &chunks) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Chunks exported successfully.\r\n");
            PRINTF("\t Storage Master chunk at address: \t 0x%x , size %d Bytes\r\n", chunks.MasterChunk,
                   MASTER_CHUNK_SIZE);
            PRINTF("\t Key Store chunk at address: \t 0x%x , size %d Bytes\r\n", chunks.KeyStoreChunk,
                   chunks.KeyStoreSize);
        }
        /* Chunks obtained here should be stored in NVM by user (usually flash) */
        /* In this example, we keep them in RAM as we are not going to do power off */

        /****************** Cleanup *******************************************/

        /* Close all services */
        if (ELE_APP_Cleanup(dataStorageID, keyHandleID, keyStoreHandleID, nvmStorageID, sessionID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }

        /****************** END of Cleanup ************************************/

        /**********************************************************************/
        /****************** Power Off/Reboot **********************************/
        /**********************************************************************/

        /****************** Init after Power On *******************************/

        /****************** Open EdgeLock session ***********************/
        PRINTF("****************** Open EdgeLock session ******************\r\n");
        if (ELE_OpenSession(S3MU, &sessionID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open session successfully. Session ID: 0x%x\r\n\r\n", sessionID);
        }

        /****************** Open NVM Storage service **************************/
        PRINTF("****************** Open NVM Storage service ****************\r\n");
        if (ELE_OpenNvmStorageService(S3MU, sessionID, &nvmStorageID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open NVM Storage service successfully. Handle ID: 0x%x\r\n\r\n", nvmStorageID);
        }

        /****************** Import Master storage chunk ***********************/
        PRINTF("****************** Import Master storage chunk ************\r\n");
        if (ELE_StorageMasterImport(S3MU, nvmStorageID, chunks.MasterChunk) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Import Master storage chunk successfully.\r\n\r\n");
        }

        /****************** Open Key Store ************************************/
        PRINTF("****************** Open Key Store ****************\r\n");

        if (ELE_OpenKeystore(S3MU, sessionID, &keystoreParam, &keyStoreHandleID, chunks.KeyStoreChunk,
                             chunks.KeyStoreSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open service and create Key Store successfully. Key Store ID: 0x%x\r\n\r\n", keyStoreHandleID);
        }

        /****************** Key Management Open *******************************/
        PRINTF("****************** Key Management Open ********************\r\n");
        if (ELE_OpenKeyService(S3MU, keyStoreHandleID, &keyHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Key management service successfully. Key Handle ID: 0x%x\r\n\r\n", keyHandleID);
        }

        /****************** Open Data Storage service **************************/
        PRINTF("****************** Open Storage service ********************\r\n");
        if (ELE_OpenDataStorage(S3MU, keyStoreHandleID, &dataStorageID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Storage service successfully. Handle ID: 0x%x\r\n\r\n", dataStorageID);
        }

        /****************** Data Storage Retrieve ******************************/
        PRINTF("****************** Data Storage Retrieve *******************\r\n");
        SDK_ALIGN(uint8_t static data_out[100u], 8u);

        ele_data_storage_t dataGetParam = {0u};
        dataGetParam.dataID             = 1u;
        dataGetParam.chunk_addr         = dataStorageParam.chunk_addr;
        dataGetParam.chunk_size         = dataStorageParam.chunk_size;
        dataGetParam.data               = (uint32_t *)data_out;
        dataGetParam.data_size          = sizeof(data_out);
        /* Output data in local buffer, set data to NULL for dynamically allocated buffer instead */
        /* Minimal size for local buffer is chunk size - 36B (ELE internal meta data) */
        if (ELE_RetrieveDataStorage(S3MU, dataStorageID, &dataGetParam) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Data Storage Retrieve successfully. Exported Chunk at address 0x%x\r\n", dataGetParam.data);

            if (memcmp(data_out, plain, sizeof(data_out)) == 0u)
            {
                PRINTF("Success: Retrieved data at addr 0x%x are same as stored plain data at 0x%x\r\n", data_out,
                       plain);
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }

        /****************** Cleanup *******************************************/

        /* Close all services */
        if (ELE_APP_Cleanup(dataStorageID, keyHandleID, keyStoreHandleID, nvmStorageID, sessionID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }

        /****************** END of Cleanup ************************************/

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
