/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdint.h>
#include <stdlib.h>

#define XMODEM_ERR_SEQNUM     1
#define XMODEM_ERR_CRC        2
#define XMODEM_ERR_SOH        3
#define XMODEM_ERR_SIZE       4
#define XMODEM_ERR_ABORTED    5
#define XMODEM_ERR_BUFSIZE    6
#define XMODEM_ERR_CALLBACK   7
#define XMODEM_ERR_UNKNOWN    99

struct xmodem_cfg
{
    /* UART setup */
    int (*putc)(int);
    int (*getc)(void);
    /* returns non-zero if there are data to be read by getc() */
    int  (*canread)(void);
    /* number of canread() retries to make ~1s delay */
    uint32_t canread_retries;
    
    /* destination address passed to the buffer_full_callback */
    uint32_t dst_addr;
    /* maximum permitted file size to be received */
    size_t maxsize;
    
    /* buffer setup used to pass data to the caller */
    uint8_t *buffer;
    /* MUST be a multiple of 1024 if 1kB packet size is used */
    size_t buffer_size;
    /* returns non-zero value to signalize transfer abort */
    int (*buffer_full_callback)(uint32_t dst_addr, uint32_t offset, uint32_t size);
};

long xmodem_receive(struct xmodem_cfg *cfg);

#endif // __XMODEM_H__
