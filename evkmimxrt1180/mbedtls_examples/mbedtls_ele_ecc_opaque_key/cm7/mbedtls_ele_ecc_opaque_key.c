/*
 *  Benchmark demonstration program
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *  Copyright 2017, 2021, 2022 NXP. Not a Contribution
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)

#if defined(FREESCALE_KSDK_BM)

#include "mbedtls/version.h"
#include <stdio.h>
#include "fsl_debug_console.h"
#include "fsl_clock.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if defined(MBEDTLS_MCUX_ELE_S400_API) && defined(NXP_ELE_ECC_OPAQUE_KEY)
#include "ecc_opaque/ele_mbedtls.h"
#else
#error "No port layer"
#endif

#define mbedtls_printf PRINTF
#define mbedtls_snprintf snprintf
#define MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED -0x0072 /**< The requested feature is not supported by the platform */
#define mbedtls_exit(x) \
    do                  \
    {                   \
    } while (1)
#define mbedtls_free free
#define fflush(x) \
    do            \
    {             \
    } while (0)

#else
#include "mbedtls/platform.h"
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_exit       exit
#define mbedtls_printf     printf
#define mbedtls_snprintf   snprintf
#define mbedtls_free       free
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif



#include <string.h>
#include <stdlib.h>

#include "mbedtls/timing.h"

#include "mbedtls/ecdsa.h"
#include "mbedtls/pk.h"
#include "mbedtls/md.h"


#include "mbedtls/error.h"

#include "fsl_cache.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CpuClk)


#define BUFSIZE         1024

#if defined(MBEDTLS_ERROR_C)
#define PRINT_ERROR                                                     \
        mbedtls_strerror( ret, ( char * )tmp, sizeof( tmp ) );          \
        mbedtls_printf( "FAILED: %s\n", tmp );
#else
#define PRINT_ERROR                                                     \
        mbedtls_printf( "FAILED: -0x%04x\n", (unsigned int) -ret );
#endif

#define HASH_SIZE 32u /* SHA-256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern ele_ctx_t g_ele_ctx; /* Global context */

/*******************************************************************************
 * Code
 ******************************************************************************/

/* NXP: Move buffer to NON-CACHED memory because of HW accel */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    AT_NONCACHEABLE_SECTION_INIT(static unsigned char buf[BUFSIZE]);
#else
    unsigned char buf[BUFSIZE];
#endif /* DCACHE */



