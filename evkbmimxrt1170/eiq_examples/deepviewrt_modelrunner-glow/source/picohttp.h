#ifndef PICOHTTP_H
#define PICOHTTP_H

#include "picohttpparser.h"

#ifndef _WIN32
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strtok_r strtok_s
#endif

#define HTTP_MORE_HEADERS 1
#define HTTP_CONNECTION_CLOSE 2
#define HTTP_HEADERS_END ((const char*) 0)

typedef int(http_route_handler)(SOCKET             sock,
                                char*              method,
                                char*              path,
                                struct phr_header* headers,
                                size_t             n_headers,
                                char*              content,
                                size_t             content_length,
                                void*              user_data);

struct http_route {
    const char*         path;
    size_t              path_len;
    void*               user_data;
    http_route_handler* handler;
};

extern int
http_response(SOCKET      sock,
              int         status_code,
              const char* content_type,
              size_t      content_length,
              const void* content,
              int         flags,
              ...);

extern int
http_route_request(SOCKET                   sock,
                   const struct http_route* routes,
                   char*                    method,
                   char*                    path,
                   struct phr_header*       headers,
                   size_t                   n_headers,
                   char*                    content,
                   size_t                   content_length);

extern int
http_handler(SOCKET                   sock,
             void*                    buffer,
             size_t                   buffer_size,
             size_t*                  offset,
             const struct http_route* routes);

#endif /* PICOHTTP_H */
