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

#include "ele_crypto.h" /* ELE Crypto SW */
#include "fsl_s3mu.h"   /* Messaging unit driver */
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
static uint8_t AesEcbPlain[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};

/* Buffer for ciphertext after running AES-ECB encrypt */
SDK_ALIGN(uint8_t AesEcbCipher[64], 8u)  = {0u};
SDK_ALIGN(uint8_t AesEcbDecrypt[64], 8u) = {0u};
size_t AesEcbCipherSize                  = 0u;
uint32_t AESkeyID                        = 0u;

/* Message to be signed */
SDK_ALIGN(uint8_t message[], 8u) =
    "Be that word our sign of parting, bird or fiend! I shrieked upstarting"
    "Get thee back into the tempest and the Nights Plutonian shore!"
    "Leave no black plume as a token of that lie thy soul hath spoken!"
    "Leave my loneliness unbroken! quit the bust above my door!"
    "Take thy beak from out my heart, and take thy form from off my door!"
    "Quoth the raven, Nevermore.  ";
uint32_t length = sizeof(message) - 1; /* no null string terminating */
/*******************************************************************************
 * Code
 ******************************************************************************/

status_t ELE_APP_Cleanup(uint32_t dataStorageID,
                         uint32_t cipherHandleID,
                         uint32_t signHandleID,
                         uint32_t verifyHandleID,
                         uint32_t keyHandleID,
                         uint32_t keyStoreHandleID,
                         uint32_t nvmStorageID,
                         uint32_t sessionID)
{
    /****************** Close Data storage session *************************/
    if (dataStorageID != 0u)
    {
        if (ELE_CloseDataStorage(S3MU, dataStorageID) != kStatus_Success)
        {
            return kStatus_Fail;
        }
        else
        {
            PRINTF("Close data storage session successfully.\r\n\r\n");
        }
    }

    /****************** Close cipher service ***********************/
    PRINTF("****************** Close cipher service ***********************\r\n");
    if (ELE_CloseCipherService(S3MU, cipherHandleID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Cipher service closed successfully.\r\n\r\n");
    }

    if (signHandleID != 0u)
    {
        /****************** Close Sign service *********************************/
        PRINTF("****************** Close Sign service *********************\r\n");
        if (ELE_CloseSignService(S3MU, signHandleID) != kStatus_Success)
        {
            return kStatus_Fail;
        }
        else
        {
            PRINTF("Sign service close successfully.\r\n\r\n");
        }
    }

    if (verifyHandleID != 0u)
    {
        /****************** Close Verify service *******************************/
        PRINTF("****************** Close Verify service *******************\r\n");
        if (ELE_CloseVerifyService(S3MU, verifyHandleID) != kStatus_Success)
        {
            return kStatus_Fail;
        }
        else
        {
            PRINTF("Verify service close successfully.\r\n\r\n");
        }
    }

    /****************** Close Key Management Service **********************/
    PRINTF("****************** Close Key Management Service ***************\r\n");
    if (ELE_CloseKeyService(S3MU, keyHandleID) != kStatus_Success)
    {
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Close Key Management Service successfully.\r\n\r\n");
    }

    /****************** Close Key Store  ***********************************/
    PRINTF("****************** Close Key Store ****************************\r\n");
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
    PRINTF("****************** Close EdgeLock session *********************\r\n");
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
     * 4.  Create Keystore
     * 5.  Open NVM Storage service
     * 6.  Open Key Management
     * 7.  Generate ECC HSM keypair
     * 8.  Open Sign service
     * 9.  Sign message with ECC private key generated earlier
     * 10. Export all necessary chunks (Master storage, Keystore and Key group with AES-ECB key)
     * 11. Open data storage service
     * 12. Store encrypted generic data and get encrypted chunk
     * 13. Retrieve generic data by decrypting encrypted chunk (from step 12.)
     * 14. Delete HSM keypair (from step 7.)
     * 15. Close all services and session
     * 16. In real world application, Power Off or reboot.
     * 17. Open new session and NVM Storage service.
     * 18. Import Master Storage chunk (exported in step 10.)
     * 19. Open Keystore using chunk (exported in step 10.)
     * 20. Import key group chunk (exported in step 10.)
     * 21. Sign message with ECC private key generated earlier (step 7) and imported back to ELE (step 20).
     * 22. Verify generated signature (step 9) using public key generated earlier (step 7).
     * 23. Verify generated signature (step 21) using public key generated earlier (step 7).
     * 24. Export key group chunk, in case of swap needed.
     * 25. Close keystore, session and services
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */

    status_t result = kStatus_Fail;
    uint32_t sessionID, keyStoreHandleID, nvmStorageID, keyHandleID, dataStorageID, NISTkeyPairID, signHandleID,
        verifyHandleID, cipherHandleID;
    uint16_t keySize;
    uint32_t keyGroupID = 42u; /* User defined key group ID */
    cipherHandleID      = 0u;  /* prevent uninitialized warning */

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
        } while ((trng_state & 0xFF) != kELE_TRNG_ready && result == kStatus_Success);

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
        keystoreParam.id             = 0x12345678u;
        keystoreParam.nonce          = 0xabcdef12u;
        keystoreParam.max_updates    = 0xff;
        keystoreParam.min_mac_check  = false;
        keystoreParam.min_mac_len    = 0u;

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
        PRINTF("****************** Open NVM Storage service ***************\r\n");
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

        /****************** Key Generate (ECC NIST P256) ***********************/
        PRINTF("****************** Key Generate (ECC NIST P256) ***********\r\n");
        /* Output buffer for public key */
        SDK_ALIGN(uint8_t pubKeyNIST[64u], 8u) = {0u};

        ele_gen_key_t NISTkeyGenParam = {0u};
        NISTkeyGenParam.key_type      = kKeyType_ECC_KEY_PAIR_SECP_R1_NIST;
        NISTkeyGenParam.key_lifetime  = kKey_Persistent;
        NISTkeyGenParam.key_lifecycle = kKeylifecycle_Open;
        NISTkeyGenParam.key_usage     = kKeyUsage_SignMessage;
        NISTkeyGenParam.key_size      = 256u;
        NISTkeyGenParam.permitted_alg = kPermitted_ECDSA_SHA256;
        NISTkeyGenParam.pub_key_addr  = (uint32_t)pubKeyNIST;
        NISTkeyGenParam.pub_key_size  = sizeof(pubKeyNIST);
        NISTkeyGenParam.key_group     = keyGroupID;
        NISTkeyGenParam.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &NISTkeyGenParam, &NISTkeyPairID, &keySize, false, false) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key generated successfully. Key Pair ID: 0x%x\r\n\r\n", NISTkeyPairID);
        }

        /****************** Key Generate (AES ECB 128) ************************/
        PRINTF("****************** Key Generate (AES ECB 128) *************\r\n");

        ele_gen_key_t AESkey = {0u};
        AESkey.key_type      = kKeyType_AES;
        AESkey.key_lifetime  = kKey_Persistent;
        AESkey.key_usage     = kKeyUsage_Encrypt | kKeyUsage_Decrypt;
        AESkey.key_size      = kKeySize_AES_128;
        AESkey.permitted_alg = kPermitted_ECB;
        AESkey.pub_key_addr  = 0u;
        AESkey.pub_key_size  = 0u;
        AESkey.key_group     = keyGroupID;
        AESkey.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &AESkey, &AESkeyID, &keySize, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key for AES-ECB generated successfully. Key ID: 0x%x\r\n\r\n", AESkeyID);
        }

        /****************** Open cipher service ********************************/
        PRINTF("****************** Open cipher service ********************\r\n");
        if (ELE_OpenCipherService(S3MU, keyStoreHandleID, &cipherHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Cipher service successfully. Handle ID: 0x%x\r\n\r\n", cipherHandleID);
        }

        /****************** Encrypt Cipher AES-ECB *****************************/
        PRINTF("****************** Encrypt Cipher AES-ECB *****************\r\n");

        ele_hsm_cipher_t AESHsmParam = {0u};
        AESHsmParam.keyID            = AESkeyID;
        AESHsmParam.input            = (uint32_t)AesEcbPlain;
        AESHsmParam.input_size       = sizeof(AesEcbPlain);
        AESHsmParam.output           = (uint32_t)AesEcbCipher;
        AESHsmParam.output_size      = &AesEcbCipherSize;
        AESHsmParam.iv               = 0u;
        AESHsmParam.iv_size          = 0u;
        AESHsmParam.alg              = kPermitted_ECB;
        AESHsmParam.mode             = kHSM_Encrypt;

        if (ELE_Cipher(S3MU, cipherHandleID, &AESHsmParam) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("AES-ECB encryption success, Output size returned: 0x%x\r\n", AesEcbCipherSize);
        }

        /****************** Key Management Export Chunks **********************/
        PRINTF("****************** Key Management Export Chunks ***********\r\n");

        ele_chunks_t chunks = {0u};
        // Master chunk is stored in chunks.MasterChunk[MASTER_CHUNK_SIZE] buffer with fixed size 100 Bytes
        chunks.KeyStoreChunk = NULL; // If NULL, heap is automatically used and size passed in KeyStoreSize
        chunks.KeyGroupChunk = NULL; // If NULL, heap is automatically used and size passed in KeyGroupSize

        if (ELE_ExportChunks(S3MU, keyHandleID, true, keyGroupID, false, &chunks) != kStatus_Success)
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
            PRINTF("\t Key Group chunk at address: \t 0x%x , size %d Bytes\r\n", chunks.KeyGroupChunk,
                   chunks.KeyGroupSize);
        }
        /* Chunks obtained here should be stored in NVM by user (usually flash) */
        /* In this example, we keep them in RAM as we are not going to do power off */

        /****************** Open Sign service **********************************/
        PRINTF("****************** Open Sign service **********************\r\n");
        if (ELE_OpenSignService(S3MU, keyStoreHandleID, &signHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Sign service open successfully. Signature generation handle ID: 0x%x\r\n\r\n", signHandleID);
        }

        /****************** ECC NIST P256 sign *********************************/
        PRINTF("****************** ECC NIST P256 sign *********************\r\n");
        /* Output buffer for signature */
        SDK_ALIGN(uint8_t signatureNIST[64u], 8u) = {0u};
        uint32_t signatureSize                    = 0u;

        ele_sign_t NISTsignGenParam = {0u};
        NISTsignGenParam.key_id     = NISTkeyPairID;
        NISTsignGenParam.msg        = (const uint8_t *)message;
        NISTsignGenParam.msg_size   = length;
        NISTsignGenParam.signature  = signatureNIST;
        NISTsignGenParam.sig_size   = sizeof(signatureNIST);
        NISTsignGenParam.scheme     = kSig_ECDSA_SHA256;
        NISTsignGenParam.input_flag = true; // Actual message as input
        NISTsignGenParam.salt_size  = 0u;

        if (ELE_Sign(S3MU, signHandleID, &NISTsignGenParam, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Signature generated successfully.\r\n\r\n");
        }

        /****************** Open Data Storage service **************************/
        PRINTF("****************** Open Storage service *******************\r\n");
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
        PRINTF("****************** Data Storage Store *********************\r\n");
        /* Prepare data to be stored */
        SDK_ALIGN(uint8_t plain[100u], 8u) = {0u};
        for (int i = 0u; i < sizeof(plain); i++)
        {
            plain[i] = i;
        }

        ele_data_storage_t dataStorageParam = {0u};
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

        /****************** Data Storage Retrieve ******************************/
        PRINTF("****************** Data Storage Retrieve ******************\r\n");
        AT_NONCACHEABLE_SECTION_ALIGN(uint8_t static data_out[100u], 8u);

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

        /****************** Key Delete (ECC NIST P256) *************************/
        PRINTF("****************** Key Delete (ECC NIST P256) *************\r\n");
        if (ELE_DeleteKey(S3MU, keyHandleID, NISTkeyPairID, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key deleted successfully. Key Pair ID: 0x%x\r\n\r\n", NISTkeyPairID);
        }

        /* Close all services */
        if (ELE_APP_Cleanup(dataStorageID, cipherHandleID, signHandleID, 0u, keyHandleID, keyStoreHandleID,
                            nvmStorageID, sessionID) != kStatus_Success)
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
        PRINTF("****************** Open NVM Storage service ***************\r\n");
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
        PRINTF("****************** Open Key Store *************************\r\n");

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

        /****************** Key Management Chunk Import ***********************/
        PRINTF("****************** Key Management Chunk Import ************\r\n");
        if (ELE_ManageKeyGroup(S3MU, keyHandleID, 42u, kELE_keyMngImport, chunks.KeyGroupChunk, chunks.KeyGroupSize) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key Management Key group chunk Import successfully\r\n\r\n");
        }

        /****************** Open cipher service ********************************/
        PRINTF("****************** Open cipher service ********************\r\n");
        if (ELE_OpenCipherService(S3MU, keyStoreHandleID, &cipherHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open Cipher service successfully. Handle ID: 0x%x\r\n\r\n", cipherHandleID);
        }

        /****************** Decrypt Cipher AES-ECB *****************************/
        PRINTF("****************** Decrypt Cipher AES-ECB *****************\r\n");

        AESHsmParam.keyID      = AESkeyID;
        AESHsmParam.input      = (uint32_t)AesEcbCipher;
        AESHsmParam.input_size = sizeof(AesEcbPlain);
        AESHsmParam.output     = (uint32_t)AesEcbDecrypt;
        AESHsmParam.mode       = kHSM_Decrypt;

        if (ELE_Cipher(S3MU, cipherHandleID, &AESHsmParam) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            /* Check decrypted data match the original plain text */
            if (memcmp(AesEcbDecrypt, AesEcbPlain, sizeof(AesEcbPlain)) == 0)
            {
                PRINTF("AES-ECB decrypted data match the original plain text - success.\r\n\r\n");
            }
            else
            {
                result = kStatus_Fail;
                break;
            }
        }

        /****************** Open Sign service **********************************/
        PRINTF("****************** Open Sign service **********************\r\n");
        if (ELE_OpenSignService(S3MU, keyStoreHandleID, &signHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Sign service open successfully. Signature generation handle ID: 0x%x\r\n\r\n", signHandleID);
        }

        /****************** ECC NIST P256 sign (second time) *******************/
        PRINTF("****************** ECC NIST P256 sign (second time) *******\r\n");
        /* Output buffer for signature */
        SDK_ALIGN(uint8_t signatureNIST2[64u], 8u) = {0u};
        signatureSize                              = 0;

        ele_sign_t NISTsignGenParam2 = {0u};
        NISTsignGenParam2.key_id     = NISTkeyPairID;
        NISTsignGenParam2.msg        = (const uint8_t *)message;
        NISTsignGenParam2.msg_size   = length;
        NISTsignGenParam2.signature  = signatureNIST2;
        NISTsignGenParam2.sig_size   = sizeof(signatureNIST2);
        NISTsignGenParam2.scheme     = kSig_ECDSA_SHA256;
        NISTsignGenParam2.input_flag = true; // Actual message as input
        NISTsignGenParam2.salt_size  = 0u;

        if (ELE_Sign(S3MU, signHandleID, &NISTsignGenParam2, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Second Signature generated successfully.\r\n\r\n");
        }

        /****************** Open Verify service ********************************/
        PRINTF("****************** Open Verify service ********************\r\n");
        if (ELE_OpenVerifyService(S3MU, sessionID, &verifyHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Verify service open successfully. Verification handle ID: 0x%x\r\n\r\n", verifyHandleID);
        }

        /****************** ECC NIST P256 verify *******************************/
        PRINTF("****************** ECC NIST P256 verify *******************\r\n");
        bool result_nist = false;

        ele_verify_t NISTverifyParam      = {0u};
        NISTverifyParam.pub_key           = (const uint8_t *)pubKeyNIST;
        NISTverifyParam.key_size          = sizeof(pubKeyNIST);
        NISTverifyParam.msg               = (const uint8_t *)message;
        NISTverifyParam.msg_size          = length;
        NISTverifyParam.signature         = signatureNIST;
        NISTverifyParam.sig_size          = sizeof(signatureNIST);
        NISTverifyParam.keypair_type      = kKeyType_ECC_PUB_KEY_SECP_NIST;
        NISTverifyParam.scheme            = kSig_ECDSA_SHA256;
        NISTverifyParam.input_flag        = true; // Actual message as input
        NISTverifyParam.key_security_size = 256u;
        NISTverifyParam.internal          = false;
        NISTverifyParam.salt_size         = 0u;

        if (ELE_Verify(S3MU, verifyHandleID, &NISTverifyParam, &result_nist) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (result_nist)
            {
                PRINTF("ECC NIST p256 Signature verified successfully!\r\n\r\n");
            }
        }

        /****************** ECC NIST P256 verify (second) *********************/
        PRINTF("****************** ECC NIST P256 verify (second) **********\r\n");
        bool result_nist2 = false;

        ele_verify_t NISTverifyParam2      = {0u};
        NISTverifyParam2.pub_key           = (const uint8_t *)pubKeyNIST;
        NISTverifyParam2.key_size          = sizeof(pubKeyNIST);
        NISTverifyParam2.msg               = (const uint8_t *)message;
        NISTverifyParam2.msg_size          = length;
        NISTverifyParam2.signature         = signatureNIST2;
        NISTverifyParam2.sig_size          = sizeof(signatureNIST2);
        NISTverifyParam2.keypair_type      = kKeyType_ECC_PUB_KEY_SECP_NIST;
        NISTverifyParam2.scheme            = kSig_ECDSA_SHA256;
        NISTverifyParam2.input_flag        = true; // Actual message as input
        NISTverifyParam2.key_security_size = 256u;
        NISTverifyParam2.internal          = false;
        NISTverifyParam2.salt_size         = 0u;

        if (ELE_Verify(S3MU, verifyHandleID, &NISTverifyParam2, &result_nist2) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (result_nist2)
            {
                PRINTF("ECC NIST p256 Second Signature verified successfully!\r\n\r\n");
            }
        }

        /****************** Key Management Chunk Export ***********************/
        PRINTF("****************** Key Management Chunk Export ************\r\n");
        /* Only 2 key groups can be loaded in ELE local memory at one time. */
        /* This API allows user to swap these groups */
        if (ELE_ManageKeyGroup(S3MU, keyHandleID, 42u, kELE_keyMngExport, chunks.KeyGroupChunk, chunks.KeyGroupSize) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key Management Key group chunk Export successfully\r\n\r\n");
        }

        /****************** Cleanup *******************************************/

        /* Close all services */
        if (ELE_APP_Cleanup(0u, cipherHandleID, 0u, verifyHandleID, keyHandleID, keyStoreHandleID, nvmStorageID,
                            sessionID) != kStatus_Success)
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
