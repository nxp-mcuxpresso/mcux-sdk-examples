/*
 * Copyright 2019-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 *      This code is auto-generated and shall not be modified!
 */

#ifndef __DSP_NN_H__
#define __DSP_NN_H__

#include "config_nn.h"
#include "srtm_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if NN_ENABLE_xa_nn_matXvec_16x16_16 == 1
void xa_nn_matXvec_16x16_16_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed short *p_out,
                                  signed short *p_mat1,
                                  signed short *p_mat2,
                                  signed short *p_vec1,
                                  signed short *p_vec2,
                                  signed short *p_bias,
                                  signed int rows,
                                  signed int cols1,
                                  signed int cols2,
                                  signed int row_stride1,
                                  signed int row_stride2,
                                  signed int acc_shift,
                                  signed int bias_shift);

int xa_nn_matXvec_16x16_16(signed short *p_out,
                           signed short *p_mat1,
                           signed short *p_mat2,
                           signed short *p_vec1,
                           signed short *p_vec2,
                           signed short *p_bias,
                           signed int rows,
                           signed int cols1,
                           signed int cols2,
                           signed int row_stride1,
                           signed int row_stride2,
                           signed int acc_shift,
                           signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_16x16_32 == 1
void xa_nn_matXvec_16x16_32_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed int *p_out,
                                  signed short *p_mat1,
                                  signed short *p_mat2,
                                  signed short *p_vec1,
                                  signed short *p_vec2,
                                  signed short *p_bias,
                                  signed int rows,
                                  signed int cols1,
                                  signed int cols2,
                                  signed int row_stride1,
                                  signed int row_stride2,
                                  signed int acc_shift,
                                  signed int bias_shift);

int xa_nn_matXvec_16x16_32(signed int *p_out,
                           signed short *p_mat1,
                           signed short *p_mat2,
                           signed short *p_vec1,
                           signed short *p_vec2,
                           signed short *p_bias,
                           signed int rows,
                           signed int cols1,
                           signed int cols2,
                           signed int row_stride1,
                           signed int row_stride2,
                           signed int acc_shift,
                           signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_16x16_64 == 1
void xa_nn_matXvec_16x16_64_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed long long *p_out,
                                  signed short *p_mat1,
                                  signed short *p_mat2,
                                  signed short *p_vec1,
                                  signed short *p_vec2,
                                  signed short *p_bias,
                                  signed int rows,
                                  signed int cols1,
                                  signed int cols2,
                                  signed int row_stride1,
                                  signed int row_stride2,
                                  signed int acc_shift,
                                  signed int bias_shift);

int xa_nn_matXvec_16x16_64(signed long long *p_out,
                           signed short *p_mat1,
                           signed short *p_mat2,
                           signed short *p_vec1,
                           signed short *p_vec2,
                           signed short *p_bias,
                           signed int rows,
                           signed int cols1,
                           signed int cols2,
                           signed int row_stride1,
                           signed int row_stride2,
                           signed int acc_shift,
                           signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_16x16_16_tanh == 1
void xa_nn_matXvec_16x16_16_tanh_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       signed short *p_mat1,
                                       signed short *p_mat2,
                                       signed short *p_vec1,
                                       signed short *p_vec2,
                                       void *p_bias,
                                       signed int rows,
                                       signed int cols1,
                                       signed int cols2,
                                       signed int row_stride1,
                                       signed int row_stride2,
                                       signed int acc_shift,
                                       signed int bias_shift,
                                       signed int bias_precision,
                                       void *p_scratch);

int xa_nn_matXvec_16x16_16_tanh(signed short *p_out,
                                signed short *p_mat1,
                                signed short *p_mat2,
                                signed short *p_vec1,
                                signed short *p_vec2,
                                void *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int cols2,
                                signed int row_stride1,
                                signed int row_stride2,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int bias_precision,
                                void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_16x16_16_sigmoid == 1
void xa_nn_matXvec_16x16_16_sigmoid_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          signed short *p_out,
                                          signed short *p_mat1,
                                          signed short *p_mat2,
                                          signed short *p_vec1,
                                          signed short *p_vec2,
                                          void *p_bias,
                                          signed int rows,
                                          signed int cols1,
                                          signed int cols2,
                                          signed int row_stride1,
                                          signed int row_stride2,
                                          signed int acc_shift,
                                          signed int bias_shift,
                                          signed int bias_precision,
                                          void *p_scratch);

int xa_nn_matXvec_16x16_16_sigmoid(signed short *p_out,
                                   signed short *p_mat1,
                                   signed short *p_mat2,
                                   signed short *p_vec1,
                                   signed short *p_vec2,
                                   void *p_bias,
                                   signed int rows,
                                   signed int cols1,
                                   signed int cols2,
                                   signed int row_stride1,
                                   signed int row_stride2,
                                   signed int acc_shift,
                                   signed int bias_shift,
                                   signed int bias_precision,
                                   void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_batch_16x16_64 == 1
void xa_nn_matXvec_batch_16x16_64_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed long long **p_out,
                                        signed short *p_mat1,
                                        signed short **p_vec1,
                                        signed short *p_bias,
                                        signed int rows,
                                        signed int cols1,
                                        signed int row_stride1,
                                        signed int acc_shift,
                                        signed int bias_shift,
                                        signed int vec_count);

int xa_nn_matXvec_batch_16x16_64(signed long long **p_out,
                                 signed short *p_mat1,
                                 signed short **p_vec1,
                                 signed short *p_bias,
                                 signed int rows,
                                 signed int cols1,
                                 signed int row_stride1,
                                 signed int acc_shift,
                                 signed int bias_shift,
                                 signed int vec_count);
#endif

#if NN_ENABLE_xa_nn_matmul_16x16_16 == 1
void xa_nn_matmul_16x16_16_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed short *p_out,
                                 const signed short *p_mat1,
                                 const signed short *p_mat2,
                                 const signed short *p_bias,
                                 signed int rows,
                                 signed int cols,
                                 signed int row_stride,
                                 signed int acc_shift,
                                 signed int bias_shift,
                                 signed int vec_count,
                                 signed int vec_offset,
                                 signed int out_offset,
                                 signed int out_stride);

int xa_nn_matmul_16x16_16(signed short *p_out,
                          const signed short *p_mat1,
                          const signed short *p_mat2,
                          const signed short *p_bias,
                          signed int rows,
                          signed int cols,
                          signed int row_stride,
                          signed int acc_shift,
                          signed int bias_shift,
                          signed int vec_count,
                          signed int vec_offset,
                          signed int out_offset,
                          signed int out_stride);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x16_16 == 1
void xa_nn_matXvec_8x16_16_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed short *p_out,
                                 signed char *p_mat1,
                                 signed char *p_mat2,
                                 signed short *p_vec1,
                                 signed short *p_vec2,
                                 signed short *p_bias,
                                 signed int rows,
                                 signed int cols1,
                                 signed int cols2,
                                 signed int row_stride1,
                                 signed int row_stride2,
                                 signed int acc_shift,
                                 signed int bias_shift);

int xa_nn_matXvec_8x16_16(signed short *p_out,
                          signed char *p_mat1,
                          signed char *p_mat2,
                          signed short *p_vec1,
                          signed short *p_vec2,
                          signed short *p_bias,
                          signed int rows,
                          signed int cols1,
                          signed int cols2,
                          signed int row_stride1,
                          signed int row_stride2,
                          signed int acc_shift,
                          signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x16_32 == 1
void xa_nn_matXvec_8x16_32_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int *p_out,
                                 signed char *p_mat1,
                                 signed char *p_mat2,
                                 signed short *p_vec1,
                                 signed short *p_vec2,
                                 signed short *p_bias,
                                 signed int rows,
                                 signed int cols1,
                                 signed int cols2,
                                 signed int row_stride1,
                                 signed int row_stride2,
                                 signed int acc_shift,
                                 signed int bias_shift);

int xa_nn_matXvec_8x16_32(signed int *p_out,
                          signed char *p_mat1,
                          signed char *p_mat2,
                          signed short *p_vec1,
                          signed short *p_vec2,
                          signed short *p_bias,
                          signed int rows,
                          signed int cols1,
                          signed int cols2,
                          signed int row_stride1,
                          signed int row_stride2,
                          signed int acc_shift,
                          signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x16_64 == 1
void xa_nn_matXvec_8x16_64_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed long long *p_out,
                                 signed char *p_mat1,
                                 signed char *p_mat2,
                                 signed short *p_vec1,
                                 signed short *p_vec2,
                                 signed short *p_bias,
                                 signed int rows,
                                 signed int cols1,
                                 signed int cols2,
                                 signed int row_stride1,
                                 signed int row_stride2,
                                 signed int acc_shift,
                                 signed int bias_shift);

int xa_nn_matXvec_8x16_64(signed long long *p_out,
                          signed char *p_mat1,
                          signed char *p_mat2,
                          signed short *p_vec1,
                          signed short *p_vec2,
                          signed short *p_bias,
                          signed int rows,
                          signed int cols1,
                          signed int cols2,
                          signed int row_stride1,
                          signed int row_stride2,
                          signed int acc_shift,
                          signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x16_16_tanh == 1
void xa_nn_matXvec_8x16_16_tanh_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      signed short *p_out,
                                      signed char *p_mat1,
                                      signed char *p_mat2,
                                      signed short *p_vec1,
                                      signed short *p_vec2,
                                      void *p_bias,
                                      signed int rows,
                                      signed int cols1,
                                      signed int cols2,
                                      signed int row_stride1,
                                      signed int row_stride2,
                                      signed int acc_shift,
                                      signed int bias_shift,
                                      signed int bias_precision,
                                      void *p_scratch);

int xa_nn_matXvec_8x16_16_tanh(signed short *p_out,
                               signed char *p_mat1,
                               signed char *p_mat2,
                               signed short *p_vec1,
                               signed short *p_vec2,
                               void *p_bias,
                               signed int rows,
                               signed int cols1,
                               signed int cols2,
                               signed int row_stride1,
                               signed int row_stride2,
                               signed int acc_shift,
                               signed int bias_shift,
                               signed int bias_precision,
                               void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x16_16_sigmoid == 1
void xa_nn_matXvec_8x16_16_sigmoid_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed short *p_out,
                                         signed char *p_mat1,
                                         signed char *p_mat2,
                                         signed short *p_vec1,
                                         signed short *p_vec2,
                                         void *p_bias,
                                         signed int rows,
                                         signed int cols1,
                                         signed int cols2,
                                         signed int row_stride1,
                                         signed int row_stride2,
                                         signed int acc_shift,
                                         signed int bias_shift,
                                         signed int bias_precision,
                                         void *p_scratch);

int xa_nn_matXvec_8x16_16_sigmoid(signed short *p_out,
                                  signed char *p_mat1,
                                  signed char *p_mat2,
                                  signed short *p_vec1,
                                  signed short *p_vec2,
                                  void *p_bias,
                                  signed int rows,
                                  signed int cols1,
                                  signed int cols2,
                                  signed int row_stride1,
                                  signed int row_stride2,
                                  signed int acc_shift,
                                  signed int bias_shift,
                                  signed int bias_precision,
                                  void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_batch_8x16_64 == 1
void xa_nn_matXvec_batch_8x16_64_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed long long **p_out,
                                       signed char *p_mat1,
                                       signed short **p_vec1,
                                       signed short *p_bias,
                                       signed int rows,
                                       signed int cols1,
                                       signed int row_stride1,
                                       signed int acc_shift,
                                       signed int bias_shift,
                                       signed int vec_count);

int xa_nn_matXvec_batch_8x16_64(signed long long **p_out,
                                signed char *p_mat1,
                                signed short **p_vec1,
                                signed short *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int row_stride1,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int vec_count);
#endif

#if NN_ENABLE_xa_nn_matmul_8x16_16 == 1
void xa_nn_matmul_8x16_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                const signed char *p_mat1,
                                const signed short *p_mat2,
                                const signed short *p_bias,
                                signed int rows,
                                signed int cols,
                                signed int row_stride,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int vec_count,
                                signed int vec_offset,
                                signed int out_offset,
                                signed int out_stride);

int xa_nn_matmul_8x16_16(signed short *p_out,
                         const signed char *p_mat1,
                         const signed short *p_mat2,
                         const signed short *p_bias,
                         signed int rows,
                         signed int cols,
                         signed int row_stride,
                         signed int acc_shift,
                         signed int bias_shift,
                         signed int vec_count,
                         signed int vec_offset,
                         signed int out_offset,
                         signed int out_stride);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x8_8 == 1
