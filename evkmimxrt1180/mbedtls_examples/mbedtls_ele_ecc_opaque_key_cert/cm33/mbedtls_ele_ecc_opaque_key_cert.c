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

#if defined(MBEDTLS_MCUX_ELE_S400_API)
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
#include "mbedtls/x509_crt.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"

#include "mbedtls/error.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CpuClk)
/* Generate keypair and keystore on each run, as it's silicon specific */
#define APP_GENERATE_NEW_KEYPAIR (1u)

#if defined(MBEDTLS_ERROR_C)
#define PRINT_ERROR                                                     \
        mbedtls_strerror( ret, ( char * )tmp, sizeof( tmp ) );          \
        mbedtls_printf( "FAILED: %s\n", tmp );
#else
#define PRINT_ERROR                                                     \
        mbedtls_printf( "FAILED: -0x%04x\n", (unsigned int) -ret );
#endif

#define HASH_SIZE 32u /* SHA-256 */

#define DFL_ISSUER_CRT          ""
#define DFL_REQUEST_FILE        ""
#define DFL_SUBJECT_KEY         "subject.key"
#define DFL_ISSUER_KEY          "ca.key"
#define DFL_SUBJECT_PWD         ""
#define DFL_ISSUER_PWD          ""
#define DFL_OUTPUT_FILENAME     "cert.crt"
#define DFL_SUBJECT_NAME        "CN=Cert,O=mbed TLS,C=UK"
#define DFL_ISSUER_NAME         "CN=CA,O=mbed TLS,C=UK"
#define DFL_NOT_BEFORE          "20010101000000"
#define DFL_NOT_AFTER           "20301231235959"
#define DFL_SERIAL              "1"
#define DFL_SELFSIGN            0
#define DFL_IS_CA               0
#define DFL_MAX_PATHLEN         -1
#define DFL_KEY_USAGE           0
#define DFL_NS_CERT_TYPE        0
#define DFL_VERSION             3
#define DFL_AUTH_IDENT          1
#define DFL_SUBJ_IDENT          1
#define DFL_CONSTRAINTS         1
#define DFL_DIGEST              MBEDTLS_MD_SHA256

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern ele_ctx_t g_ele_ctx; /* Global context */