int main( int argc, char *argv[] )
{
    unsigned char sig[200]; /* 64 bytes for ECDSA P256 signature + ASN.1 encoding (72B) */

    uint8_t ECDSAkeyblobBuffer[sizeof(mbedtls_ele_chunks_t)];

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    if( CRYPTO_InitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Initialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }
    mbedtls_printf( "Initialization of crypto HW Success\n" );

    mbedtls_printf( "\n" );

    mbedtls_ecdsa_context ecdsa;
    mbedtls_pk_context pkey;
    size_t sig_len;

    memset( buf, 0x2A, sizeof( buf ) );

    g_ele_ctx.key_store_id    = ELE_KEYSTORE_ID;
    g_ele_ctx.key_store_nonce = ELE_KEYSTORE_NONCE;
    g_ele_ctx.key_group_id    = ELE_KEYGROUP_ID;


    if( !mbedtls_ecdsa_can_do(MBEDTLS_ECP_DP_SECP256R1) )
        mbedtls_exit( 1 );

    /* Init ECDSA CTX */
    mbedtls_ecdsa_init( &ecdsa );

    /* Generate ECDSA Key inside ELE and export into global structure (this can be done only once during PoR) */
    mbedtls_printf( "Generating ECC keypair inside ELE " );
    if( mbedtls_ecdsa_genkey( &ecdsa, MBEDTLS_ECP_DP_SECP256R1, NULL, NULL ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Sign with generated key */
    mbedtls_printf( "Sign message with ECC private key inside ELE " );
    if( mbedtls_ecdsa_write_signature( &ecdsa, MBEDTLS_MD_SHA256, buf, HASH_SIZE,
                                        sig, &sig_len, NULL, NULL ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Verify */
    mbedtls_printf( "Verify signature with ECC public key " );
    if( mbedtls_ecdsa_read_signature( &ecdsa, buf, HASH_SIZE , sig, sig_len ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Write ELE Keyblob into ECDSAkeyblobBuffer */
    mbedtls_printf( "Writing keyblob into buffer" );
    mbedtls_pk_write_key_der(&pkey, ECDSAkeyblobBuffer, sizeof(mbedtls_ele_chunks_t));
    mbedtls_printf( "- *Success*\n\n" );

    /* Deinit and close sessions */
    mbedtls_printf( "Deinit of ELE (Close sessions, services and keystore)" );
    if( CRYPTO_DeinitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Deinitialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }
    mbedtls_printf( "- *Success*\n\n" );

    /* Clear global structure to act like after PoR */
    memset((void*)&g_ele_ctx, 0u, sizeof(ele_ctx_t));

    /* Init again and open new session */
    mbedtls_printf( "Initialization of crypto HW" );
    if( CRYPTO_InitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Initialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }
    mbedtls_printf( "- *Success*\n\n" );

    g_ele_ctx.key_store_id    = ELE_KEYSTORE_ID;
    g_ele_ctx.key_store_nonce = ELE_KEYSTORE_NONCE;
    g_ele_ctx.key_group_id    = ELE_KEYGROUP_ID;

    /* PK Parse ELE keyblob and load it into ELE */
    mbedtls_printf( "Read encrypted keyblob into ELE and open keystore and reconstruct Public key" );
    if( mbedtls_pk_parse_key(&pkey, ECDSAkeyblobBuffer, sizeof(mbedtls_ele_chunks_t), NULL, 0u) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Sign with loaded key */
    mbedtls_printf( "Sign message with ECC private key loaded back inside ELE " );
    if( mbedtls_ecdsa_write_signature((mbedtls_ecdsa_context*)pkey.pk_ctx, MBEDTLS_MD_SHA256, buf, HASH_SIZE,
                                        sig, &sig_len, NULL, NULL ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Verify */
    mbedtls_printf( "Verify signature with ECC public key reconstructed into CTX " );
    if( mbedtls_ecdsa_read_signature((mbedtls_ecdsa_context*)pkey.pk_ctx, buf, HASH_SIZE, sig, sig_len ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Cleanup and deinit */
    mbedtls_ecdsa_free( &ecdsa );
    mbedtls_pk_free( &pkey );

    /******************************** HMAC *************************************/
    const mbedtls_md_info_t *md_info;

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    /* HMAC plaintext key */
    SDK_ALIGN(uint8_t MACkey[32], 8u) = {0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61,
                                         0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61, 0x61};
    /* Message for HMAC */
    SDK_ALIGN(unsigned const char MACmessage[], 8u) =  "Hello World!";
    /* Message size */
    size_t length = sizeof(MACmessage) - 1u;
    /* Output buffer */
    SDK_ALIGN(static uint8_t output[32], 8u);
    /* Expected HMAC for the message. */
    static const uint8_t hmac256[32u] = {0x62, 0xe2, 0x6a, 0x00, 0xa2, 0x85, 0x5f, 0xd7, 0xf6, 0x19, 0x2c, 0x79, 0xbc, 0x0b, 0x17, 0xe3,
                                         0x23, 0x4c, 0x42, 0xcb, 0x8f, 0x12, 0x34, 0x0c, 0x5b, 0xc9, 0x77, 0x4d, 0xb1, 0x5b, 0xe3, 0x2f};

    /* Compute HMAC */
    mbedtls_printf( "Compute HMAC with plaintext key " );
    if( mbedtls_md_hmac(md_info, MACkey, sizeof(MACkey), MACmessage, length, output) != 0u)
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    mbedtls_printf( "Compare expected and generated HMAC " );

    /* Check output Hash digest data */
    if (memcmp(output, hmac256, length) != 0)
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Deinit and close sessions */
    if( CRYPTO_DeinitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Deinitialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }

    mbedtls_printf( "\n End of example\n" );


    while (1)
    {
        char ch = GETCHAR();
        PUTCHAR(ch);
    }
}