void xa_nn_matXvec_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               signed char *p_mat1,
                               signed char *p_mat2,
                               signed char *p_vec1,
                               signed char *p_vec2,
                               signed char *p_bias,
                               signed int rows,
                               signed int cols1,
                               signed int cols2,
                               signed int row_stride1,
                               signed int row_stride2,
                               signed int acc_shift,
                               signed int bias_shift);

int xa_nn_matXvec_8x8_8(signed char *p_out,
                        signed char *p_mat1,
                        signed char *p_mat2,
                        signed char *p_vec1,
                        signed char *p_vec2,
                        signed char *p_bias,
                        signed int rows,
                        signed int cols1,
                        signed int cols2,
                        signed int row_stride1,
                        signed int row_stride2,
                        signed int acc_shift,
                        signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x8_16 == 1
void xa_nn_matXvec_8x8_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                signed char *p_mat1,
                                signed char *p_mat2,
                                signed char *p_vec1,
                                signed char *p_vec2,
                                signed char *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int cols2,
                                signed int row_stride1,
                                signed int row_stride2,
                                signed int acc_shift,
                                signed int bias_shift);

int xa_nn_matXvec_8x8_16(signed short *p_out,
                         signed char *p_mat1,
                         signed char *p_mat2,
                         signed char *p_vec1,
                         signed char *p_vec2,
                         signed char *p_bias,
                         signed int rows,
                         signed int cols1,
                         signed int cols2,
                         signed int row_stride1,
                         signed int row_stride2,
                         signed int acc_shift,
                         signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x8_32 == 1
void xa_nn_matXvec_8x8_32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed int *p_out,
                                signed char *p_mat1,
                                signed char *p_mat2,
                                signed char *p_vec1,
                                signed char *p_vec2,
                                signed char *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int cols2,
                                signed int row_stride1,
                                signed int row_stride2,
                                signed int acc_shift,
                                signed int bias_shift);

int xa_nn_matXvec_8x8_32(signed int *p_out,
                         signed char *p_mat1,
                         signed char *p_mat2,
                         signed char *p_vec1,
                         signed char *p_vec2,
                         signed char *p_bias,
                         signed int rows,
                         signed int cols1,
                         signed int cols2,
                         signed int row_stride1,
                         signed int row_stride2,
                         signed int acc_shift,
                         signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x8_8_tanh == 1
void xa_nn_matXvec_8x8_8_tanh_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed char *p_out,
                                    signed char *p_mat1,
                                    signed char *p_mat2,
                                    signed char *p_vec1,
                                    signed char *p_vec2,
                                    void *p_bias,
                                    signed int rows,
                                    signed int cols1,
                                    signed int cols2,
                                    signed int row_stride1,
                                    signed int row_stride2,
                                    signed int acc_shift,
                                    signed int bias_shift,
                                    signed int bias_precision,
                                    void *p_scratch);

int xa_nn_matXvec_8x8_8_tanh(signed char *p_out,
                             signed char *p_mat1,
                             signed char *p_mat2,
                             signed char *p_vec1,
                             signed char *p_vec2,
                             void *p_bias,
                             signed int rows,
                             signed int cols1,
                             signed int cols2,
                             signed int row_stride1,
                             signed int row_stride2,
                             signed int acc_shift,
                             signed int bias_shift,
                             signed int bias_precision,
                             void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_8x8_8_sigmoid == 1
void xa_nn_matXvec_8x8_8_sigmoid_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed char *p_out,
                                       signed char *p_mat1,
                                       signed char *p_mat2,
                                       signed char *p_vec1,
                                       signed char *p_vec2,
                                       void *p_bias,
                                       signed int rows,
                                       signed int cols1,
                                       signed int cols2,
                                       signed int row_stride1,
                                       signed int row_stride2,
                                       signed int acc_shift,
                                       signed int bias_shift,
                                       signed int bias_precision,
                                       void *p_scratch);

int xa_nn_matXvec_8x8_8_sigmoid(signed char *p_out,
                                signed char *p_mat1,
                                signed char *p_mat2,
                                signed char *p_vec1,
                                signed char *p_vec2,
                                void *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int cols2,
                                signed int row_stride1,
                                signed int row_stride2,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int bias_precision,
                                void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_batch_8x8_32 == 1
void xa_nn_matXvec_batch_8x8_32_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      signed int **p_out,
                                      signed char *p_mat1,
                                      signed char **p_vec1,
                                      signed char *p_bias,
                                      signed int rows,
                                      signed int cols1,
                                      signed int row_stride1,
                                      signed int acc_shift,
                                      signed int bias_shift,
                                      signed int vec_count);

int xa_nn_matXvec_batch_8x8_32(signed int **p_out,
                               signed char *p_mat1,
                               signed char **p_vec1,
                               signed char *p_bias,
                               signed int rows,
                               signed int cols1,
                               signed int row_stride1,
                               signed int acc_shift,
                               signed int bias_shift,
                               signed int vec_count);
#endif

#if NN_ENABLE_xa_nn_matmul_8x8_8 == 1
void xa_nn_matmul_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                              void *params,
                              signed char *p_out,
                              const signed char *p_mat1,
                              const signed char *p_mat2,
                              const signed char *p_bias,
                              signed int rows,
                              signed int cols,
                              signed int row_stride,
                              signed int acc_shift,
                              signed int bias_shift,
                              signed int vec_count,
                              signed int vec_offset,
                              signed int out_offset,
                              signed int out_stride);

int xa_nn_matmul_8x8_8(signed char *p_out,
                       const signed char *p_mat1,
                       const signed char *p_mat2,
                       const signed char *p_bias,
                       signed int rows,
                       signed int cols,
                       signed int row_stride,
                       signed int acc_shift,
                       signed int bias_shift,
                       signed int vec_count,
                       signed int vec_offset,
                       signed int out_offset,
                       signed int out_stride);
#endif

#if NN_ENABLE_xa_nn_matXvec_f32xf32_f32_sigmoid == 1
void xa_nn_matXvec_f32xf32_f32_sigmoid_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             float *p_out,
                                             float *p_mat1,
                                             float *p_mat2,
                                             float *p_vec1,
                                             float *p_vec2,
                                             float *p_bias,
                                             signed int rows,
                                             signed int cols1,
                                             signed int cols2,
                                             signed int row_stride1,
                                             signed int row_stride2,
                                             float *p_scratch);

int xa_nn_matXvec_f32xf32_f32_sigmoid(float *p_out,
                                      float *p_mat1,
                                      float *p_mat2,
                                      float *p_vec1,
                                      float *p_vec2,
                                      float *p_bias,
                                      signed int rows,
                                      signed int cols1,
                                      signed int cols2,
                                      signed int row_stride1,
                                      signed int row_stride2,
                                      float *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_f32xf32_f32_tanh == 1
void xa_nn_matXvec_f32xf32_f32_tanh_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          float *p_out,
                                          float *p_mat1,
                                          float *p_mat2,
                                          float *p_vec1,
                                          float *p_vec2,
                                          float *p_bias,
                                          signed int rows,
                                          signed int cols1,
                                          signed int cols2,
                                          signed int row_stride1,
                                          signed int row_stride2,
                                          float *p_scratch);

int xa_nn_matXvec_f32xf32_f32_tanh(float *p_out,
                                   float *p_mat1,
                                   float *p_mat2,
                                   float *p_vec1,
                                   float *p_vec2,
                                   float *p_bias,
                                   signed int rows,
                                   signed int cols1,
                                   signed int cols2,
                                   signed int row_stride1,
                                   signed int row_stride2,
                                   float *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_f32xf32_f32 == 1
void xa_nn_matXvec_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_mat1,
                                     const float *p_mat2,
                                     const float *p_vec1,
                                     const float *p_vec2,
                                     const float *p_bias,
                                     signed int rows,
                                     signed int cols1,
                                     signed int cols2,
                                     signed int row_stride1,
                                     signed int row_stride2);

int xa_nn_matXvec_f32xf32_f32(float *p_out,
                              const float *p_mat1,
                              const float *p_mat2,
                              const float *p_vec1,
                              const float *p_vec2,
                              const float *p_bias,
                              signed int rows,
                              signed int cols1,
                              signed int cols2,
                              signed int row_stride1,
                              signed int row_stride2);
#endif

#if NN_ENABLE_xa_nn_matXvec_batch_f32xf32_f32 == 1
void xa_nn_matXvec_batch_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           float **p_out,
                                           float *p_mat1,
                                           float **p_vec1,
                                           float *p_bias,
                                           signed int rows,
                                           signed int cols1,
                                           signed int row_stride1,
                                           signed int vec_count);

int xa_nn_matXvec_batch_f32xf32_f32(float **p_out,
                                    float *p_mat1,
                                    float **p_vec1,
                                    float *p_bias,
                                    signed int rows,
                                    signed int cols1,
                                    signed int row_stride1,
                                    signed int vec_count);
#endif

#if NN_ENABLE_xa_nn_matmul_f32xf32_f32 == 1
void xa_nn_matmul_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    float *p_out,
                                    const float *p_mat1,
                                    const float *p_mat2,
                                    const float *p_bias,
                                    signed int rows,
                                    signed int cols,
                                    signed int row_stride,
                                    signed int vec_count,
                                    signed int vec_offset,
                                    signed int out_offset,
                                    signed int out_stride);

int xa_nn_matmul_f32xf32_f32(float *p_out,
                             const float *p_mat1,
                             const float *p_mat2,
                             const float *p_bias,
                             signed int rows,
                             signed int cols,
                             signed int row_stride,
                             signed int vec_count,
                             signed int vec_offset,
                             signed int out_offset,
                             signed int out_stride);
#endif

#if NN_ENABLE_xa_nn_matXvec_asym8uxasym8u_asym8u == 1
void xa_nn_matXvec_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              unsigned char *p_out,
                                              const unsigned char *p_mat1,
                                              const unsigned char *p_mat2,
                                              const unsigned char *p_vec1,
                                              const unsigned char *p_vec2,
                                              const signed int *p_bias,
                                              signed int rows,
                                              signed int cols1,
                                              signed int cols2,
                                              signed int row_stride1,
                                              signed int row_stride2,
                                              signed int mat1_zero_bias,
                                              signed int mat2_zero_bias,
                                              signed int vec1_zero_bias,
                                              signed int vec2_zero_bias,
                                              signed int out_multiplier,
                                              signed int out_shift,
                                              signed int out_zero_bias);

int xa_nn_matXvec_asym8uxasym8u_asym8u(unsigned char *p_out,
                                       const unsigned char *p_mat1,
                                       const unsigned char *p_mat2,
                                       const unsigned char *p_vec1,
                                       const unsigned char *p_vec2,
                                       const signed int *p_bias,
                                       signed int rows,
                                       signed int cols1,
                                       signed int cols2,
                                       signed int row_stride1,
                                       signed int row_stride2,
                                       signed int mat1_zero_bias,
                                       signed int mat2_zero_bias,
                                       signed int vec1_zero_bias,
                                       signed int vec2_zero_bias,
                                       signed int out_multiplier,
                                       signed int out_shift,
                                       signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_matXvec_sym8sxasym8s_asym8s == 1
void xa_nn_matXvec_sym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             signed char *p_out,
                                             const signed char *p_mat1,
                                             const signed char *p_mat2,
                                             const signed char *p_vec1,
                                             const signed char *p_vec2,
                                             const signed int *p_bias,
                                             signed int rows,
                                             signed int cols1,
                                             signed int cols2,
                                             signed int row_stride1,
                                             signed int row_stride2,
                                             signed int vec1_zero_bias,
                                             signed int vec2_zero_bias,
                                             signed int out_multiplier,
                                             signed int out_shift,
                                             signed int out_zero_bias);

int xa_nn_matXvec_sym8sxasym8s_asym8s(signed char *p_out,
                                      const signed char *p_mat1,
                                      const signed char *p_mat2,
                                      const signed char *p_vec1,
                                      const signed char *p_vec2,
                                      const signed int *p_bias,
                                      signed int rows,
                                      signed int cols1,
                                      signed int cols2,
                                      signed int row_stride1,
                                      signed int row_stride2,
                                      signed int vec1_zero_bias,
                                      signed int vec2_zero_bias,
                                      signed int out_multiplier,
                                      signed int out_shift,
                                      signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_matXvec_out_stride_sym8sxasym8s_16 == 1
void xa_nn_matXvec_out_stride_sym8sxasym8s_16_async(void (*cb)(void *, srtm_message *msg),
                                                    void *params,
                                                    signed short *p_out,
                                                    const signed char *p_mat1,
                                                    const signed char *p_vec1,
                                                    const signed int *p_bias,
                                                    signed int rows,
                                                    signed int cols1,
                                                    signed int row_stride1,
                                                    signed int out_stride,
                                                    signed int vec1_zero_bias,
                                                    signed int out_multiplier,
                                                    signed int out_shift);

int xa_nn_matXvec_out_stride_sym8sxasym8s_16(signed short *p_out,
                                             const signed char *p_mat1,
                                             const signed char *p_vec1,
                                             const signed int *p_bias,
                                             signed int rows,
                                             signed int cols1,
                                             signed int row_stride1,
                                             signed int out_stride,
                                             signed int vec1_zero_bias,
                                             signed int out_multiplier,
                                             signed int out_shift);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_32 == 1
void xa_nn_vec_sigmoid_32_32_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed int *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length);

