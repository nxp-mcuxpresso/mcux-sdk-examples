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

#include "config_nn.h"
#include "dsp_nn.h"
#include "dsp_ipc.h"
#include "srtm_config.h"

// ARM CM33 bus base addresses
#define ARM_BUS_CACHEABLE_NONSECURE     0x00000000
#define ARM_BUS_NON_CACHEABLE_NONSECURE 0x20000000

// Macro function to modify address in order to access
// memory through a specific bus.
#define ARM_ADDR_WITH_BUS(addr, bus) (((unsigned int)(addr)&0x0FFFFFFF) + (bus))


void xa_nn_get_version_async(void (*cb)(void *, srtm_message *msg), void *params)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_check_version;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    dsp_ipc_send_async(&msg_async);
}

unsigned int xa_nn_get_version()
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_check_version;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (unsigned int)(msg.param[0]);
}

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
                                  signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;


    msg_async.msg.param[0]  = (unsigned int)(p_out);
	msg_async.msg.param[1]  = (unsigned int)(p_mat1);
	msg_async.msg.param[2]  = (unsigned int)(p_mat2);
	msg_async.msg.param[3]  = (unsigned int)(p_vec1);
	msg_async.msg.param[4]  = (unsigned int)(p_vec2);
	msg_async.msg.param[5]  = (unsigned int)(p_bias);

    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                           signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);

    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                  signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                           signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                  signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_64;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                           signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_64;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16_tanh;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16_tanh;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16_sigmoid;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                   void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_16x16_16_sigmoid;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int vec_count)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_16x16_64;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_mat1);
    msg_async.msg.param[2] = (unsigned int)(p_vec1);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(rows);
    msg_async.msg.param[5] = (unsigned int)(cols1);
    msg_async.msg.param[6] = (unsigned int)(row_stride1);
    msg_async.msg.param[7] = (unsigned int)(acc_shift);
    msg_async.msg.param[8] = (unsigned int)(bias_shift);
    msg_async.msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_matXvec_batch_16x16_64(signed long long **p_out,
                                 signed short *p_mat1,
                                 signed short **p_vec1,
                                 signed short *p_bias,
                                 signed int rows,
                                 signed int cols1,
                                 signed int row_stride1,
                                 signed int acc_shift,
                                 signed int bias_shift,
                                 signed int vec_count)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_16x16_64;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_mat1);
    msg.param[2] = (unsigned int)(p_vec1);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(rows);
    msg.param[5] = (unsigned int)(cols1);
    msg.param[6] = (unsigned int)(row_stride1);
    msg.param[7] = (unsigned int)(acc_shift);
    msg.param[8] = (unsigned int)(bias_shift);
    msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int out_stride)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_16x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols);
    msg_async.msg.param[6]  = (unsigned int)(row_stride);
    msg_async.msg.param[7]  = (unsigned int)(acc_shift);
    msg_async.msg.param[8]  = (unsigned int)(bias_shift);
    msg_async.msg.param[9]  = (unsigned int)(vec_count);
    msg_async.msg.param[10] = (unsigned int)(vec_offset);
    msg_async.msg.param[11] = (unsigned int)(out_offset);
    msg_async.msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int out_stride)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_16x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols);
    msg.param[6]  = (unsigned int)(row_stride);
    msg.param[7]  = (unsigned int)(acc_shift);
    msg.param[8]  = (unsigned int)(bias_shift);
    msg.param[9]  = (unsigned int)(vec_count);
    msg.param[10] = (unsigned int)(vec_offset);
    msg.param[11] = (unsigned int)(out_offset);
    msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_64;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_64;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16_tanh;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                               void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16_tanh;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                         void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16_sigmoid;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                  void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x16_16_sigmoid;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       signed int vec_count)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_8x16_64;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_mat1);
    msg_async.msg.param[2] = (unsigned int)(p_vec1);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(rows);
    msg_async.msg.param[5] = (unsigned int)(cols1);
    msg_async.msg.param[6] = (unsigned int)(row_stride1);
    msg_async.msg.param[7] = (unsigned int)(acc_shift);
    msg_async.msg.param[8] = (unsigned int)(bias_shift);
    msg_async.msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_matXvec_batch_8x16_64(signed long long **p_out,
                                signed char *p_mat1,
                                signed short **p_vec1,
                                signed short *p_bias,
                                signed int rows,
                                signed int cols1,
                                signed int row_stride1,
                                signed int acc_shift,
                                signed int bias_shift,
                                signed int vec_count)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_8x16_64;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_mat1);
    msg.param[2] = (unsigned int)(p_vec1);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(rows);
    msg.param[5] = (unsigned int)(cols1);
    msg.param[6] = (unsigned int)(row_stride1);
    msg.param[7] = (unsigned int)(acc_shift);
    msg.param[8] = (unsigned int)(bias_shift);
    msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                signed int out_stride)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_8x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols);
    msg_async.msg.param[6]  = (unsigned int)(row_stride);
    msg_async.msg.param[7]  = (unsigned int)(acc_shift);
    msg_async.msg.param[8]  = (unsigned int)(bias_shift);
    msg_async.msg.param[9]  = (unsigned int)(vec_count);
    msg_async.msg.param[10] = (unsigned int)(vec_offset);
    msg_async.msg.param[11] = (unsigned int)(out_offset);
    msg_async.msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_async(&msg_async);
}
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
                         signed int out_stride)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_8x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols);
    msg.param[6]  = (unsigned int)(row_stride);
    msg.param[7]  = (unsigned int)(acc_shift);
    msg.param[8]  = (unsigned int)(bias_shift);
    msg.param[9]  = (unsigned int)(vec_count);
    msg.param[10] = (unsigned int)(vec_offset);
    msg.param[11] = (unsigned int)(out_offset);
    msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                               signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                        signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                         signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                         signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                    void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8_tanh;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                             void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8_tanh;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8_sigmoid;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(acc_shift);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(bias_precision);
    msg_async.msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_8x8_8_sigmoid;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(acc_shift);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(bias_precision);
    msg.param[14] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      signed int vec_count)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_8x8_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_mat1);
    msg_async.msg.param[2] = (unsigned int)(p_vec1);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(rows);
    msg_async.msg.param[5] = (unsigned int)(cols1);
    msg_async.msg.param[6] = (unsigned int)(row_stride1);
    msg_async.msg.param[7] = (unsigned int)(acc_shift);
    msg_async.msg.param[8] = (unsigned int)(bias_shift);
    msg_async.msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_matXvec_batch_8x8_32(signed int **p_out,
                               signed char *p_mat1,
                               signed char **p_vec1,
                               signed char *p_bias,
                               signed int rows,
                               signed int cols1,
                               signed int row_stride1,
                               signed int acc_shift,
                               signed int bias_shift,
                               signed int vec_count)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_8x8_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_mat1);
    msg.param[2] = (unsigned int)(p_vec1);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(rows);
    msg.param[5] = (unsigned int)(cols1);
    msg.param[6] = (unsigned int)(row_stride1);
    msg.param[7] = (unsigned int)(acc_shift);
    msg.param[8] = (unsigned int)(bias_shift);
    msg.param[9] = (unsigned int)(vec_count);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                              signed int out_stride)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_8x8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols);
    msg_async.msg.param[6]  = (unsigned int)(row_stride);
    msg_async.msg.param[7]  = (unsigned int)(acc_shift);
    msg_async.msg.param[8]  = (unsigned int)(bias_shift);
    msg_async.msg.param[9]  = (unsigned int)(vec_count);
    msg_async.msg.param[10] = (unsigned int)(vec_offset);
    msg_async.msg.param[11] = (unsigned int)(out_offset);
    msg_async.msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_async(&msg_async);
}
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
                       signed int out_stride)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_8x8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols);
    msg.param[6]  = (unsigned int)(row_stride);
    msg.param[7]  = (unsigned int)(acc_shift);
    msg.param[8]  = (unsigned int)(bias_shift);
    msg.param[9]  = (unsigned int)(vec_count);
    msg.param[10] = (unsigned int)(vec_offset);
    msg.param[11] = (unsigned int)(out_offset);
    msg.param[12] = (unsigned int)(out_stride);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             float *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32_sigmoid;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                      float *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32_sigmoid;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          float *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32_tanh;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                   float *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32_tanh;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                     signed int row_stride2)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);

    dsp_ipc_send_async(&msg_async);
}
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
                              signed int row_stride2)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           signed int vec_count)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_mat1);
    msg_async.msg.param[2] = (unsigned int)(p_vec1);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(rows);
    msg_async.msg.param[5] = (unsigned int)(cols1);
    msg_async.msg.param[6] = (unsigned int)(row_stride1);
    msg_async.msg.param[7] = (unsigned int)(vec_count);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_matXvec_batch_f32xf32_f32(float **p_out,
                                    float *p_mat1,
                                    float **p_vec1,
                                    float *p_bias,
                                    signed int rows,
                                    signed int cols1,
                                    signed int row_stride1,
                                    signed int vec_count)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_mat1);
    msg.param[2] = (unsigned int)(p_vec1);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(rows);
    msg.param[5] = (unsigned int)(cols1);
    msg.param[6] = (unsigned int)(row_stride1);
    msg.param[7] = (unsigned int)(vec_count);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                    signed int out_stride)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols);
    msg_async.msg.param[6]  = (unsigned int)(row_stride);
    msg_async.msg.param[7]  = (unsigned int)(vec_count);
    msg_async.msg.param[8]  = (unsigned int)(vec_offset);
    msg_async.msg.param[9]  = (unsigned int)(out_offset);
    msg_async.msg.param[10] = (unsigned int)(out_stride);

    dsp_ipc_send_async(&msg_async);
}
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
                             signed int out_stride)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols);
    msg.param[6]  = (unsigned int)(row_stride);
    msg.param[7]  = (unsigned int)(vec_count);
    msg.param[8]  = (unsigned int)(vec_offset);
    msg.param[9]  = (unsigned int)(out_offset);
    msg.param[10] = (unsigned int)(out_stride);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(mat1_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(mat2_zero_bias);
    msg_async.msg.param[13] = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[14] = (unsigned int)(vec2_zero_bias);
    msg_async.msg.param[15] = (unsigned int)(out_multiplier);
    msg_async.msg.param[16] = (unsigned int)(out_shift);
    msg_async.msg.param[17] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(mat1_zero_bias);
    msg.param[12] = (unsigned int)(mat2_zero_bias);
    msg.param[13] = (unsigned int)(vec1_zero_bias);
    msg.param[14] = (unsigned int)(vec2_zero_bias);
    msg.param[15] = (unsigned int)(out_multiplier);
    msg.param[16] = (unsigned int)(out_shift);
    msg.param[17] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_sym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_vec1);
    msg_async.msg.param[4]  = (unsigned int)(p_vec2);
    msg_async.msg.param[5]  = (unsigned int)(p_bias);
    msg_async.msg.param[6]  = (unsigned int)(rows);
    msg_async.msg.param[7]  = (unsigned int)(cols1);
    msg_async.msg.param[8]  = (unsigned int)(cols2);
    msg_async.msg.param[9]  = (unsigned int)(row_stride1);
    msg_async.msg.param[10] = (unsigned int)(row_stride2);
    msg_async.msg.param[11] = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(vec2_zero_bias);
    msg_async.msg.param[13] = (unsigned int)(out_multiplier);
    msg_async.msg.param[14] = (unsigned int)(out_shift);
    msg_async.msg.param[15] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                      signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_sym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_vec1);
    msg.param[4]  = (unsigned int)(p_vec2);
    msg.param[5]  = (unsigned int)(p_bias);
    msg.param[6]  = (unsigned int)(rows);
    msg.param[7]  = (unsigned int)(cols1);
    msg.param[8]  = (unsigned int)(cols2);
    msg.param[9]  = (unsigned int)(row_stride1);
    msg.param[10] = (unsigned int)(row_stride2);
    msg.param[11] = (unsigned int)(vec1_zero_bias);
    msg.param[12] = (unsigned int)(vec2_zero_bias);
    msg.param[13] = (unsigned int)(out_multiplier);
    msg.param[14] = (unsigned int)(out_shift);
    msg.param[15] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                    signed int out_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_out_stride_sym8sxasym8s_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_vec1);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols1);
    msg_async.msg.param[6]  = (unsigned int)(row_stride1);
    msg_async.msg.param[7]  = (unsigned int)(out_stride);
    msg_async.msg.param[8]  = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[9]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[10] = (unsigned int)(out_shift);

    dsp_ipc_send_async(&msg_async);
}
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
                                             signed int out_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_out_stride_sym8sxasym8s_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_vec1);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols1);
    msg.param[6]  = (unsigned int)(row_stride1);
    msg.param[7]  = (unsigned int)(out_stride);
    msg.param[8]  = (unsigned int)(vec1_zero_bias);
    msg.param[9]  = (unsigned int)(out_multiplier);
    msg.param[10] = (unsigned int)(out_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_32 == 1
void xa_nn_vec_sigmoid_32_32_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed int *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_32 == 1
void xa_nn_vec_tanh_32_32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed int *p_out,
                                const signed int *p_vec,
                                signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_tanh_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_32_32 == 1
void xa_nn_vec_relu_std_32_32_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int *p_out,
                                    const signed int *p_vec,
                                    signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_std_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_32_32 == 1
void xa_nn_vec_relu_32_32_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed int *p_out,
                                const signed int *p_vec,
                                signed int threshold,
                                signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(threshold);
    msg_async.msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_32_32(signed int *p_out, const signed int *p_vec, signed int threshold, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(threshold);
    msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu1_32_32 == 1
void xa_nn_vec_relu1_32_32_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int *p_out,
                                 const signed int *p_vec,
                                 signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu1_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu1_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu1_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu6_32_32 == 1
void xa_nn_vec_relu6_32_32_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 signed int *p_out,
                                 const signed int *p_vec,
                                 signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu6_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu6_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu6_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_softmax_32_32 == 1
void xa_nn_vec_softmax_32_32_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed int *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_softmax_32_32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_softmax_32_32(signed int *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_softmax_32_32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_f32_f32 == 1
void xa_nn_vec_sigmoid_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_tanh_f32_f32 == 1
void xa_nn_vec_tanh_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_tanh_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_tanh_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_tanh_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_f32_f32 == 1
void xa_nn_vec_relu_f32_f32_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  float *p_out,
                                  const float *p_vec,
                                  float threshold,
                                  signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(threshold);
    msg_async.msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_f32_f32(float *p_out, const float *p_vec, float threshold, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(threshold);
    msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_f32_f32 == 1
void xa_nn_vec_relu_std_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_std_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu1_f32_f32 == 1
void xa_nn_vec_relu1_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu1_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu1_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu1_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu6_f32_f32 == 1
void xa_nn_vec_relu6_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu6_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu6_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu6_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_softmax_f32_f32 == 1
void xa_nn_vec_softmax_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_softmax_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_softmax_f32_f32(float *p_out, const float *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_softmax_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_16 == 1
void xa_nn_vec_sigmoid_32_16_async(void (*cb)(void *, srtm_message *msg),
                                   void *params,
                                   signed short *p_out,
                                   const signed int *p_vec,
                                   signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_32_16(signed short *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_16 == 1
void xa_nn_vec_tanh_32_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                const signed int *p_vec,
                                signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_tanh_32_16(signed short *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_sigmoid_32_8 == 1
void xa_nn_vec_sigmoid_32_8_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed char *p_out,
                                  const signed int *p_vec,
                                  signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_32_8(signed char *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_32_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_tanh_32_8 == 1
void xa_nn_vec_tanh_32_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed int *p_vec,
                               signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_tanh_32_8(signed char *p_out, const signed int *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_tanh_32_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_16_16 == 1
void xa_nn_vec_relu_16_16_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                signed short *p_out,
                                const signed short *p_vec,
                                signed short threshold,
                                signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(threshold);
    msg_async.msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_16_16(signed short *p_out, const signed short *p_vec, signed short threshold, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(threshold);
    msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_16_16 == 1
void xa_nn_vec_relu_std_16_16_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed short *p_out,
                                    const signed short *p_vec,
                                    signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_std_16_16(signed short *p_out, const signed short *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_8_8 == 1
void xa_nn_vec_relu_8_8_async(void (*cb)(void *, srtm_message *msg),
                              void *params,
                              signed char *p_out,
                              const signed char *p_vec,
                              signed char threshold,
                              signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(threshold);
    msg_async.msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_8_8(signed char *p_out, const signed char *p_vec, signed char threshold, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(threshold);
    msg.param[3] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_relu_std_8_8 == 1
void xa_nn_vec_relu_std_8_8_async(void (*cb)(void *, srtm_message *msg),
                                  void *params,
                                  signed char *p_out,
                                  const signed char *p_vec,
                                  signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_std_8_8(signed char *p_out, const signed char *p_vec, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_std_8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_interpolation_q15 == 1
void xa_nn_vec_interpolation_q15_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed short *p_out,
                                       const signed short *p_ifact,
                                       const signed short *p_inp1,
                                       const signed short *p_inp2,
                                       signed int num_elements)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_interpolation_q15;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_ifact);
    msg_async.msg.param[2] = (unsigned int)(p_inp1);
    msg_async.msg.param[3] = (unsigned int)(p_inp2);
    msg_async.msg.param[4] = (unsigned int)(num_elements);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_interpolation_q15(signed short *p_out,
                                const signed short *p_ifact,
                                const signed short *p_inp1,
                                const signed short *p_inp2,
                                signed int num_elements)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_interpolation_q15;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_ifact);
    msg.param[2] = (unsigned int)(p_inp1);
    msg.param[3] = (unsigned int)(p_inp2);
    msg.param[4] = (unsigned int)(num_elements);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_conv1d_std_getsize == 1
void xa_nn_conv1d_std_getsize_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int kernel_height,
                                    signed int input_width,
                                    signed int input_channels,
                                    signed int input_precision)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_getsize;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(kernel_height);
    msg_async.msg.param[1] = (unsigned int)(input_width);
    msg_async.msg.param[2] = (unsigned int)(input_channels);
    msg_async.msg.param[3] = (unsigned int)(input_precision);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_conv1d_std_getsize(signed int kernel_height,
                             signed int input_width,
                             signed int input_channels,
                             signed int input_precision)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_getsize;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(kernel_height);
    msg.param[1] = (unsigned int)(input_width);
    msg.param[2] = (unsigned int)(input_channels);
    msg.param[3] = (unsigned int)(input_precision);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_8x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(out_channels);
    msg_async.msg.param[9]  = (unsigned int)(y_stride);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(acc_shift);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                          void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_8x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(out_channels);
    msg.param[9]  = (unsigned int)(y_stride);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(acc_shift);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_8x8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(out_channels);
    msg_async.msg.param[9]  = (unsigned int)(y_stride);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(acc_shift);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_8x8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(out_channels);
    msg.param[9]  = (unsigned int)(y_stride);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(acc_shift);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                  void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_16x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(out_channels);
    msg_async.msg.param[9]  = (unsigned int)(y_stride);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(bias_shift);
    msg_async.msg.param[13] = (unsigned int)(acc_shift);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                           void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_16x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(out_channels);
    msg.param[9]  = (unsigned int)(y_stride);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(bias_shift);
    msg.param[13] = (unsigned int)(acc_shift);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(out_channels);
    msg_async.msg.param[9]  = (unsigned int)(y_stride);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_data_format);
    msg_async.msg.param[13] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(out_channels);
    msg.param[9]  = (unsigned int)(y_stride);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_data_format);
    msg.param[13] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                    signed int input_precision)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_getsize;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(input_height);
    msg_async.msg.param[1] = (unsigned int)(input_channels);
    msg_async.msg.param[2] = (unsigned int)(kernel_height);
    msg_async.msg.param[3] = (unsigned int)(kernel_width);
    msg_async.msg.param[4] = (unsigned int)(y_stride);
    msg_async.msg.param[5] = (unsigned int)(y_padding);
    msg_async.msg.param[6] = (unsigned int)(out_height);
    msg_async.msg.param[7] = (unsigned int)(out_channels);
    msg_async.msg.param[8] = (unsigned int)(input_precision);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_conv2d_std_getsize(signed int input_height,
                             signed int input_channels,
                             signed int kernel_height,
                             signed int kernel_width,
                             signed int y_stride,
                             signed int y_padding,
                             signed int out_height,
                             signed int out_channels,
                             signed int input_precision)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_getsize;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(input_height);
    msg.param[1] = (unsigned int)(input_channels);
    msg.param[2] = (unsigned int)(kernel_height);
    msg.param[3] = (unsigned int)(kernel_width);
    msg.param[4] = (unsigned int)(y_stride);
    msg.param[5] = (unsigned int)(y_padding);
    msg.param[6] = (unsigned int)(out_height);
    msg.param[7] = (unsigned int)(out_channels);
    msg.param[8] = (unsigned int)(input_precision);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_8x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(bias_shift);
    msg_async.msg.param[17] = (unsigned int)(acc_shift);
    msg_async.msg.param[18] = (unsigned int)(out_data_format);
    msg_async.msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                          void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_8x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
	msg.param[1]  = (unsigned int)(p_inp);
	msg.param[2]  = (unsigned int)(p_kernel);
	msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(bias_shift);
    msg.param[17] = (unsigned int)(acc_shift);
    msg.param[18] = (unsigned int)(out_data_format);
    msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_8x8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(bias_shift);
    msg_async.msg.param[17] = (unsigned int)(acc_shift);
    msg_async.msg.param[18] = (unsigned int)(out_data_format);
    msg_async.msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_8x8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(bias_shift);
    msg.param[17] = (unsigned int)(acc_shift);
    msg.param[18] = (unsigned int)(out_data_format);
    msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                  void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_16x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(bias_shift);
    msg_async.msg.param[17] = (unsigned int)(acc_shift);
    msg_async.msg.param[18] = (unsigned int)(out_data_format);
    msg_async.msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                           void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_16x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(bias_shift);
    msg.param[17] = (unsigned int)(acc_shift);
    msg.param[18] = (unsigned int)(out_data_format);
    msg.param[19] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_handle)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(out_data_format);
    msg_async.msg.param[17] = (unsigned int)(p_handle);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_handle)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(out_data_format);
    msg.param[17] = (unsigned int)(p_handle);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_kernel);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(input_height);
    msg_async.msg.param[5] = (unsigned int)(input_width);
    msg_async.msg.param[6] = (unsigned int)(input_channels);
    msg_async.msg.param[7] = (unsigned int)(out_channels);
    msg_async.msg.param[8] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_conv2d_pointwise_f32(float *p_out,
                               float *p_kernel,
                               float *p_inp,
                               float *p_bias,
                               signed int input_height,
                               signed int input_width,
                               signed int input_channels,
                               signed int out_channels,
                               signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_kernel);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(input_height);
    msg.param[5] = (unsigned int)(input_width);
    msg.param[6] = (unsigned int)(input_channels);
    msg.param[7] = (unsigned int)(out_channels);
    msg.param[8] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_8x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(out_channels);
    msg_async.msg.param[8]  = (unsigned int)(acc_shift);
    msg_async.msg.param[9]  = (unsigned int)(bias_shift);
    msg_async.msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                                signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_8x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(out_channels);
    msg.param[8]  = (unsigned int)(acc_shift);
    msg.param[9]  = (unsigned int)(bias_shift);
    msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_8x8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(out_channels);
    msg_async.msg.param[8]  = (unsigned int)(acc_shift);
    msg_async.msg.param[9]  = (unsigned int)(bias_shift);
    msg_async.msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                               signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_8x8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(out_channels);
    msg.param[8]  = (unsigned int)(acc_shift);
    msg.param[9]  = (unsigned int)(bias_shift);
    msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          signed int inp_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_getsize;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(input_height);
    msg_async.msg.param[1]  = (unsigned int)(input_width);
    msg_async.msg.param[2]  = (unsigned int)(input_channels);
    msg_async.msg.param[3]  = (unsigned int)(kernel_height);
    msg_async.msg.param[4]  = (unsigned int)(kernel_width);
    msg_async.msg.param[5]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[6]  = (unsigned int)(x_stride);
    msg_async.msg.param[7]  = (unsigned int)(y_stride);
    msg_async.msg.param[8]  = (unsigned int)(x_padding);
    msg_async.msg.param[9]  = (unsigned int)(y_padding);
    msg_async.msg.param[10] = (unsigned int)(output_height);
    msg_async.msg.param[11] = (unsigned int)(output_width);
    msg_async.msg.param[12] = (unsigned int)(circ_buf_precision);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                                   signed int inp_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_getsize;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(input_height);
    msg.param[1]  = (unsigned int)(input_width);
    msg.param[2]  = (unsigned int)(input_channels);
    msg.param[3]  = (unsigned int)(kernel_height);
    msg.param[4]  = (unsigned int)(kernel_width);
    msg.param[5]  = (unsigned int)(channels_multiplier);
    msg.param[6]  = (unsigned int)(x_stride);
    msg.param[7]  = (unsigned int)(y_stride);
    msg.param[8]  = (unsigned int)(x_padding);
    msg.param[9]  = (unsigned int)(y_padding);
    msg.param[10] = (unsigned int)(output_height);
    msg.param[11] = (unsigned int)(output_width);
    msg.param[12] = (unsigned int)(circ_buf_precision);
    msg.param[13] = (unsigned int)(inp_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_8x8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(acc_shift);
    msg_async.msg.param[17] = (unsigned int)(bias_shift);
    msg_async.msg.param[18] = (unsigned int)(inp_data_format);
    msg_async.msg.param[19] = (unsigned int)(out_data_format);
    msg_async.msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                               void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_8x8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(acc_shift);
    msg.param[17] = (unsigned int)(bias_shift);
    msg.param[18] = (unsigned int)(inp_data_format);
    msg.param[19] = (unsigned int)(out_data_format);
    msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                      void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(inp_data_format);
    msg_async.msg.param[17] = (unsigned int)(out_data_format);
    msg_async.msg.param[18] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                               void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(inp_data_format);
    msg.param[17] = (unsigned int)(out_data_format);
    msg.param[18] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_8x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(acc_shift);
    msg_async.msg.param[17] = (unsigned int)(bias_shift);
    msg_async.msg.param[18] = (unsigned int)(inp_data_format);
    msg_async.msg.param[19] = (unsigned int)(out_data_format);
    msg_async.msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_8x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(acc_shift);
    msg.param[17] = (unsigned int)(bias_shift);
    msg.param[18] = (unsigned int)(inp_data_format);
    msg.param[19] = (unsigned int)(out_data_format);
    msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_16x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(acc_shift);
    msg_async.msg.param[17] = (unsigned int)(bias_shift);
    msg_async.msg.param[18] = (unsigned int)(inp_data_format);
    msg_async.msg.param[19] = (unsigned int)(out_data_format);
    msg_async.msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                 void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_16x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(acc_shift);
    msg.param[17] = (unsigned int)(bias_shift);
    msg.param[18] = (unsigned int)(inp_data_format);
    msg.param[19] = (unsigned int)(out_data_format);
    msg.param[20] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_16x16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(out_channels);
    msg_async.msg.param[8]  = (unsigned int)(acc_shift);
    msg_async.msg.param[9]  = (unsigned int)(bias_shift);
    msg_async.msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                                 signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_16x16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(out_channels);
    msg.param[8]  = (unsigned int)(acc_shift);
    msg.param[9]  = (unsigned int)(bias_shift);
    msg.param[10] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                           void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_avgpool_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);

    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                    void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_avgpool_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                            void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_avgpool_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                     void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_avgpool_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                             void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_avgpool_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                      void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_avgpool_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_avgpool_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_avgpool_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_avgpool_getsize;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(input_channels);
    msg_async.msg.param[1]  = (unsigned int)(inp_precision);
    msg_async.msg.param[2]  = (unsigned int)(out_precision);
    msg_async.msg.param[3]  = (unsigned int)(input_height);
    msg_async.msg.param[4]  = (unsigned int)(input_width);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_avgpool_getsize;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(input_channels);
    msg.param[1]  = (unsigned int)(inp_precision);
    msg.param[2]  = (unsigned int)(out_precision);
    msg.param[3]  = (unsigned int)(input_height);
    msg.param[4]  = (unsigned int)(input_width);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                           void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_maxpool_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                    void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_maxpool_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);
    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                            void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_maxpool_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                     void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_maxpool_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                             void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_maxpool_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                      void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_maxpool_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_maxpool_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(input_height);
    msg_async.msg.param[3]  = (unsigned int)(input_width);
    msg_async.msg.param[4]  = (unsigned int)(input_channels);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);
    msg_async.msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                         void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_maxpool_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(input_height);
    msg.param[3]  = (unsigned int)(input_width);
    msg.param[4]  = (unsigned int)(input_channels);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);
    msg.param[15] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                 signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_maxpool_getsize;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(input_channels);
    msg_async.msg.param[1]  = (unsigned int)(inp_precision);
    msg_async.msg.param[2]  = (unsigned int)(out_precision);
    msg_async.msg.param[3]  = (unsigned int)(input_height);
    msg_async.msg.param[4]  = (unsigned int)(input_width);
    msg_async.msg.param[5]  = (unsigned int)(kernel_height);
    msg_async.msg.param[6]  = (unsigned int)(kernel_width);
    msg_async.msg.param[7]  = (unsigned int)(x_stride);
    msg_async.msg.param[8]  = (unsigned int)(y_stride);
    msg_async.msg.param[9]  = (unsigned int)(x_padding);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(out_width);
    msg_async.msg.param[13] = (unsigned int)(inp_data_format);
    msg_async.msg.param[14] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                          signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_maxpool_getsize;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(input_channels);
    msg.param[1]  = (unsigned int)(inp_precision);
    msg.param[2]  = (unsigned int)(out_precision);
    msg.param[3]  = (unsigned int)(input_height);
    msg.param[4]  = (unsigned int)(input_width);
    msg.param[5]  = (unsigned int)(kernel_height);
    msg.param[6]  = (unsigned int)(kernel_width);
    msg.param[7]  = (unsigned int)(x_stride);
    msg.param[8]  = (unsigned int)(y_stride);
    msg.param[9]  = (unsigned int)(x_padding);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(out_width);
    msg.param[13] = (unsigned int)(inp_data_format);
    msg.param[14] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_fully_connected_f32 == 1
void xa_nn_fully_connected_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_weight,
                                     const float *p_inp,
                                     const float *p_bias,
                                     signed int weight_depth,
                                     signed int out_depth)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_weight);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(weight_depth);
    msg_async.msg.param[5] = (unsigned int)(out_depth);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_fully_connected_f32(float *p_out,
                              const float *p_weight,
                              const float *p_inp,
                              const float *p_bias,
                              signed int weight_depth,
                              signed int out_depth)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_weight);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(weight_depth);
    msg.param[5] = (unsigned int)(out_depth);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_16x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_weight);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(weight_depth);
    msg_async.msg.param[5] = (unsigned int)(out_depth);
    msg_async.msg.param[6] = (unsigned int)(acc_shift);
    msg_async.msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_fully_connected_16x16_16(signed short *p_out,
                                   signed short *p_weight,
                                   signed short *p_inp,
                                   signed short *p_bias,
                                   signed int weight_depth,
                                   signed int out_depth,
                                   signed int acc_shift,
                                   signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_16x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_weight);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(weight_depth);
    msg.param[5] = (unsigned int)(out_depth);
    msg.param[6] = (unsigned int)(acc_shift);
    msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                         signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_8x16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_weight);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(weight_depth);
    msg_async.msg.param[5] = (unsigned int)(out_depth);
    msg_async.msg.param[6] = (unsigned int)(acc_shift);
    msg_async.msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_fully_connected_8x16_16(signed short *p_out,
                                  signed char *p_weight,
                                  signed short *p_inp,
                                  signed short *p_bias,
                                  signed int weight_depth,
                                  signed int out_depth,
                                  signed int acc_shift,
                                  signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_8x16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_weight);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(weight_depth);
    msg.param[5] = (unsigned int)(out_depth);
    msg.param[6] = (unsigned int)(acc_shift);
    msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       signed int bias_shift)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_8x8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_weight);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_bias);
    msg_async.msg.param[4] = (unsigned int)(weight_depth);
    msg_async.msg.param[5] = (unsigned int)(out_depth);
    msg_async.msg.param[6] = (unsigned int)(acc_shift);
    msg_async.msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_fully_connected_8x8_8(signed char *p_out,
                                signed char *p_weight,
                                signed char *p_inp,
                                signed char *p_bias,
                                signed int weight_depth,
                                signed int out_depth,
                                signed int acc_shift,
                                signed int bias_shift)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_8x8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_weight);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_bias);
    msg.param[4] = (unsigned int)(weight_depth);
    msg.param[5] = (unsigned int)(out_depth);
    msg.param[6] = (unsigned int)(acc_shift);
    msg.param[7] = (unsigned int)(bias_shift);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                      signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_weight);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(weight_depth);
    msg_async.msg.param[5]  = (unsigned int)(out_depth);
    msg_async.msg.param[6]  = (unsigned int)(input_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(weight_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(out_shift);
    msg_async.msg.param[10] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                               signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_weight);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(weight_depth);
    msg.param[5]  = (unsigned int)(out_depth);
    msg.param[6]  = (unsigned int)(input_zero_bias);
    msg.param[7]  = (unsigned int)(weight_zero_bias);
    msg.param[8]  = (unsigned int)(out_multiplier);
    msg.param[9]  = (unsigned int)(out_shift);
    msg.param[10] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                     signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_fully_connected_sym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_weight);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);

    msg_async.msg.param[4] = (unsigned int)(weight_depth);
    msg_async.msg.param[5] = (unsigned int)(out_depth);
    msg_async.msg.param[6] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[7] = (unsigned int)(out_multiplier);
    msg_async.msg.param[8] = (unsigned int)(out_shift);
    msg_async.msg.param[9] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_fully_connected_sym8sxasym8s_asym8s(signed char *p_out,
                                              const signed char *p_weight,
                                              const signed char *p_inp,
                                              const signed int *p_bias,
                                              signed int weight_depth,
                                              signed int out_depth,
                                              signed int input_zero_bias,
                                              signed int out_multiplier,
                                              signed int out_shift,
                                              signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_fully_connected_sym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_weight);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);

    msg.param[4] = (unsigned int)(weight_depth);
    msg.param[5] = (unsigned int)(out_depth);
    msg.param[6] = (unsigned int)(input_zero_bias);
    msg.param[7] = (unsigned int)(out_multiplier);
    msg.param[8] = (unsigned int)(out_shift);
    msg.param[9] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_asym8u_asym8u == 1