/* Keyblob with encrypted key material and metadata specific for earch silicon. */
/* If APP_GENERATE_NEW_KEYPAIR=1, new keypair and keystore is created           */
/* User can ectract keyblob from mbedtls_pk_write_key_der() and set in this buf */
uint8_t ECDSAkeyblobBuffer[sizeof(mbedtls_ele_chunks_t)] =
#if defined(APP_GENERATE_NEW_KEYPAIR)
{0u};
#elif !defined(APP_GENERATE_NEW_KEYPAIR) /* SoC specific test vector */
{
0x00, 0x00, 0x00, 0x00, 0xf7, 0x19, 0xb3, 0xf8, 0xca, 0xcc, 0xe1, 0x8e, 0x80, 0xe2,
0x0e, 0x35, 0x58, 0x48, 0x52, 0x0f, 0x3f, 0x6c, 0x32, 0x0f, 0xcb, 0x75, 0x40, 0xda,
0x57, 0x2d, 0x09, 0x2e, 0xc4, 0x1b, 0x74, 0x2c, 0x2b, 0x69, 0x15, 0xc1, 0x0f, 0x08,
0xa8, 0xe7, 0xdf, 0x67, 0xad, 0xcd, 0x5f, 0x4c, 0x4e, 0xd7, 0x1a, 0xda, 0x52, 0xfd,
0x24, 0x13, 0x2e, 0x43, 0xbd, 0xb0, 0x3c, 0x42, 0xf1, 0x39, 0xee, 0x5d, 0x80, 0xdd,
0x49, 0x93, 0xf3, 0x1f, 0x9f, 0xc5, 0x04, 0x98, 0x42, 0x46, 0x8d, 0x1a, 0x45, 0x5d,
0x79, 0x07, 0xc3, 0xa5, 0x7e, 0x97, 0x3f, 0x8f, 0x01, 0xc9, 0xd9, 0x8c, 0xb8, 0x00,
0x00, 0x20, 0x18, 0x01, 0x00, 0x20, 0x5c, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x74, 0xc0, 0x7a, 0xcd, 0x22, 0x8e, 0xd3, 0x0c, 0x4a, 0x43,
0x2d, 0x3d, 0x3c, 0x89, 0x8d, 0xf5, 0xb4, 0x3e, 0x80, 0x04, 0x2f, 0x86, 0x07, 0x3a,
0x60, 0xb0, 0xc9, 0x34, 0xde, 0xef, 0xc2, 0xaa, 0x16, 0x8a, 0x53, 0xdc, 0x0b, 0x2b,
0x0a, 0x86, 0x82, 0xd5, 0x42, 0x15, 0x1e, 0x9a, 0x65, 0x46, 0x4f, 0x56, 0x7d, 0xa4,
0xab, 0x84, 0x98, 0x80, 0x68, 0xdf, 0x98, 0xf9, 0x7b, 0x56, 0x86, 0x7f, 0x7b, 0x5d,
0xe0, 0x20, 0xe8, 0x39, 0x6b, 0x35, 0xc5, 0x56, 0xf9, 0xcd, 0x3f, 0xc7, 0x74, 0xbd,
0xab, 0x82, 0x4b, 0x9b, 0x38, 0xc4, 0xa8, 0xea, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x66, 0x5b, 0x27, 0x40, 0x56, 0xe6, 0xbf, 0xb3, 0x12, 0x45, 0x4a, 0xb1,
0x97, 0x27, 0x3c, 0xfe, 0xf6, 0x07, 0xcb, 0xd7, 0x35, 0xc9, 0x15, 0xca, 0x2d, 0x18,
0x9e, 0xc0, 0x9b, 0x45, 0xff, 0xe1, 0x95, 0xe8, 0x60, 0xaa, 0x5b, 0x15, 0x65, 0x05,
0x85, 0xc5, 0x14, 0x3e, 0x53, 0xe6, 0xfe, 0x91, 0x94, 0x57, 0x2d, 0x84, 0xa2, 0xf4,
0x25, 0x0f, 0x31, 0xea, 0xf4, 0x0e, 0x30, 0xd0, 0xfa, 0x61, 0x20, 0xaf, 0x3e, 0x3d,
0xc1, 0x26, 0x3a, 0xcf, 0x0e, 0x5f, 0x4b, 0x68, 0x08, 0xca, 0x15, 0x48, 0x35, 0xc6,
0xeb, 0x12, 0x9a, 0x45, 0x9f, 0xc2, 0xdf, 0xf9, 0x64, 0x92, 0xbb, 0x47, 0xe9, 0xc7,
0x9c, 0x74, 0x4d, 0x2a, 0xbe, 0xdd, 0x1a, 0x5d, 0xbf, 0xa3, 0x76, 0x2b, 0x59, 0x72,
0x17, 0xc0, 0xfa, 0x2c, 0x07, 0x9c, 0x70, 0xbe, 0xae, 0x84, 0xb6, 0x89, 0xda, 0x48,
0x9f, 0xdf, 0xf0, 0x8b, 0x82, 0x36, 0x92, 0x8c, 0x6f, 0xbc, 0x1e, 0x11, 0x88, 0xd0,
0xf2, 0x23, 0x64, 0xe4, 0x03, 0x46};
#endif /* APP_GENERATE_NEW_KEYPAIR */

/* Print buffer */
char buf[1024] = {0u};

/*
 * global cert options
 */
struct options {
    const char *subject_name;   /* subject name for certificate         */
    const char *issuer_name;    /* issuer name for certificate          */
    const char *not_before;     /* validity period not before           */
    const char *not_after;      /* validity period not after            */
    const char *serial;         /* serial number string                 */
    int is_ca;                  /* is a CA certificate                  */
    int max_pathlen;            /* maximum CA path length               */
    int authority_identifier;   /* add authority identifier to CRT      */
    int subject_identifier;     /* add subject identifier to CRT        */
    int basic_constraints;      /* add basic constraints ext to CRT     */
    int version;                /* CRT version                          */
    mbedtls_md_type_t md;       /* Hash used for signing                */
    unsigned char key_usage;    /* key usage flags                      */
    unsigned char ns_cert_type; /* NS cert type                         */
} opt;

