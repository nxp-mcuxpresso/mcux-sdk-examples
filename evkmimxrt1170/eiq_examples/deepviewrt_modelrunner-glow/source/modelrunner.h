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

#include "picohttp.h"
#include "model.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Find a way to automate following to model.h or change manually
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define MODEL_NAME     "mobilenet_v2_0.35_224_quant"
// model input information
#ifndef MODEL_input
#define MODEL_input    0
#endif
#define INPUT_NAME     "input"
#define INPUT_NCHW     false
#define INPUT_SIZE     150528
#define INPUT_TYPE     0   //0-INT8; 1-FLOAT; Expand as needed
#define INPUT_SHAPE    {1,224,224,3}
#define NUM_INPUT_DIMS 4
#define INPUT_SCALE    0.007874016f
#define INPUT_ZEROP    0
// model output information.
#ifndef MODEL_output
#define MODEL_output    MODEL_MobilenetV2_Predictions_Softmax
#endif
#define OUTPUT_NAME     "MobilenetV2_Predictions_Softmax"
#define OUTPUT_SIZE     4004
#define OUTPUT_TYPE     1   //0-INT8; 1-FLOAT
#define OUTPUT_SHAPE    {1,1001}
#define NUM_OUTPUT_DIMS 2
#define NUM_OUTPUTS     1

#if NUM_OUTPUTS > 1
#ifndef MODEL_output1
#define MODEL_output1    MODEL_MobilenetV2_Predictions_Softmax
#endif
#define OUTPUT1_NAME     "MobilenetV2_Predictions_Softmax"
#define OUTPUT1_SIZE     4004
#define OUTPUT1_TYPE     1   //0-INT8; 1-FLOAT
#define OUTPUT1_SHAPE    {1,1001}
#define NUM_OUTPUT1_DIMS 2
#endif

#if NUM_OUTPUTS > 2
#ifndef MODEL_output2
#define MODEL_output2    MODEL_MobilenetV2_Predictions_Softmax
#endif
#define OUTPUT2_NAME     "MobilenetV2_Predictions_Softmax"
#define OUTPUT2_SIZE     4004
#define OUTPUT2_TYPE     1   //0-INT8; 1-FLOAT
#define OUTPUT2_SHAPE    {1,1001}
#define NUM_OUTPUT2_DIMS 2
#endif

#if NUM_OUTPUTS > 3
#ifndef MODEL_output3
#define MODEL_output3    MODEL_MobilenetV2_Predictions_Softmax
#endif
#define OUTPUT3_NAME     "MobilenetV2_Predictions_Softmax"
#define OUTPUT3_SIZE     4004
#define OUTPUT3_TYPE     1   //0-INT8; 1-FLOAT
#define OUTPUT3_SHAPE    {1,1001}
#define NUM_OUTPUT3_DIMS 2
#endif
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Find a way to automate above
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#define MODELRUNNER_DEFAULT_PORT 10818
/**
 * DeepView ModelRunner Server
 */
typedef int(inference_func)(uint8_t *constantWeight, uint8_t *mutableWeight, uint8_t *activations);
struct nn_server {
    uint8_t*   activation;
    uint8_t*   weights;
    uint8_t*   mutable;
    uint8_t*   input;
    uint8_t*   output[NUM_OUTPUTS];
    inference_func* inference;
    const char* model_name;
    size_t      model_size;
    size_t      mem_size;
    const char* input_name;
    const char* output_name[NUM_OUTPUTS];
    char*       json_buffer;
    size_t      json_size;
    struct {
        int64_t run;
        int64_t decode;
        int64_t input;
        int64_t output;
    } timing;
    //Input/Output information
    int32_t*   input_shape;
    int32_t    input_dims;
    int32_t    input_type;
    bool       input_NCHW;
    float      input_scale;
    int32_t    input_zero;
    //size_t input_size;
    //Pointers for multiple outputs
    int32_t*   output_shape[NUM_OUTPUTS];
    int32_t    output_dims[NUM_OUTPUTS];
    int32_t    output_type[NUM_OUTPUTS];
    int32_t    num_outputs;
    //size_t* output_size[NUM_OUTPUTS];
    //float   output_scale[NUM_OUTPUTS];
    //int     output_zero[NUM_OUTPUTS];
    //int64_t* layer_timings;
};


typedef struct nn_server NNServer;
/**
 *
 */


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

#define NN_IMAGE_PROC_UNSIGNED_NORM 0x0001
#define NN_IMAGE_PROC_WHITENING 0x0002
#define NN_IMAGE_PROC_SIGNED_NORM 0x0004
#define NN_IMAGE_PROC_MIRROR 0x1000
#define NN_IMAGE_PROC_FLIP 0x2000

extern int
tensor_load_image(NNServer*   server,
                  const void* image,
                  size_t      image_size,
                  uint32_t    proc);

extern size_t
tensor_size(int32_t* shape, int32_t num_dims, int32_t glow_type);

extern int32_t
tensor_volume(int32_t* shape, int32_t num_dims);

extern int
glow_argmax(NNServer* server, int* index, void* value, size_t value_size);

extern
int64_t os_clock_now();
#endif /* __MODELRUNNER_H__ */
