/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <string.h>

#include "httpsrv.h"
#include "httpsrv_multipart.h"

int multipart_read_init(struct multipart_read_ctx *ctx, uint32_t ses_handle)
{
    int read;

    memset(ctx, 0, sizeof(*ctx));

    ctx->ses_handle = ses_handle;
    ctx->buf_size   = MULTIPART_READ_BUFSIZE;
    ctx->buf_end = ctx->buf_start = ctx->buffer;

    /* Fill in the buffer with data */
    read = HTTPSRV_cgi_read(ctx->ses_handle, ctx->buffer, ctx->buf_size);
    if (read <= 0)
    {
        ctx->state = MULTIPART_ERROR;
        return -1;
    }
    ctx->buf_end += read;

    /* Determine length of boundary string by scanning its first occurence */
    while (ctx->buf_start < ctx->buf_end)
    {
        if (*ctx->buf_start == ' ' || *ctx->buf_start == '\r')
        {
            /* End of boundary string */
            break;
        }
        ctx->boundary_len++;
        ctx->buf_start++;
    }

    if (ctx->buf_start == ctx->buf_end)
    {
        /* End of buffer reached while boundary end was not found */
        ctx->state = MULTIPART_ERROR;
        return -1;
    }

    if (ctx->boundary_len < 1 || ctx->boundary_len > 70)
    {
        /* Length of  boundary string is out of spec */
        ctx->state = MULTIPART_ERROR;
        return -1;
    }

    /*  RFC: The boundary delimiter MUST occur at the beginning of a line.
        Implementation: Use 2 reserved bytes to prepend boundary with CRLF for convenient matching using a state machine
     */
    ctx->boundary[0] = '\r';
    ctx->boundary[1] = '\n';
    memcpy(ctx->boundary + 2, ctx->buffer, ctx->boundary_len);
    ctx->boundary_len += 2;

    /* There may be whitespaces at the end of boundary line, skip them */
    while (ctx->buf_start < ctx->buf_end && *ctx->buf_start == ' ')
    {
        ctx->buf_start++;
    }

    /* There should be at least 2 chars for line termination remaining */
    if (ctx->buf_end - ctx->buf_start < 2)
    {
        /* Either the buffer is small to fit a single line or we reached the end of the stream */
        ctx->state = MULTIPART_ERROR;
        return -1;
    }

    /* Consume CRLF at the end to the boundary line */
    if (ctx->buf_start[0] != '\r' || ctx->buf_start[1] != '\n')
    {
        ctx->state = MULTIPART_ERROR;
        return -1;
    }
    ctx->buf_start += 2;

    /* Expect headers */
    ctx->state = MULTIPART_EXPECT_HEADERS;

    return 0;
}

int multipart_proc_header(struct multipart_read_ctx *ctx, char *buffer)
{
    int ret;
    char *param_ptr;

    PRINTF("%s: %s\n", __func__, buffer);

    const char *matchstr = "Content-Disposition:";

    if (strncmp(buffer, matchstr, strlen(matchstr)) != 0)
    {
        goto done;
    }
    param_ptr = buffer + strlen(matchstr);

    if (!strstr(param_ptr, " form-data;"))
    {
        goto done;
    }

    /* 'name' attribute, required */

    ret = multipart_parse_header_param("name", buffer, ctx->form_data_name, sizeof(ctx->form_data_name));
    if (ret <= 0)
    {
        return -1;
    }

    /* 'filename' attribute, optional */

    ctx->form_data_filename_present = 0;
    ret = multipart_parse_header_param("filename", buffer, ctx->form_data_filename, sizeof(ctx->form_data_filename));
    if (ret >= 0)
    {
        ctx->form_data_filename_present = 1;
    }

done:
    return 0;
}

int multipart_parse_header_param(const char *name, const char *src, char *dst, size_t dstsize)
{
    char *ptr;
    char *endptr;
    int len;

    ptr = strstr(src, name);
    if (!ptr)
    {
        /* not found */
        return -1;
    }

    ptr += strlen(name);
    if (ptr[0] != '=' || ptr[1] != '"')
    {
        /* expected =" part missing */
        return -2;
    }
    ptr += 2;

    endptr = strchr(ptr, '"');
    if (!endptr)
    {
        /* trailing " not found */
        return -3;
    }

    if (dst == NULL || dstsize == 0)
    {
        /* attribute found but no data requested */
        return 0;
    }

    len = (endptr - ptr) < dstsize - 1 ? (endptr - ptr) : dstsize - 1;
    strncpy(dst, ptr, len);
    dst[len] = '\0';

    return len;
}

void multipart_clear_headers(struct multipart_read_ctx *ctx)
{
    memset(ctx->form_data_name, 0, sizeof(ctx->form_data_name));
    memset(ctx->form_data_filename, 0, sizeof(ctx->form_data_filename));
}

