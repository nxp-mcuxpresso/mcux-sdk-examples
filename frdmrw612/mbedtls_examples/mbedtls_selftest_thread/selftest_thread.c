/*
 *  Self-test demonstration program
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *  Copyright 2017, 2021-2023 NXP. Not a Contribution
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
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "mbedtls/entropy.h"
#include "mbedtls/entropy_poll.h"
#include "mbedtls/hmac_drbg.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/dhm.h"
#include "mbedtls/gcm.h"
#include "mbedtls/ccm.h"
#include "mbedtls/cmac.h"
#include "mbedtls/md2.h"
#include "mbedtls/md4.h"
#include "mbedtls/md5.h"
#include "mbedtls/ripemd160.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/sha512.h"
#include "mbedtls/arc4.h"
#include "mbedtls/des.h"
#include "mbedtls/aes.h"
#include "mbedtls/camellia.h"
#include "mbedtls/aria.h"
#include "mbedtls/chacha20.h"
#include "mbedtls/poly1305.h"
#include "mbedtls/chachapoly.h"
#include "mbedtls/base64.h"
#include "mbedtls/bignum.h"
#include "mbedtls/rsa.h"
#include "mbedtls/x509.h"
#include "mbedtls/xtea.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/ecp.h"
#include "mbedtls/ecjpake.h"
#include "mbedtls/timing.h"
#include "mbedtls/nist_kw.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/platform.h"

#include <string.h>
#if defined(MBEDTLS_PLATFORM_C)
#if defined(FREESCALE_KSDK_BM)
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_debug_console.h"
#if defined(MBEDTLS_NXP_SSSAPI)
#include "sssapi_mbedtls.h"
#elif defined(MBEDTLS_MCUX_CSS_API)
#include "platform_hw_ip.h"
#include "css_mbedtls.h"
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
#include "platform_hw_ip.h"
#include "css_pkc_mbedtls.h"
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
#include "platform_hw_ip.h"
#include "els_pkc_mbedtls.h"
#elif defined(MBEDTLS_MCUX_ELS_API)
#include "platform_hw_ip.h"
#include "els_mbedtls.h"
#else
#include "ksdk_mbedtls.h"
#endif
#include "mbedtls/version.h"
#define MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED -0x0072 /**< The requested feature is not supported by the platform */
#define return_SUCCESS 0
#define return_FAILURE 1
#else
#include "mbedtls/platform.h"
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_printf     printf
#define mbedtls_snprintf   snprintf
#define mbedtls_exit       exit
#define return_SUCCESS EXIT_SUCCESS
#define return_FAILURE EXIT_FAILURE
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
#include "mbedtls/memory_buffer_alloc.h"
#endif

#if ( !defined(MBEDTLS_THREADING_C) || !defined(MBEDTLS_THREADING_ALT) )
#error "SDK example aims to test mbedTLS threading support. So MBEDTLS_THREADING_C and MBEDTLS_THREADING_ALT macros must be defined together."
#endif

#if ( !defined(MBEDTLS_PLATFORM_MEMORY) )
#error "SDK example aims to test mbedTLS threading support. So MBEDTLS_PLATFORM_MEMORY macro must be defined to enable thread safe implementations of memory allocation functions"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CORE_CLK_FREQ (CLOCK_GetCoreSysClkFreq())
#define TASK_PRIORITY (configMAX_PRIORITIES - 1)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void thread_one(void *param);
static void thread_two(void *param);
mbedtls_threading_mutex_t alloc_mutex;
/*******************************************************************************
 * Variables
 ******************************************************************************/

const int TASK_MAIN_STACK_SIZE = 1500;
portSTACK_TYPE *task_main_stack = NULL;
TaskHandle_t task_main_task_handler;
/*******************************************************************************
 * Code
 ******************************************************************************/

static void *buffer_alloc_calloc_mutexed_custom( size_t n, size_t size )
{
    void *buf = NULL;
    if( mbedtls_mutex_lock( &alloc_mutex ) != 0 )
        return( NULL );
    buf = calloc( n, size );
    if( mbedtls_mutex_unlock( &alloc_mutex ) != 0 )
    {
        mbedtls_free(buf);
        return( NULL );
    }
    return( buf );
}

int free_count = 0;
static void buffer_alloc_free_mutexed_custom( void *ptr )
{
    if( mbedtls_mutex_lock( &alloc_mutex ) != 0 )
        return;
    free( ptr );
    if( mbedtls_mutex_unlock( &alloc_mutex ) != 0 )
        return;

}