void xa_nn_vec_activation_min_max_asym8u_asym8u_async(void (*cb)(void *, srtm_message *msg),
                                                      void *params,
                                                      unsigned char *p_out,
                                                      const unsigned char *p_vec,
                                                      signed int activation_min,
                                                      signed int activation_max,
                                                      signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_asym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(activation_min);
    msg_async.msg.param[3] = (unsigned int)(activation_max);
    msg_async.msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_activation_min_max_asym8u_asym8u(unsigned char *p_out,
                                               const unsigned char *p_vec,
                                               signed int activation_min,
                                               signed int activation_max,
                                               signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_asym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(activation_min);
    msg.param[3] = (unsigned int)(activation_max);
    msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_f32_f32 == 1
void xa_nn_vec_activation_min_max_f32_f32_async(void (*cb)(void *, srtm_message *msg),
                                                void *params,
                                                float *p_out,
                                                const float *p_vec,
                                                float activation_min,
                                                float activation_max,
                                                signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(activation_min);
    msg_async.msg.param[3] = (unsigned int)(activation_max);
    msg_async.msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_activation_min_max_f32_f32(
    float *p_out, const float *p_vec, float activation_min, float activation_max, signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(activation_min);
    msg.param[3] = (unsigned int)(activation_max);
    msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(diffmin);
    msg_async.msg.param[3] = (unsigned int)(input_left_shift);
    msg_async.msg.param[4] = (unsigned int)(input_multiplier);
    msg_async.msg.param[5] = (unsigned int)(vec_length);
    msg_async.msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_softmax_asym8u_asym8u(unsigned char *p_out,
                                    const unsigned char *p_vec,
                                    signed int diffmin,
                                    signed int input_left_shift,
                                    signed int input_multiplier,
                                    signed int vec_length,
                                    void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(diffmin);
    msg.param[3] = (unsigned int)(input_left_shift);
    msg.param[4] = (unsigned int)(input_multiplier);
    msg.param[5] = (unsigned int)(vec_length);
    msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(diffmin);
    msg_async.msg.param[3] = (unsigned int)(input_beta_left_shift);
    msg_async.msg.param[4] = (unsigned int)(input_beta_multiplier);
    msg_async.msg.param[5] = (unsigned int)(vec_length);
    msg_async.msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_softmax_asym8s_asym8s(signed char *p_out,
                                    const signed char *p_vec,
                                    signed int diffmin,
                                    signed int input_beta_left_shift,
                                    signed int input_beta_multiplier,
                                    signed int vec_length,
                                    void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(diffmin);
    msg.param[3] = (unsigned int)(input_beta_left_shift);
    msg.param[4] = (unsigned int)(input_beta_multiplier);
    msg.param[5] = (unsigned int)(vec_length);
    msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8s_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(diffmin);
    msg_async.msg.param[3] = (unsigned int)(input_beta_left_shift);
    msg_async.msg.param[4] = (unsigned int)(input_beta_multiplier);
    msg_async.msg.param[5] = (unsigned int)(vec_length);
    msg_async.msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_softmax_asym8s_16(signed short *p_out,
                                const signed char *p_vec,
                                signed int diffmin,
                                signed int input_beta_left_shift,
                                signed int input_beta_multiplier,
                                signed int vec_length,
                                void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_softmax_asym8s_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(diffmin);
    msg.param[3] = (unsigned int)(input_beta_left_shift);
    msg.param[4] = (unsigned int)(input_beta_multiplier);
    msg.param[5] = (unsigned int)(vec_length);
    msg.param[6] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_asym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(zero_point);
    msg_async.msg.param[3] = (unsigned int)(input_range_radius);
    msg_async.msg.param[4] = (unsigned int)(input_multiplier);
    msg_async.msg.param[5] = (unsigned int)(input_left_shift);
    msg_async.msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_asym8u_asym8u(unsigned char *p_out,
                                    const unsigned char *p_vec,
                                    signed int zero_point,
                                    signed int input_range_radius,
                                    signed int input_multiplier,
                                    signed int input_left_shift,
                                    signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_asym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(zero_point);
    msg.param[3] = (unsigned int)(input_range_radius);
    msg.param[4] = (unsigned int)(input_multiplier);
    msg.param[5] = (unsigned int)(input_left_shift);
    msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(zero_point);
    msg_async.msg.param[3] = (unsigned int)(input_range_radius);
    msg_async.msg.param[4] = (unsigned int)(input_multiplier);
    msg_async.msg.param[5] = (unsigned int)(input_left_shift);
    msg_async.msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_sigmoid_asym8s_asym8s(signed char *p_out,
                                    const signed char *p_vec,
                                    signed int zero_point,
                                    signed int input_range_radius,
                                    signed int input_multiplier,
                                    signed int input_left_shift,
                                    signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_sigmoid_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(zero_point);
    msg.param[3] = (unsigned int)(input_range_radius);
    msg.param[4] = (unsigned int)(input_multiplier);
    msg.param[5] = (unsigned int)(input_left_shift);
    msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_get_softmax_scratch_size == 1
void get_softmax_scratch_size_async(void (*cb)(void *, srtm_message *msg),
                                    void *params,
                                    signed int inp_precision,
                                    signed int out_precision,
                                    signed int length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_get_softmax_scratch_size;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(inp_precision);
    msg_async.msg.param[1] = (unsigned int)(out_precision);
    msg_async.msg.param[2] = (unsigned int)(length);

    dsp_ipc_send_async(&msg_async);
}
int get_softmax_scratch_size(signed int inp_precision, signed int out_precision, signed int length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_get_softmax_scratch_size;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(inp_precision);
    msg.param[1] = (unsigned int)(out_precision);
    msg.param[2] = (unsigned int)(length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_8_8 == 1
void xa_nn_vec_activation_min_max_8_8_async(void (*cb)(void *, srtm_message *msg),
                                            void *params,
                                            signed char *p_out,
                                            const signed char *p_vec,
                                            signed int activation_min,
                                            signed int activation_max,
                                            signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(activation_min);
    msg_async.msg.param[3] = (unsigned int)(activation_max);
    msg_async.msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_activation_min_max_8_8(signed char *p_out,
                                     const signed char *p_vec,
                                     signed int activation_min,
                                     signed int activation_max,
                                     signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(activation_min);
    msg.param[3] = (unsigned int)(activation_max);
    msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_vec_activation_min_max_16_16 == 1
void xa_nn_vec_activation_min_max_16_16_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed short *p_out,
                                              const signed short *p_vec,
                                              signed int activation_min,
                                              signed int activation_max,
                                              signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_16_16;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(activation_min);
    msg_async.msg.param[3] = (unsigned int)(activation_max);
    msg_async.msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_activation_min_max_16_16(signed short *p_out,
                                       const signed short *p_vec,
                                       signed int activation_min,
                                       signed int activation_max,
                                       signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_activation_min_max_16_16;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(activation_min);
    msg.param[3] = (unsigned int)(activation_max);
    msg.param[4] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_asym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(inp_zero_bias);
    msg_async.msg.param[3] = (unsigned int)(out_multiplier);
    msg_async.msg.param[4] = (unsigned int)(out_shift);
    msg_async.msg.param[5] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[6] = (unsigned int)(quantized_activation_min);
    msg_async.msg.param[7] = (unsigned int)(quantized_activation_max);
    msg_async.msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_asym8u_asym8u(unsigned char *p_out,
                                 const unsigned char *p_vec,
                                 signed int inp_zero_bias,
                                 signed int out_multiplier,
                                 signed int out_shift,
                                 signed int out_zero_bias,
                                 signed int quantized_activation_min,
                                 signed int quantized_activation_max,
                                 signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_asym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(inp_zero_bias);
    msg.param[3] = (unsigned int)(out_multiplier);
    msg.param[4] = (unsigned int)(out_shift);
    msg.param[5] = (unsigned int)(out_zero_bias);
    msg.param[6] = (unsigned int)(quantized_activation_min);
    msg.param[7] = (unsigned int)(quantized_activation_max);
    msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_relu_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(inp_zero_bias);
    msg_async.msg.param[3] = (unsigned int)(out_multiplier);
    msg_async.msg.param[4] = (unsigned int)(out_shift);
    msg_async.msg.param[5] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[6] = (unsigned int)(quantized_activation_min);
    msg_async.msg.param[7] = (unsigned int)(quantized_activation_max);
    msg_async.msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_relu_asym8s_asym8s(signed char *p_out,
                                 const signed char *p_vec,
                                 signed int inp_zero_bias,
                                 signed int out_multiplier,
                                 signed int out_shift,
                                 signed int out_zero_bias,
                                 signed int quantized_activation_min,
                                 signed int quantized_activation_max,
                                 signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_relu_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(inp_zero_bias);
    msg.param[3] = (unsigned int)(out_multiplier);
    msg.param[4] = (unsigned int)(out_shift);
    msg.param[5] = (unsigned int)(out_zero_bias);
    msg.param[6] = (unsigned int)(quantized_activation_min);
    msg.param[7] = (unsigned int)(quantized_activation_max);
    msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                         signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_prelu_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_vec);
    msg_async.msg.param[2]  = (unsigned int)(p_vec_alpha);
    msg_async.msg.param[3]  = (unsigned int)(inp_zero_bias);
    msg_async.msg.param[4]  = (unsigned int)(alpha_zero_bias);
    msg_async.msg.param[5]  = (unsigned int)(alpha_multiplier);
    msg_async.msg.param[6]  = (unsigned int)(alpha_shift);
    msg_async.msg.param[7]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[8]  = (unsigned int)(out_shift);
    msg_async.msg.param[9]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[10] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
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
                                  signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_prelu_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_vec);
    msg.param[2]  = (unsigned int)(p_vec_alpha);
    msg.param[3]  = (unsigned int)(inp_zero_bias);
    msg.param[4]  = (unsigned int)(alpha_zero_bias);
    msg.param[5]  = (unsigned int)(alpha_multiplier);
    msg.param[6]  = (unsigned int)(alpha_shift);
    msg.param[7]  = (unsigned int)(out_multiplier);
    msg.param[8]  = (unsigned int)(out_shift);
    msg.param[9]  = (unsigned int)(out_zero_bias);
    msg.param[10] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_hard_swish_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(inp_zero_bias);
    msg_async.msg.param[3] = (unsigned int)(reluish_multiplier);
    msg_async.msg.param[4] = (unsigned int)(reluish_shift);
    msg_async.msg.param[5] = (unsigned int)(out_multiplier);
    msg_async.msg.param[6] = (unsigned int)(out_shift);
    msg_async.msg.param[7] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_hard_swish_asym8s_asym8s(signed char *p_out,
                                       const signed char *p_vec,
                                       signed int inp_zero_bias,
                                       signed short reluish_multiplier,
                                       signed int reluish_shift,
                                       signed short out_multiplier,
                                       signed int out_shift,
                                       signed int out_zero_bias,
                                       signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_hard_swish_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(inp_zero_bias);
    msg.param[3] = (unsigned int)(reluish_multiplier);
    msg.param[4] = (unsigned int)(reluish_shift);
    msg.param[5] = (unsigned int)(out_multiplier);
    msg.param[6] = (unsigned int)(out_shift);
    msg.param[7] = (unsigned int)(out_zero_bias);
    msg.param[8] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int vec_length)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_vec_tanh_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_vec);
    msg_async.msg.param[2] = (unsigned int)(zero_point);
    msg_async.msg.param[3] = (unsigned int)(input_range_radius);
    msg_async.msg.param[4] = (unsigned int)(input_multiplier);
    msg_async.msg.param[5] = (unsigned int)(input_left_shift);
    msg_async.msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_vec_tanh_asym8s_asym8s(signed char *p_out,
                                 const signed char *p_vec,
                                 signed int zero_point,
                                 signed int input_range_radius,
                                 signed int input_multiplier,
                                 signed int input_left_shift,
                                 signed int vec_length)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_vec_tanh_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_vec);
    msg.param[2] = (unsigned int)(zero_point);
    msg.param[3] = (unsigned int)(input_range_radius);
    msg.param[4] = (unsigned int)(input_multiplier);
    msg.param[5] = (unsigned int)(input_left_shift);
    msg.param[6] = (unsigned int)(vec_length);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv1d_std_asym8uxasym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(out_channels);
    msg_async.msg.param[9]  = (unsigned int)(y_stride);
    msg_async.msg.param[10] = (unsigned int)(y_padding);
    msg_async.msg.param[11] = (unsigned int)(out_height);
    msg_async.msg.param[12] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[13] = (unsigned int)(kernel_zero_bias);
    msg_async.msg.param[14] = (unsigned int)(out_multiplier);
    msg_async.msg.param[15] = (unsigned int)(out_shift);
    msg_async.msg.param[16] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[17] = (unsigned int)(out_data_format);
    msg_async.msg.param[18] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                   void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv1d_std_asym8uxasym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(out_channels);
    msg.param[9]  = (unsigned int)(y_stride);
    msg.param[10] = (unsigned int)(y_padding);
    msg.param[11] = (unsigned int)(out_height);
    msg.param[12] = (unsigned int)(input_zero_bias);
    msg.param[13] = (unsigned int)(kernel_zero_bias);
    msg.param[14] = (unsigned int)(out_multiplier);
    msg.param[15] = (unsigned int)(out_shift);
    msg.param[16] = (unsigned int)(out_zero_bias);
    msg.param[17] = (unsigned int)(out_data_format);
    msg.param[18] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                          void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_asym8uxasym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

//    msg_async.msg.param[0]  = (unsigned int)(p_out);
//    msg_async.msg.param[1]  = (unsigned int)(p_inp);
//    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
//    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp);
    msg_async.msg.param[2]  = (unsigned int)(p_kernel);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);


    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[17] = (unsigned int)(kernel_zero_bias);
    msg_async.msg.param[18] = (unsigned int)(out_multiplier);
    msg_async.msg.param[19] = (unsigned int)(out_shift);
    msg_async.msg.param[20] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[21] = (unsigned int)(out_data_format);
    msg_async.msg.param[22] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                   void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_asym8uxasym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

//    msg.param[0]  = (unsigned int)(p_out);
//    msg.param[1]  = (unsigned int)(p_inp);
//    msg.param[2]  = (unsigned int)(p_kernel);
//    msg.param[3]  = (unsigned int)(p_bias);

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);


    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(input_zero_bias);
    msg.param[17] = (unsigned int)(kernel_zero_bias);
    msg.param[18] = (unsigned int)(out_multiplier);
    msg.param[19] = (unsigned int)(out_shift);
    msg.param[20] = (unsigned int)(out_zero_bias);
    msg.param[21] = (unsigned int)(out_data_format);
//    msg.param[22] = (unsigned int)(p_scratch);
    msg.param[22] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                  void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_std_per_chan_sym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
	msg_async.msg.param[1]  = (unsigned int)(p_inp);
	msg_async.msg.param[2]  = (unsigned int)(p_kernel);
	msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(out_channels);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[17] = (unsigned int)(p_out_multiplier);
    msg_async.msg.param[18] = (unsigned int)(p_out_shift);
    msg_async.msg.param[19] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[20] = (unsigned int)(out_data_format);
    msg_async.msg.param[21] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                           void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_std_per_chan_sym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

//    msg.param[0]  = (unsigned int)(p_out);
//    msg.param[1]  = (unsigned int)(p_inp);
//    msg.param[2]  = (unsigned int)(p_kernel);
//    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp);
    msg.param[2]  = (unsigned int)(p_kernel);
    msg.param[3]  = (unsigned int)(p_bias);

    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(out_channels);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(input_zero_bias);
//    msg.param[17] = (unsigned int)(p_out_multiplier);
//    msg.param[18] = (unsigned int)(p_out_shift);
    msg.param[17] = (unsigned int)(p_out_multiplier);
    msg.param[18] = (unsigned int)(p_out_shift);
    msg.param[19] = (unsigned int)(out_zero_bias);
    msg.param[20] = (unsigned int)(out_data_format);
//    msg.param[21] = (unsigned int)(p_scratch);
    msg.param[21] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                    signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_vec1);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols1);
    msg_async.msg.param[6]  = (unsigned int)(row_stride1);
    msg_async.msg.param[7]  = (unsigned int)(vec_count);
    msg_async.msg.param[8]  = (unsigned int)(mat1_zero_bias);
    msg_async.msg.param[9]  = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[10] = (unsigned int)(out_multiplier);
    msg_async.msg.param[11] = (unsigned int)(out_shift);
    msg_async.msg.param[12] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                             signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matXvec_batch_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_vec1);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols1);
    msg.param[6]  = (unsigned int)(row_stride1);
    msg.param[7]  = (unsigned int)(vec_count);
    msg.param[8]  = (unsigned int)(mat1_zero_bias);
    msg.param[9]  = (unsigned int)(vec1_zero_bias);
    msg.param[10] = (unsigned int)(out_multiplier);
    msg.param[11] = (unsigned int)(out_shift);
    msg.param[12] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_mat2);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols);
    msg_async.msg.param[6]  = (unsigned int)(row_stride);
    msg_async.msg.param[7]  = (unsigned int)(vec_count);
    msg_async.msg.param[8]  = (unsigned int)(vec_offset);
    msg_async.msg.param[9]  = (unsigned int)(out_offset);
    msg_async.msg.param[10] = (unsigned int)(out_stride);
    msg_async.msg.param[11] = (unsigned int)(mat1_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[13] = (unsigned int)(out_multiplier);
    msg_async.msg.param[14] = (unsigned int)(out_shift);
    msg_async.msg.param[15] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                      signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_mat2);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols);
    msg.param[6]  = (unsigned int)(row_stride);
    msg.param[7]  = (unsigned int)(vec_count);
    msg.param[8]  = (unsigned int)(vec_offset);
    msg.param[9]  = (unsigned int)(out_offset);
    msg.param[10] = (unsigned int)(out_stride);
    msg.param[11] = (unsigned int)(mat1_zero_bias);
    msg.param[12] = (unsigned int)(vec1_zero_bias);
    msg.param[13] = (unsigned int)(out_multiplier);
    msg.param[14] = (unsigned int)(out_shift);
    msg.param[15] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                     signed int out_zero_bias)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_matmul_per_chan_sym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_mat1);
    msg_async.msg.param[2]  = (unsigned int)(p_vec1);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(rows);
    msg_async.msg.param[5]  = (unsigned int)(cols1);
    msg_async.msg.param[6]  = (unsigned int)(row_stride1);
    msg_async.msg.param[7]  = (unsigned int)(vec_count);
    msg_async.msg.param[8]  = (unsigned int)(vec_offset);
    msg_async.msg.param[9]  = (unsigned int)(out_offset);
    msg_async.msg.param[10] = (unsigned int)(out_stride);
    msg_async.msg.param[11] = (unsigned int)(vec1_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(p_out_multiplier);
    msg_async.msg.param[13] = (unsigned int)(p_out_shift);
    msg_async.msg.param[14] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_async(&msg_async);
}
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
                                              signed int out_zero_bias)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_matmul_per_chan_sym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_mat1);
    msg.param[2]  = (unsigned int)(p_vec1);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(rows);
    msg.param[5]  = (unsigned int)(cols1);
    msg.param[6]  = (unsigned int)(row_stride1);
    msg.param[7]  = (unsigned int)(vec_count);
    msg.param[8]  = (unsigned int)(vec_offset);
    msg.param[9]  = (unsigned int)(out_offset);
    msg.param[10] = (unsigned int)(out_stride);
    msg.param[11] = (unsigned int)(vec1_zero_bias);
    msg.param[12] = (unsigned int)(p_out_multiplier);
    msg.param[13] = (unsigned int)(p_out_shift);
    msg.param[14] = (unsigned int)(out_zero_bias);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_asym8uxasym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[17] = (unsigned int)(kernel_zero_bias);
    msg_async.msg.param[18] = (unsigned int)(out_multiplier);
    msg_async.msg.param[19] = (unsigned int)(out_shift);
    msg_async.msg.param[20] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[21] = (unsigned int)(inp_data_format);
    msg_async.msg.param[22] = (unsigned int)(out_data_format);
    msg_async.msg.param[23] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                         void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_asym8uxasym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(input_zero_bias);
    msg.param[17] = (unsigned int)(kernel_zero_bias);
    msg.param[18] = (unsigned int)(out_multiplier);
    msg.param[19] = (unsigned int)(out_shift);
    msg.param[20] = (unsigned int)(out_zero_bias);
    msg.param[21] = (unsigned int)(inp_data_format);
    msg.param[22] = (unsigned int)(out_data_format);
    msg.param[23] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_asym8uxasym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(out_channels);
    msg_async.msg.param[8]  = (unsigned int)(input_zero_bias);
    msg_async.msg.param[9]  = (unsigned int)(kernel_zero_bias);
    msg_async.msg.param[10] = (unsigned int)(out_multiplier);
    msg_async.msg.param[11] = (unsigned int)(out_shift);
    msg_async.msg.param[12] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[13] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                                         signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_asym8uxasym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(out_channels);
    msg.param[8]  = (unsigned int)(input_zero_bias);
    msg.param[9]  = (unsigned int)(kernel_zero_bias);
    msg.param[10] = (unsigned int)(out_multiplier);
    msg.param[11] = (unsigned int)(out_shift);
    msg.param[12] = (unsigned int)(out_zero_bias);
    msg.param[13] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                        void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_per_chan_sym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(kernel_height);
    msg_async.msg.param[8]  = (unsigned int)(kernel_width);
    msg_async.msg.param[9]  = (unsigned int)(channels_multiplier);
    msg_async.msg.param[10] = (unsigned int)(x_stride);
    msg_async.msg.param[11] = (unsigned int)(y_stride);
    msg_async.msg.param[12] = (unsigned int)(x_padding);
    msg_async.msg.param[13] = (unsigned int)(y_padding);
    msg_async.msg.param[14] = (unsigned int)(out_height);
    msg_async.msg.param[15] = (unsigned int)(out_width);
    msg_async.msg.param[16] = (unsigned int)(input_zero_bias);
    msg_async.msg.param[17] = (unsigned int)(p_out_multiplier);
    msg_async.msg.param[18] = (unsigned int)(p_out_shift);
    msg_async.msg.param[19] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[20] = (unsigned int)(inp_data_format);
    msg_async.msg.param[21] = (unsigned int)(out_data_format);
    msg_async.msg.param[22] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
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
                                                 void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_depthwise_per_chan_sym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(kernel_height);
    msg.param[8]  = (unsigned int)(kernel_width);
    msg.param[9]  = (unsigned int)(channels_multiplier);
    msg.param[10] = (unsigned int)(x_stride);
    msg.param[11] = (unsigned int)(y_stride);
    msg.param[12] = (unsigned int)(x_padding);
    msg.param[13] = (unsigned int)(y_padding);
    msg.param[14] = (unsigned int)(out_height);
    msg.param[15] = (unsigned int)(out_width);
    msg.param[16] = (unsigned int)(input_zero_bias);
    msg.param[17] = (unsigned int)(p_out_multiplier);
    msg.param[18] = (unsigned int)(p_out_shift);
    msg.param[19] = (unsigned int)(out_zero_bias);
    msg.param[20] = (unsigned int)(inp_data_format);
    msg.param[21] = (unsigned int)(out_data_format);
    msg.param[22] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                        signed int out_data_format)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_per_chan_sym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_kernel);
    msg_async.msg.param[2]  = (unsigned int)(p_inp);
    msg_async.msg.param[3]  = (unsigned int)(p_bias);
    msg_async.msg.param[4]  = (unsigned int)(input_height);
    msg_async.msg.param[5]  = (unsigned int)(input_width);
    msg_async.msg.param[6]  = (unsigned int)(input_channels);
    msg_async.msg.param[7]  = (unsigned int)(out_channels);
    msg_async.msg.param[8]  = (unsigned int)(input_zero_bias);
    msg_async.msg.param[9]  = (unsigned int)(p_out_multiplier);
    msg_async.msg.param[10] = (unsigned int)(p_out_shift);
    msg_async.msg.param[11] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(out_data_format);

    dsp_ipc_send_async(&msg_async);
}
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
                                                 signed int out_data_format)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_conv2d_pointwise_per_chan_sym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_kernel);
    msg.param[2]  = (unsigned int)(p_inp);
    msg.param[3]  = (unsigned int)(p_bias);
    msg.param[4]  = (unsigned int)(input_height);
    msg.param[5]  = (unsigned int)(input_width);
    msg.param[6]  = (unsigned int)(input_channels);
    msg.param[7]  = (unsigned int)(out_channels);
    msg.param[8]  = (unsigned int)(input_zero_bias);
    msg.param[9]  = (unsigned int)(p_out_multiplier);
    msg.param[10] = (unsigned int)(p_out_shift);
    msg.param[11] = (unsigned int)(out_zero_bias);
    msg.param[12] = (unsigned int)(out_data_format);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_mul_f32xf32_f32 == 1
