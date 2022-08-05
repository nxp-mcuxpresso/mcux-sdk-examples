/**
 * Copyright 2018 by Au-Zone Technologies.  All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 *
 * Authorization of this file is not implied by any DeepViewRT license
 * agreements unless explicitly stated.
 */

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"
#include "stb_image_resize.h"
#include "modelrunner.h"

size_t
tensor_size(int32_t* shape, int32_t num_dims, int32_t glow_type)
{
	size_t size = 1;
    for (int dim = 0; dim < num_dims; ++dim) {
        size *= (size_t)(shape[dim]);
    }

    if(glow_type) size = size * 4; //float

    return size;
}

int32_t
tensor_volume(int32_t* shape, int32_t num_dims)
{
	int32_t vol = 1;
    for (int dim = 0; dim < num_dims; ++dim) {
        vol *= shape[dim];
    }
    return vol;
}

int
glow_argmax(NNServer* server, int* index, void* value, size_t value_size)
{
    if (!server) return 1;
    if (!(server->output[0])) return 1;
    int          maxidx   = 0;
    float        maxval   = 0.0f;
    int          volume   = tensor_volume(server->output_shape[0], server->output_dims[0]);
    int          out_type = server->output_type[0];

    void* map   = (void *)server->output[0];
    // Float
    if (out_type == 1) {
        float* outptr = (float*) map;
        maxval        = outptr[0];
        for (int i = 1; i < volume; i++) {
            if (maxval < outptr[i]) {
                maxval = outptr[i];
                maxidx = i;
            }
        }
    }
    // int8
    else {
        int8_t* outptrint8 = (int8_t*) map;
        int8_t  maxvalint8 = outptrint8[0];
        for (int i = 1; i < volume; i++) {
            if (maxvalint8 < outptrint8[i]) {
                maxvalint8 = outptrint8[i];
                maxidx     = i;
            }
        }
        maxval = 0.5f + ((float) maxvalint8 / 256.0f);
    }

    if (index) *index = maxidx;
    if (value) {
        if (value_size < sizeof(float)) return 1;
        memcpy(value, &maxval, sizeof(maxval));
    }

    return 0;
}

static void
rgb_statsf(const float* data, int n, float* mean, float* std)
{
    if (n == 0) {
        *mean = 0;
        *std  = 0;
        return;
    }

    float sum    = 0.0f;
    float sq_sum = 0.0f;

    // single channel
    for (int i = 0; i < n; i++) {
        sum += data[i];
        sq_sum += data[i] * data[i];
    }

    *mean     = sum / n;
    float var = sq_sum / n - (*mean) * (*mean);
    *std      = sqrtf(var);
}

static void
rgb_prewhitenf(float* data, int n)
{
    float mean, std, std_adj;

    rgb_statsf(data, n, &mean, &std);
    std_adj = fmaxf(std, 1.0f / sqrtf((float) n));

    for (int i = 0; i < n; i++) { data[i] = (data[i] - mean) / std_adj; }
}

int
tensor_load_image(NNServer*   server,
                  const void* image,
                  size_t      image_size,
                  uint32_t    proc)
{
    if (!server) { return 1; }
    if (!image) { return 1; }
    if (image_size > INT_MAX) { return 1; }

    int32_t* shape = server->input_shape;
    int32_t num_dims = server->input_dims;
    int32_t volume = tensor_volume(shape,num_dims);

    int32_t height,width,channels;
    height   = shape[1];
    width    = shape[2];
    channels = shape[3];

    bool do_transpose = false;
    //Check if NCHW and re-assign H/W/C
    if (server->input_NCHW) {
        height       =  shape[2];
        width        =  shape[3];
        channels     =  shape[1];
        if(channels > 1) {
            do_transpose = true;
            PRINTF("Tranposing image to NCHW tensor\n");
        }
    }

    //INT8 or FLOAT processing - Only FLOAT supports proc modes
    if (server->input_type == 0) {
        /* cannot whiten uint8_t tensors currently. */
        if (proc) return 2;

        int      w, h, c;
        uint8_t* source = stbi_load_from_memory(image,
                                                (int) image_size,
                                                &w,
                                                &h,
                                                &c,
                                                channels);
        if (!source) return 1;

        uint8_t* map =server->input;
        if (!map) {
            if (source) free(source);
            return 1;
        }

        if (h != height || w != width) {
        	uint8_t* tmp = (uint8_t*) malloc(volume);
            stbir_resize_uint8(source,
                               w,
                               h,
                               0,
                               tmp,
                               width,
                               height,
                               0,
                               channels);
            stbi_image_free(source);
            source = tmp;
        }

        if (do_transpose) {
            int cnt = 0;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    for (int k = 0; k < channels; k++) {
                        int tr_ind  = k * height * width + i * width + j;
                        map[tr_ind] = source[cnt];
                        cnt++;
                    }
                }
            }
        } else {
            memcpy((void*) map, (const void*) source, volume);
        }

        //Reduce by 128 for INT8
        int8_t* map_inter = (int8_t*) map;
        for (int i = 0; i < (int)volume; i++) {
            int inter     = (int)(map_inter[i]);
            inter         = inter - 128;
            map_inter[i]  = (int8_t) inter;
        }

        stbi_image_free(source);
        return 0;
    } else if (server->input_type == 1) {
        int w, h, c;

        uint8_t* source = stbi_load_from_memory(image,
                                                (int) image_size,
                                                &w,
                                                &h,
                                                &c,
                                                channels);
        if (!source) return 1;

        if (h != height || w != width) {
            uint8_t* tmp = (uint8_t *)malloc(volume);
            stbir_resize_uint8(source,
                               w,
                               h,
                               0,
                               tmp,
                               width,
                               height,
                               0,
                               channels);
            stbi_image_free(source);
            source = tmp;
        }

        float* map = (float*) server->input;
        if (!map) {
            if (source) free(source);
            return 1;
        }

        if (do_transpose) {
            int cnt = 0;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    for (int k = 0; k < channels; k++) {
                        int tr_ind = k*height*width + i*width +j;
                        map[tr_ind] = (float) source[cnt];
                        cnt++;
                    }
                }
            }
        } else {
            for (int i = 0; i < volume; i++) {
                map[i] = (float) source[i];
            }
        }

        if (proc & NN_IMAGE_PROC_UNSIGNED_NORM) {
            for (int i = 0; i < volume; i++) {
                map[i] *= (1.0f / 255.0f);
            }
        }

        if (proc & NN_IMAGE_PROC_SIGNED_NORM) {
            for (int i = 0; i < volume; i++) {
                map[i] = map[i] * (1.0f / 127.5f) - 1.0f;
            }
        }

        if (proc & NN_IMAGE_PROC_WHITENING) {
            rgb_prewhitenf(map, volume);
        }

        stbi_image_free(source);
        return 0;
    }

    return 1;
}