int xa_nn_vec_sigmoid_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_32 == 1
void xa_nn_vec_tanh_32_32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed int *p_out,
                                const signed int *p_vec,
                                signed int vec_length);

int xa_nn_vec_tanh_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_32_32 == 1
void xa_nn_vec_relu_std_32_32_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int *p_out,
                                    const signed int *p_vec,
                                    signed int vec_length);

int xa_nn_vec_relu_std_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_32_32 == 1
void xa_nn_vec_relu_32_32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed int *p_out,
                                const signed int *p_vec,
                                signed int threshold,
                                signed int vec_length);

int xa_nn_vec_relu_32_32(signed int *p_out, const signed int *p_vec, signed int threshold, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu1_32_32 == 1
void xa_nn_vec_relu1_32_32_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int *p_out,
                                 const signed int *p_vec,
                                 signed int vec_length);

int xa_nn_vec_relu1_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu6_32_32 == 1
void xa_nn_vec_relu6_32_32_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int *p_out,
                                 const signed int *p_vec,
                                 signed int vec_length);

int xa_nn_vec_relu6_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_softmax_32_32 == 1
void xa_nn_vec_softmax_32_32_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed int *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length);

int xa_nn_vec_softmax_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_f32_f32 == 1
void xa_nn_vec_sigmoid_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_sigmoid_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_tanh_f32_f32 == 1
void xa_nn_vec_tanh_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_tanh_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_f32_f32 == 1
void xa_nn_vec_relu_f32_f32_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  float *p_out,
                                  const float *p_vec,
                                  float threshold,
                                  signed int vec_length);

int xa_nn_vec_relu_f32_f32(float *p_out, const float *p_vec, float threshold, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_f32_f32 == 1
void xa_nn_vec_relu_std_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_relu_std_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu1_f32_f32 == 1
void xa_nn_vec_relu1_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_relu1_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu6_f32_f32 == 1
void xa_nn_vec_relu6_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_relu6_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_softmax_f32_f32 == 1
void xa_nn_vec_softmax_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length);

int xa_nn_vec_softmax_f32_f32(float *p_out, const float *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_16 == 1
void xa_nn_vec_sigmoid_32_16_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed short *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length);

int xa_nn_vec_sigmoid_32_16(signed short *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_16 == 1
void xa_nn_vec_tanh_32_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                const signed int *p_vec,
                                signed int vec_length);

int xa_nn_vec_tanh_32_16(signed short *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_8 == 1
void xa_nn_vec_sigmoid_32_8_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed char *p_out,
                                  const signed int *p_vec,
                                  signed int vec_length);

int xa_nn_vec_sigmoid_32_8(signed char *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_8 == 1
void xa_nn_vec_tanh_32_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed int *p_vec,
                               signed int vec_length);

int xa_nn_vec_tanh_32_8(signed char *p_out, const signed int *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_16_16 == 1
void xa_nn_vec_relu_16_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                const signed short *p_vec,
                                signed short threshold,
                                signed int vec_length);

int xa_nn_vec_relu_16_16(signed short *p_out, const signed short *p_vec, signed short threshold, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_16_16 == 1
void xa_nn_vec_relu_std_16_16_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed short *p_out,
                                    const signed short *p_vec,
                                    signed int vec_length);

int xa_nn_vec_relu_std_16_16(signed short *p_out, const signed short *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_8_8 == 1
void xa_nn_vec_relu_8_8_async(void (*cb)(void *, srtm_message *msg),
                              void *params,
                              signed char *p_out,
                              const signed char *p_vec,
                              signed char threshold,
                              signed int vec_length);

int xa_nn_vec_relu_8_8(signed char *p_out, const signed char *p_vec, signed char threshold, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_8_8 == 1
void xa_nn_vec_relu_std_8_8_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed char *p_out,
                                  const signed char *p_vec,
                                  signed int vec_length);

int xa_nn_vec_relu_std_8_8(signed char *p_out, const signed char *p_vec, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_interpolation_q15 == 1
void xa_nn_vec_interpolation_q15_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       const signed short *p_ifact,
                                       const signed short *p_inp1,
                                       const signed short *p_inp2,
                                       signed int num_elements);

int xa_nn_vec_interpolation_q15(signed short *p_out,
                                const signed short *p_ifact,
                                const signed short *p_inp1,
                                const signed short *p_inp2,
                                signed int num_elements);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_getsize == 1
void xa_nn_conv1d_std_getsize_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int kernel_height,
                                    signed int input_width,
                                    signed int input_channels,
                                    signed int input_precision);

int xa_nn_conv1d_std_getsize(signed int kernel_height,
                             signed int input_width,
                             signed int input_channels,
                             signed int input_precision);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_8x16 == 1
void xa_nn_conv1d_std_8x16_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed short *p_out,
                                 signed short *p_inp,
                                 signed char *p_kernel,
                                 signed short *p_bias,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int input_channels,
                                 signed int kernel_height,
                                 signed int out_channels,
                                 signed int y_stride,
                                 signed int y_padding,
                                 signed int out_height,
                                 signed int bias_shift,
                                 signed int acc_shift,
                                 signed int out_data_format,
                                 void *p_handle);

int xa_nn_conv1d_std_8x16(signed short *p_out,
                          signed short *p_inp,
                          signed char *p_kernel,
                          signed short *p_bias,
                          signed int input_height,
                          signed int input_width,
                          signed int input_channels,
                          signed int kernel_height,
                          signed int out_channels,
                          signed int y_stride,
                          signed int y_padding,
                          signed int out_height,
                          signed int bias_shift,
                          signed int acc_shift,
                          signed int out_data_format,
                          void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_8x8 == 1
void xa_nn_conv1d_std_8x8_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed char *p_out,
                                signed char *p_inp,
                                signed char *p_kernel,
                                signed char *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int out_channels,
                                signed int y_stride,
                                signed int y_padding,
                                signed int out_height,
                                signed int bias_shift,
                                signed int acc_shift,
                                signed int out_data_format,
                                void *p_handle);

int xa_nn_conv1d_std_8x8(signed char *p_out,
                         signed char *p_inp,
                         signed char *p_kernel,
                         signed char *p_bias,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int out_channels,
                         signed int y_stride,
                         signed int y_padding,
                         signed int out_height,
                         signed int bias_shift,
                         signed int acc_shift,
                         signed int out_data_format,
                         void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_16x16 == 1
void xa_nn_conv1d_std_16x16_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed short *p_out,
                                  signed short *p_inp,
                                  signed short *p_kernel,
                                  signed short *p_bias,
                                  signed int input_height,
                                  signed int input_width,
                                  signed int input_channels,
                                  signed int kernel_height,
                                  signed int out_channels,
                                  signed int y_stride,
                                  signed int y_padding,
                                  signed int out_height,
                                  signed int bias_shift,
                                  signed int acc_shift,
                                  signed int out_data_format,
                                  void *p_handle);

int xa_nn_conv1d_std_16x16(signed short *p_out,
                           signed short *p_inp,
                           signed short *p_kernel,
                           signed short *p_bias,
                           signed int input_height,
                           signed int input_width,
                           signed int input_channels,
                           signed int kernel_height,
                           signed int out_channels,
                           signed int y_stride,
                           signed int y_padding,
                           signed int out_height,
                           signed int bias_shift,
                           signed int acc_shift,
                           signed int out_data_format,
                           void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_f32 == 1
void xa_nn_conv1d_std_f32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                float *p_out,
                                float *p_inp,
                                float *p_kernel,
                                float *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int out_channels,
                                signed int y_stride,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_data_format,
                                void *p_handle);

int xa_nn_conv1d_std_f32(float *p_out,
                         float *p_inp,
                         float *p_kernel,
                         float *p_bias,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int out_channels,
                         signed int y_stride,
                         signed int y_padding,
                         signed int out_height,
                         signed int out_data_format,
                         void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_getsize == 1
void xa_nn_conv2d_std_getsize_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int input_height,
                                    signed int input_channels,
                                    signed int kernel_height,
                                    signed int kernel_width,
                                    signed int y_stride,
                                    signed int y_padding,
                                    signed int out_height,
                                    signed int out_channels,
                                    signed int input_precision);

int xa_nn_conv2d_std_getsize(signed int input_height,
                             signed int input_channels,
                             signed int kernel_height,
                             signed int kernel_width,
                             signed int y_stride,
                             signed int y_padding,
                             signed int out_height,
                             signed int out_channels,
                             signed int input_precision);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_8x16 == 1
void xa_nn_conv2d_std_8x16_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed short *p_out,
                                 signed short *p_inp,
                                 signed char *p_kernel,
                                 signed short *p_bias,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int input_channels,
                                 signed int kernel_height,
                                 signed int kernel_width,
                                 signed int out_channels,
                                 signed int x_stride,
                                 signed int y_stride,
                                 signed int x_padding,
                                 signed int y_padding,
                                 signed int out_height,
                                 signed int out_width,
                                 signed int bias_shift,
                                 signed int acc_shift,
                                 signed int out_data_format,
                                 void *p_handle);

int xa_nn_conv2d_std_8x16(signed short *p_out,
                          signed short *p_inp,
                          signed char *p_kernel,
                          signed short *p_bias,
                          signed int input_height,
                          signed int input_width,
                          signed int input_channels,
                          signed int kernel_height,
                          signed int kernel_width,
                          signed int out_channels,
                          signed int x_stride,
                          signed int y_stride,
                          signed int x_padding,
                          signed int y_padding,
                          signed int out_height,
                          signed int out_width,
                          signed int bias_shift,
                          signed int acc_shift,
                          signed int out_data_format,
                          void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_8x8 == 1
void xa_nn_conv2d_std_8x8_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed char *p_out,
                                signed char *p_inp,
                                signed char *p_kernel,
                                signed char *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int kernel_width,
                                signed int out_channels,
                                signed int x_stride,
                                signed int y_stride,
                                signed int x_padding,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_width,
                                signed int bias_shift,
                                signed int acc_shift,
                                signed int out_data_format,
                                void *p_handle);

int xa_nn_conv2d_std_8x8(signed char *p_out,
                         signed char *p_inp,
                         signed char *p_kernel,
                         signed char *p_bias,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int kernel_width,
                         signed int out_channels,
                         signed int x_stride,
                         signed int y_stride,
                         signed int x_padding,
                         signed int y_padding,
                         signed int out_height,
                         signed int out_width,
                         signed int bias_shift,
                         signed int acc_shift,
                         signed int out_data_format,
                         void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_16x16 == 1
void xa_nn_conv2d_std_16x16_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed short *p_out,
                                  signed short *p_inp,
                                  signed short *p_kernel,
                                  signed short *p_bias,
                                  signed int input_height,
                                  signed int input_width,
                                  signed int input_channels,
                                  signed int kernel_height,
                                  signed int kernel_width,
                                  signed int out_channels,
                                  signed int x_stride,
                                  signed int y_stride,
                                  signed int x_padding,
                                  signed int y_padding,
                                  signed int out_height,
                                  signed int out_width,
                                  signed int bias_shift,
                                  signed int acc_shift,
                                  signed int out_data_format,
                                  void *p_handle);

int xa_nn_conv2d_std_16x16(signed short *p_out,
                           signed short *p_inp,
                           signed short *p_kernel,
                           signed short *p_bias,
                           signed int input_height,
                           signed int input_width,
                           signed int input_channels,
                           signed int kernel_height,
                           signed int kernel_width,
                           signed int out_channels,
                           signed int x_stride,
                           signed int y_stride,
                           signed int x_padding,
                           signed int y_padding,
                           signed int out_height,
                           signed int out_width,
                           signed int bias_shift,
                           signed int acc_shift,
                           signed int out_data_format,
                           void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_f32 == 1
void xa_nn_conv2d_std_f32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                float *p_out,
                                const float *p_inp,
                                const float *p_kernel,
                                const float *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int kernel_width,
                                signed int out_channels,
                                signed int x_stride,
                                signed int y_stride,
                                signed int x_padding,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_width,
                                signed int out_data_format,
                                void *p_handle);