void xa_nn_elm_mul_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_mul_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_mul_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_mul_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_add_f32xf32_f32 == 1
void xa_nn_elm_add_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_add_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_add_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_add_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_mul_acc_f32xf32_f32 == 1
void xa_nn_elm_mul_acc_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         float *p_out,
                                         const float *p_inp1,
                                         const float *p_inp2,
                                         signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_mul_acc_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_mul_acc_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_mul_acc_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_sub_f32xf32_f32 == 1
void xa_nn_elm_sub_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_sub_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_sub_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_sub_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_div_f32xf32_f32 == 1
void xa_nn_elm_div_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                     void *params,
                                     float *p_out,
                                     const float *p_inp1,
                                     const float *p_inp2,
                                     signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_div_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_div_f32xf32_f32(float *p_out, const float *p_inp1, const float *p_inp2, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_div_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_floor_f32_f32 == 1
void xa_nn_elm_floor_f32_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_inp, signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_floor_f32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp);
    msg_async.msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_floor_f32_f32(float *p_out, const float *p_inp, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_floor_f32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp);
    msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_add_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_left_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg_async.msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[10] = (unsigned int)(p_inp2);
    msg_async.msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(inp2_left_shift);
    msg_async.msg.param[13] = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[14] = (unsigned int)(left_shift);
    msg_async.msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_add_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_left_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg.param[10] = (unsigned int)(p_inp2);
    msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg.param[12] = (unsigned int)(inp2_left_shift);
    msg.param[13] = (unsigned int)(inp2_multiplier);
    msg.param[14] = (unsigned int)(left_shift);
    msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_add_asym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_left_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg_async.msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[10] = (unsigned int)(p_inp2);
    msg_async.msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(inp2_left_shift);
    msg_async.msg.param[13] = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[14] = (unsigned int)(left_shift);
    msg_async.msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_add_asym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_left_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg.param[10] = (unsigned int)(p_inp2);
    msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg.param[12] = (unsigned int)(inp2_left_shift);
    msg.param[13] = (unsigned int)(inp2_multiplier);
    msg.param[14] = (unsigned int)(left_shift);
    msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_sub_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_left_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg_async.msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[10] = (unsigned int)(p_inp2);
    msg_async.msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(inp2_left_shift);
    msg_async.msg.param[13] = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[14] = (unsigned int)(left_shift);
    msg_async.msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_sub_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_left_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg.param[10] = (unsigned int)(p_inp2);
    msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg.param[12] = (unsigned int)(inp2_left_shift);
    msg.param[13] = (unsigned int)(inp2_multiplier);
    msg.param[14] = (unsigned int)(left_shift);
    msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_sub_asym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_left_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg_async.msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[10] = (unsigned int)(p_inp2);
    msg_async.msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[12] = (unsigned int)(inp2_left_shift);
    msg_async.msg.param[13] = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[14] = (unsigned int)(left_shift);
    msg_async.msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_sub_asym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_left_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(inp1_left_shift);
    msg.param[9]  = (unsigned int)(inp1_multiplier);
    msg.param[10] = (unsigned int)(p_inp2);
    msg.param[11] = (unsigned int)(inp2_zero_bias);
    msg.param[12] = (unsigned int)(inp2_left_shift);
    msg.param[13] = (unsigned int)(inp2_multiplier);
    msg.param[14] = (unsigned int)(left_shift);
    msg.param[15] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_mul_asym8uxasym8u_asym8u;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(p_inp2);
    msg_async.msg.param[9]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_mul_asym8uxasym8u_asym8u;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(p_inp2);
    msg.param[9]  = (unsigned int)(inp2_zero_bias);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_mul_asym8sxasym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(out_zero_bias);
    msg_async.msg.param[2]  = (unsigned int)(out_shift);
    msg_async.msg.param[3]  = (unsigned int)(out_multiplier);
    msg_async.msg.param[4]  = (unsigned int)(out_activation_min);
    msg_async.msg.param[5]  = (unsigned int)(out_activation_max);
    msg_async.msg.param[6]  = (unsigned int)(p_inp1);
    msg_async.msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[8]  = (unsigned int)(p_inp2);
    msg_async.msg.param[9]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_mul_asym8sxasym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(out_zero_bias);
    msg.param[2]  = (unsigned int)(out_shift);
    msg.param[3]  = (unsigned int)(out_multiplier);
    msg.param[4]  = (unsigned int)(out_activation_min);
    msg.param[5]  = (unsigned int)(out_activation_max);
    msg.param[6]  = (unsigned int)(p_inp1);
    msg.param[7]  = (unsigned int)(inp1_zero_bias);
    msg.param[8]  = (unsigned int)(p_inp2);
    msg.param[9]  = (unsigned int)(inp2_zero_bias);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_quantize_asym16s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp);
    msg_async.msg.param[2] = (unsigned int)(inp_zero_bias);
    msg_async.msg.param[3] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[4] = (unsigned int)(out_shift);
    msg_async.msg.param[5] = (unsigned int)(out_multiplier);
    msg_async.msg.param[6] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_quantize_asym16s_asym8s(signed char *p_out,
                                      const signed short *p_inp,
                                      signed int inp_zero_bias,
                                      signed int out_zero_bias,
                                      signed int out_shift,
                                      signed int out_multiplier,
                                      signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_quantize_asym16s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp);
    msg.param[2] = (unsigned int)(inp_zero_bias);
    msg.param[3] = (unsigned int)(out_zero_bias);
    msg.param[4] = (unsigned int)(out_shift);
    msg.param[5] = (unsigned int)(out_multiplier);
    msg.param[6] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_max_8x8_8 == 1
