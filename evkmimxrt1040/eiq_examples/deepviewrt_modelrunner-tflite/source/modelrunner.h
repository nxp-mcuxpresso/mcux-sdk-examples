#ifndef __MODELRUNNER_H__
#define __MODELRUNNER_H__

#include "modelrunner_server.h"
#include "picohttp.h"

#define MODELRUNNER_DEFAULT_PORT 10818

extern int
Model_RunInference(NNServer* server);
extern int
Model_Setup(NNServer* server);

extern int
v1_handler(SOCKET             sock,
           char*              method,
           char*              path,
           struct phr_header* headers,
           size_t             n_headers,
           char*              content,
           size_t             content_length,
           void*              user_data);
extern int
v1_handler_post(SOCKET             sock,
                char*              method,
                char*              path,
                struct phr_header* headers,
                size_t             n_headers,
                char*              content,
                size_t             content_length,
                void*              user_data);
extern int
v1_handler_put(SOCKET             sock,
               char*              method,
               char*              path,
               struct phr_header* headers,
               size_t             n_headers,
               char*              content,
               size_t             content_length,
               void*              user_data);
extern int
model_handler(SOCKET             sock,
              char*              method,
              char*              path,
              struct phr_header* headers,
              size_t             n_headers,
              char*              content,
              size_t             content_length,
              void*              user_data);

extern int
nn_server_http_handler(SOCKET             sock,
                       char*              method,
                       char*              path,
                       struct phr_header* headers,
                       size_t             n_headers,
                       char*              content,
                       size_t             content_length,
                       void*              user_data);


extern void modelrunner_task(void* arg);

extern int
tensor_load_image(NNServer*   server,
                  const void* image,
                  size_t      image_size,
                  uint32_t    proc);

#endif /* __MODELRUNNER_H__ */

