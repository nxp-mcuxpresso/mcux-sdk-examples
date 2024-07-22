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

#include "ele_crypto.h"      /* ELE Crypto SW */
#include "fsl_s3mu.h"        /* Messaging unit driver */
#include "ele_fw_baseline.h" /* ELE FW, to be placed in bootable container in real world app */

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
static uint8_t AES256_key[32u] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22,
                                  0x22, 0x22, 0x22, 0x22, 0x22, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
                                  0xaa, 0xaa, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb};

/* Large enough buffer for any blob type */
SDK_ALIGN(static uint8_t key_blob_output[136u], 8u) = {0u};
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
     * 0.  Initialize ELE services
     * 1.  Get FW status - Check if FW ELE is loaded
     * 2.  Start RNG - Needed for attestation
     * 3.  Get FW version - Check if bit 27 in version is set so we can differentiate between ROM and FW response
     * 4.  load and authenticate EdgeLock Enclave FW (to be done in secure boot flow in real world app)
     * 5.  Get FW version - Check if bit 27 in version is set so we can differentiate between ROM and FW response and
     *     check if bit 27 in version is set so Base or Alternative FW is running
     * 6.  Get FW status - Check if FW ELE is loaded
     * 7.  Read a fuse word and show value of a fuse as an example
     * 8.  Ping ELE
     * 9.  Generate IEE key blob
     * 10. Load IEE key blob
     * 11. Get info from ELE
     * 12. Change ELE clock rate
     * 13. Get device attestation
     * 14. Exercise BBSM ELE services and Get Event
     * Note: This example does not close already opened contexts or objects in case of failed command.
     */

    status_t result = kStatus_Fail;
    uint32_t fwstatus, fwversion, fuse_value, keyID;
    do
    {
        /* HW init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

        PRINTF("EdgeLock Enclave Sub-System crypto base API example:\r\n\r\n");

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

        /****************** FW status  service ***********************/
        PRINTF("****************** Get FW status ELE **********************\r\n");
        if (ELE_GetFwStatus(S3MU, &fwstatus) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get FW status successfully. Status: 0x%x  \r\nThis means ", fwstatus);
            if (fwstatus == 0u)
            {
                PRINTF("no ELE FW in place \r\n\r\n");
            }
            else
            {
                PRINTF("ELE FW is authenticated and operational \r\n\r\n");
            }
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

        /****************** FW version  service ***********************/
        PRINTF("****************** Get FW version ELE *********************\r\n");
        if (ELE_GetFwVersion(S3MU, &fwversion) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get FW version successfully. Version: 0x%x\r\n", fwversion);
            /* Check if bit 27 is set, this can be used to differentiate between ROM and FW*/
            if ((fwversion & 0x08000000) == 0u)
            {
                PRINTF("Bit 27 of fwversion is not set - this means response come from ROM \r\n\r\n");
            }
            else
            {
                PRINTF("Bit 27 of fwversion is set - this means response come from FW \r\n\r\n");
            }
        }

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

        /****************** FW version  service ***********************/
        PRINTF("****************** Get FW version ELE *********************\r\n");
        if (ELE_GetFwVersion(S3MU, &fwversion) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get FW version successfully. Version: 0x%x\r\n", fwversion);
            /* Check if bit 27 is set, this can be used to differentiate between ROM and FW*/
            if ((fwversion & 0x08000000) == 0U)
            {
                PRINTF("Bit 27 is not set - this means response come from ROM \r\n");
            }
            else
            {
                PRINTF("Bit 27 is set - this means response come from FW \r\n");
            }

            /* Check if bit 24 is set, when set it indicate that alternative FW is running */
            if ((fwversion & 0x01000000) == 0U)
            {
                PRINTF("Bit 24 is not set - BASE FW is running \r\n\r\n");
            }
            else
            {
                PRINTF("Bit 24 is set - alternative FW is running \r\n\r\n");
            }
        }

        /****************** FW status  service ***********************/
        PRINTF("****************** Get FW status ELE **********************\r\n");
        if (ELE_GetFwStatus(S3MU, &fwstatus) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get FW status successfully. Status: 0x%x  \r\nThis means ", fwstatus);
            if (fwstatus == 0u)
            {
                PRINTF("no ELE FW in place \r\n\r\n");
            }
            else
            {
                PRINTF("ELE FW is authenticated and operational \r\n\r\n");
            }
        }

        /****************** Read Common Fuse ***********************/
        PRINTF("****************** Read common fuse ***********************\r\n");
        if (ELE_ReadFuse(S3MU, 63u, &fuse_value) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Fuse read successfully. Value of EARLY_FUSES_PGRM is %u.\r\n\r\n",
                   (0x4000u & fuse_value) ? 1u : 0u);
        }

        /****************** Ping ELE ***********************/
        PRINTF("****************** PING ELE *******************************\r\n");
        if (ELE_Ping(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Ping ELE successfully. \r\n\r\n");
        }

        /****************** Generate IEE key blob in ELE *******************/
        PRINTF("****************** Generate IEE Key Blob ******************\r\n");

        keyID = 0x100u;

        generate_key_blob_input_t key_blob_input;
        key_blob_input.algorithm         = kBlob_Algo_AES_XTS;
        key_blob_input.blob_type         = kBlob_Type_IEE;
        key_blob_input.key               = AES256_key;
        key_blob_input.ctr               = NULL;
        key_blob_input.size              = kBlob_Size_256;
        key_blob_input.iee.bypass        = kBlob_IEE_UseModeField;
        key_blob_input.iee.key_size      = kBlob_IEE_AES_CTR128XTS256;
        key_blob_input.iee.lock          = kBlob_IEE_NoLock;
        key_blob_input.iee.mode          = kBlob_IEE_Mode_AES_XTS;
        key_blob_input.iee.page_offset   = 0u;
        key_blob_input.iee.rand_keys     = kBlob_IEE_UseInputKeys;
        key_blob_input.iee.region_number = 1u;

        if (ELE_GenerateKeyBlob(S3MU, keyID, &key_blob_input, key_blob_output, sizeof(key_blob_output)) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("IEE key blob generated successfully.\r\n\r\n");
        }

        /****************** Load IEE Key Blob ***********************/
        PRINTF("****************** Load IEE Key Blob **********************\r\n");

        if (ELE_LoadKeyBlob(S3MU, keyID, key_blob_output) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("IEE key blob loaded successfully.\r\n\r\n");
        }

        /****************** Get info from ELE ***********************/
        PRINTF("****************** Get Info *******************************\r\n");

        uint8_t info[160u] = {0u};
        if (ELE_GetInfo(S3MU, info) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get Info from ELE successfully. \r\n");
            PRINTF("SOC ID number 0x%x%x \r\n\r\n", info[5], info[4]);
        }

        /****************** ELE clock change start ***********************/
        PRINTF("****************** ELE clock change start *****************\r\n");
        if (ELE_ClockChangeStart(S3MU) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("ELE clock change start completed successfully.\r\n\r\n");
        }

        /* Configure EDGELOCK using ROSC400M/4 = 100MHz */
        clock_root_config_t rootCfg = {
            .div      = 4u,
            .mux      = kCLOCK_EDGELOCK_ClockRoot_MuxOscRc400M,
            .clockOff = false,
        };

        CLOCK_SetRootClock(kCLOCK_Root_Edgelock, &rootCfg);

        /****************** ELE clock change finish ***********************/
        PRINTF("****************** ELE clock change finish ****************\r\n");
        if (ELE_ClockChangeFinish(S3MU, 100u, 0u) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("ELE clock change finish completed successfully.\r\n\r\n");
        }

        /* Check actual ELE clock */
        uint32_t ele_clock = CLOCK_GetRootClockFreq(kCLOCK_Root_Edgelock);

        if (ele_clock != 100000000u)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("ELE clock is now 100Mhz as expected.\r\n\r\n");
        }

        /****************** Device attestation ***********************/
        PRINTF("****************** Device attestation *********************\r\n");
        uint8_t attestation[272u] = {0u};
        attestation_nonce_t nonce;
        nonce.nonce_word_1 = 111u;
        nonce.nonce_word_2 = 0u;
        nonce.nonce_word_3 = 0u;
        nonce.nonce_word_4 = 0u;

        if (ELE_Attest(S3MU, nonce, attestation) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Device attestation completed successfully.\r\n\r\n");
        }

        /****************** BBSM (Battery-Backed Security Module) *************/
        PRINTF("****************** BBSM (Battery-Backed Security Module) **\r\n");

        /* For offset/register information, please refer to SRM */
        /* Read and write BBSM registers test */
        if (ELE_WriteBbsm(S3MU, 0x200, 0xCAFECAFE) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("BBSM register write completed successfully.\r\n");
        }

        uint32_t read_value = 0u;

        if (ELE_ReadBbsm(S3MU, 0x200, &read_value) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("BBSM register read success, value=%0x\r\n", read_value);
        }

        /* Get ELE events */
        ele_events_t events = {0u};

        if (ELE_GetEvent(S3MU, &events) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get ELE Events successfully\r\n");
        }

        /* Set ELE Event policy to log event */
        if (ELE_SetPolicyBbsm(S3MU, kELE_BBSM_Alert1_BBSM_log_event | kELE_BBSM_Alert2_BBSM_log_event |
                                        kELE_BBSM_Alert3_BBSM_log_event | kELE_BBSM_Alert4_BBSM_log_event) !=
            kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Set ELE Event policy successfully\r\n");
        }

        /* Important!! Some bit fields may be set/reserved and their override can lead to unexpected behaviour       */
        /* It is recommended to always read the current value first and modify only the required bits before writing */
        /* the new value to the register, unless the entire world is being written.                                  */
        /* For offset/register information, please refer to SRM */
        if (ELE_ReadBbsm(S3MU, 0x8u, &read_value) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (ELE_WriteBbsm(S3MU, 0x8u, read_value | 0x300) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (ELE_WriteBbsm(S3MU, 0x38u, 0xFFFFFFF0) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (ELE_WriteBbsm(S3MU, 0x30u, 0xFF) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        if (ELE_GetEvent(S3MU, &events) != kStatus_Success)
        {
            result = kStatus_Fail;
            break;
        }
        else
        {
            PRINTF("Get ELE Events successfully\r\n");

            if (events.event[0] != 0u)
            {
                PRINTF("ELE Event triggered successfully\r\n\r\n");
            }
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