void xa_nn_elm_max_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed char *p_in1,
                               const signed char *p_in2,
                               signed int num_element)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_max_8x8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_in1);
    msg_async.msg.param[2] = (unsigned int)(p_in2);
    msg_async.msg.param[3] = (unsigned int)(num_element);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_max_8x8_8(signed char *p_out, const signed char *p_in1, const signed char *p_in2, signed int num_element)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_max_8x8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_in1);
    msg.param[2] = (unsigned int)(p_in2);
    msg.param[3] = (unsigned int)(num_element);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_min_8x8_8 == 1
void xa_nn_elm_min_8x8_8_async(void (*cb)(void *, srtm_message *msg),
                               void *params,
                               signed char *p_out,
                               const signed char *p_in1,
                               const signed char *p_in2,
                               signed int num_element)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_min_8x8_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_in1);
    msg_async.msg.param[2] = (unsigned int)(p_in2);
    msg_async.msg.param[3] = (unsigned int)(num_element);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_min_8x8_8(signed char *p_out, const signed char *p_in1, const signed char *p_in2, signed int num_element)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_min_8x8_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_in1);
    msg.param[2] = (unsigned int)(p_in2);
    msg.param[3] = (unsigned int)(num_element);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                         signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_equal_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                  signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_equal_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                            signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_notequal_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                     signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_notequal_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                           signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_greater_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                    signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_greater_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                                signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_greaterequal_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                         signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_greaterequal_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                        signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_less_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                 signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_less_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_lessequal_asym8sxasym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0]  = (unsigned int)(p_out);
    msg_async.msg.param[1]  = (unsigned int)(p_inp1);
    msg_async.msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg_async.msg.param[3]  = (unsigned int)(inp1_shift);
    msg_async.msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg_async.msg.param[5]  = (unsigned int)(p_inp2);
    msg_async.msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg_async.msg.param[7]  = (unsigned int)(inp2_shift);
    msg_async.msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg_async.msg.param[9]  = (unsigned int)(left_shift);
    msg_async.msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
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
                                      signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_lessequal_asym8sxasym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0]  = (unsigned int)(p_out);
    msg.param[1]  = (unsigned int)(p_inp1);
    msg.param[2]  = (unsigned int)(inp1_zero_bias);
    msg.param[3]  = (unsigned int)(inp1_shift);
    msg.param[4]  = (unsigned int)(inp1_multiplier);
    msg.param[5]  = (unsigned int)(p_inp2);
    msg.param[6]  = (unsigned int)(inp2_zero_bias);
    msg.param[7]  = (unsigned int)(inp2_shift);
    msg.param[8]  = (unsigned int)(inp2_multiplier);
    msg.param[9]  = (unsigned int)(left_shift);
    msg.param[10] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_reduce_max_getsize_nhwc == 1