int xa_nn_conv2d_std_f32(float *p_out,
                         const float *p_inp,
                         const float *p_kernel,
                         const float *p_bias,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int kernel_width,
                         signed int out_channels,
                         signed int x_stride,
                         signed int y_stride,
                         signed int x_padding,
                         signed int y_padding,
                         signed int out_height,
                         signed int out_width,
                         signed int out_data_format,
                         void *p_handle);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_f32 == 1
void xa_nn_conv2d_pointwise_f32_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      float *p_out,
                                      float *p_kernel,
                                      float *p_inp,
                                      float *p_bias,
                                      signed int input_height,
                                      signed int input_width,
                                      signed int input_channels,
                                      signed int out_channels,
                                      signed int out_data_format);

int xa_nn_conv2d_pointwise_f32(float *p_out,
                               float *p_kernel,
                               float *p_inp,
                               float *p_bias,
                               signed int input_height,
                               signed int input_width,
                               signed int input_channels,
                               signed int out_channels,
                               signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_8x16 == 1
void xa_nn_conv2d_pointwise_8x16_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       signed char *p_kernel,
                                       signed short *p_inp,
                                       signed short *p_bias,
                                       signed int input_height,
                                       signed int input_width,
                                       signed int input_channels,
                                       signed int out_channels,
                                       signed int acc_shift,
                                       signed int bias_shift,
                                       signed int out_data_format);

int xa_nn_conv2d_pointwise_8x16(signed short *p_out,
                                signed char *p_kernel,
                                signed short *p_inp,
                                signed short *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int out_channels,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_8x8 == 1
void xa_nn_conv2d_pointwise_8x8_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      signed char *p_out,
                                      signed char *p_kernel,
                                      signed char *p_inp,
                                      signed char *p_bias,
                                      signed int input_height,
                                      signed int input_width,
                                      signed int input_channels,
                                      signed int out_channels,
                                      signed int acc_shift,
                                      signed int bias_shift,
                                      signed int out_data_format);

int xa_nn_conv2d_pointwise_8x8(signed char *p_out,
                               signed char *p_kernel,
                               signed char *p_inp,
                               signed char *p_bias,
                               signed int input_height,
                               signed int input_width,
                               signed int input_channels,
                               signed int out_channels,
                               signed int acc_shift,
                               signed int bias_shift,
                               signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_getsize == 1
void xa_nn_conv2d_depthwise_getsize_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          signed int input_height,
                                          signed int input_width,
                                          signed int input_channels,
                                          signed int kernel_height,
                                          signed int kernel_width,
                                          signed int channels_multiplier,
                                          signed int x_stride,
                                          signed int y_stride,
                                          signed int x_padding,
                                          signed int y_padding,
                                          signed int output_height,
                                          signed int output_width,
                                          signed int circ_buf_precision,
                                          signed int inp_data_format);

int xa_nn_conv2d_depthwise_getsize(signed int input_height,
                                   signed int input_width,
                                   signed int input_channels,
                                   signed int kernel_height,
                                   signed int kernel_width,
                                   signed int channels_multiplier,
                                   signed int x_stride,
                                   signed int y_stride,
                                   signed int x_padding,
                                   signed int y_padding,
                                   signed int output_height,
                                   signed int output_width,
                                   signed int circ_buf_precision,
                                   signed int inp_data_format);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_8x8 == 1
void xa_nn_conv2d_depthwise_8x8_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      signed char *p_out,
                                      const signed char *p_kernel,
                                      const signed char *p_inp,
                                      const signed char *p_bias,
                                      signed int input_height,
                                      signed int input_width,
                                      signed int input_channels,
                                      signed int kernel_height,
                                      signed int kernel_width,
                                      signed int channels_multiplier,
                                      signed int x_stride,
                                      signed int y_stride,
                                      signed int x_padding,
                                      signed int y_padding,
                                      signed int out_height,
                                      signed int out_width,
                                      signed int acc_shift,
                                      signed int bias_shift,
                                      signed int inp_data_format,
                                      signed int out_data_format,
                                      void *p_scratch);

int xa_nn_conv2d_depthwise_8x8(signed char *p_out,
                               const signed char *p_kernel,
                               const signed char *p_inp,
                               const signed char *p_bias,
                               signed int input_height,
                               signed int input_width,
                               signed int input_channels,
                               signed int kernel_height,
                               signed int kernel_width,
                               signed int channels_multiplier,
                               signed int x_stride,
                               signed int y_stride,
                               signed int x_padding,
                               signed int y_padding,
                               signed int out_height,
                               signed int out_width,
                               signed int acc_shift,
                               signed int bias_shift,
                               signed int inp_data_format,
                               signed int out_data_format,
                               void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_f32 == 1
void xa_nn_conv2d_depthwise_f32_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      float *p_out,
                                      const float *p_kernel,
                                      const float *p_inp,
                                      const float *p_bias,
                                      signed int input_height,
                                      signed int input_width,
                                      signed int input_channels,
                                      signed int kernel_height,
                                      signed int kernel_width,
                                      signed int channels_multiplier,
                                      signed int x_stride,
                                      signed int y_stride,
                                      signed int x_padding,
                                      signed int y_padding,
                                      signed int out_height,
                                      signed int out_width,
                                      signed int inp_data_format,
                                      signed int out_data_format,
                                      void *p_scratch);

int xa_nn_conv2d_depthwise_f32(float *p_out,
                               const float *p_kernel,
                               const float *p_inp,
                               const float *p_bias,
                               signed int input_height,
                               signed int input_width,
                               signed int input_channels,
                               signed int kernel_height,
                               signed int kernel_width,
                               signed int channels_multiplier,
                               signed int x_stride,
                               signed int y_stride,
                               signed int x_padding,
                               signed int y_padding,
                               signed int out_height,
                               signed int out_width,
                               signed int inp_data_format,
                               signed int out_data_format,
                               void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_8x16 == 1
void xa_nn_conv2d_depthwise_8x16_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       const signed char *p_kernel,
                                       const signed short *p_inp,
                                       const signed short *p_bias,
                                       signed int input_height,
                                       signed int input_width,
                                       signed int input_channels,
                                       signed int kernel_height,
                                       signed int kernel_width,
                                       signed int channels_multiplier,
                                       signed int x_stride,
                                       signed int y_stride,
                                       signed int x_padding,
                                       signed int y_padding,
                                       signed int out_height,
                                       signed int out_width,
                                       signed int acc_shift,
                                       signed int bias_shift,
                                       signed int inp_data_format,
                                       signed int out_data_format,
                                       void *p_scratch);

int xa_nn_conv2d_depthwise_8x16(signed short *p_out,
                                const signed char *p_kernel,
                                const signed short *p_inp,
                                const signed short *p_bias,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int kernel_width,
                                signed int channels_multiplier,
                                signed int x_stride,
                                signed int y_stride,
                                signed int x_padding,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_width,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int inp_data_format,
                                signed int out_data_format,
                                void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_16x16 == 1
void xa_nn_conv2d_depthwise_16x16_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed short *p_out,
                                        const signed short *p_kernel,
                                        const signed short *p_inp,
                                        const signed short *p_bias,
                                        signed int input_height,
                                        signed int input_width,
                                        signed int input_channels,
                                        signed int kernel_height,
                                        signed int kernel_width,
                                        signed int channels_multiplier,
                                        signed int x_stride,
                                        signed int y_stride,
                                        signed int x_padding,
                                        signed int y_padding,
                                        signed int out_height,
                                        signed int out_width,
                                        signed int acc_shift,
                                        signed int bias_shift,
                                        signed int inp_data_format,
                                        signed int out_data_format,
                                        void *p_scratch);

int xa_nn_conv2d_depthwise_16x16(signed short *p_out,
                                 const signed short *p_kernel,
                                 const signed short *p_inp,
                                 const signed short *p_bias,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int input_channels,
                                 signed int kernel_height,
                                 signed int kernel_width,
                                 signed int channels_multiplier,
                                 signed int x_stride,
                                 signed int y_stride,
                                 signed int x_padding,
                                 signed int y_padding,
                                 signed int out_height,
                                 signed int out_width,
                                 signed int acc_shift,
                                 signed int bias_shift,
                                 signed int inp_data_format,
                                 signed int out_data_format,
                                 void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_16x16 == 1
void xa_nn_conv2d_pointwise_16x16_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed short *p_out,
                                        signed short *p_kernel,
                                        signed short *p_inp,
                                        signed short *p_bias,
                                        signed int input_height,
                                        signed int input_width,
                                        signed int input_channels,
                                        signed int out_channels,
                                        signed int acc_shift,
                                        signed int bias_shift,
                                        signed int out_data_format);