static int myrand( void *rng_state, unsigned char *output, size_t len )
{
    size_t use_len = 0;
    int rnd = 0;

    if( rng_state != NULL )
        rng_state  = NULL;
    while( len > 0 )
    {
        use_len = len;
        if( use_len > sizeof(int) )
            use_len = sizeof(int);
        rnd = rand();
        memcpy( output, &rnd, use_len );
        output += use_len;
        len -= use_len;
    }

    return( 0 );
}

#define CHECK_AND_CONTINUE( R )                                         \
    {                                                                   \
        int CHECK_AND_CONTINUE_ret = ( R );                             \
        if( CHECK_AND_CONTINUE_ret == MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED ) { \
            mbedtls_printf( "Feature not supported. Skipping.\n" );     \
            continue;                                                   \
        }                                                               \
        else if( CHECK_AND_CONTINUE_ret != 0 ) {                        \
            return( 1 );                                                \
        }                                                               \
    }

#if RUN_TEST_SNPRINTF
static int test_snprintf( size_t n, const char *ref_buf, int ref_ret )
{
    int ret = 0;
    char buf[10] = "xxxxxxxxx";
    const char ref[10] = "xxxxxxxxx";

    ret = mbedtls_snprintf( buf, n, "%s", "123" );
    if( ret < 0 || (size_t) ret >= n )
        ret = -1;

    if( strncmp( ref_buf, buf, sizeof( buf ) ) != 0 ||
        ref_ret != ret ||
        memcmp( buf + n, ref + n, sizeof( buf ) - n ) != 0 )
    {
        return( 1 );
    }

    return( 0 );
}

static int run_test_snprintf( void )
{
    return( test_snprintf( 0, "xxxxxxxxx",  -1 ) != 0 ||
            test_snprintf( 1, "",           -1 ) != 0 ||
            test_snprintf( 2, "1",          -1 ) != 0 ||
            test_snprintf( 3, "12",         -1 ) != 0 ||
            test_snprintf( 4, "123",         3 ) != 0 ||
            test_snprintf( 5, "123",         3 ) != 0 );
}
#endif /* RUN_TEST_SNPRINTF */

/*
 * Check if a seed file is present, and if not create one for the entropy
 * self-test. If this fails, we attempt the test anyway, so no error is passed
 * back.
 */
#if defined(MBEDTLS_SELF_TEST) && defined(MBEDTLS_ENTROPY_C)
#if defined(MBEDTLS_ENTROPY_NV_SEED) && !defined(MBEDTLS_NO_PLATFORM_ENTROPY)
static void create_entropy_seed_file( void )
{
    int result = 0;
    size_t output_len = 0;
    unsigned char seed_value[MBEDTLS_ENTROPY_BLOCK_SIZE];

    /* Attempt to read the entropy seed file. If this fails - attempt to write
     * to the file to ensure one is present. */
    result = mbedtls_platform_std_nv_seed_read( seed_value,
                                                    MBEDTLS_ENTROPY_BLOCK_SIZE );
    if( 0 == result )
        return;

    result = mbedtls_platform_entropy_poll( NULL,
                                            seed_value,
                                            MBEDTLS_ENTROPY_BLOCK_SIZE,
                                            &output_len );
    if( 0 != result )
        return;

    if( MBEDTLS_ENTROPY_BLOCK_SIZE != output_len )
        return;

    mbedtls_platform_std_nv_seed_write( seed_value, MBEDTLS_ENTROPY_BLOCK_SIZE );
}
#endif

int mbedtls_entropy_self_test_wrapper( int verbose )
{
#if defined(MBEDTLS_ENTROPY_NV_SEED) && !defined(MBEDTLS_NO_PLATFORM_ENTROPY)
    create_entropy_seed_file( );
#endif
    return( mbedtls_entropy_self_test( verbose ) );
}
#endif

#if defined(MBEDTLS_SELF_TEST)
#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
int mbedtls_memory_buffer_alloc_free_and_self_test( int verbose )
{
    if( verbose != 0 )
    {
#if defined(MBEDTLS_MEMORY_DEBUG)
        mbedtls_memory_buffer_alloc_status( );
#endif
    }
    mbedtls_memory_buffer_alloc_free( );
    return( mbedtls_memory_buffer_alloc_self_test( verbose ) );
}
#endif

void ecp_clear_precomputed( mbedtls_ecp_group *grp )
{
    if( grp->T != NULL )
    {
        size_t i;
        for( i = 0; i < grp->T_size; i++ )
            mbedtls_ecp_point_free( &grp->T[i] );
        free( grp->T );
    }
    grp->T = NULL;
    grp->T_size = 0;
}

