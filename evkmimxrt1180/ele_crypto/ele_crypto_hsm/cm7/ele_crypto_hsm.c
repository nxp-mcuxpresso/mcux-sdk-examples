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

/*!
 * @brief Main function
 */
int main(void)
{
    /*
     * This code example demonstrates EdgeLock usage in following steps:
     * 0.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 1.  Start RNG
     * 2.  Initialize EdgeLock Enclave services
     * 3.  Open EdgeLock session
     * 4.  Compute hash and verify result with expected value
     * 5.  Get RNG random data
     * 6.  Open and Create Key Store
     * 7.  Key management service open
     * 8.  Key generate for AES-ECB with user selected key ID
     * 9.  Open Cipher service
     * 10. Encrypt and decrypt data using AES-ECB
     * 11. Key generate for AES-AEAD
     * 12. Encrypt and decrypt data using AEAD-GCM
     * 13. Key generate for HMAC_SHA256
     * 14. Generate and Verify MAC
     * 15. Key pair generate for ECC (NIST P256)
     * 16. Sign service open
     * 17. Generate ECDSA signature based on previously generated key pair
     * 18. Open verify service
     * 19. Verify signature
     * 20. Delete ECC key pair (from step 15)
     * 21. Key pair generate for ECC (Brainpool)
     * 22. Generate ECDSA signature based on previously generated key pair
     * 23. Verify signature
     * 24. Delete ECC key pair (from step 21)
     * 25. Key pair generate for RSA PKCS1 V1.5
     * 26. Generate RSA signature based on previously generated key pair
     * 27. Verify signature
     * 28. Delete RSA PKCS1 V1.5 key
     * 29. Key pair generate for RSA PSS
     * 30. Generate RSA signature based on previously generated key pair
     * 31. Verify signature
     * 32. Close keystore, session and services
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */

    status_t result = kStatus_Fail;
    uint32_t sessionID, keyStoreHandleID, keyHandleID, NISTkeyPairID, RSAkeyPairID, brainpoolKeyPairID, signHandleID,
        verifyHandleID, AESkeyID, cipherHandleID, macHandleID, MACkeyID;
    uint16_t keySize;
    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave Sub-System crypto example:\r\n\r\n");

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

        /****************** Initialize EdgeLock services ************/
        PRINTF("****************** Initialize EdgeLock services ***********\r\n");
        if (ELE_InitServices(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("EdgeLock services initialized successfully.\r\n\r\n");
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

        /****************** Open and Create Key Store  ***********************************/
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

        /****************** Key Generate (AES ECB 128) ***********************/
        PRINTF("****************** Key Generate (AES ECB 128) *************\r\n");

        /* For this key, show how to select a user specified key ID.
         * Note the required "persistent" key lifetime, and different key group
         * from the rest of the "volatile" keygens. */
        ele_gen_key_t AESkey = {0u};

        AESkey.key_type      = kKeyType_AES;
        AESkey.key_lifetime  = kKey_Persistent;
        AESkey.key_usage     = kKeyUsage_Encrypt | kKeyUsage_Decrypt;
        AESkey.key_size      = kKeySize_AES_128;
        AESkey.permitted_alg = kPermitted_ECB;
        AESkey.pub_key_addr  = 0u;
        AESkey.pub_key_size  = 0u;
        AESkey.key_group     = 10u;
        AESkey.key_id        = 0xcafe;
        AESkey.key_lifecycle = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &AESkey, &AESkeyID, &keySize, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key for AES-ECB generated successfully. Key ID: 0x%x\r\n\r\n", AESkeyID);
        }

        /****************** Open cipher service ***********************/
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

        /****************** Crypto AES-EBC ***********************/
        PRINTF("****************** Cipher AES ECB *************************\r\n");

        static uint8_t input[] = {0x63, 0x76, 0xea, 0xcc, 0xc9, 0xa2, 0xc0, 0x43, 0xf4, 0xfb, 0x01,
                                  0x34, 0x69, 0xb3, 0x0c, 0xf5, 0x28, 0x63, 0x5c, 0xfa, 0xa5, 0x65,
                                  0x60, 0xef, 0x59, 0x7b, 0xd9, 0x1c, 0xac, 0xaa, 0x31, 0xf7};

        uint32_t inputSize = sizeof(input);

        /* Buffer for ciphertext after running AES-ECB encrypt */
        SDK_ALIGN(uint8_t AesEcbCipher[64u], 8u) = {0u};
        size_t AesEcbCipherSize                  = sizeof(AesEcbCipher);

        /* Buffer for plaintext after running AES-ECB decrypt on AesEcbCipher data  */
        AT_NONCACHEABLE_SECTION_INIT(SDK_ALIGN(static uint8_t AesEcbDecrypted[64], 8u));
        size_t AesEcbDecryptedSize = sizeof(AesEcbDecrypted);

        ele_hsm_cipher_t AESHsmParam = {0u};

        AESHsmParam.keyID       = AESkeyID;
        AESHsmParam.input       = (uint32_t)input;
        AESHsmParam.input_size  = inputSize;
        AESHsmParam.output      = (uint32_t)AesEcbCipher;
        AESHsmParam.output_size = &AesEcbCipherSize;
        AESHsmParam.iv          = 0u;
        AESHsmParam.iv_size     = 0u;
        AESHsmParam.alg         = kPermitted_ECB;
        AESHsmParam.mode        = kHSM_Encrypt;

        /* AES-ECB Encrypt */
        if (ELE_Cipher(S3MU, cipherHandleID, &AESHsmParam) == kStatus_Success)
        {
            PRINTF("Output size returned by AES-ECB encryption is : 0x%x\r\n", AesEcbCipherSize);
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        AESHsmParam.input       = (uint32_t)AesEcbCipher;
        AESHsmParam.input_size  = inputSize;
        AESHsmParam.output      = (uint32_t)AesEcbDecrypted;
        AESHsmParam.output_size = &AesEcbDecryptedSize;
        AESHsmParam.mode        = kHSM_Decrypt;

        /* AES-ECB Decrypt */
        if (ELE_Cipher(S3MU, cipherHandleID, &AESHsmParam) == kStatus_Success)
        {
            /* Compare AesEcbDecrypted with input */
            if (memcmp(input, AesEcbDecrypted, inputSize) == 0)
            {
                PRINTF("AES-ECB crypto encryption and decryption using keys in ELE success.\r\n\r\n");
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

        /****************** Key Generate (AEAD) ***********************/
        PRINTF("****************** Key Generate (AEAD) ********************\r\n");

        AESkey.key_type      = kKeyType_AES;
        AESkey.key_lifetime  = kKey_Volatile;
        AESkey.key_usage     = kKeyUsage_Encrypt | kKeyUsage_Decrypt;
        AESkey.key_size      = kKeySize_AES_128;
        AESkey.permitted_alg = kPermitted_GCM;
        AESkey.pub_key_addr  = 0u;
        AESkey.pub_key_size  = 0u;
        AESkey.key_group     = 42u;
        AESkey.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &AESkey, &AESkeyID, &keySize, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key for AES-GCM generated successfully. Key ID: 0x%x\r\n\r\n", AESkeyID);
        }

        /****************** Cipher AES AEAD ***********************/
        PRINTF("****************** Cipher AES AEAD (GCM) ******************\r\n");

        /* Buffer for ciphertext after running AES-AEAD encrypt */
        SDK_ALIGN(uint8_t AesAeadCipher[64], 8u) = {0u};
        size_t AesAeadCipherSize                 = sizeof(AesAeadCipher);

        /* Buffer for plantext after running AES-AEAD decrypt on AesAeadCipher data  */
        AT_NONCACHEABLE_SECTION_INIT(SDK_ALIGN(static uint8_t AesAeadDecrypted[64], 8u));
        size_t AesAeadDecryptedSize = sizeof(AesAeadDecrypted);

        /*! @brief Initialization vector for GCM method. */
        SDK_ALIGN(static uint8_t s_GcmIv[12], 8u) = {0xca, 0xfe, 0xba, 0xbe, 0xfa, 0xce,
                                                     0xdb, 0xad, 0xde, 0xca, 0xf8, 0x88};

        /*! @brief Additional authenticated data for GCM method. */
        SDK_ALIGN(static uint8_t s_GcmAad[], 8u) = {0xfe, 0xed, 0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed,
                                                    0xfa, 0xce, 0xde, 0xad, 0xbe, 0xef, 0xab, 0xad, 0xda, 0xd2};

        ele_hsm_cipher_t AEADHsmParam = {0u};

        AEADHsmParam.keyID       = AESkeyID;
        AEADHsmParam.input       = (uint32_t)input;
        AEADHsmParam.input_size  = sizeof(input);
        AEADHsmParam.output      = (uint32_t)AesAeadCipher;
        AEADHsmParam.output_size = &AesAeadCipherSize;
        AEADHsmParam.iv          = (uint32_t)s_GcmIv;
        AEADHsmParam.iv_size     = sizeof(s_GcmIv);
        AEADHsmParam.alg         = kPermitted_GCM;
        AEADHsmParam.mode        = kHSM_Encrypt;

        /* AES-GCM Encrypt */
        if (ELE_Aead(S3MU, cipherHandleID, &AEADHsmParam, (uint32_t)s_GcmAad, sizeof(s_GcmAad), kHSM_IV_User) ==
            kStatus_Success)
        {
            PRINTF("Output size returned by AES-AEAD encryption is : 0x%x\r\n", AesAeadCipherSize);
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        AEADHsmParam.input       = (uint32_t)AesAeadCipher;
        AEADHsmParam.input_size  = AesAeadCipherSize;
        AEADHsmParam.output      = (uint32_t)AesAeadDecrypted;
        AEADHsmParam.output_size = &AesAeadDecryptedSize;
        AEADHsmParam.mode        = kHSM_Decrypt;

        /* AES-GCM Decrypt */
        if (ELE_Aead(S3MU, cipherHandleID, &AEADHsmParam, (uint32_t)s_GcmAad, sizeof(s_GcmAad), kHSM_IV_User) ==
            kStatus_Success)
        {
            /* Check output AES data */
            if (memcmp(input, AesAeadDecrypted, inputSize) == 0)
            {
                PRINTF("AES-AEAD crypto encryption and decryption using keys in ELE success.\r\n\r\n");
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

        /****************** Open MAC service ***********************/
        PRINTF("****************** Open MAC service ***********************\r\n");
        if (ELE_OpenMacService(S3MU, keyStoreHandleID, &macHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Open MAC service successfully. Handle ID: 0x%x\r\n\r\n", macHandleID);
        }

        /****************** Key Generate (HMAC_SHA256) ***********************/
        PRINTF("****************** Key Generate (HMAC_SHA256) *************\r\n");

        ele_gen_key_t MACkey = {0u};

        MACkey.key_type      = kKeyType_HMAC;
        MACkey.key_lifetime  = kKey_Volatile;
        MACkey.key_usage     = kKeyUsage_SignMessage | kKeyUsage_VerifyMessage;
        MACkey.key_size      = kKeySize_HMAC_256;
        MACkey.permitted_alg = kPermitted_HMAC_SHA256;
        MACkey.pub_key_addr  = 0u;
        MACkey.pub_key_size  = 0u;
        MACkey.key_group     = 42u;
        MACkey.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &MACkey, &MACkeyID, &keySize, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key for HMAC_SHA256 generated successfully. Key ID: 0x%x\r\n\r\n", MACkeyID);
        }
        /****************** HMAC_SHA256  ***********************/
        PRINTF("****************** HMAC_SHA256 ****************************\r\n");

        /* Buffer for generated MAC */
        SDK_ALIGN(uint8_t GeneratedMAC[32], 8u) = {0u};
        uint32_t GeneratedMACSize               = sizeof(GeneratedMAC);
        uint32_t verifyStatus                   = 0u;
        uint16_t macSize                        = 0u;

        ele_mac_t MACgenerateParam = {0u};

        MACgenerateParam.mac_handle_id = macHandleID;
        MACgenerateParam.key_id        = MACkeyID;
        MACgenerateParam.payload       = (uint32_t)message;
        MACgenerateParam.payload_size  = sizeof(message);
        MACgenerateParam.mac           = (uint32_t)GeneratedMAC;
        MACgenerateParam.mac_size      = GeneratedMACSize;
        MACgenerateParam.alg           = kPermitted_HMAC_SHA256;
        MACgenerateParam.mode          = kMAC_Generate;

        /* Check that response corresponds to the sent command */

        /* Run ELE_Mac in kMAC_Generate mode to  generate MAC from message and store it at GeneratedMAC   */
        if (ELE_Mac(S3MU, &MACgenerateParam, NULL, &macSize) == kStatus_Success)
        {
            PRINTF("HMAC_SHA256 has been generated successfully\r\n");
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /* Verify */
        ele_mac_t MACverifyParam = {0u};

        MACverifyParam.mac_handle_id = macHandleID;
        MACverifyParam.key_id        = MACkeyID;
        MACverifyParam.payload       = (uint32_t)message;
        MACverifyParam.payload_size  = sizeof(message);
        MACverifyParam.mac           = (uint32_t)GeneratedMAC;
        MACverifyParam.mac_size      = GeneratedMACSize;
        MACverifyParam.alg           = kPermitted_HMAC_SHA256;
        MACverifyParam.mode          = kMAC_Verify;

        /* Run ELE_Mac in kMAC_Verify to compute MAC from message and compare it with GeneratedMAC  */
        if (ELE_Mac(S3MU, &MACverifyParam, &verifyStatus, &macSize) == kStatus_Success)
        {
            if (verifyStatus == MAC_VERIFY_SUCCESS)
            {
                PRINTF("HMAC_SHA256 has been verified successfully\r\n\r\n");
            }
            else
            {
                PRINTF("HMAC_SHA256 verify operation failed\r\n\r\n");
                result = kStatus_Fail;
            }
        }
        else
        {
            result = kStatus_Fail;
            break;
        }

        /****************** Key Generate (ECC NIST P256) ***********************/
        PRINTF("****************** Key Generate (ECC NIST P256) ***********\r\n");
        /* Output buffer for public key */
        SDK_ALIGN(uint8_t pubKeyNIST[64u], 8u) = {0u};
        ele_gen_key_t NISTkeyGenParam          = {0u};

        NISTkeyGenParam.key_type      = kKeyType_ECC_KEY_PAIR_SECP_R1_NIST;
        NISTkeyGenParam.key_lifetime  = kKey_Volatile;
        NISTkeyGenParam.key_lifecycle = kKeylifecycle_Open;
        NISTkeyGenParam.key_usage     = kKeyUsage_SignMessage;
        NISTkeyGenParam.key_size      = 256u;
        NISTkeyGenParam.permitted_alg = kPermitted_ECDSA_SHA256;
        NISTkeyGenParam.pub_key_addr  = (uint32_t)pubKeyNIST;
        NISTkeyGenParam.pub_key_size  = sizeof(pubKeyNIST);
        NISTkeyGenParam.key_group     = 42u;
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
        ele_sign_t NISTsignGenParam               = {0u};

        NISTsignGenParam.key_id     = NISTkeyPairID;
        NISTsignGenParam.msg        = (const uint8_t *)message;
        NISTsignGenParam.msg_size   = length;
        NISTsignGenParam.signature  = signatureNIST;
        NISTsignGenParam.sig_size   = sizeof(signatureNIST);
        NISTsignGenParam.scheme     = kSig_ECDSA_SHA256;
        NISTsignGenParam.input_flag = true; // Actual message as input

        if (ELE_Sign(S3MU, signHandleID, &NISTsignGenParam, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Signature generated successfully.\r\n\r\n");
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
        bool result_nist             = false;
        ele_verify_t NISTverifyParam = {0u};

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

        /****************** Key Generate (ECC Brainpool) *********************/
        PRINTF("****************** Key Generate (ECC Brainpool) ***********\r\n");
        /* Output buffer for public key */
        SDK_ALIGN(uint8_t pubKeyBrainpool[96u], 8u) = {0u};
        ele_gen_key_t BrainpoolKeyGenParam          = {0u};

        BrainpoolKeyGenParam.key_type      = kKeyType_ECC_KEY_PAIR_BRAINPOOL_R1;
        BrainpoolKeyGenParam.key_lifetime  = kKey_Volatile;
        BrainpoolKeyGenParam.key_lifecycle = kKeylifecycle_Open;
        BrainpoolKeyGenParam.key_usage     = kKeyUsage_SignMessage;
        BrainpoolKeyGenParam.key_size      = kKeySize_ECC_KEY_PAIR_BRAINPOOL_R1_384;
        BrainpoolKeyGenParam.permitted_alg = kPermitted_ECDSA_SHA384;
        BrainpoolKeyGenParam.pub_key_addr  = (uint32_t)pubKeyBrainpool;
        BrainpoolKeyGenParam.pub_key_size  = sizeof(pubKeyBrainpool);
        BrainpoolKeyGenParam.key_group     = 42u;
        BrainpoolKeyGenParam.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &BrainpoolKeyGenParam, &brainpoolKeyPairID, &keySize, false, false) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key generated successfully. Key Pair ID: 0x%x\r\n\r\n", brainpoolKeyPairID);
        }

        /****************** ECC Brainpool sign ******************************/
        PRINTF("****************** ECC Brainpool sign *********************\r\n");
        /* Output buffer for signature */
        SDK_ALIGN(uint8_t signatureBrainpool[96u], 8u) = {0u};
        ele_sign_t BrainpoolSignGenParam               = {0u};

        BrainpoolSignGenParam.key_id     = brainpoolKeyPairID;
        BrainpoolSignGenParam.msg        = (const uint8_t *)message;
        BrainpoolSignGenParam.msg_size   = length;
        BrainpoolSignGenParam.signature  = signatureBrainpool;
        BrainpoolSignGenParam.sig_size   = sizeof(signatureBrainpool);
        BrainpoolSignGenParam.scheme     = kSig_ECDSA_SHA384;
        BrainpoolSignGenParam.input_flag = true; // Actual message as input

        if (ELE_Sign(S3MU, signHandleID, &BrainpoolSignGenParam, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Signature generated successfully.\r\n\r\n");
        }

        /****************** ECC Brainpool verify ****************************/
        PRINTF("****************** ECC Brainpool verify *******************\r\n");
        bool result_brainpool             = false;
        ele_verify_t BrainpoolVerifyParam = {0u};

        BrainpoolVerifyParam.pub_key           = (const uint8_t *)pubKeyBrainpool;
        BrainpoolVerifyParam.key_size          = sizeof(pubKeyBrainpool);
        BrainpoolVerifyParam.msg               = (const uint8_t *)message;
        BrainpoolVerifyParam.msg_size          = length;
        BrainpoolVerifyParam.signature         = signatureBrainpool;
        BrainpoolVerifyParam.sig_size          = sizeof(signatureBrainpool);
        BrainpoolVerifyParam.keypair_type      = kKeyType_ECC_PUB_KEY_BRAINPOOL_R1;
        BrainpoolVerifyParam.scheme            = kSig_ECDSA_SHA384;
        BrainpoolVerifyParam.input_flag        = true; // Actual message as input
        BrainpoolVerifyParam.key_security_size = kKeySize_ECC_KEY_PAIR_BRAINPOOL_R1_384;
        BrainpoolVerifyParam.internal          = false;

        if (ELE_Verify(S3MU, verifyHandleID, &BrainpoolVerifyParam, &result_brainpool) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (result_brainpool)
            {
                PRINTF("Brainpool Signature verified successfully!\r\n\r\n");
            }
        }

        /****************** Key Delete (ECC Brainpool) ***********************/
        PRINTF("****************** Key Delete (ECC Brainpool) *************\r\n");
        if (ELE_DeleteKey(S3MU, keyHandleID, brainpoolKeyPairID, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key deleted successfully. Key Pair ID: 0x%x\r\n\r\n", brainpoolKeyPairID);
        }

        /****************** Key Generate (RSA PKCS1 V1.5) ********************/
        PRINTF("****************** Key Generate (RSA PKCS1 V1.5) **********\r\n");
        /* Output buffer for public key */
        SDK_ALIGN(uint8_t pubKeyRSA[256u], 8u) = {0u};
        ele_gen_key_t RSAkeyGenParam           = {0u};

        RSAkeyGenParam.key_type      = kKeyType_RSA_KEY_PAIR;
        RSAkeyGenParam.key_lifetime  = kKey_Volatile;
        RSAkeyGenParam.key_lifecycle = kKeylifecycle_Open;
        RSAkeyGenParam.key_usage     = kKeyUsage_SignMessage;
        RSAkeyGenParam.key_size      = kKeySize_RSA_KEY_PAIR_2048;
        RSAkeyGenParam.permitted_alg = kPermitted_RSA_PKCS1_V1_5_SHA256;
        RSAkeyGenParam.pub_key_addr  = (uint32_t)pubKeyRSA;
        RSAkeyGenParam.pub_key_size  = sizeof(pubKeyRSA);
        RSAkeyGenParam.key_group     = 42u;
        RSAkeyGenParam.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &RSAkeyGenParam, &RSAkeyPairID, &keySize, false, false) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key generated successfully. Key Pair ID: 0x%x\r\n\r\n", RSAkeyPairID);
        }

        /****************** RSA PKCS1 V1.5 sign ******************************/
        PRINTF("****************** RSA PKCS1 V1.5 sign ********************\r\n");
        /* Output buffer for signature */
        SDK_ALIGN(uint8_t signatureRSA[256u], 8u) = {0u};
        ele_sign_t RSAsignGenParam                = {0u};

        RSAsignGenParam.key_id     = RSAkeyPairID;
        RSAsignGenParam.msg        = (const uint8_t *)message;
        RSAsignGenParam.msg_size   = length;
        RSAsignGenParam.signature  = signatureRSA;
        RSAsignGenParam.sig_size   = sizeof(signatureRSA);
        RSAsignGenParam.scheme     = kSig_RSA_PKCS1_V1_5_SHA256;
        RSAsignGenParam.input_flag = true; // Actual message as input
        RSAsignGenParam.salt_size  = 0u;

        if (ELE_Sign(S3MU, signHandleID, &RSAsignGenParam, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Signature generated successfully.\r\n\r\n");
        }

        /****************** RSA PKCS1 V1.5 verify ****************************/
        PRINTF("****************** RSA PKCS1 V1.5 verify ******************\r\n");
        bool result_rsa             = false;
        ele_verify_t RSAverifyParam = {0u};

        RSAverifyParam.pub_key           = (const uint8_t *)pubKeyRSA;
        RSAverifyParam.key_size          = sizeof(pubKeyRSA);
        RSAverifyParam.msg               = (const uint8_t *)message;
        RSAverifyParam.msg_size          = length;
        RSAverifyParam.signature         = signatureRSA;
        RSAverifyParam.sig_size          = sizeof(signatureRSA);
        RSAverifyParam.keypair_type      = kKeyType_RSA_PUB_KEY;
        RSAverifyParam.scheme            = kSig_RSA_PKCS1_V1_5_SHA256;
        RSAverifyParam.input_flag        = true; // Actual message as input
        RSAverifyParam.key_security_size = kKeySize_RSA_KEY_PAIR_2048;
        RSAverifyParam.internal          = false;
        RSAverifyParam.salt_size         = 0u;

        if (ELE_Verify(S3MU, verifyHandleID, &RSAverifyParam, &result_rsa) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (result_rsa)
            {
                PRINTF("RSA Signature verified successfully!\r\n\r\n");
            }
        }

        /****************** Key Delete (RSA PKCS1 V1.5) **********************/
        PRINTF("****************** Key Delete (RSA PKCS1 V1.5) ************\r\n");
        if (ELE_DeleteKey(S3MU, keyHandleID, RSAkeyPairID, false, false) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key deleted successfully. Key Pair ID: 0x%x\r\n\r\n", RSAkeyPairID);
        }

        /****************** Key Generate (RSA PSS) ***************************/
        PRINTF("****************** Key Generate (RSA PSS) *****************\r\n");
        RSAkeyGenParam.key_type      = kKeyType_RSA_KEY_PAIR;
        RSAkeyGenParam.key_lifetime  = kKey_Volatile;
        RSAkeyGenParam.key_lifecycle = kKeylifecycle_Open;
        RSAkeyGenParam.key_usage     = kKeyUsage_SignMessage;
        RSAkeyGenParam.key_size      = kKeySize_RSA_KEY_PAIR_2048;
        RSAkeyGenParam.permitted_alg = kPermitted_RSA_PKCS1_PSS_MGF1_SHA256;
        RSAkeyGenParam.pub_key_addr  = (uint32_t)pubKeyRSA;
        RSAkeyGenParam.pub_key_size  = sizeof(pubKeyRSA);
        RSAkeyGenParam.key_group     = 42u;
        RSAkeyGenParam.key_id        = 0u;

        if (ELE_GenerateKey(S3MU, keyHandleID, &RSAkeyGenParam, &RSAkeyPairID, &keySize, false, false) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Key generated successfully. Key Pair ID: 0x%x\r\n\r\n", RSAkeyPairID);
        }

        /****************** RSA PSS sign ******************************/
        PRINTF("****************** RSA PSS sign ***************************\r\n");
        const uint16_t salt_size = 20u;

        RSAsignGenParam.key_id     = RSAkeyPairID;
        RSAsignGenParam.msg        = (const uint8_t *)message;
        RSAsignGenParam.msg_size   = length;
        RSAsignGenParam.signature  = signatureRSA;
        RSAsignGenParam.sig_size   = sizeof(signatureRSA);
        RSAsignGenParam.scheme     = kSig_RSA_PKCS1_PSS_MGF1_SHA256;
        RSAsignGenParam.input_flag = true;
        RSAsignGenParam.salt_size  = salt_size;

        if (ELE_Sign(S3MU, signHandleID, &RSAsignGenParam, &signatureSize) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Signature generated successfully.\r\n\r\n");
        }

        /****************** RSA PSS verify ***********************************/
        PRINTF("****************** RSA PSS verify *************************\r\n");
        RSAverifyParam.pub_key           = (const uint8_t *)pubKeyRSA;
        RSAverifyParam.key_size          = sizeof(pubKeyRSA);
        RSAverifyParam.msg               = (const uint8_t *)message;
        RSAverifyParam.msg_size          = length;
        RSAverifyParam.signature         = signatureRSA;
        RSAverifyParam.sig_size          = sizeof(signatureRSA);
        RSAverifyParam.keypair_type      = kKeyType_RSA_PUB_KEY;
        RSAverifyParam.scheme            = kSig_RSA_PKCS1_PSS_MGF1_SHA256;
        RSAverifyParam.input_flag        = true;
        RSAverifyParam.key_security_size = kKeySize_RSA_KEY_PAIR_2048;
        RSAverifyParam.internal          = false;
        RSAverifyParam.salt_size         = salt_size;

        if (ELE_Verify(S3MU, verifyHandleID, &RSAverifyParam, &result_rsa) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            if (result_rsa)
            {
                PRINTF("RSA Signature verified successfully!\r\n\r\n");
            }
        }

        /****************** Cleanup ********************************************/

        /****************** Close Verify service *******************************/
        PRINTF("****************** Close Verify service *******************\r\n");
        if (ELE_CloseVerifyService(S3MU, verifyHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Verify service close successfully.\r\n\r\n");
        }

        /****************** Close Sign service *********************************/
        PRINTF("****************** Close Sign service *********************\r\n");
        if (ELE_CloseSignService(S3MU, signHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Sign service close successfully.\r\n\r\n");
        }

        /****************** Close cipher service ***********************/
        PRINTF("****************** Close cipher service *******************\r\n");
        if (ELE_CloseCipherService(S3MU, cipherHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Cipher service closed successfully.\r\n\r\n");
        }

        /****************** Close MAC service ***********************/
        PRINTF("****************** Close MAC service **********************\r\n");
        if (ELE_CloseMacService(S3MU, macHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("MAC service closed successfully.\r\n\r\n");
        }

        /****************** Key Management Close *******************************/
        PRINTF("****************** Key Management Close *******************\r\n");
        if (ELE_CloseKeyService(S3MU, keyHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Close Key management service successfully.\r\n\r\n");
        }

        /****************** Close Key Store  ***********************************/
        PRINTF("****************** Close Key Store ************************\r\n");
        if (ELE_CloseKeystore(S3MU, keyStoreHandleID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Close Key Store successfully.\r\n\r\n");
        }

        /****************** Close EdgeLock session *****************************/
        PRINTF("****************** Close EdgeLock session *****************\r\n");
        if (ELE_CloseSession(S3MU, sessionID) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Close session successfully.\r\n\r\n");
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
