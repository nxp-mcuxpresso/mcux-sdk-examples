/**
 * Copyright 2018 by Au-Zone Technologies.  All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 *
 * Authorization of this file is not implied by any DeepViewRT license
 * agreements unless explicitly stated.
 */

//#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "picohttp.h"

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif /* MSG_NOSIGNAL */

#ifndef SHUT_WR
#define SHUT_WR 1
#endif /* SHUT_WR */

#define array_sizeof(T) (sizeof(T) / sizeof(*T))

static const char*
http_status_code(int status_code)
{
    switch (status_code) {
    case 100:
        return "Continue";
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 428:
        return "Precondition Failed";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 503:
        return "Service Unavailable";
    }

    return "";
}

int
vsnoprintf(char* buf, size_t len, size_t* off, const char* msg, va_list args)
{
    int ret;

    if (!buf || !off || !msg || len == 0) {
        errno = EINVAL;
        return -1;
    }

    if (*off >= len) {
        errno = ENOMEM;
        return -1;
    }

    ret = vsnprintf(&buf[*off], len - *off, msg, args);

    if (ret == -1) return -1;

    size_t num = (size_t) ret;
    if (num > len - *off) {
        *off  = len - 1;
        errno = ENOMEM;
        return -1;
    }

    *off += num;
    return ret;
}

int
snoprintf(char* buf, size_t len, size_t* off, const char* msg, ...)
{
    int     ret;
    va_list args;

    va_start(args, msg);
    ret = vsnoprintf(buf, len, off, msg, args);
    va_end(args);

    return ret;
}

static int
http_response_(SOCKET      sock,
               int         status_code,
               const char* content_type,
               size_t      content_length,
               int         flags,
               va_list     args)
{
    int    ret;
    char   buf[1024];
    size_t off = 0;

    ret = snoprintf(buf,
                    sizeof(buf),
                    &off,
                    "HTTP/1.1 %d %s\r\n",
                    status_code,
                    http_status_code(status_code));
    if (ret == -1) {
        fprintf(stderr, "failed to write response code: %s\n", strerror(errno));
        return -1;
    }

    ret = snoprintf(buf,
                    sizeof(buf),
                    &off,
                    "Access-Control-Allow-Origin: *\r\n"
                    "Access-Control-Allow-Methods: GET, PUT, POST, HEAD, "
                    "DELETE\r\n"
                    "Access-Control-Expose-Headers: *\r\n"
                    "Access-Control-Max-Age: 86400\r\n");
    if (ret == -1) {
        fprintf(stderr, "failed to write response code: %s\n", strerror(errno));
        return -1;
    }

    if (content_type) {
        ret = snoprintf(buf,
                        sizeof(buf),
                        &off,
                        "Content-Type: %s\r\n",
                        content_type);
        if (ret == -1) {
            fprintf(stderr,
                    "failed to write header Content-Type: %s\n",
                    strerror(errno));
            return -1;
        }
    }

    if (content_length) {
        ret = snoprintf(buf,
                        sizeof(buf),
                        &off,
                        "Content-Length: %zu\r\n",
                        content_length);
        if (ret == -1) {
            fprintf(stderr,
                    "failed to write header Content-Length: %s\n",
                    strerror(errno));
            return -1;
        }
    }
    if (flags & HTTP_CONNECTION_CLOSE) {
        ret = snoprintf(buf, sizeof(buf), &off, "Connection: close\r\n");
        if (ret == -1) {
            fprintf(stderr,
                    "failed to write header Connection: %s\n",
                    strerror(errno));
            return -1;
        }
    }

    for (const char* header = va_arg(args, const char*);
         header != HTTP_HEADERS_END;
         header = va_arg(args, const char*)) {
        const char* value = va_arg(args, const char*);

        if (!value) {
            fprintf(stderr, "invalid header %s missing value\n", header);
            return -1;
        }

        ret = snoprintf(buf, sizeof(buf), &off, "%s: %s\r\n", header, value);
        if (ret == -1) {
            fprintf(stderr,
                    "failed to write header %s: %s\n",
                    header,
                    strerror(errno));
            return -1;
        }
    }

    if (!(flags & HTTP_MORE_HEADERS)) {
        ret = snoprintf(buf, sizeof(buf), &off, "\r\n");
        if (ret == -1) {
            fprintf(stderr,
                    "failed to write end of headers: %s\n",
                    strerror(errno));
            return -1;
        }
    }

    ret = send(sock, buf, off, MSG_NOSIGNAL);

    if (ret == 0) {
        errno = ECONNRESET;
        return -1;
    } else if (ret == -1) {
        if (errno) {
            fprintf(stderr,
                    "failed to send http response: %s\n",
                    strerror(errno));
        }

        return -1;
    }

    return 0;
}