int mbedtls_ecdh_self_test(int verbose)
{
      int ret = 1;
      mbedtls_ecdh_context ecdh;
      unsigned char buf[1024] = {0}; 

#if defined(MBEDTLS_ECP_C)
    const mbedtls_ecp_curve_info *curve_list = mbedtls_ecp_curve_list( );
#endif
       const mbedtls_ecp_curve_info *curve_info;
       size_t olen;

       curve_info = curve_list;
       if( ! mbedtls_ecdh_can_do( curve_info->grp_id ) )
           goto exit;

       mbedtls_ecdh_init( &ecdh );

       mbedtls_ecp_group_load( &ecdh.grp, curve_info->grp_id );
       mbedtls_ecdh_make_public( &ecdh, &olen, buf, sizeof( buf), myrand, NULL );
       mbedtls_ecp_copy( &ecdh.Qp, &ecdh.Q );
       mbedtls_ecdh_make_public( &ecdh, &olen, buf, sizeof( buf), myrand, NULL );
       ecp_clear_precomputed( &ecdh.grp );

       ret = mbedtls_ecdh_calc_secret( &ecdh, &olen, buf, sizeof( buf ), myrand, NULL );
       mbedtls_ecdh_free( &ecdh );

exit:
       return ret;
}

int mbedtls_ecdsa_self_test(int verbose)
{
      const mbedtls_ecp_curve_info *curve_info;
      mbedtls_ecdsa_context ecdsa;
      size_t sig_len = 0;
      unsigned char buf[1024] = {0};
      int ret = 1;
      unsigned char tmp[200] = {0};
#if defined(MBEDTLS_ECP_C)
      const mbedtls_ecp_curve_info *curve_list = mbedtls_ecp_curve_list( );
#endif
       curve_info = curve_list;
       if( ! mbedtls_ecdsa_can_do( curve_info->grp_id ) )
       {
            ret = 1;
            goto exit;
       }
       mbedtls_ecdsa_init( &ecdsa );
       if( mbedtls_ecdsa_genkey( &ecdsa, curve_info->grp_id, myrand, NULL ) != 0 ||
           mbedtls_ecdsa_write_signature( &ecdsa, MBEDTLS_MD_SHA256, buf, (curve_info->bit_size / 8U),
                                            tmp, &sig_len, myrand, NULL ) != 0 )
       {
            return( 1 );
       }
       ecp_clear_precomputed( &ecdsa.grp );
       ret = mbedtls_ecdsa_read_signature( &ecdsa, buf, (curve_info->bit_size / 8U), tmp, sig_len );
       mbedtls_ecdsa_free( &ecdsa );
exit:
       return ret;
}

typedef struct
{
    const char *name;
    int ( *function )( int );
} selftest_t;

const selftest_t selftests[] =
{
#if defined(MBEDTLS_SHA256_C)
    {"sha256", mbedtls_sha256_self_test},
#endif
#if defined(MBEDTLS_SHA512_C)
    {"sha512", mbedtls_sha512_self_test},
#endif
#if defined(MBEDTLS_AES_C)
    {"aes", mbedtls_aes_self_test},
#endif
#if defined(MBEDTLS_GCM_C) && defined(MBEDTLS_AES_C)
    {"gcm", mbedtls_gcm_self_test},
#endif
#if defined(MBEDTLS_CCM_C) && defined(MBEDTLS_AES_C)
    {"ccm", mbedtls_ccm_self_test},
#endif
#if defined(MBEDTLS_CMAC_C)
    {"cmac", mbedtls_cmac_self_test},
#endif
#if defined(MBEDTLS_RSA_C)
    {"rsa", mbedtls_rsa_self_test},
#endif /* NXP remove CTR_DRBG setftest for CSS PKC since there is different implementation */
#if defined(MBEDTLS_CTR_DRBG_C) && !defined(MBEDTLS_MCUX_CSS_PKC_API) && !defined(MBEDTLS_MCUX_CSS_API) && !defined(MBEDTLS_MCUX_ELS_PKC_API) && !defined(MBEDTLS_ELS_CSS_API)
    {"ctr_drbg", mbedtls_ctr_drbg_self_test},
#endif
#if defined(MBEDTLS_ENTROPY_C)
    {"entropy", mbedtls_entropy_self_test_wrapper},
#endif
#if defined(MBEDTLS_ECDSA_C) && defined(MBEDTLS_SHA256_C)
    {"ecdsa", mbedtls_ecdsa_self_test},
#endif
#if defined(MBEDTLS_ECDH_C)
    {"ecdh", mbedtls_ecdh_self_test},
#endif
    {NULL, NULL}
};
#endif /* MBEDTLS_SELF_TEST */