int multipart_read_headers(struct multipart_read_ctx *ctx)
{
    char *line_start;
    char *line_lf;
    int read;
    int num_headers = 0;

    if (ctx->state != MULTIPART_EXPECT_HEADERS)
    {
        return 0;
    }

    multipart_clear_headers(ctx);

    /* Process buffer line by line. End of line is \n or \r\n */
    while (1)
    {
        line_start = ctx->buf_start;
        line_lf    = memchr(line_start, '\n', ctx->buf_end - ctx->buf_start);

        if (line_lf == NULL)
        {
            /* No end of line found in the buffer */
            if (ctx->buf_end - ctx->buf_start == ctx->buf_size)
            {
                /* The buffer is full but probably not large enough to keep the whole header line */
                ctx->state = MULTIPART_ERROR;
                return -1;
            }

            /* Move unprocessed data to the beginning of the buffer */
            memmove(ctx->buffer, ctx->buf_start, ctx->buf_end - ctx->buf_start);
            ctx->buf_end -= ctx->buf_start - ctx->buffer;
            ctx->buf_start = ctx->buffer;

            /* Top up the buffer */
            read = HTTPSRV_cgi_read(ctx->ses_handle, ctx->buf_end, ctx->buf_size - (ctx->buf_end - ctx->buf_start));
            if (read == 0)
            {
                /* End od stream */
                ctx->state = MULTIPART_ERROR;
                return -1;
            }
            ctx->buf_end += read;

            /* And restart parsing */
            continue;
        }

        /* Null terminate the line */
        *line_lf = '\0';
        if ((line_lf > line_start) && (*(line_lf - 1) == '\r'))
        {
            /* Discard optional CR */
            *(line_lf - 1) = '\0';
        }

        /* Move start of valid data in the buffer according to data consumed */
        ctx->buf_start = line_lf + 1;

        /* Empty line indicates end of headers */
        if (*line_start == '\0')
        {
            break;
        }

        /* Process the header */
        multipart_proc_header(ctx, line_start);
        num_headers++;
    }

    ctx->state = MULTIPART_EXPECT_DATA;
    return num_headers;
}

int32_t multipart_read_data(struct multipart_read_ctx *ctx, uint8_t *buffer, int32_t len)
{
    int match_idx  = 0;
    int read_total = 0;

    if (ctx->state == MULTIPART_ERROR)
    {
        return -1;
    }

    if (ctx->state != MULTIPART_EXPECT_DATA)
    {
        return 0;
    }

    /* Copy data from receive buffer to caller buffer while searching for boundary string using a state machine */
    while (read_total != len)
    {
        /* If the buffer contains just partially matched boundary string (or is completely empty) we need to receive
         * more data */
        if (ctx->buf_start + match_idx >= ctx->buf_end)
        {
            uint32_t read;

            /* Move the unprocessed data (partially matched boundary string) to the beginning of the buffer */
            memmove(ctx->buffer, ctx->buf_start, ctx->buf_end - ctx->buf_start);
            ctx->buf_end -= ctx->buf_start - ctx->buffer;
            ctx->buf_start = ctx->buffer;

            /* Top up the buffer */
            read = HTTPSRV_cgi_read(ctx->ses_handle, ctx->buf_end, ctx->buf_size - (ctx->buf_end - ctx->buf_start));
            if (read == 0)
            {
                /* End od stream unexpected at this point */
                ctx->state = MULTIPART_ERROR;
                return -1;
            }
            ctx->buf_end += read;
        }

        /* If there is a match with boundary string */
        if (ctx->buf_start[match_idx] == ctx->boundary[match_idx])
        {
            /* If this is the last character of the bundary string */
            if (++match_idx == ctx->boundary_len)
            {
                /* Boundary found, consume it and exit the loop (end of data part) */
                ctx->buf_start += match_idx;
                break;
            }
            continue;
        }

        /* Mismatch, reset matching index */
        match_idx = 0;

        /* The character is not part of valid boundary string for sure, copy it to the caller provided buffer */
        if (buffer != NULL)
        {
            *buffer++ = *ctx->buf_start;
        }
        ctx->buf_start++;

        read_total++;
    }

    if (match_idx == ctx->boundary_len)
    {
        /* Boundary was matched, presume that headers will follow unless further reading of the stream indicates the
         * processing should stop */
        ctx->state = MULTIPART_EXPECT_HEADERS;

        /* For simplicity of implementation, the closing double dash of last boundary is not strictly required.
           Just read until encountering single dash, LF or end of stream.  */
        do
        {
            /* Make sure the buffer is not empty */
            if (ctx->buf_end == ctx->buf_start)
            {
                uint32_t read;
                ctx->buf_start = ctx->buf_end = ctx->buffer;
                read                          = HTTPSRV_cgi_read(ctx->ses_handle, ctx->buffer, ctx->buf_size);
                if (read == 0)
                {
                    /* End od stream unexpected at this point */
                    ctx->state = MULTIPART_ERROR;
                    break;
                }
                ctx->buf_end += read;
            }
            if (*ctx->buf_start == '-')
            {
                /* Dash found, assume end of multipart content, rest of the buffer will be ignored */
                ctx->state = MULTIPART_END;
                break;
            }
        } while (*ctx->buf_start++ != '\n');
    }

    return (read_total);
}