int xa_nn_conv2d_pointwise_16x16(signed short *p_out,
                                 signed short *p_kernel,
                                 signed short *p_inp,
                                 signed short *p_bias,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int input_channels,
                                 signed int out_channels,
                                 signed int acc_shift,
                                 signed int bias_shift,
                                 signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_avgpool_8 == 1
void xa_nn_avgpool_8_async(void (*cb)(void *, srtm_message *msg),
                           void *params,
                           signed char *p_out,
                           const signed char *p_inp,
                           signed int input_height,
                           signed int input_width,
                           signed int input_channels,
                           signed int kernel_height,
                           signed int kernel_width,
                           signed int x_stride,
                           signed int y_stride,
                           signed int x_padding,
                           signed int y_padding,
                           signed int out_height,
                           signed int out_width,
                           signed int inp_data_format,
                           signed int out_data_format,
                           void *p_scratch);

int xa_nn_avgpool_8(signed char *p_out,
                    const signed char *p_inp,
                    signed int input_height,
                    signed int input_width,
                    signed int input_channels,
                    signed int kernel_height,
                    signed int kernel_width,
                    signed int x_stride,
                    signed int y_stride,
                    signed int x_padding,
                    signed int y_padding,
                    signed int out_height,
                    signed int out_width,
                    signed int inp_data_format,
                    signed int out_data_format,
                    void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_avgpool_16 == 1
void xa_nn_avgpool_16_async(void (*cb)(void *, srtm_message *msg),
                            void *params,
                            signed short *p_out,
                            const signed short *p_inp,
                            signed int input_height,
                            signed int input_width,
                            signed int input_channels,
                            signed int kernel_height,
                            signed int kernel_width,
                            signed int x_stride,
                            signed int y_stride,
                            signed int x_padding,
                            signed int y_padding,
                            signed int out_height,
                            signed int out_width,
                            signed int inp_data_format,
                            signed int out_data_format,
                            void *p_scratch);

int xa_nn_avgpool_16(signed short *p_out,
                     const signed short *p_inp,
                     signed int input_height,
                     signed int input_width,
                     signed int input_channels,
                     signed int kernel_height,
                     signed int kernel_width,
                     signed int x_stride,
                     signed int y_stride,
                     signed int x_padding,
                     signed int y_padding,
                     signed int out_height,
                     signed int out_width,
                     signed int inp_data_format,
                     signed int out_data_format,
                     void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_avgpool_f32 == 1
void xa_nn_avgpool_f32_async(void (*cb)(void *, srtm_message *msg),
                             void *params,
                             float *p_out,
                             const float *p_inp,
                             signed int input_height,
                             signed int input_width,
                             signed int input_channels,
                             signed int kernel_height,
                             signed int kernel_width,
                             signed int x_stride,
                             signed int y_stride,
                             signed int x_padding,
                             signed int y_padding,
                             signed int out_height,
                             signed int out_width,
                             signed int inp_data_format,
                             signed int out_data_format,
                             void *p_scratch);

int xa_nn_avgpool_f32(float *p_out,
                      const float *p_inp,
                      signed int input_height,
                      signed int input_width,
                      signed int input_channels,
                      signed int kernel_height,
                      signed int kernel_width,
                      signed int x_stride,
                      signed int y_stride,
                      signed int x_padding,
                      signed int y_padding,
                      signed int out_height,
                      signed int out_width,
                      signed int inp_data_format,
                      signed int out_data_format,
                      void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_avgpool_asym8u == 1
void xa_nn_avgpool_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                unsigned char *p_out,
                                const unsigned char *p_inp,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int kernel_width,
                                signed int x_stride,
                                signed int y_stride,
                                signed int x_padding,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_width,
                                signed int inp_data_format,
                                signed int out_data_format,
                                void *p_scratch);

int xa_nn_avgpool_asym8u(unsigned char *p_out,
                         const unsigned char *p_inp,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int kernel_width,
                         signed int x_stride,
                         signed int y_stride,
                         signed int x_padding,
                         signed int y_padding,
                         signed int out_height,
                         signed int out_width,
                         signed int inp_data_format,
                         signed int out_data_format,
                         void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_avgpool_getsize == 1
void xa_nn_avgpool_getsize_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int input_channels,
                                 signed int inp_precision,
                                 signed int out_precision,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int kernel_height,
                                 signed int kernel_width,
                                 signed int x_stride,
                                 signed int y_stride,
                                 signed int x_padding,
                                 signed int y_padding,
                                 signed int out_height,
                                 signed int out_width,
                                 signed int inp_data_format,
                                 signed int out_data_format);

int xa_nn_avgpool_getsize(signed int input_channels,
                          signed int inp_precision,
                          signed int out_precision,
                          signed int input_height,
                          signed int input_width,
                          signed int kernel_height,
                          signed int kernel_width,
                          signed int x_stride,
                          signed int y_stride,
                          signed int x_padding,
                          signed int y_padding,
                          signed int out_height,
                          signed int out_width,
                          signed int inp_data_format,
                          signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_maxpool_8 == 1
void xa_nn_maxpool_8_async(void (*cb)(void *, srtm_message *msg),
                           void *params,
                           signed char *p_out,
                           const signed char *p_inp,
                           signed int input_height,
                           signed int input_width,
                           signed int input_channels,
                           signed int kernel_height,
                           signed int kernel_width,
                           signed int x_stride,
                           signed int y_stride,
                           signed int x_padding,
                           signed int y_padding,
                           signed int out_height,
                           signed int out_width,
                           signed int inp_data_format,
                           signed int out_data_format,
                           void *p_scratch);

int xa_nn_maxpool_8(signed char *p_out,
                    const signed char *p_inp,
                    signed int input_height,
                    signed int input_width,
                    signed int input_channels,
                    signed int kernel_height,
                    signed int kernel_width,
                    signed int x_stride,
                    signed int y_stride,
                    signed int x_padding,
                    signed int y_padding,
                    signed int out_height,
                    signed int out_width,
                    signed int inp_data_format,
                    signed int out_data_format,
                    void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_maxpool_16 == 1
void xa_nn_maxpool_16_async(void (*cb)(void *, srtm_message *msg),
                            void *params,
                            signed short *p_out,
                            const signed short *p_inp,
                            signed int input_height,
                            signed int input_width,
                            signed int input_channels,
                            signed int kernel_height,
                            signed int kernel_width,
                            signed int x_stride,
                            signed int y_stride,
                            signed int x_padding,
                            signed int y_padding,
                            signed int out_height,
                            signed int out_width,
                            signed int inp_data_format,
                            signed int out_data_format,
                            void *p_scratch);

int xa_nn_maxpool_16(signed short *p_out,
                     const signed short *p_inp,
                     signed int input_height,
                     signed int input_width,
                     signed int input_channels,
                     signed int kernel_height,
                     signed int kernel_width,
                     signed int x_stride,
                     signed int y_stride,
                     signed int x_padding,
                     signed int y_padding,
                     signed int out_height,
                     signed int out_width,
                     signed int inp_data_format,
                     signed int out_data_format,
                     void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_maxpool_f32 == 1
void xa_nn_maxpool_f32_async(void (*cb)(void *, srtm_message *msg),
                             void *params,
                             float *p_out,
                             const float *p_inp,
                             signed int input_height,
                             signed int input_width,
                             signed int input_channels,
                             signed int kernel_height,
                             signed int kernel_width,
                             signed int x_stride,
                             signed int y_stride,
                             signed int x_padding,
                             signed int y_padding,
                             signed int out_height,
                             signed int out_width,
                             signed int inp_data_format,
                             signed int out_data_format,
                             void *p_scratch);

int xa_nn_maxpool_f32(float *p_out,
                      const float *p_inp,
                      signed int input_height,
                      signed int input_width,
                      signed int input_channels,
                      signed int kernel_height,
                      signed int kernel_width,
                      signed int x_stride,
                      signed int y_stride,
                      signed int x_padding,
                      signed int y_padding,
                      signed int out_height,
                      signed int out_width,
                      signed int inp_data_format,
                      signed int out_data_format,
                      void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_maxpool_asym8u == 1
void xa_nn_maxpool_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                unsigned char *p_out,
                                const unsigned char *p_inp,
                                signed int input_height,
                                signed int input_width,
                                signed int input_channels,
                                signed int kernel_height,
                                signed int kernel_width,
                                signed int x_stride,
                                signed int y_stride,
                                signed int x_padding,
                                signed int y_padding,
                                signed int out_height,
                                signed int out_width,
                                signed int inp_data_format,
                                signed int out_data_format,
                                void *p_scratch);

int xa_nn_maxpool_asym8u(unsigned char *p_out,
                         const unsigned char *p_inp,
                         signed int input_height,
                         signed int input_width,
                         signed int input_channels,
                         signed int kernel_height,
                         signed int kernel_width,
                         signed int x_stride,
                         signed int y_stride,
                         signed int x_padding,
                         signed int y_padding,
                         signed int out_height,
                         signed int out_width,
                         signed int inp_data_format,
                         signed int out_data_format,
                         void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_maxpool_getsize == 1
void xa_nn_maxpool_getsize_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int input_channels,
                                 signed int inp_precision,
                                 signed int out_precision,
                                 signed int input_height,
                                 signed int input_width,
                                 signed int kernel_height,
                                 signed int kernel_width,
                                 signed int x_stride,
                                 signed int y_stride,
                                 signed int x_padding,
                                 signed int y_padding,
                                 signed int out_height,
                                 signed int out_width,
                                 signed int inp_data_format,
                                 signed int out_data_format);

int xa_nn_maxpool_getsize(signed int input_channels,
                          signed int inp_precision,
                          signed int out_precision,
                          signed int input_height,
                          signed int input_width,
                          signed int kernel_height,
                          signed int kernel_width,
                          signed int x_stride,
                          signed int y_stride,
                          signed int x_padding,
                          signed int y_padding,
                          signed int out_height,
                          signed int out_width,
                          signed int inp_data_format,
                          signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_fully_connected_f32 == 1
void xa_nn_fully_connected_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_weight,
                                     const float *p_inp,
                                     const float *p_bias,
                                     signed int weight_depth,
                                     signed int out_depth);

int xa_nn_fully_connected_f32(float *p_out,
                              const float *p_weight,
                              const float *p_inp,
                              const float *p_bias,
                              signed int weight_depth,
                              signed int out_depth);
#endif

#if NN_ENABLE_xa_nn_fully_connected_16x16_16 == 1
void xa_nn_fully_connected_16x16_16_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          signed short *p_out,
                                          signed short *p_weight,
                                          signed short *p_inp,
                                          signed short *p_bias,
                                          signed int weight_depth,
                                          signed int out_depth,
                                          signed int acc_shift,
                                          signed int bias_shift);

int xa_nn_fully_connected_16x16_16(signed short *p_out,
                                   signed short *p_weight,
                                   signed short *p_inp,
                                   signed short *p_bias,
                                   signed int weight_depth,
                                   signed int out_depth,
                                   signed int acc_shift,
                                   signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_fully_connected_8x16_16 == 1
void xa_nn_fully_connected_8x16_16_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed short *p_out,
                                         signed char *p_weight,
                                         signed short *p_inp,
                                         signed short *p_bias,
                                         signed int weight_depth,
                                         signed int out_depth,
                                         signed int acc_shift,
                                         signed int bias_shift);

int xa_nn_fully_connected_8x16_16(signed short *p_out,
                                  signed char *p_weight,
                                  signed short *p_inp,
                                  signed short *p_bias,
                                  signed int weight_depth,
                                  signed int out_depth,
                                  signed int acc_shift,
                                  signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_fully_connected_8x8_8 == 1
void xa_nn_fully_connected_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed char *p_out,
                                       signed char *p_weight,
                                       signed char *p_inp,
                                       signed char *p_bias,
                                       signed int weight_depth,
                                       signed int out_depth,
                                       signed int acc_shift,
                                       signed int bias_shift);

int xa_nn_fully_connected_8x8_8(signed char *p_out,
                                signed char *p_weight,
                                signed char *p_inp,
                                signed char *p_bias,
                                signed int weight_depth,
                                signed int out_depth,
                                signed int acc_shift,
                                signed int bias_shift);
#endif

#if NN_ENABLE_xa_nn_fully_connected_asym8uxasym8u_asym8u == 1
void xa_nn_fully_connected_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                                      void *params,
                                                      unsigned char *p_out,
                                                      const unsigned char *p_weight,
                                                      const unsigned char *p_inp,
                                                      const signed int *p_bias,
                                                      signed int weight_depth,
                                                      signed int out_depth,
                                                      signed int input_zero_bias,
                                                      signed int weight_zero_bias,
                                                      signed int out_multiplier,
                                                      signed int out_shift,
                                                      signed int out_zero_bias);

int xa_nn_fully_connected_asym8uxasym8u_asym8u(unsigned char *p_out,
                                               const unsigned char *p_weight,
                                               const unsigned char *p_inp,
                                               const signed int *p_bias,
                                               signed int weight_depth,
                                               signed int out_depth,
                                               signed int input_zero_bias,
                                               signed int weight_zero_bias,
                                               signed int out_multiplier,
                                               signed int out_shift,
                                               signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_fully_connected_sym8sxasym8s_asym8s == 1
void xa_nn_fully_connected_sym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                                     void *params,
                                                     signed char *p_out,
                                                     const signed char *p_weight,
                                                     const signed char *p_inp,
                                                     const signed int *p_bias,
                                                     signed int weight_depth,
                                                     signed int out_depth,
                                                     signed int input_zero_bias,
                                                     signed int out_multiplier,
                                                     signed int out_shift,
                                                     signed int out_zero_bias);

int xa_nn_fully_connected_sym8sxasym8s_asym8s(signed char *p_out,
                                              const signed char *p_weight,
                                              const signed char *p_inp,
                                              const signed int *p_bias,
                                              signed int weight_depth,
                                              signed int out_depth,
                                              signed int input_zero_bias,
                                              signed int out_multiplier,
                                              signed int out_shift,
                                              signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_asym8u_asym8u == 1
void xa_nn_vec_activation_min_max_asym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                                      void *params,
                                                      unsigned char *p_out,
                                                      const unsigned char *p_vec,
                                                      signed int activation_min,
                                                      signed int activation_max,
                                                      signed int vec_length);

int xa_nn_vec_activation_min_max_asym8u_asym8u(unsigned char *p_out,
                                               const unsigned char *p_vec,
                                               signed int activation_min,
                                               signed int activation_max,
                                               signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_f32_f32 == 1
void xa_nn_vec_activation_min_max_f32_f32_async(void (*cb)(void *, srtm_message *msg),
                                                void *params,
                                                float *p_out,
                                                const float *p_vec,
                                                float activation_min,
                                                float activation_max,
                                                signed int vec_length);

int xa_nn_vec_activation_min_max_f32_f32(
    float *p_out, const float *p_vec, float activation_min, float activation_max, signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_softmax_asym8u_asym8u == 1
void xa_nn_vec_softmax_asym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           unsigned char *p_out,
                                           const unsigned char *p_vec,
                                           signed int diffmin,
                                           signed int input_left_shift,
                                           signed int input_multiplier,
                                           signed int vec_length,
                                           void *p_scratch);

int xa_nn_vec_softmax_asym8u_asym8u(unsigned char *p_out,
                                    const unsigned char *p_vec,
                                    signed int diffmin,
                                    signed int input_left_shift,
                                    signed int input_multiplier,
                                    signed int vec_length,
                                    void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_vec_softmax_asym8s_asym8s == 1
void xa_nn_vec_softmax_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           signed char *p_out,
                                           const signed char *p_vec,
                                           signed int diffmin,
                                           signed int input_beta_left_shift,
                                           signed int input_beta_multiplier,
                                           signed int vec_length,
                                           void *p_scratch);

int xa_nn_vec_softmax_asym8s_asym8s(signed char *p_out,
                                    const signed char *p_vec,
                                    signed int diffmin,
                                    signed int input_beta_left_shift,
                                    signed int input_beta_multiplier,
                                    signed int vec_length,
                                    void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_vec_softmax_asym8s_16 == 1
void xa_nn_vec_softmax_asym8s_16_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       const signed char *p_vec,
                                       signed int diffmin,
                                       signed int input_beta_left_shift,
                                       signed int input_beta_multiplier,
                                       signed int vec_length,
                                       void *p_scratch);

int xa_nn_vec_softmax_asym8s_16(signed short *p_out,
                                const signed char *p_vec,
                                signed int diffmin,
                                signed int input_beta_left_shift,
                                signed int input_beta_multiplier,
                                signed int vec_length,
                                void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_asym8u_asym8u == 1
void xa_nn_vec_sigmoid_asym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           unsigned char *p_out,
                                           const unsigned char *p_vec,
                                           signed int zero_point,
                                           signed int input_range_radius,
                                           signed int input_multiplier,
                                           signed int input_left_shift,
                                           signed int vec_length);

int xa_nn_vec_sigmoid_asym8u_asym8u(unsigned char *p_out,
                                    const unsigned char *p_vec,
                                    signed int zero_point,
                                    signed int input_range_radius,
                                    signed int input_multiplier,
                                    signed int input_left_shift,
                                    signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_asym8s_asym8s == 1
void xa_nn_vec_sigmoid_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           signed char *p_out,
                                           const signed char *p_vec,
                                           signed int zero_point,
                                           signed int input_range_radius,
                                           signed int input_multiplier,
                                           signed int input_left_shift,
                                           signed int vec_length);

int xa_nn_vec_sigmoid_asym8s_asym8s(signed char *p_out,
                                    const signed char *p_vec,
                                    signed int zero_point,
                                    signed int input_range_radius,
                                    signed int input_multiplier,
                                    signed int input_left_shift,
                                    signed int vec_length);
#endif

#if NN_ENABLE_get_softmax_scratch_size == 1
void get_softmax_scratch_size_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int inp_precision,
                                    signed int out_precision,
                                    signed int length);

int get_softmax_scratch_size(signed int inp_precision, signed int out_precision, signed int length);
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_8_8 == 1
void xa_nn_vec_activation_min_max_8_8_async(void (*cb)(void *, srtm_message *msg),
                                            void *params,
                                            signed char *p_out,
                                            const signed char *p_vec,
                                            signed int activation_min,
                                            signed int activation_max,
                                            signed int vec_length);

int xa_nn_vec_activation_min_max_8_8(signed char *p_out,
                                     const signed char *p_vec,
                                     signed int activation_min,
                                     signed int activation_max,
                                     signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_16_16 == 1
void xa_nn_vec_activation_min_max_16_16_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed short *p_out,
                                              const signed short *p_vec,
                                              signed int activation_min,
                                              signed int activation_max,
                                              signed int vec_length);

int xa_nn_vec_activation_min_max_16_16(signed short *p_out,
                                       const signed short *p_vec,
                                       signed int activation_min,
                                       signed int activation_max,
                                       signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_asym8u_asym8u == 1
void xa_nn_vec_relu_asym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        unsigned char *p_out,
                                        const unsigned char *p_vec,
                                        signed int inp_zero_bias,
                                        signed int out_multiplier,
                                        signed int out_shift,
                                        signed int out_zero_bias,
                                        signed int quantized_activation_min,
                                        signed int quantized_activation_max,
                                        signed int vec_length);

int xa_nn_vec_relu_asym8u_asym8u(unsigned char *p_out,
                                 const unsigned char *p_vec,
                                 signed int inp_zero_bias,
                                 signed int out_multiplier,
                                 signed int out_shift,
                                 signed int out_zero_bias,
                                 signed int quantized_activation_min,
                                 signed int quantized_activation_max,
                                 signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_relu_asym8s_asym8s == 1
void xa_nn_vec_relu_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed char *p_out,
                                        const signed char *p_vec,
                                        signed int inp_zero_bias,
                                        signed int out_multiplier,
                                        signed int out_shift,
                                        signed int out_zero_bias,
                                        signed int quantized_activation_min,
                                        signed int quantized_activation_max,
                                        signed int vec_length);

int xa_nn_vec_relu_asym8s_asym8s(signed char *p_out,
                                 const signed char *p_vec,
                                 signed int inp_zero_bias,
                                 signed int out_multiplier,
                                 signed int out_shift,
                                 signed int out_zero_bias,
                                 signed int quantized_activation_min,
                                 signed int quantized_activation_max,
                                 signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_prelu_asym8s_asym8s == 1
void xa_nn_vec_prelu_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed char *p_out,
                                         const signed char *p_vec,
                                         const signed char *p_vec_alpha,
                                         signed int inp_zero_bias,
                                         signed int alpha_zero_bias,
                                         signed int alpha_multiplier,
                                         signed int alpha_shift,
                                         signed int out_multiplier,
                                         signed int out_shift,
                                         signed int out_zero_bias,
                                         signed int vec_length);

int xa_nn_vec_prelu_asym8s_asym8s(signed char *p_out,
                                  const signed char *p_vec,
                                  const signed char *p_vec_alpha,
                                  signed int inp_zero_bias,
                                  signed int alpha_zero_bias,
                                  signed int alpha_multiplier,
                                  signed int alpha_shift,
                                  signed int out_multiplier,
                                  signed int out_shift,
                                  signed int out_zero_bias,
                                  signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_hard_swish_asym8s_asym8s == 1
void xa_nn_vec_hard_swish_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              const signed char *p_vec,
                                              signed int inp_zero_bias,
                                              signed short reluish_multiplier,
                                              signed int reluish_shift,
                                              signed short out_multiplier,
                                              signed int out_shift,
                                              signed int out_zero_bias,
                                              signed int vec_length);

int xa_nn_vec_hard_swish_asym8s_asym8s(signed char *p_out,
                                       const signed char *p_vec,
                                       signed int inp_zero_bias,
                                       signed short reluish_multiplier,
                                       signed int reluish_shift,
                                       signed short out_multiplier,
                                       signed int out_shift,
                                       signed int out_zero_bias,
                                       signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_vec_tanh_asym8s_asym8s == 1
void xa_nn_vec_tanh_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed char *p_out,
                                        const signed char *p_vec,
                                        signed int zero_point,
                                        signed int input_range_radius,
                                        signed int input_multiplier,
                                        signed int input_left_shift,
                                        signed int vec_length);

int xa_nn_vec_tanh_asym8s_asym8s(signed char *p_out,
                                 const signed char *p_vec,
                                 signed int zero_point,
                                 signed int input_range_radius,
                                 signed int input_multiplier,
                                 signed int input_left_shift,
                                 signed int vec_length);
#endif

#if NN_ENABLE_xa_nn_conv1d_std_asym8uxasym8u == 1
void xa_nn_conv1d_std_asym8uxasym8u_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          unsigned char *p_out,
                                          unsigned char *p_inp,
                                          unsigned char *p_kernel,
                                          signed int *p_bias,
                                          signed int input_height,
                                          signed int input_width,
                                          signed int input_channels,
                                          signed int kernel_height,
                                          signed int out_channels,
                                          signed int y_stride,
                                          signed int y_padding,
                                          signed int out_height,
                                          signed int input_zero_bias,
                                          signed int kernel_zero_bias,
                                          signed int out_multiplier,
                                          signed int out_shift,
                                          signed int out_zero_bias,
                                          signed int out_data_format,
                                          void *p_scratch);

int xa_nn_conv1d_std_asym8uxasym8u(unsigned char *p_out,
                                   unsigned char *p_inp,
                                   unsigned char *p_kernel,
                                   signed int *p_bias,
                                   signed int input_height,
                                   signed int input_width,
                                   signed int input_channels,
                                   signed int kernel_height,
                                   signed int out_channels,
                                   signed int y_stride,
                                   signed int y_padding,
                                   signed int out_height,
                                   signed int input_zero_bias,
                                   signed int kernel_zero_bias,
                                   signed int out_multiplier,
                                   signed int out_shift,
                                   signed int out_zero_bias,
                                   signed int out_data_format,
                                   void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_asym8uxasym8u == 1
void xa_nn_conv2d_std_asym8uxasym8u_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          unsigned char *p_out,
                                          const unsigned char *p_inp,
                                          const unsigned char *p_kernel,
                                          const signed int *p_bias,
                                          signed int input_height,
                                          signed int input_width,
                                          signed int input_channels,
                                          signed int kernel_height,
                                          signed int kernel_width,
                                          signed int out_channels,
                                          signed int x_stride,
                                          signed int y_stride,
                                          signed int x_padding,
                                          signed int y_padding,
                                          signed int out_height,
                                          signed int out_width,
                                          signed int input_zero_bias,
                                          signed int kernel_zero_bias,
                                          signed int out_multiplier,
                                          signed int out_shift,
                                          signed int out_zero_bias,
                                          signed int out_data_format,
                                          void *p_scratch);

int xa_nn_conv2d_std_asym8uxasym8u(unsigned char *p_out,
                                   const unsigned char *p_inp,
                                   const unsigned char *p_kernel,
                                   const signed int *p_bias,
                                   signed int input_height,
                                   signed int input_width,
                                   signed int input_channels,
                                   signed int kernel_height,
                                   signed int kernel_width,
                                   signed int out_channels,
                                   signed int x_stride,
                                   signed int y_stride,
                                   signed int x_padding,
                                   signed int y_padding,
                                   signed int out_height,
                                   signed int out_width,
                                   signed int input_zero_bias,
                                   signed int kernel_zero_bias,
                                   signed int out_multiplier,
                                   signed int out_shift,
                                   signed int out_zero_bias,
                                   signed int out_data_format,
                                   void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_std_per_chan_sym8sxasym8s == 1
void xa_nn_conv2d_std_per_chan_sym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                                  void *params,
                                                  signed char *p_out,
                                                  const signed char *p_inp,
                                                  const signed char *p_kernel,
                                                  const signed int *p_bias,
                                                  signed int input_height,
                                                  signed int input_width,
                                                  signed int input_channels,
                                                  signed int kernel_height,
                                                  signed int kernel_width,
                                                  signed int out_channels,
                                                  signed int x_stride,
                                                  signed int y_stride,
                                                  signed int x_padding,
                                                  signed int y_padding,
                                                  signed int out_height,
                                                  signed int out_width,
                                                  signed int input_zero_bias,
                                                  signed int *p_out_multiplier,
                                                  signed int *p_out_shift,
                                                  signed int out_zero_bias,
                                                  signed int out_data_format,
                                                  void *p_scratch);

int xa_nn_conv2d_std_per_chan_sym8sxasym8s(signed char *p_out,
                                           const signed char *p_inp,
                                           const signed char *p_kernel,
                                           const signed int *p_bias,
                                           signed int input_height,
                                           signed int input_width,
                                           signed int input_channels,
                                           signed int kernel_height,
                                           signed int kernel_width,
                                           signed int out_channels,
                                           signed int x_stride,
                                           signed int y_stride,
                                           signed int x_padding,
                                           signed int y_padding,
                                           signed int out_height,
                                           signed int out_width,
                                           signed int input_zero_bias,
                                           signed int *p_out_multiplier,
                                           signed int *p_out_shift,
                                           signed int out_zero_bias,
                                           signed int out_data_format,
                                           void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_matXvec_batch_asym8uxasym8u_asym8u == 1
void xa_nn_matXvec_batch_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                                    void *params,
                                                    unsigned char **p_out,
                                                    unsigned char *p_mat1,
                                                    unsigned char **p_vec1,
                                                    signed int *p_bias,
                                                    signed int rows,
                                                    signed int cols1,
                                                    signed int row_stride1,
                                                    signed int vec_count,
                                                    signed int mat1_zero_bias,
                                                    signed int vec1_zero_bias,
                                                    signed int out_multiplier,
                                                    signed int out_shift,
                                                    signed int out_zero_bias);

int xa_nn_matXvec_batch_asym8uxasym8u_asym8u(unsigned char **p_out,
                                             unsigned char *p_mat1,
                                             unsigned char **p_vec1,
                                             signed int *p_bias,
                                             signed int rows,
                                             signed int cols1,
                                             signed int row_stride1,
                                             signed int vec_count,
                                             signed int mat1_zero_bias,
                                             signed int vec1_zero_bias,
                                             signed int out_multiplier,
                                             signed int out_shift,
                                             signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_matmul_asym8uxasym8u_asym8u == 1
void xa_nn_matmul_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             unsigned char *p_out,
                                             const unsigned char *p_mat1,
                                             const unsigned char *p_mat2,
                                             const signed int *p_bias,
                                             signed int rows,
                                             signed int cols,
                                             signed int row_stride,
                                             signed int vec_count,
                                             signed int vec_offset,
                                             signed int out_offset,
                                             signed int out_stride,
                                             signed int mat1_zero_bias,
                                             signed int vec1_zero_bias,
                                             signed int out_multiplier,
                                             signed int out_shift,
                                             signed int out_zero_bias);

int xa_nn_matmul_asym8uxasym8u_asym8u(unsigned char *p_out,
                                      const unsigned char *p_mat1,
                                      const unsigned char *p_mat2,
                                      const signed int *p_bias,
                                      signed int rows,
                                      signed int cols,
                                      signed int row_stride,
                                      signed int vec_count,
                                      signed int vec_offset,
                                      signed int out_offset,
                                      signed int out_stride,
                                      signed int mat1_zero_bias,
                                      signed int vec1_zero_bias,
                                      signed int out_multiplier,
                                      signed int out_shift,
                                      signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_matmul_per_chan_sym8sxasym8s_asym8s == 1
void xa_nn_matmul_per_chan_sym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                                     void *params,
                                                     signed char *p_out,
                                                     const signed char *p_mat1,
                                                     const signed char *p_vec1,
                                                     const signed int *p_bias,
                                                     signed int rows,
                                                     signed int cols1,
                                                     signed int row_stride1,
                                                     signed int vec_count,
                                                     signed int vec_offset,
                                                     signed int out_offset,
                                                     signed int out_stride,
                                                     signed int vec1_zero_bias,
                                                     const signed int *p_out_multiplier,
                                                     const signed int *p_out_shift,
                                                     signed int out_zero_bias);

int xa_nn_matmul_per_chan_sym8sxasym8s_asym8s(signed char *p_out,
                                              const signed char *p_mat1,
                                              const signed char *p_vec1,
                                              const signed int *p_bias,
                                              signed int rows,
                                              signed int cols1,
                                              signed int row_stride1,
                                              signed int vec_count,
                                              signed int vec_offset,
                                              signed int out_offset,
                                              signed int out_stride,
                                              signed int vec1_zero_bias,
                                              const signed int *p_out_multiplier,
                                              const signed int *p_out_shift,
                                              signed int out_zero_bias);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_asym8uxasym8u == 1
void xa_nn_conv2d_depthwise_asym8uxasym8u_async(void (*cb)(void *, srtm_message *msg),
                                                void *params,
                                                unsigned char *p_out,
                                                const unsigned char *p_kernel,
                                                const unsigned char *p_inp,
                                                const signed int *p_bias,
                                                signed int input_height,
                                                signed int input_width,
                                                signed int input_channels,
                                                signed int kernel_height,
                                                signed int kernel_width,
                                                signed int channels_multiplier,
                                                signed int x_stride,
                                                signed int y_stride,
                                                signed int x_padding,
                                                signed int y_padding,
                                                signed int out_height,
                                                signed int out_width,
                                                signed int input_zero_bias,
                                                signed int kernel_zero_bias,
                                                signed int out_multiplier,
                                                signed int out_shift,
                                                signed int out_zero_bias,
                                                signed int inp_data_format,
                                                signed int out_data_format,
                                                void *p_scratch);

int xa_nn_conv2d_depthwise_asym8uxasym8u(unsigned char *p_out,
                                         const unsigned char *p_kernel,
                                         const unsigned char *p_inp,
                                         const signed int *p_bias,
                                         signed int input_height,
                                         signed int input_width,
                                         signed int input_channels,
                                         signed int kernel_height,
                                         signed int kernel_width,
                                         signed int channels_multiplier,
                                         signed int x_stride,
                                         signed int y_stride,
                                         signed int x_padding,
                                         signed int y_padding,
                                         signed int out_height,
                                         signed int out_width,
                                         signed int input_zero_bias,
                                         signed int kernel_zero_bias,
                                         signed int out_multiplier,
                                         signed int out_shift,
                                         signed int out_zero_bias,
                                         signed int inp_data_format,
                                         signed int out_data_format,
                                         void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_asym8uxasym8u == 1
void xa_nn_conv2d_pointwise_asym8uxasym8u_async(void (*cb)(void *, srtm_message *msg),
                                                void *params,
                                                unsigned char *p_out,
                                                unsigned char *p_kernel,
                                                unsigned char *p_inp,
                                                signed int *p_bias,
                                                signed int input_height,
                                                signed int input_width,
                                                signed int input_channels,
                                                signed int out_channels,
                                                signed int input_zero_bias,
                                                signed int kernel_zero_bias,
                                                signed int out_multiplier,
                                                signed int out_shift,
                                                signed int out_zero_bias,
                                                signed int out_data_format);

int xa_nn_conv2d_pointwise_asym8uxasym8u(unsigned char *p_out,
                                         unsigned char *p_kernel,
                                         unsigned char *p_inp,
                                         signed int *p_bias,
                                         signed int input_height,
                                         signed int input_width,
                                         signed int input_channels,
                                         signed int out_channels,
                                         signed int input_zero_bias,
                                         signed int kernel_zero_bias,
                                         signed int out_multiplier,
                                         signed int out_shift,
                                         signed int out_zero_bias,
                                         signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_conv2d_depthwise_per_chan_sym8sxasym8s == 1
void xa_nn_conv2d_depthwise_per_chan_sym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                                        void *params,
                                                        signed char *p_out,
                                                        const signed char *p_kernel,
                                                        const signed char *p_inp,
                                                        const signed int *p_bias,
                                                        signed int input_height,
                                                        signed int input_width,
                                                        signed int input_channels,
                                                        signed int kernel_height,
                                                        signed int kernel_width,
                                                        signed int channels_multiplier,
                                                        signed int x_stride,
                                                        signed int y_stride,
                                                        signed int x_padding,
                                                        signed int y_padding,
                                                        signed int out_height,
                                                        signed int out_width,
                                                        signed int input_zero_bias,
                                                        const signed int *p_out_multiplier,
                                                        const signed int *p_out_shift,
                                                        signed int out_zero_bias,
                                                        signed int inp_data_format,
                                                        signed int out_data_format,
                                                        void *p_scratch);

int xa_nn_conv2d_depthwise_per_chan_sym8sxasym8s(signed char *p_out,
                                                 const signed char *p_kernel,
                                                 const signed char *p_inp,
                                                 const signed int *p_bias,
                                                 signed int input_height,
                                                 signed int input_width,
                                                 signed int input_channels,
                                                 signed int kernel_height,
                                                 signed int kernel_width,
                                                 signed int channels_multiplier,
                                                 signed int x_stride,
                                                 signed int y_stride,
                                                 signed int x_padding,
                                                 signed int y_padding,
                                                 signed int out_height,
                                                 signed int out_width,
                                                 signed int input_zero_bias,
                                                 const signed int *p_out_multiplier,
                                                 const signed int *p_out_shift,
                                                 signed int out_zero_bias,
                                                 signed int inp_data_format,
                                                 signed int out_data_format,
                                                 void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_conv2d_pointwise_per_chan_sym8sxasym8s == 1
void xa_nn_conv2d_pointwise_per_chan_sym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                                        void *params,
                                                        signed char *p_out,
                                                        signed char *p_kernel,
                                                        signed char *p_inp,
                                                        signed int *p_bias,
                                                        signed int input_height,
                                                        signed int input_width,
                                                        signed int input_channels,
                                                        signed int out_channels,
                                                        signed int input_zero_bias,
                                                        signed int *p_out_multiplier,
                                                        signed int *p_out_shift,
                                                        signed int out_zero_bias,
                                                        signed int out_data_format);

int xa_nn_conv2d_pointwise_per_chan_sym8sxasym8s(signed char *p_out,
                                                 signed char *p_kernel,
                                                 signed char *p_inp,
                                                 signed int *p_bias,
                                                 signed int input_height,
                                                 signed int input_width,
                                                 signed int input_channels,
                                                 signed int out_channels,
                                                 signed int input_zero_bias,
                                                 signed int *p_out_multiplier,
                                                 signed int *p_out_shift,
                                                 signed int out_zero_bias,
                                                 signed int out_data_format);
#endif

#if NN_ENABLE_xa_nn_elm_mul_f32xf32_f32 == 1
void xa_nn_elm_mul_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm);

int xa_nn_elm_mul_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_add_f32xf32_f32 == 1
void xa_nn_elm_add_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm);

int xa_nn_elm_add_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_mul_acc_f32xf32_f32 == 1
void xa_nn_elm_mul_acc_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         float *p_out,
                                         const float *p_inp1,
                                         const float *p_inp2,
                                         signed int num_elm);

int xa_nn_elm_mul_acc_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_sub_f32xf32_f32 == 1
void xa_nn_elm_sub_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm);