/*******************************************************************************
 * Code
 ******************************************************************************/


int main( int argc, char *argv[] )
{
    int ret;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    if( CRYPTO_InitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Initialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }
    mbedtls_printf( "Initialization of crypto HW Success\n\n" );

    unsigned char sig[200];   /* 64 bytes for ECDSA P256 signature + ASN.1 encoding (72B) */
    unsigned char digest[32]; /* Dummy data to exercise sign/verif with loaded key */


    mbedtls_pk_context pkey;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_mpi serial;
    size_t sig_len;

    g_ele_ctx.key_store_id    = ELE_KEYSTORE_ID;
    g_ele_ctx.key_store_nonce = ELE_KEYSTORE_NONCE;
    g_ele_ctx.key_group_id    = ELE_KEYGROUP_ID;

    const char *pers = "crt example app";

    memset( digest, 0x2A, sizeof( digest ) );

    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);


    if( !mbedtls_ecdsa_can_do(MBEDTLS_ECP_DP_SECP256R1) )
        mbedtls_exit( 1 );

#if defined(APP_GENERATE_NEW_KEYPAIR)
    mbedtls_ecdsa_context ecdsa;

    /* Init ECDSA CTX */
    mbedtls_ecdsa_init( &ecdsa );

    /* Generate ECDSA Key inside ELE and export into global structure (this can be done only once during PoR) */
    mbedtls_printf( "Generating ECC keypair inside ELE " );
    if( mbedtls_ecdsa_genkey( &ecdsa, MBEDTLS_ECP_DP_SECP256R1, NULL, NULL ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Write ELE Keyblob into ECDSAkeyblobBuffer */
    mbedtls_printf( "Writing keyblob into buffer" );
    mbedtls_pk_write_key_der(&pkey, ECDSAkeyblobBuffer, sizeof(mbedtls_ele_chunks_t));
    mbedtls_printf( "- *Success*\n\n" );

    /** Closing Keystore as mbedtls_pk_parse_key() will try to reopen it **/
    /* Deinit and close sessions */
    if( CRYPTO_DeinitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Deinitialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }

#endif

    /* PK Parse ELE keyblob and load it into ELE */
    mbedtls_printf( "Read encypted keyblob into ELE and open keystore and reconstruct Public key" );
    if( mbedtls_pk_parse_key(&pkey, ECDSAkeyblobBuffer, sizeof(mbedtls_ele_chunks_t), NULL, 0u) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /** Try to use loaded key for sign and verify just to test it's loaded properly **/

    /* Sign with loaded key */
    mbedtls_printf( "Sign message with ECC private key loaded back inside ELE " );
    if( mbedtls_ecdsa_write_signature((mbedtls_ecdsa_context*)pkey.pk_ctx, MBEDTLS_MD_SHA256, digest, HASH_SIZE,
                                        sig, &sig_len, NULL, NULL ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /* Verify */
    mbedtls_printf( "Verify signature with ECC public key reconstructed into CTX " );
    if( mbedtls_ecdsa_read_signature((mbedtls_ecdsa_context*)pkey.pk_ctx, digest, HASH_SIZE, sig, sig_len ) != 0 )
        mbedtls_exit( 1 );
    mbedtls_printf( "- *Success*\n\n" );

    /** Generate self signed certificate with ELE private key **/
    opt.subject_name        = DFL_SUBJECT_NAME;
    opt.issuer_name         = DFL_ISSUER_NAME;
    opt.not_before          = DFL_NOT_BEFORE;
    opt.not_after           = DFL_NOT_AFTER;
    opt.serial              = DFL_SERIAL;
    opt.is_ca               = DFL_IS_CA;
    opt.max_pathlen         = DFL_MAX_PATHLEN;
    opt.key_usage           = DFL_KEY_USAGE;
    opt.ns_cert_type        = DFL_NS_CERT_TYPE;
    opt.version             = DFL_VERSION - 1;
    opt.md                  = DFL_DIGEST;

    mbedtls_x509write_cert crt;

    /* For self-signed cert issuer and subject key is the same */
    mbedtls_pk_context *issuer_key = &pkey,
                       *subject_key = &pkey;
    opt.subject_name = opt.issuer_name;

    mbedtls_x509write_crt_init(&crt);


    mbedtls_x509write_crt_set_subject_key(&crt, subject_key);
    mbedtls_x509write_crt_set_issuer_key(&crt, issuer_key);

    /* Parse serial to MPI */
    mbedtls_printf("  . Reading serial number...");


    if ((ret = mbedtls_mpi_read_string(&serial, 10, opt.serial)) != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_mpi_read_string "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /*
     * 0. Seed the PRNG
     */
    mbedtls_printf("  . Seeding the random number generator...");
    fflush(stdout);

    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_ctr_drbg_seed returned %d - %s\n",
                       ret, buf);
        goto exit;
    }
    mbedtls_printf(" ok\n");

    /*
     * 1.0. Check the names for validity
     */
    if ((ret = mbedtls_x509write_crt_set_subject_name(&crt, opt.subject_name)) != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_subject_name "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    if ((ret = mbedtls_x509write_crt_set_issuer_name(&crt, opt.issuer_name)) != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_issuer_name "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf("  . Setting certificate values ...");

    mbedtls_x509write_crt_set_version(&crt, opt.version);
    mbedtls_x509write_crt_set_md_alg(&crt, opt.md);


    ret = mbedtls_x509write_crt_set_serial(&crt, &serial);
    if (ret != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_serial "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    ret = mbedtls_x509write_crt_set_validity(&crt, opt.not_before, opt.not_after);
    if (ret != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_validity "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");


    mbedtls_printf("  . Adding the Basic Constraints extension ...");

    ret = mbedtls_x509write_crt_set_basic_constraints(&crt, opt.is_ca,
                                                      opt.max_pathlen);
    if (ret != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  x509write_crt_set_basic_constraints "
                       "returned -0x%04x - %s\n\n", (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    mbedtls_printf("  . Adding the Subject Key Identifier ...");

    ret = mbedtls_x509write_crt_set_subject_key_identifier(&crt);
    if (ret != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_subject"
                       "_key_identifier returned -0x%04x - %s\n\n",
                       (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    mbedtls_printf("  . Adding the Authority Key Identifier ...");

    ret = mbedtls_x509write_crt_set_authority_key_identifier(&crt);
    if (ret != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  mbedtls_x509write_crt_set_authority_"
                       "key_identifier returned -0x%04x - %s\n\n",
                       (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");


     /*
     * 1.2. Writing the certificate
     */
    mbedtls_printf("  . Writing the certificate...");

    unsigned char cert_buf[4096];
    size_t cert_len = 0u;
    mbedtls_x509_crt parsed_cert;


    memset(cert_buf, 0, 4096);

    if ((ret = mbedtls_x509write_crt_pem(&crt, cert_buf, 4096,
                                         mbedtls_ctr_drbg_random, &ctr_drbg)) != 0) {
        mbedtls_strerror(ret, buf, 1024);
        mbedtls_printf(" failed\n  !  write_certificate -0x%04x - %s\n\n",
                       (unsigned int) -ret, buf);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /*
     * 1.2. Parsing the certificate
     */

    cert_len = strlen((char const*)cert_buf) + 1;

    PRINTF("  . Loading the certificate ...");
    ret = mbedtls_x509_crt_parse(&parsed_cert, (const unsigned char *)cert_buf,
                                 cert_len);

    if (ret < 0)
    {
        PRINTF(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing root cert\n\n", -ret);
        goto exit;
    }

    mbedtls_printf(" ok\n");

    /* Deinit and close sessions */
    if( CRYPTO_DeinitHardware() != kStatus_Success )
    {
        mbedtls_printf( "Deinitialization of crypto HW failed\n" );
        mbedtls_exit( MBEDTLS_EXIT_FAILURE );
    }

    mbedtls_printf( "\n End of example\n" );

exit:
    mbedtls_pk_free( &pkey );
    mbedtls_x509_crt_free(&parsed_cert);
    mbedtls_x509write_crt_free(&crt);
    mbedtls_mpi_free(&serial);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    while (1)
    {
        char ch = GETCHAR();
        PUTCHAR(ch);
    }
}

