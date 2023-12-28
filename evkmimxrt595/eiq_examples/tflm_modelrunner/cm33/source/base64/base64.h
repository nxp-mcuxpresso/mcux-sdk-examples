/*
 *  * Base64 encoding/decoding (RFC1341)
 *   * Copyright (c) 2005, Jouni Malinen <j@w1.fi>
 *    *
 *     * This software may be distributed under the terms of the BSD license.
 *      * See README for more details.
 *       */

#include <stddef.h>


#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
extern "C"
{
#endif

unsigned char * base64_encode(const unsigned char *src, size_t len,
							      size_t *out_len);
unsigned char * base64_decode(const unsigned char *src, size_t len,
							      size_t *out_len);

#ifdef __cplusplus
}
#endif
#endif /*  BASE64_H */ 