int xa_nn_elm_sub_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_div_f32xf32_f32 == 1
void xa_nn_elm_div_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm);

int xa_nn_elm_div_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_floor_f32_f32 == 1
void xa_nn_elm_floor_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_inp, signed int num_elm);

int xa_nn_elm_floor_f32_f32(float *p_out, const float *p_inp, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_add_asym8uxasym8u_asym8u == 1
void xa_nn_elm_add_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              unsigned char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_left_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const unsigned char *p_inp1,
                                              signed int inp1_zero_bias,
                                              signed int inp1_left_shift,
                                              signed int inp1_multiplier,
                                              const unsigned char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int inp2_left_shift,
                                              signed int inp2_multiplier,
                                              signed int left_shift,
                                              signed int num_elm);

int xa_nn_elm_add_asym8uxasym8u_asym8u(unsigned char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_left_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const unsigned char *p_inp1,
                                       signed int inp1_zero_bias,
                                       signed int inp1_left_shift,
                                       signed int inp1_multiplier,
                                       const unsigned char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int inp2_left_shift,
                                       signed int inp2_multiplier,
                                       signed int left_shift,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_add_asym8sxasym8s_asym8s == 1
void xa_nn_elm_add_asym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_left_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const signed char *p_inp1,
                                              signed int inp1_zero_bias,
                                              signed int inp1_left_shift,
                                              signed int inp1_multiplier,
                                              const signed char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int inp2_left_shift,
                                              signed int inp2_multiplier,
                                              signed int left_shift,
                                              signed int num_elm);