void xa_nn_reduce_max_getsize_nhwc_async(void (*cb)(void *, srtm_message *msg),
                                         void *params,
                                         signed int inp_precision,
                                         const signed int *p_inp_shape,
                                         signed int num_inp_dims,
                                         const signed int *p_axis,
                                         signed int num_axis_dims)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_reduce_max_getsize_nhwc;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(inp_precision);
    msg_async.msg.param[1] = (unsigned int)(p_inp_shape);
    msg_async.msg.param[2] = (unsigned int)(num_inp_dims);
    msg_async.msg.param[3] = (unsigned int)(p_axis);
    msg_async.msg.param[4] = (unsigned int)(num_axis_dims);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_reduce_max_getsize_nhwc(signed int inp_precision,
                                  const signed int *p_inp_shape,
                                  signed int num_inp_dims,
                                  const signed int *p_axis,
                                  signed int num_axis_dims)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_reduce_max_getsize_nhwc;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(inp_precision);
    msg.param[1] = (unsigned int)(p_inp_shape);
    msg.param[2] = (unsigned int)(num_inp_dims);
    msg.param[3] = (unsigned int)(p_axis);
    msg.param[4] = (unsigned int)(num_axis_dims);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                             void *p_scratch)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_reduce_max_4D_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_out_shape);
    msg_async.msg.param[2] = (unsigned int)(p_inp);
    msg_async.msg.param[3] = (unsigned int)(p_inp_shape);
    msg_async.msg.param[4] = (unsigned int)(p_axis);
    msg_async.msg.param[5] = (unsigned int)(num_out_dims);
    msg_async.msg.param[6] = (unsigned int)(num_inp_dims);
    msg_async.msg.param[7] = (unsigned int)(num_axis_dims);
    msg_async.msg.param[8] = (unsigned int)(p_scratch);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_reduce_max_4D_asym8s_asym8s(signed char *p_out,
                                      const signed int *p_out_shape,
                                      const signed char *p_inp,
                                      const signed int *p_inp_shape,
                                      const signed int *p_axis,
                                      signed int num_out_dims,
                                      signed int num_inp_dims,
                                      signed int num_axis_dims,
                                      void *p_scratch)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_reduce_max_4D_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_out_shape);
    msg.param[2] = (unsigned int)(p_inp);
    msg.param[3] = (unsigned int)(p_inp_shape);
    msg.param[4] = (unsigned int)(p_axis);
    msg.param[5] = (unsigned int)(num_out_dims);
    msg.param[6] = (unsigned int)(num_inp_dims);
    msg.param[7] = (unsigned int)(num_axis_dims);
    msg.param[8] = (unsigned int)(p_scratch);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_logicaland_boolxbool_bool == 1
