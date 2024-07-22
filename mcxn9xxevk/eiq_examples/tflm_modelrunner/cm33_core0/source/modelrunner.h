/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MODELRUNNER_H
#define MODELRUNNER_H

#include "fsl_debug_console.h"
#include <inttypes.h>
#include <stdlib.h>
#include "flash_opts.h"

struct nn_server {
    const char* model_name;
    size_t      model_size;
    size_t      kTensorArenaSize;
    char*       json_buffer;
    size_t      json_size;
    int*   input_dims_data;
    size_t request_size;
    bool model_flash_load;
    int profiling_type;
    char* params;
    struct {
        int64_t run;
        int64_t decode;
        int64_t input;
        int64_t output;
    } timing;
    struct {
        int32_t num_outputs;
        char* name[256];
        char* type[256];
        int64_t timing[256];
        int32_t index[16];
        size_t bytes[16];
        char* data[16];
        char data_type[16][20];
        int32_t outputs_size;
        int* shape_data[16];
        int32_t shape_size[16];
        float scale[16];
        int32_t zero_point[16];
    } output;
    struct {
        char* name[16];
        size_t bytes[16];
        char* data[16];
        char data_type[16][20];
        const int32_t* shape_data[16];
        int32_t shape_size[16];
        int32_t inputs_size;
        float scale[16];
        int32_t zero_point[16];
        char* input_data[16];
    } input;
    uint8_t* image_upload_data;
    char* model_upload;
    int inference_count;
    FlashConfig* flash_config;
    char* rem_mem;
    char* received_input_names[16];
    int received_inputs_size;
    int outputs_idx[16];
    int n_outputs;
    int64_t run_ns;
    int block_count;
    int32_t block_size_last;
    char *tensor_name_to_load;
};

typedef struct nn_server NNServer;

int cmd_router(char* cmd, NNServer* server);
void parse_cmd(void* arg);
int modelrunner();
char* inference_results(NNServer* server, size_t* data_len, int outputs_idx[], int n_outputs);
char* model_info(NNServer* server, size_t* data_len);

int64_t os_clock_now();

/**
 * Send hostname to client.
 * Hostname is used by profiling to choose what type of parsers are needed for the profiling log
 */
void send_hostname(NNServer* server);

/**
 * Sends a response client is expecting when server performed task succesfully.
 */
void send_ok_response(NNServer* server);

/**
 * Receive information about model inputs from client.
 */
void receive_inference_inputs(NNServer *server);

/**
 * Receive inference parameters like number of runs from client
 */
void receive_inference_params(NNServer* server);

/**
 * Runs inference for validation i.e. without sending back profiling information
 */
void run_inference(NNServer* server);

/**
 * Send this to client to indicate that modelrunner has sent the whole response and client can process it.
 */
void response_end();

/**
 * Send error message if something went wrong in executing commands.
 */
void error_message(const char* message);

void post_request_params(NNServer* server);

void post_model(NNServer* server);

void receive_data(NNServer server, char* buffer, void (*exec_command)(const char* received_command, NNServer* server));

/**
 * Send hostinfo to client
 * Hostinfo is used by validator to get maximum model memory sizes
 */
void get_hostinfo();

void reset_system();

/**
 * Receive binary model input
 */
void post_input(NNServer* server);

/**
 * Send model information to client.
 * Model information is needed by several eIQ Toolkit components to read several model properties.
 */
void get_modelinfo(NNServer* server);

void post_inference_params(NNServer* server);

void run_profiling(NNServer* server);

#endif
