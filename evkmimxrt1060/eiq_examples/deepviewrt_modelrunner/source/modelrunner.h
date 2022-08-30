/**
 * Copyright 2018 by Au-Zone Technologies.  All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 *
 * Authorization of this file is not implied by any DeepViewRT license
 * agreements unless explicitly stated.
 */

#ifndef __MODELRUNNER_H__
#define __MODELRUNNER_H__

#include "deepview_rt.h"
#include "picohttp.h"

#define MODELRUNNER_DEFAULT_PORT 10818
/**
 * DeepView ModelRunner Server
 */
struct nn_server {
    NNContext* context;
    NNEngine*  engine;
    char*      input_name;
    char*      output_name;
    bool       model_mmap;
    void*      model_memory;
    NNModel*   model;
    size_t     max_model_size;
    size_t     model_size;
    size_t     model_offset;
    int        model_block_size;
    int        model_block_index;
    int        model_block_count;
    bool       model_validated;
    char*      json_buffer;
    size_t     json_size;
    void*      unix_buffer;
    size_t     unix_size;
    void*      unix_input_memory;
    size_t     unix_input_size;
    struct {
        int64_t run;
        int64_t decode;
        int64_t input;
        int64_t output;
    } timing;
    int64_t* layer_timings;
};


typedef struct nn_server NNServer;
/**
 *
 */
NN_AVAILABLE_SINCE_2_4
extern void*
nn_server_init(int       backend,
               size_t    memory_size,
               void*     memory,
               size_t    cache_size,
               void*     cache,
               size_t    max_model_size,
               void*     model_memory);

NN_AVAILABLE_SINCE_2_4
extern int
nn_server_http_handler(SOCKET             sock,
                       char*              method,
                       char*              path,
                       struct phr_header* headers,
                       size_t             n_headers,
                       char*              content,
                       size_t             content_length,
                       void*              user_data);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern NNContext*
nn_server_context(NNServer* server);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern const NNModel*
nn_server_model(NNServer* server);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern void
nn_server_set_input_name(NNServer* server, const char* name);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern const char*
nn_server_input_name(NNServer* server);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern void
nn_server_set_output_name(NNServer* server, const char* name);

/**
 *
 */
NN_AVAILABLE_SINCE_2_2
extern const char*
nn_server_output_name(NNServer* server);


extern void modelrunner_task(void* arg);
#endif /* __MODELRUNNER_H__ */