int xa_nn_elm_add_asym8sxasym8s_asym8s(signed char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_left_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const signed char *p_inp1,
                                       signed int inp1_zero_bias,
                                       signed int inp1_left_shift,
                                       signed int inp1_multiplier,
                                       const signed char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int inp2_left_shift,
                                       signed int inp2_multiplier,
                                       signed int left_shift,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_sub_asym8uxasym8u_asym8u == 1
void xa_nn_elm_sub_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              unsigned char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_left_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const unsigned char *p_inp1,
                                              signed int inp1_zero_bias,
                                              signed int inp1_left_shift,
                                              signed int inp1_multiplier,
                                              const unsigned char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int inp2_left_shift,
                                              signed int inp2_multiplier,
                                              signed int left_shift,
                                              signed int num_elm);

int xa_nn_elm_sub_asym8uxasym8u_asym8u(unsigned char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_left_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const unsigned char *p_inp1,
                                       signed int inp1_zero_bias,
                                       signed int inp1_left_shift,
                                       signed int inp1_multiplier,
                                       const unsigned char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int inp2_left_shift,
                                       signed int inp2_multiplier,
                                       signed int left_shift,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_sub_asym8sxasym8s_asym8s == 1
void xa_nn_elm_sub_asym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_left_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const signed char *p_inp1,
                                              signed int inp1_zero_bias,
                                              signed int inp1_left_shift,
                                              signed int inp1_multiplier,
                                              const signed char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int inp2_left_shift,
                                              signed int inp2_multiplier,
                                              signed int left_shift,
                                              signed int num_elm);

int xa_nn_elm_sub_asym8sxasym8s_asym8s(signed char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_left_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const signed char *p_inp1,
                                       signed int inp1_zero_bias,
                                       signed int inp1_left_shift,
                                       signed int inp1_multiplier,
                                       const signed char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int inp2_left_shift,
                                       signed int inp2_multiplier,
                                       signed int left_shift,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_mul_asym8uxasym8u_asym8u == 1
void xa_nn_elm_mul_asym8uxasym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              unsigned char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const unsigned char *p_inp1,
                                              signed int inp1_zero_bias,
                                              const unsigned char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int num_elm);

int xa_nn_elm_mul_asym8uxasym8u_asym8u(unsigned char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const unsigned char *p_inp1,
                                       signed int inp1_zero_bias,
                                       const unsigned char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_mul_asym8sxasym8s_asym8s == 1
void xa_nn_elm_mul_asym8sxasym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              signed int out_zero_bias,
                                              signed int out_shift,
                                              signed int out_multiplier,
                                              signed int out_activation_min,
                                              signed int out_activation_max,
                                              const signed char *p_inp1,
                                              signed int inp1_zero_bias,
                                              const signed char *p_inp2,
                                              signed int inp2_zero_bias,
                                              signed int num_elm);