void xa_nn_elm_logicaland_boolxbool_bool_async(void (*cb)(void *, srtm_message *msg),
                                               void *params,
                                               signed char *p_out,
                                               const signed char *p_inp1,
                                               const signed char *p_inp2,
                                               signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_logicaland_boolxbool_bool;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_logicaland_boolxbool_bool(signed char *p_out,
                                        const signed char *p_inp1,
                                        const signed char *p_inp2,
                                        signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_logicaland_boolxbool_bool;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_logicalor_boolxbool_bool == 1
void xa_nn_elm_logicalor_boolxbool_bool_async(void (*cb)(void *, srtm_message *msg),
                                              void *params,
                                              signed char *p_out,
                                              const signed char *p_inp1,
                                              const signed char *p_inp2,
                                              signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_logicalor_boolxbool_bool;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_logicalor_boolxbool_bool(signed char *p_out,
                                       const signed char *p_inp1,
                                       const signed char *p_inp2,
                                       signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_logicalor_boolxbool_bool;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_elm_logicalnot_bool_bool == 1
void xa_nn_elm_logicalnot_bool_bool_async(void (*cb)(void *, srtm_message *msg),
                                          void *params,
                                          signed char *p_out,
                                          const signed char *p_inp,
                                          signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_elm_logicalnot_bool_bool;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp);
    msg_async.msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_elm_logicalnot_bool_bool(signed char *p_out, const signed char *p_inp, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_elm_logicalnot_bool_bool;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp);
    msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_l2_norm_f32 == 1
void xa_nn_l2_norm_f32_async(
    void (*cb)(void *, srtm_message *msg), void *params, float *p_out, const float *p_inp, signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_l2_norm_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp);
    msg_async.msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_l2_norm_f32(float *p_out, const float *p_inp, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_l2_norm_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp);
    msg.param[2] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_l2_norm_asym8s_asym8s == 1
void xa_nn_l2_norm_asym8s_asym8s_async(void (*cb)(void *, srtm_message *msg),
                                       void *params,
                                       signed char *p_out,
                                       const signed char *p_inp,
                                       signed int zero_point,
                                       signed int num_elm)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_l2_norm_asym8s_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp);
    msg_async.msg.param[2] = (unsigned int)(zero_point);
    msg_async.msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_l2_norm_asym8s_asym8s(signed char *p_out, const signed char *p_inp, signed int zero_point, signed int num_elm)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_l2_norm_asym8s_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp);
    msg.param[2] = (unsigned int)(zero_point);
    msg.param[3] = (unsigned int)(num_elm);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

#if NN_ENABLE_xa_nn_dot_prod_f32xf32_f32 == 1
void xa_nn_dot_prod_f32xf32_f32_async(void (*cb)(void *, srtm_message *msg),
                                      void *params,
                                      float *p_out,
                                      const float *p_inp1,
                                      const float *p_inp2,
                                      signed int vec_length,
                                      signed int num_vecs)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_dot_prod_f32xf32_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1);
    msg_async.msg.param[2] = (unsigned int)(p_inp2);
    msg_async.msg.param[3] = (unsigned int)(vec_length);
    msg_async.msg.param[4] = (unsigned int)(num_vecs);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_dot_prod_f32xf32_f32(
    float *p_out, const float *p_inp1, const float *p_inp2, signed int vec_length, signed int num_vecs)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_dot_prod_f32xf32_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1);
    msg.param[2] = (unsigned int)(p_inp2);
    msg.param[3] = (unsigned int)(vec_length);
    msg.param[4] = (unsigned int)(num_vecs);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
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
                                       signed int vec_count)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_xa_nn_dot_prod_16x16_asym8s;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(p_out);
    msg_async.msg.param[1] = (unsigned int)(p_inp1_start);
    msg_async.msg.param[2] = (unsigned int)(p_inp2_start);
    msg_async.msg.param[3] = (unsigned int)(bias_ptr);
    msg_async.msg.param[4] = (unsigned int)(vec_length);
    msg_async.msg.param[5] = (unsigned int)(out_multiplier);
    msg_async.msg.param[6] = (unsigned int)(out_shift);
    msg_async.msg.param[7] = (unsigned int)(out_zero_bias);
    msg_async.msg.param[8] = (unsigned int)(vec_count);

    dsp_ipc_send_async(&msg_async);
}
int xa_nn_dot_prod_16x16_asym8s(signed char *p_out,
                                const signed short *p_inp1_start,
                                const signed short *p_inp2_start,
                                const signed int *bias_ptr,
                                signed int vec_length,
                                signed int out_multiplier,
                                signed int out_shift,
                                signed int out_zero_bias,
                                signed int vec_count)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_xa_nn_dot_prod_16x16_asym8s;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(p_out);
    msg.param[1] = (unsigned int)(p_inp1_start);
    msg.param[2] = (unsigned int)(p_inp2_start);
    msg.param[3] = (unsigned int)(bias_ptr);
    msg.param[4] = (unsigned int)(vec_length);
    msg.param[5] = (unsigned int)(out_multiplier);
    msg.param[6] = (unsigned int)(out_shift);
    msg.param[7] = (unsigned int)(out_zero_bias);
    msg.param[8] = (unsigned int)(vec_count);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
