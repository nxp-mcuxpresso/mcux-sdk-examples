/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <bootutil/sign_key.h>

/*
 * Even though this is in principle a Zephyr-specific file, the
 * simulator builds it and uses it as well. Because of that, we can't
 * use Kconfig symbols for key types, and have to rely on the MCUBoot
 * symbols (which Zephyr provides via this header, and the simulator
 * provides via the compiler command line).
 */
#include <mcuboot_config.h>

#if defined(MCUBOOT_SIGN_RSA)
#include "sign-rsa2048-pub.c"
#elif defined(MCUBOOT_SIGN_EC256)
#include "sign-ecdsa-p256-pub.c"
#else
#error "No public key available for given signing algorithm."
#endif


const struct bootutil_key bootutil_keys[] = {
    {
#if defined(MCUBOOT_SIGN_RSA)
        .key = rsa_pub_key,
        .len = &rsa_pub_key_len,
#elif defined(MCUBOOT_SIGN_EC256)
        .key = ecdsa_pub_key,
        .len = &ecdsa_pub_key_len,
#endif
    },
};

const int bootutil_key_cnt = 1;

#if defined(MCUBOOT_ENCRYPT_RSA)

#include "enc-rsa2048-priv-minimal.c"

const struct bootutil_key bootutil_enc_key = {
    .key = enc_priv_key,
    .len = &enc_priv_key_len,
};
#elif defined(MCUBOOT_ENCRYPT_KW)
#error "Encrypted images with AES-KW is not implemented yet."
#endif