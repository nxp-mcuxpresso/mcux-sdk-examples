/*
 * Copyright (c) 2022, Freescale Semiconductor, Inc.
 * Copyright 2022 NXP. Not a Contribution
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crypto_support.h"

status_t sha1_init(sha1_ctx_t *ctx)
{
    return HASHCRYPT_SHA_Init(HASHCRYPT, ctx, kHASHCRYPT_Sha1);
}

status_t sha1_update(sha1_ctx_t *ctx, const void *data, size_t size)
{
    return HASHCRYPT_SHA_Update(HASHCRYPT, ctx, data, size);
}

status_t sha1_finish(sha1_ctx_t *ctx, void *output)
{
    status_t ret;
    size_t size = 20; /* expected size of hash */

    ret = HASHCRYPT_SHA_Finish(HASHCRYPT, ctx, output, &size);
    if (ret == kStatus_Success && size != 20)
    {
        return kStatus_Fail;
    }

    return ret;
}