int
http_response(SOCKET      sock,
              int         status_code,
              const char* content_type,
              size_t      content_length,
              const void* content,
              int         flags,
              ...)
{
    int     err;
    va_list args;

    va_start(args, flags);
    err = http_response_(sock,
                         status_code,
                         content_type,
                         content_length,
                         flags,
                         args);
    va_end(args);
    if (err) { return -1; }

    if (content) {
        err = send(sock, content, content_length, MSG_NOSIGNAL);

        if (err == 0) {
            errno = ECONNRESET;
            return -1;
        }

        if (err == -1) {
            if (errno) {
                fprintf(stderr,
                        "failed to send http response: %s\n",
                        strerror(errno));
            }

            return -1;
        }
    }

    //Socket close is needed to avoid crashes
    if (flags & HTTP_CONNECTION_CLOSE) {
        int  n_read;
        char buf[1024];

        err = shutdown(sock, SHUT_WR);
        if (err) {
            fprintf(stderr, "failed to shutdown socket: %s\n", strerror(errno));
            return -1;
        }

        // Read outstanding data, if any, before closing socket.
        do {
            n_read = recv(sock, buf, sizeof(buf), 0);
        } while (n_read == -1 && errno == EINTR);

#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }

    return 0;
}

int
http_route_request(SOCKET                   sock,
                   const struct http_route* routes,
                   char*                    method,
                   char*                    path,
                   struct phr_header*       headers,
                   size_t                   n_headers,
                   char*                    content,
                   size_t                   content_length)
{
    size_t path_len = strlen(path);

    for (const struct http_route* route = routes; route->path != NULL;
         ++route) {
        if (path_len >= route->path_len &&
            strncmp(route->path, path, route->path_len) == 0) {
            return route->handler(sock,
                                  method,
                                  path,
                                  headers,
                                  n_headers,
                                  content,
                                  content_length,
                                  route->user_data);
        }
    }

    const char* errmsg = "Invalid Path";
    size_t      errlen = strlen(errmsg);

    return http_response(sock,
                         404,
                         "text/plain",
                         errlen,
                         errmsg,
                         0,
                         HTTP_HEADERS_END);
}

int
http_handler(SOCKET                   sock,
             void*                    buffer,
             size_t                   buffer_size,
             size_t*                  offset,
             const struct http_route* routes)
{
    struct phr_header headers[32];
    int               minor_version  = 0;
    char*             method         = NULL;
    char*             path           = NULL;
    char*             request        = buffer;
    int               reqlen         = 0;
    size_t            method_len     = 0;
    size_t            path_len       = 0;
    size_t            n_headers      = array_sizeof(headers);
    ssize_t           n_read         = 0;
    int               expect_100     = 0;
    size_t            content_length = 0;
    void*             content        = NULL;

    while (1) {
        size_t buflen_prev;
        errno = 0;

        do {
            n_read = recv(sock, request + *offset, buffer_size - *offset, 0);
        } while (n_read == -1 && errno == EINTR);

        if (n_read == 0) {
            errno = ECONNRESET;
            return -1;
        }

        if (n_read < 0) { return -1; }

        buflen_prev = 0;
        *offset += n_read;
        n_headers = array_sizeof(headers);

        reqlen = phr_parse_request(request,
                                   *offset,
                                   (const char**) &method,
                                   &method_len,
                                   (const char**) &path,
                                   &path_len,
                                   &minor_version,
                                   headers,
                                   &n_headers,
                                   buflen_prev);

        if (reqlen > 0) {
            break;
        } else if (reqlen == -1) {
            fprintf(stderr, "invalid http request %s\n", request);
            errno = EINVAL;
            return -1;
        }

        if (*offset >= buffer_size) {
            /* the caller is responsible for closing the connection */
            errno = ENOBUFS;
            return -1;
        }
    }

    for (size_t i = 0; i < n_headers; ++i) {
        struct phr_header* hdr = &headers[i];

        if (strncmp("Expect", hdr->name, hdr->name_len) == 0) {
            if (strncmp("100-continue", hdr->value, hdr->value_len) == 0) {
                expect_100 = 1;
            } else {
                fprintf(stderr,
                        "Invalid Expect: %.*s\n",
                        (int) hdr->value_len,
                        hdr->value);
                errno = EINVAL;
                return -1;
            }
        } else if (strncmp("Content-Length", hdr->name, hdr->name_len) == 0) {
            char content_length_str[32] = {0};
            strncpy(content_length_str, hdr->value, hdr->value_len);
            content_length = strtoul(content_length_str, NULL, 0);
            content        = &request[reqlen];
        } else if (strncmp("Connection", hdr->name, hdr->name_len) == 0) {
            // printf("REQUEST CONNECTION: %.*s\n", hdr->value_len, hdr->value);
        }
    }

    if (content_length) {
        size_t message_size = reqlen + content_length;

        if (message_size > buffer_size) {
            /* the caller is responsible for closing the connection */
            errno = ENOBUFS;
            return -1;
        }

        while (*offset < message_size) {
            do {
                n_read =
                    recv(sock, request + *offset, message_size - *offset, 0);
            } while (n_read == -1 && errno == EINTR);

            if (n_read == 0) {
                errno = ECONNRESET;
                return -1;
            }

            if (n_read < 0) { return -1; }

            *offset += n_read;
        }
    }

    if (*offset - (reqlen + content_length)) {
        printf("HTTP REQUEST %p CAPTURED %d BYTES -- REQLEN: %d CONTENT: %d "
               "REMAIN: %d\n",
               (void*) sock,
               *offset,
               reqlen,
               content_length,
               *offset - (reqlen + content_length));
    }

    if (expect_100) {
        if (http_response(sock, 100, NULL, 0, NULL, 0, HTTP_HEADERS_END)) {
            return -1;
        }
    }

    method[method_len] = '\0';
    path[path_len]     = '\0';
    *offset            = 0;

    return http_route_request(sock,
                              routes,
                              method,
                              path,
                              headers,
                              n_headers,
                              content,
                              content_length);
}