#endif

void hifi_resizenearest_f_async(void (*cb)(void *, srtm_message *msg),
                                void *params,
                                float *dst,
                                const float *src,
                                const float scaleH,
                                const float scaleW,
                                const unsigned int *inWdims,
                                const unsigned int *outWdims)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_resizenearest_f32;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(ARM_ADDR_WITH_BUS(dst, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[1] = (unsigned int)(ARM_ADDR_WITH_BUS(src, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[2] = (unsigned int)scaleH;
    msg_async.msg.param[3] = (unsigned int)scaleW;
    msg_async.msg.param[4] = (unsigned int)(ARM_ADDR_WITH_BUS(inWdims, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[5] = (unsigned int)(ARM_ADDR_WITH_BUS(outWdims, ARM_BUS_CACHEABLE_NONSECURE));

    dsp_ipc_send_async(&msg_async);
}
signed int hifi_resizenearest_f(float *dst,
                                const float *src,
                                const float scaleH,
                                const float scaleW,
                                const unsigned int *inWdims,
                                const unsigned int *outWdims)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_resizenearest_f32;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(ARM_ADDR_WITH_BUS(dst, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[1] = (unsigned int)(ARM_ADDR_WITH_BUS(src, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[2] = (unsigned int)scaleH;
    msg.param[3] = (unsigned int)scaleW;
    msg.param[4] = (unsigned int)(ARM_ADDR_WITH_BUS(inWdims, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[5] = (unsigned int)(ARM_ADDR_WITH_BUS(outWdims, ARM_BUS_CACHEABLE_NONSECURE));

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}

void hifi_resizenearest_i8_async(void (*cb)(void *, srtm_message *msg),
                                 void *params,
                                 char *dst,
                                 const char *src,
                                 const float scaleH,
                                 const float scaleW,
                                 const unsigned int *inWdims,
                                 const unsigned int *outWdims)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_resizenearest_8;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(ARM_ADDR_WITH_BUS(dst, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[1] = (unsigned int)(ARM_ADDR_WITH_BUS(src, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[2] = (unsigned int)scaleH;
    msg_async.msg.param[3] = (unsigned int)scaleW;
    msg_async.msg.param[4] = (unsigned int)(ARM_ADDR_WITH_BUS(inWdims, ARM_BUS_CACHEABLE_NONSECURE));
    msg_async.msg.param[5] = (unsigned int)(ARM_ADDR_WITH_BUS(outWdims, ARM_BUS_CACHEABLE_NONSECURE));

    dsp_ipc_send_async(&msg_async);
}
signed int hifi_resizenearest_i8(char *dst,
                                 const char *src,
                                 const float scaleH,
                                 const float scaleW,
                                 const unsigned int *inWdims,
                                 const unsigned int *outWdims)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_resizenearest_8;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(ARM_ADDR_WITH_BUS(dst, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[1] = (unsigned int)(ARM_ADDR_WITH_BUS(src, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[2] = (unsigned int)scaleH;
    msg.param[3] = (unsigned int)scaleW;
    msg.param[4] = (unsigned int)(ARM_ADDR_WITH_BUS(inWdims, ARM_BUS_CACHEABLE_NONSECURE));
    msg.param[5] = (unsigned int)(ARM_ADDR_WITH_BUS(outWdims, ARM_BUS_CACHEABLE_NONSECURE));

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}

void hifi_inference_async(void (*cb)(void *, srtm_message *msg),
                          void *params,
                          float *dst,
                          unsigned char *constantWeight,
                          unsigned char *mutableWeight,
                          unsigned char *activations)
{
    srtm_message_async msg_async;

    msg_async.cb     = cb;
    msg_async.params = params;

    msg_async.msg.head.type         = SRTM_MessageTypeRequest;
    msg_async.msg.head.category     = SRTM_MessageCategory_NN;
    msg_async.msg.head.command      = SRTM_Command_inference;
    msg_async.msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg_async.msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg_async.msg.param[0] = (unsigned int)(constantWeight);
	msg_async.msg.param[1] = (unsigned int)(mutableWeight);
	msg_async.msg.param[2] = (unsigned int)(activations);

    dsp_ipc_send_async(&msg_async);
}
signed int hifi_inference(unsigned char *constantWeight, unsigned char *mutableWeight, unsigned char *activations)
{
    srtm_message msg;

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.category     = SRTM_MessageCategory_NN;
    msg.head.command      = SRTM_Command_inference;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.param[0] = (unsigned int)(constantWeight);
    msg.param[1] = (unsigned int)(mutableWeight);
    msg.param[2] = (unsigned int)(activations);

    dsp_ipc_send_sync(&msg);
    dsp_ipc_recv_sync(&msg);

    return (signed int)(msg.param[0]);
}