int xa_nn_elm_mul_asym8sxasym8s_asym8s(signed char *p_out,
                                       signed int out_zero_bias,
                                       signed int out_shift,
                                       signed int out_multiplier,
                                       signed int out_activation_min,
                                       signed int out_activation_max,
                                       const signed char *p_inp1,
                                       signed int inp1_zero_bias,
                                       const signed char *p_inp2,
                                       signed int inp2_zero_bias,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_quantize_asym16s_asym8s == 1
void xa_nn_elm_quantize_asym16s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             signed char *p_out,
                                             const signed short *p_inp,
                                             signed int inp_zero_bias,
                                             signed int out_zero_bias,
                                             signed int out_shift,
                                             signed int out_multiplier,
                                             signed int num_elm);

int xa_nn_elm_quantize_asym16s_asym8s(signed char *p_out,
                                      const signed short *p_inp,
                                      signed int inp_zero_bias,
                                      signed int out_zero_bias,
                                      signed int out_shift,
                                      signed int out_multiplier,
                                      signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_max_8x8_8 == 1
void xa_nn_elm_max_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed char *p_in1,
                               const signed char *p_in2,
                               signed int num_element);

int xa_nn_elm_max_8x8_8(signed char *p_out, const signed char *p_in1, const signed char *p_in2, signed int num_element);
#endif

#if NN_ENABLE_xa_nn_elm_min_8x8_8 == 1
void xa_nn_elm_min_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed char *p_in1,
                               const signed char *p_in2,
                               signed int num_element);

int xa_nn_elm_min_8x8_8(signed char *p_out, const signed char *p_in1, const signed char *p_in2, signed int num_element);
#endif

#if NN_ENABLE_xa_nn_elm_equal_asym8sxasym8s == 1
void xa_nn_elm_equal_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed char *p_out,
                                         const signed char *p_inp1,
                                         signed int inp1_zero_bias,
                                         signed int inp1_shift,
                                         signed int inp1_multiplier,
                                         const signed char *p_inp2,
                                         signed int inp2_zero_bias,
                                         signed int inp2_shift,
                                         signed int inp2_multiplier,
                                         signed int left_shift,
                                         signed int num_elm);

int xa_nn_elm_equal_asym8sxasym8s(signed char *p_out,
                                  const signed char *p_inp1,
                                  signed int inp1_zero_bias,
                                  signed int inp1_shift,
                                  signed int inp1_multiplier,
                                  const signed char *p_inp2,
                                  signed int inp2_zero_bias,
                                  signed int inp2_shift,
                                  signed int inp2_multiplier,
                                  signed int left_shift,
                                  signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_notequal_asym8sxasym8s == 1
void xa_nn_elm_notequal_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                            void *params,
                                            signed char *p_out,
                                            const signed char *p_inp1,
                                            signed int inp1_zero_bias,
                                            signed int inp1_shift,
                                            signed int inp1_multiplier,
                                            const signed char *p_inp2,
                                            signed int inp2_zero_bias,
                                            signed int inp2_shift,
                                            signed int inp2_multiplier,
                                            signed int left_shift,
                                            signed int num_elm);

int xa_nn_elm_notequal_asym8sxasym8s(signed char *p_out,
                                     const signed char *p_inp1,
                                     signed int inp1_zero_bias,
                                     signed int inp1_shift,
                                     signed int inp1_multiplier,
                                     const signed char *p_inp2,
                                     signed int inp2_zero_bias,
                                     signed int inp2_shift,
                                     signed int inp2_multiplier,
                                     signed int left_shift,
                                     signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_greater_asym8sxasym8s == 1
void xa_nn_elm_greater_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                           void *params,
                                           signed char *p_out,
                                           const signed char *p_inp1,
                                           signed int inp1_zero_bias,
                                           signed int inp1_shift,
                                           signed int inp1_multiplier,
                                           const signed char *p_inp2,
                                           signed int inp2_zero_bias,
                                           signed int inp2_shift,
                                           signed int inp2_multiplier,
                                           signed int left_shift,
                                           signed int num_elm);

int xa_nn_elm_greater_asym8sxasym8s(signed char *p_out,
                                    const signed char *p_inp1,
                                    signed int inp1_zero_bias,
                                    signed int inp1_shift,
                                    signed int inp1_multiplier,
                                    const signed char *p_inp2,
                                    signed int inp2_zero_bias,
                                    signed int inp2_shift,
                                    signed int inp2_multiplier,
                                    signed int left_shift,
                                    signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_greaterequal_asym8sxasym8s == 1
void xa_nn_elm_greaterequal_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                                void *params,
                                                signed char *p_out,
                                                const signed char *p_inp1,
                                                signed int inp1_zero_bias,
                                                signed int inp1_shift,
                                                signed int inp1_multiplier,
                                                const signed char *p_inp2,
                                                signed int inp2_zero_bias,
                                                signed int inp2_shift,
                                                signed int inp2_multiplier,
                                                signed int left_shift,
                                                signed int num_elm);

int xa_nn_elm_greaterequal_asym8sxasym8s(signed char *p_out,
                                         const signed char *p_inp1,
                                         signed int inp1_zero_bias,
                                         signed int inp1_shift,
                                         signed int inp1_multiplier,
                                         const signed char *p_inp2,
                                         signed int inp2_zero_bias,
                                         signed int inp2_shift,
                                         signed int inp2_multiplier,
                                         signed int left_shift,
                                         signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_less_asym8sxasym8s == 1
void xa_nn_elm_less_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                        void *params,
                                        signed char *p_out,
                                        const signed char *p_inp1,
                                        signed int inp1_zero_bias,
                                        signed int inp1_shift,
                                        signed int inp1_multiplier,
                                        const signed char *p_inp2,
                                        signed int inp2_zero_bias,
                                        signed int inp2_shift,
                                        signed int inp2_multiplier,
                                        signed int left_shift,
                                        signed int num_elm);

int xa_nn_elm_less_asym8sxasym8s(signed char *p_out,
                                 const signed char *p_inp1,
                                 signed int inp1_zero_bias,
                                 signed int inp1_shift,
                                 signed int inp1_multiplier,
                                 const signed char *p_inp2,
                                 signed int inp2_zero_bias,
                                 signed int inp2_shift,
                                 signed int inp2_multiplier,
                                 signed int left_shift,
                                 signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_lessequal_asym8sxasym8s == 1
void xa_nn_elm_lessequal_asym8sxasym8s_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             signed char *p_out,
                                             const signed char *p_inp1,
                                             signed int inp1_zero_bias,
                                             signed int inp1_shift,
                                             signed int inp1_multiplier,
                                             const signed char *p_inp2,
                                             signed int inp2_zero_bias,
                                             signed int inp2_shift,
                                             signed int inp2_multiplier,
                                             signed int left_shift,
                                             signed int num_elm);

int xa_nn_elm_lessequal_asym8sxasym8s(signed char *p_out,
                                      const signed char *p_inp1,
                                      signed int inp1_zero_bias,
                                      signed int inp1_shift,
                                      signed int inp1_multiplier,
                                      const signed char *p_inp2,
                                      signed int inp2_zero_bias,
                                      signed int inp2_shift,
                                      signed int inp2_multiplier,
                                      signed int left_shift,
                                      signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_reduce_max_getsize_nhwc == 1
void xa_nn_reduce_max_getsize_nhwc_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed int inp_precision,
                                         const signed int *p_inp_shape,
                                         signed int num_inp_dims,
                                         const signed int *p_axis,
                                         signed int num_axis_dims);

int xa_nn_reduce_max_getsize_nhwc(signed int inp_precision,
                                  const signed int *p_inp_shape,
                                  signed int num_inp_dims,
                                  const signed int *p_axis,
                                  signed int num_axis_dims);
#endif

#if NN_ENABLE_xa_nn_reduce_max_4D_asym8s_asym8s == 1
void xa_nn_reduce_max_4D_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                             void *params,
                                             signed char *p_out,
                                             const signed int *p_out_shape,
                                             const signed char *p_inp,
                                             const signed int *p_inp_shape,
                                             const signed int *p_axis,
                                             signed int num_out_dims,
                                             signed int num_inp_dims,
                                             signed int num_axis_dims,
                                             void *p_scratch);

int xa_nn_reduce_max_4D_asym8s_asym8s(signed char *p_out,
                                      const signed int *p_out_shape,
                                      const signed char *p_inp,
                                      const signed int *p_inp_shape,
                                      const signed int *p_axis,
                                      signed int num_out_dims,
                                      signed int num_inp_dims,
                                      signed int num_axis_dims,
                                      void *p_scratch);
#endif

#if NN_ENABLE_xa_nn_elm_logicaland_boolxbool_bool == 1
void xa_nn_elm_logicaland_boolxbool_bool_async(void (*cb)(void *, srtm_message *msg),
                                               void *params,
                                               signed char *p_out,
                                               const signed char *p_inp1,
                                               const signed char *p_inp2,
                                               signed int num_elm);

int xa_nn_elm_logicaland_boolxbool_bool(signed char *p_out,
                                        const signed char *p_inp1,
                                        const signed char *p_inp2,
                                        signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_logicalor_boolxbool_bool == 1
void xa_nn_elm_logicalor_boolxbool_bool_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              const signed char *p_inp1,
                                              const signed char *p_inp2,
                                              signed int num_elm);

int xa_nn_elm_logicalor_boolxbool_bool(signed char *p_out,
                                       const signed char *p_inp1,
                                       const signed char *p_inp2,
                                       signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_elm_logicalnot_bool_bool == 1
void xa_nn_elm_logicalnot_bool_bool_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          signed char *p_out,
                                          const signed char *p_inp,
                                          signed int num_elm);

int xa_nn_elm_logicalnot_bool_bool(signed char *p_out, const signed char *p_inp, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_l2_norm_f32 == 1
void xa_nn_l2_norm_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_inp, signed int num_elm);

int xa_nn_l2_norm_f32(float *p_out, const float *p_inp, signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_l2_norm_asym8s_asym8s == 1
void xa_nn_l2_norm_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed char *p_out,
                                       const signed char *p_inp,
                                       signed int zero_point,
                                       signed int num_elm);

int xa_nn_l2_norm_asym8s_asym8s(signed char *p_out,
                                const signed char *p_inp,
                                signed int zero_point,
                                signed int num_elm);
#endif

#if NN_ENABLE_xa_nn_dot_prod_f32xf32_f32 == 1
void xa_nn_dot_prod_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      float *p_out,
                                      const float *p_inp1,
                                      const float *p_inp2,
                                      signed int vec_length,
                                      signed int num_vecs);

int xa_nn_dot_prod_f32xf32_f32(
    float *p_out, const float *p_inp1, const float *p_inp2, signed int vec_length, signed int num_vecs);
#endif

#if NN_ENABLE_xa_nn_dot_prod_16x16_asym8s == 1
void xa_nn_dot_prod_16x16_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed char *p_out,
                                       const signed short *p_inp1_start,
                                       const signed short *p_inp2_start,
                                       const signed int *bias_ptr,
                                       signed int vec_length,
                                       signed int out_multiplier,
                                       signed int out_shift,
                                       signed int out_zero_bias,
                                       signed int vec_count);

int xa_nn_dot_prod_16x16_asym8s(signed char *p_out,
                                const signed short *p_inp1_start,
                                const signed short *p_inp2_start,
                                const signed int *bias_ptr,
                                signed int vec_length,
                                signed int out_multiplier,
                                signed int out_shift,
                                signed int out_zero_bias,
                                signed int vec_count);
#endif

void hifi_resizenearest_f_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                float *dst,
                                const float *src,
                                const float scaleH,
                                const float scaleW,
                                const unsigned int *inWdims,
                                const unsigned int *outWdims);
signed int hifi_resizenearest_f(float *dst,
                                const float *src,
                                const float scaleH,
                                const float scaleW,
                                const unsigned int *inWdims,
                                const unsigned int *outWdims);

void hifi_resizenearest_i8_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 char *dst,
                                 const char *src,
                                 const float scaleH,
                                 const float scaleW,
                                 const unsigned int *inWdims,
                                 const unsigned int *outWdims);
signed int hifi_resizenearest_i8(char *dst,
                                 const char *src,
                                 const float scaleH,
                                 const float scaleW,
                                 const unsigned int *inWdims,
                                 const unsigned int *outWdims);

void hifi_inference_async(void (*cb)(void *, srtm_message *msg),
                          void *params,
                          float *dst,
                          unsigned char *constantWeight,
                          unsigned char *mutableWeight,
                          unsigned char *activations);
signed int hifi_inference(unsigned char *constantWeight, unsigned char *mutableWeight, unsigned char *activations);

void xa_nn_get_version_async(void (*cb)(void *, srtm_message *msg), void *params);
unsigned int xa_nn_get_version();

#ifdef __cplusplus
}
#endif
#endif // __DSP_NN_H__