static int bench_print_features(void)
{
    char *text;
    PRINTF("mbedTLS version %s\r\n", MBEDTLS_VERSION_STRING);
    PRINTF("fsys=%d\r\n", ((CORE_CLK_FREQ)));
    PRINTF("Using following implementations:\r\n");
#if defined(MBEDTLS_FREESCALE_LTC_SHA256)
    text = "LTC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_MMCAU_SHA256)
    text = "MMCAU HW accelerated";
#elif defined(MBEDTLS_FREESCALE_LPC_SHA256)
    text = "LPC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAU3_SHA256)
    text = "CAU3 HW accelerated";
#elif defined(MBEDTLS_FREESCALE_DCP_SHA256)
    text = "DCP HW accelerated";
#elif defined(MBEDTLS_FREESCALE_HASHCRYPT_SHA256)
    text = "HASHCRYPT HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAAM_SHA256)
    text = "CAAM HW accelerated";
#elif defined(MBEDTLS_NXP_SENTINEL200)
    text = "S200 HW accelerated";
#elif defined(MBEDTLS_NXP_SENTINEL300)
    text = "S300 HW accelerated";
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
    text = "CSS PKC HW accelerated";
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
    text = "ELS PKC HW accelerated";
#else
    text = "Software implementation";
#endif
    PRINTF("  SHA: %s\r\n", text);
#if defined(MBEDTLS_FREESCALE_LTC_AES)
    text = "LTC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    text = "MMCAU HW accelerated";
#elif defined(MBEDTLS_FREESCALE_LPC_AES)
    text = "LPC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAU3_AES)
    text = "CAU3 HW accelerated";
#elif defined(MBEDTLS_FREESCALE_DCP_AES)
    text = "DCP HW accelerated";
#elif defined(MBEDTLS_FREESCALE_HASHCRYPT_AES)
    text = "HASHCRYPT HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
    text = "CAAM HW accelerated";
#elif defined(MBEDTLS_NXP_SENTINEL200)
    text = "SW AES, S200 HW accelerated CCM and CMAC";
#elif defined(MBEDTLS_NXP_SENTINEL300)
    text = "SW AES, S300 HW accelerated CCM and CMAC";
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
    text = "CSS PKC HW accelerated";
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
    text = "ELS PKC HW accelerated";
#else
    text = "Software implementation";
#endif
    PRINTF("  AES: %s\r\n", text);
#if defined(MBEDTLS_FREESCALE_LTC_AES_GCM)
    text = "LTC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    text = "MMCAU HW accelerated";
#elif defined(MBEDTLS_FREESCALE_LPC_AES_GCM)
    text = "LPC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAU3_AES)
    text = "CAU3 HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAAM_AES_GCM)
    text = "CAAM HW accelerated";
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
    text = "CSS PKC HW accelerated";
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
    text = "ELS PKC HW accelerated";
#else
    text = "Software implementation";
#endif
    PRINTF("  AES GCM: %s\r\n", text);
#if defined(MBEDTLS_FREESCALE_LTC_DES)
    text = "LTC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    text = "MMCAU HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAU3_DES)
    text = "CAU3 HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAAM_DES)
    text = "CAAM HW accelerated";
#else
    text = "Software implementation";
#endif
    PRINTF("  DES: %s\r\n", text);
#if defined(MBEDTLS_FREESCALE_LTC_PKHA)
    text = "LTC HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CAU3_PKHA)
    text = "CAU3 HW accelerated";
#elif defined(MBEDTLS_FREESCALE_CASPER_PKHA)
    text = "CASPER HW accelerated ECC256/384/521 and RSA verify";
#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
    text = "CAAM HW accelerated";
#elif defined(MBEDTLS_NXP_SENTINEL200)
    text = "S200 HW accelerated ECDSA and ECDH";
#elif defined(MBEDTLS_NXP_SENTINEL300)
    text = "S300 HW accelerated ECDSA and ECDH";
#elif defined(MBEDTLS_MCUX_CSS_PKC_API)
    text = "CSS PKC HW accelerated";
#elif defined(MBEDTLS_MCUX_ELS_PKC_API)
    text = "ELS PKC HW accelerated";
#else
    text = "Software implementation";
#endif
    PRINTF("  Asymmetric cryptography: %s\r\n", text);


    return 0;
}

int main(int argc, char *argv[])
{

#if !defined(FREESCALE_KSDK_BM)
    char **argp;
    int exclude_mode = 0;
#endif

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C) && defined(MBEDTLS_SELF_TEST)
    unsigned char buf[1000000];
