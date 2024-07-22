/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "platform_bindings.h"
#include "fsl_usart.h"
#include "board.h"

#include "tinycrypt/sha256.h"
#include "tinycrypt/constants.h"

/* Hash bindings */

status_t sha256_init(hashctx_t *ctx)
{
    return tc_sha256_init(ctx);
}


status_t sha256_update(hashctx_t *ctx, const void *data, size_t size)
{
    return tc_sha256_update(ctx, (uint8_t*) data, size);
}


status_t sha256_finish(hashctx_t *ctx, void *output)
{
    status_t ret;
    size_t size = 32; /* expected size of hash */
    
    memset(output, 0, size);

    ret = tc_sha256_final(output, ctx);
    if (ret != TC_CRYPTO_SUCCESS)
    {
        return kStatus_Fail;
    }

    return kStatus_Success;
}

/* Board specific code to access Debug UART by XMODEM */

static USART_Type *xmodem_usart = (USART_Type *)USART12_BASE;


int xmodem_putc(int c)
{
    uint8_t c8 = c;
    USART_WriteBlocking(xmodem_usart, &c8, 1);
    return c;
}

/* The debug console layer input is already taken by the shell implementation so the read
 * function for xmodem must be done using lower layer
 */
int xmodem_getc(void)
{
    while (USART_GetRxFifoCount(xmodem_usart) == 0)
    {
        ;
    }
    
    return USART_ReadByte(xmodem_usart);
}

int xmodem_canread(void)
{
    return USART_GetRxFifoCount(xmodem_usart);
}
