/*
 * Copyright 2016-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* -------------------------------------------------------------
 * A very basic implementation of XMODEM-CRC data receiving.
 * -------------------------------------------------------------
 */

#include "xmodem.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"

#define XMODEM_SOH  0x01
#define XMODEM_S1K  0x02
#define XMODEM_EOT  0x04
#define XMODEM_ACK  0x06
#define XMODEM_NAK  0x15
#define XMODEM_CAN  0x18


static uint16_t xmodem_crc_update(uint8_t c, uint16_t crc)
{
    crc = crc ^ ((uint16_t) c << 8);
    
    for (int i=0; i < 8; i++)
    {
        if (crc & 0x8000)
        {
            crc = (crc << 1) ^ 0x1021;
        }
        else
        {
            crc = crc << 1;
        }
    }
    
    return crc;
}

static int xmodem_peekc(struct xmodem_cfg *cfg, uint32_t retries)
{
    for (uint32_t i = 0; i < retries; i++)
    {
        if (cfg->canread())
            return 1;
    }
    
    return 0;
}


long xmodem_receive(struct xmodem_cfg *cfg)
{
    int c;
    int ret;
    size_t packet_cnt = 0;
    size_t bytes_rxed = 0;
    size_t buffer_load_size = 0;
    int err = XMODEM_ERR_UNKNOWN;
    
    int (*putc)(int)  = cfg->putc;
    int (*getc)(void) = cfg->getc;
    
    /* Could be downsized to 128 if 1kB packets are disabled */
    if (cfg->buffer_size % 1024)
    {
        return -XMODEM_ERR_BUFSIZE;
    }
    
    /* Empty receving FIFO */
    while (cfg->canread())
    {
        getc();
    }
    
    /* Start signalizing ready-to-receive state */
    
    while (1)
    {
        putc('C');
        
        /* To avoid messing with non-blocking mode.
           Retry count should roughly correspond to 1 second
         */
        if (xmodem_peekc(cfg, cfg->canread_retries))
        {
            c = getc();
        }
        else
        {
            continue;
        }
        
        if (c == 'x' || c == 'X')
        {
            /* Receiving canceled */
            return -XMODEM_ERR_ABORTED;
        }
        
        break;   
    }
    
    /* Process data from sender */
    
    while (1)
    {
        uint8_t seq, nseq;
        uint16_t crc = 0;
        uint16_t crc_recv;
        uint16_t payload_size;
        uint32_t buffer_offset;
        
        switch (c)
        {
        case XMODEM_EOT:
            putc(XMODEM_ACK);
            if (buffer_load_size)
            {
                /* last chunk of data to hand over */
                cfg->buffer_full_callback(cfg->dst_addr, bytes_rxed-buffer_load_size, buffer_load_size);
            }
            return bytes_rxed;
            
        case XMODEM_SOH:
            payload_size = 128;
            break;
            
        case XMODEM_S1K:
            payload_size = 1024;
            break;
            
        case XMODEM_CAN:
            err = XMODEM_ERR_ABORTED;
            goto abort;
            
        default:
            err = XMODEM_ERR_SOH;
            goto abort;
        }

        /* process packet sequence number */
        
        seq  = getc();
        nseq = getc();
        
        if (seq != (255-nseq) ||
            seq != ((packet_cnt+1) % 256))
        {
            err = XMODEM_ERR_SEQNUM;
            goto abort;
        }

        /* receive packet payload */
        
        buffer_offset = bytes_rxed % cfg->buffer_size;
        
        for (int i=0; i < payload_size; i++)
        {            
            if (bytes_rxed >= cfg->maxsize)
            {
                err = XMODEM_ERR_SIZE;
                goto abort;
            }
            c = getc();
            bytes_rxed++;
            cfg->buffer[buffer_offset + i] = c;
            crc = xmodem_crc_update(c, crc);
        }
       
        /* receive packet crc */
        
        crc_recv  = getc() << 8;
        crc_recv |= getc();

        
        /* packet confirmation */
        
        if (crc == crc_recv)
        {

            buffer_load_size += payload_size;
            if (buffer_load_size == cfg->buffer_size)
            {
                /* handover of data should be done before ACK */
                ret = cfg->buffer_full_callback(cfg->dst_addr, bytes_rxed-buffer_load_size, buffer_load_size);
                if (ret)
                {
                    err = XMODEM_ERR_CALLBACK;
                    goto abort;
                }
                buffer_load_size = 0;
            }
            putc(XMODEM_ACK);
            packet_cnt++;
        }
        else
        {
            putc(XMODEM_NAK);
            bytes_rxed -= payload_size;
        }
        
        c = getc();
    }
    
abort:
    putc(XMODEM_CAN);
    putc(XMODEM_CAN);
    putc(XMODEM_CAN);
    return -err;
}
