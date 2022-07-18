/*
 * Copyright (c) 2022, Freescale Semiconductor, Inc.
 * Copyright 2022 NXP. Not a Contribution
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CRYPTO_SUPPORT_H__
#define __CRYPTO_SUPPORT_H__

#include "fsl_hashcrypt.h"

typedef hashcrypt_hash_ctx_t sha1_ctx_t;

status_t sha1_init(sha1_ctx_t *ctx);
status_t sha1_update(sha1_ctx_t *ctx, const void *data, size_t size);
status_t sha1_finish(sha1_ctx_t *ctx, void *output);

#endif /* __CRYPTO_SUPPORT_H__ */
