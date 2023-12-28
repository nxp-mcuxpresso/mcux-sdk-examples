/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HTTP_Handler_
#define HTTP_Handler_


#include "picohttpparser/picohttpparser.h"
#include "modelrunner.h"

typedef int http_handler(int        sock,
                         size_t*           content_length,
                         const char*        path,
                         struct phr_header* headers,
                         size_t             n_headers,
                         NNServer*          server);


struct http_router {
    const char*   path;
    const char*   method;
    NNServer*     server;
    http_handler* handler;
};

int handle_client(int sock, struct http_router* router);

int v1_handler_get(int                sock,
                   size_t*           content_length,
                   const char*        path,
                   struct phr_header* headers,
                   size_t             n_headers,
                   NNServer*          server);

int v1_handler_put(int                sock,
                   size_t*           content_length,
                   const char*        path,
                   struct phr_header* headers,
                   size_t             n_headers,
                   NNServer*          server);

int v1_handler_post(int                sock,
                    size_t*           content_length,
                    const char*        path,
                    struct phr_header* headers,
                    size_t             n_headers,
                    NNServer*          server);

int v1_model_handler_get(int                sock,
                         size_t*           content_length,
                         const char*        path,
                         struct phr_header* headers,
                         size_t             n_headers,
                         NNServer*          server);

#endif
