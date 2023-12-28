/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

/* Implementation for processing file upload using HTTP multipart/form-data */

#define MULTIPART_READ_BUFSIZE  (200)
#define MULTIPART_MAX_BOUNDARY  (70)
#define FORM_DATA_NAME_SIZE     (20)
#define FORM_DATA_FILENAME_SIZE (20)

enum multipart_state
{
    MULTIPART_END = 0,
    MULTIPART_EXPECT_HEADERS,
    MULTIPART_EXPECT_DATA,
    MULTIPART_ERROR,
};

struct multipart_read_ctx
{
    uint32_t ses_handle;

    enum multipart_state state;

    /* Boundary string is 1..70 chars long. Keep 2 more reserved for CRLF */
    char boundary[MULTIPART_MAX_BOUNDARY + 2];
    uint32_t boundary_len; /* Actual lenght of the stored boundary string */

    char buffer[MULTIPART_READ_BUFSIZE];
    uint32_t buf_size; /* Size of the buffer (allocated space), this is to allow for possible dynamic allocation of the
                          buffer */
    char *buf_start;   /* Pointer to valid (not consumed so far) data in the buffer */
    char
        *buf_end; /* Points to location following the valid data, i.e. it may point one byte past the allocated space */

    char form_data_name[FORM_DATA_NAME_SIZE + 1]; /* Extra char for null termination */

    /* determines presence and content of filename attribute */
    int form_data_filename_present;
    char form_data_filename[FORM_DATA_FILENAME_SIZE + 1];
};

int multipart_read_init(struct multipart_read_ctx *ctx, uint32_t ses_handle);
int multipart_proc_header(struct multipart_read_ctx *ctx, char *buffer);
int multipart_parse_header_param(const char *name, const char *src, char *dst, size_t dstsize);
void multipart_clear_headers(struct multipart_read_ctx *ctx);
int multipart_read_headers(struct multipart_read_ctx *ctx);
int32_t multipart_read_data(struct multipart_read_ctx *ctx, uint8_t *buffer, int32_t len);