#endif
    void *pointer;
    TaskHandle_t task_one;
    TaskHandle_t task_two;
    BaseType_t result = 0;

#if defined(FREESCALE_KSDK_BM)
    /* HW init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    if( CRYPTO_InitHardware() != kStatus_Success )
    {
        PRINTF( "Initialization of crypto HW failed\n" );
        return( return_FAILURE );
    }

#endif
    bench_print_features();

    /*
     * The C standard doesn't guarantee that all-bits-0 is the representation
     * of a NULL pointer. We do however use that in our code for initializing
     * structures, which should work on every modern platform. Let's be sure.
     */
    memset( &pointer, 0, sizeof( void * ) );
    if( pointer != NULL )
    {
        PRINTF( "all-bits-zero is not a NULL pointer\n" );
        return( return_FAILURE );
    }

    /*
     * Make sure we have a snprintf that correctly zero-terminates
     */
#if RUN_TEST_SNPRINTF /* Test is failed for UV */
    if( run_test_snprintf() != 0 )
    {
        PRINTF( "the snprintf implementation is broken\n" );
        return( return_FAILURE );
    }
#endif
#if !defined(FREESCALE_KSDK_BM)
    for( argp = argv + ( argc >= 1 ? 1 : argc ); *argp != NULL; ++argp )
    {
        if( strcmp( *argp, "--quiet" ) == 0 ||
            strcmp( *argp, "-q" ) == 0 )
        {
            v = 0;
        }
        else if( strcmp( *argp, "--exclude" ) == 0 ||
                 strcmp( *argp, "-x" ) == 0 )
        {
            exclude_mode = 1;
        }
        else
            break;
    }

    if( v != 0 )
        PRINTF( "\n" );
#endif

#if defined(MBEDTLS_SELF_TEST)

#if defined(MBEDTLS_MEMORY_BUFFER_ALLOC_C)
    mbedtls_memory_buffer_alloc_init( buf, sizeof(buf) );
#endif
    mbedtls_mutex_init( &alloc_mutex );
    mbedtls_platform_set_calloc_free( buffer_alloc_calloc_mutexed_custom, buffer_alloc_free_mutexed_custom );
    result = xTaskCreate(thread_one, "thread_one", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_PRIORITY, &task_one);
    if (pdPASS == result)
        PRINTF("Create thread successfully\r\n");

    result = xTaskCreate(thread_two, "thread_two", TASK_MAIN_STACK_SIZE, task_main_stack, TASK_PRIORITY, &task_two);
    if (pdPASS == result)
        PRINTF("Create thread successfully\r\n");
    vTaskStartScheduler();

#else
    (void) exclude_mode;
    PRINTF( " MBEDTLS_SELF_TEST not defined.\n" );
#endif
    while (1)
    {
        char ch = GETCHAR();
        PUTCHAR(ch);
    }
}

void thread_one(void *param)
{
#if defined(MBEDTLS_SELF_TEST)
    const selftest_t *test;
#endif /* MBEDTLS_SELF_TEST */
    int count = 0;
    int v             = 1; /* v=1 for verbose mode */
    int suites_tested = 0, suites_failed = 0;
    while(1)
    {
        suites_tested = 0;
        /* Run all the tests */
        for (test = selftests; test->name != NULL; test++)
        {
            if (test->function(v) != 0)
            {
                suites_failed++;
            }
            suites_tested++;
        }
        count ++;
        if( v != 0 )
        {

            if( suites_failed > 0)
            {
                PRINTF( "  [ %d tests FAIL in Task 1]\n\n", suites_failed );
                vTaskSuspend(NULL);
            }
            else
            {
                if (count % 20 == 0)
                  PRINTF( "  Task 1 is running\n\n" );
                
            }
        }

    }
}

void thread_two(void *param)
{
#if defined(MBEDTLS_SELF_TEST)
    const selftest_t *test;
#endif /* MBEDTLS_SELF_TEST */
    int count = 0;
    int v             = 1; /* v=1 for verbose mode */
    int suites_tested = 0, suites_failed = 0;
    while(1)
    {
        suites_tested = 0;
        /* Run all the tests */
        for (test = selftests; test->name != NULL; test++)
        {
            if (test->function(v) != 0)
            {
                suites_failed++;
            }
            suites_tested++;
        }
        count++;
        if( v != 0 )
        {

            if( suites_failed > 0)
            {
                PRINTF( "  [ %d tests FAIL in Task 2]\n\n", suites_failed );
                vTaskSuspend(NULL);
            }
            else
            {
              if( count % 30 == 0)
                  PRINTF( "  Task 2 is running\n\n" );
            }
        }

    }
}
