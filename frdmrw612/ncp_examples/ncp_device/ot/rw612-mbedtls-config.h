/*
 *  Copyright (c) 2022-2023, NXP.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RW612_MBEDTLS_CONFIG_H
#define RW612_MBEDTLS_CONFIG_H

/* FreeRTOS is always supported for RW612 platform so enable threading */
#define MBEDTLS_MCUX_FREERTOS_THREADING_ALT
#define MBEDTLS_THREADING_C
#define MBEDTLS_THREADING_ALT

/* SDK mbetdls config include */
#include "els_pkc_mbedtls_config.h"

//#ifdef CONFIG_WPA_SUPP_MBEDTLS
/* wpa_supplicant mbedtls extend config */
#include "wpa_supp_els_pkc_mbedtls_config.h"
#undef MBEDTLS_PLATFORM_STD_CALLOC
#undef MBEDTLS_PLATFORM_STD_FREE
#undef MBEDTLS_SSL_MAX_CONTENT_LEN
//#endif

/* els_pkc_mbedtls_config.h uses the same include guard than OT mbedtls-config.h
 * so we can undef the include guard as a workaround */
#undef MBEDTLS_CONFIG_H

/* OpenThread mbedtls config defines MBEDTLS_ECP_MAX_BITS to 256 so we need to disable
 * els_pkc features that require a higher value */
#undef MBEDTLS_ECP_DP_SECP384R1_ENABLED
#undef MBEDTLS_ECP_DP_SECP521R1_ENABLED
#undef MBEDTLS_ECP_DP_BP384R1_ENABLED
#undef MBEDTLS_ECP_DP_BP512R1_ENABLED

#if CONFIG_NCP_OT
/* Openthread mbetdls config include */
#include "mbedtls-config.h"
#endif

/* Undef this flag to make sure to use hardware entropy */
#undef MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES

#endif // RW612_MBEDTLS_CONFIG_H
