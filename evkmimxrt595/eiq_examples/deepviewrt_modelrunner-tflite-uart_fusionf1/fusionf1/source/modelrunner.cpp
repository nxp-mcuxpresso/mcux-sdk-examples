/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "modelrunner.h"
#include "tf_benchmark.h"

#include "json/flex.h"
#include "fsl_iap.h"
#include "flash_opts.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CMD_SIZE 50

#if defined (CPU_MCXN947VNL_cm33_core0)
#define MODEL_SIZE  100 * 1024
#else
#define MODEL_SIZE  750 * 1024
#endif


#ifndef TF_VERSION_STRING
#define TF_VERSION_STRING "2.6.0"
#endif

static char* model_buf = nullptr;
FlashConfig config;

char cmd[CMD_SIZE + 1];
size_t cmd_pos = 0u;
bool echo = 1;

static size_t s_recv(char* buf);
static int do_cmd_model_loadb(char* buf, NNServer* server);
static int do_cmd_tensor_loadb(NNServer* server);
static int do_cmd_model_run(NNServer* server);
static int do_cmd_model_print(NNServer* server);
static int do_cmd_echo(NNServer* server);
static int do_cmd_reset();


char
url_decode(const char* in)
{
    if (strncmp(in, "%3A", 3) == 0) {
        return ':';
    } else if (strncmp(in, "%2F", 3) == 0) {
        return '/';
    } else if (strncmp(in, "%3B", 3) == 0) {
        return ';';
    } else if (strncmp(in, "%20", 3) == 0) {
        return ' ';
    } else {
        return 0;
    }
}

int parse_cmd(char c, NNServer* server){
    if(c == '\r'){
        cmd[cmd_pos] = 0;
        if(cmd_pos > 0)
        {
            cmd_pos = 0;
            cmd_router(cmd, server);
        }
        PRINTF("\r\n=> ");
    }else if(c == 127){
        if (cmd_pos > 0){
            cmd_pos--;
            PRINTF("\b \b");
        }
    }else{
        if (cmd_pos > CMD_SIZE){
            PRINTF("\b \b");
        }else{
            cmd[cmd_pos] = c;
            cmd_pos++;
        }
    }
    return 0;
}

int modelrunner(){
    char c;
    NNServer server;
    memset(&server,0,sizeof(NNServer));
#ifndef __XCC__
    FlashInit(&config);
#endif
    PRINTF("\r\n=> ");
    while(1){
        c = GETCHAR();
        if(c == 0x1b || c == 0x5b || c == 0x41 || c == 0x42 || c == 0x43 || c == 0x44){
            continue;
        }
        if(echo){
            PRINTF("%c", c);
        }
        parse_cmd(c, &server);
    }
    return 0;
}

int cmd_router(char* cmd, NNServer* server){
    char* r;
    r =  strtok_r(cmd , " ", &server->params);

    if (strcmp(r, "model_loadb") == 0){
        do_cmd_model_loadb(model_buf, server);
    }else if( strcmp(r, "tensor_loadb") == 0){
        do_cmd_tensor_loadb(server);
    }else if( strcmp(r, "run") == 0){
        do_cmd_model_run(server);
    }else if( strcmp(r, "model") == 0){
        do_cmd_model_print(server);
    }else if( strcmp(r, "echo") == 0){
        do_cmd_echo(server);
    }else if( strcmp(r, "reset") == 0){
        do_cmd_reset();
    }
    return 0;
}

static int print_results(char* buf){
    PRINTF("\r\n\r\nResults:");
    PRINTF("\r\n%s\r\n", buf);
    return 0;
}

static int do_cmd_reset(){
#ifndef __XCC__
    NVIC_SystemReset();
#endif
	return 0;
}

static int do_cmd_echo(NNServer* server){
    char* param;
    param =  strtok(server->params , " ");
    if (strcmp(param, "on") == 0){
        echo = 1;
    }else if(strcmp(param, "off") == 0){
        echo = 0;
    }
    return 0;
}

static int do_cmd_tensor_loadb(NNServer* server){
    char* name;
    int size = 1;
    name =  strtok(server->params , " ");
    for (int i=0; i<server->input.inputs_size; i++){
        if (strcmp (name, server->input.name [i] ) == 0){
            for(int j=0; j<server->input.shape_size[i]; j++){
                size *= server->input.shape_data[i][j];
            }
            server->input.input_data [i] = (char*) malloc(size);
            //PRINTF("name: %s size: %d\r\n", server->input.name[i], size);
            PRINTF("\r\n######### Ready for %s tensor download ", server->input.name[i]);
            s_recv(server->input.input_data[i]);
            return 0;
        }
    }

    //server->model_upload = model_buf;
    //Model_Setup(server);
    PRINTF("No such input tensor\r\n");
    return 1;
}

static int do_cmd_model_loadb(char* model_buf, NNServer* server){
    PRINTF("\r\n######### Ready for TFLite model download");
    int size = atoi(strtok(server->params , " "));

    if (model_buf){
        free(model_buf);
        model_buf = nullptr;
    }
    if (size <= MODEL_SIZE){
        model_buf = (char*)malloc(size);
        server->model_upload = model_buf;
    } else {
#ifndef __XCC__
        server->model_upload = (char*)(FLASH_BASE_ADDR + FLASH_MODEL_ADDR);
        server->model_flash_load = true;
#else
		return -1;
#endif
    }
    s_recv(model_buf);
    Model_Setup(server);
    return 0;
}

static int do_cmd_model_print(NNServer* server){
    char*       json;
    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            return -1;
        }
        server->json_buffer = (char *)ptr;
        server->json_size   = 4096;
    }

    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_arrOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "inputs");
    for (int i=0; i < server->input.inputs_size; i++){
        json = json_objOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL);
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "name",
                             server->input.name[i]);
        json = json_double_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "scale",
                                server->input.scale[i]);
        json = json_int_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "zero_point",
                             server->input.zero_point[i]);
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "datatype",
                             server->input.data_type[i]);
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "shape");
        for (int dim = 0; dim < server->input.shape_size [i]; ++dim) {
            json = json_int_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL,
                                 (int)server->input.shape_data[i] [dim]);
        }
        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
    }
    json = json_arrClose_flex(&server->json_buffer, &server->json_size, json);

    json = json_arrOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "outputs");
    for (int i=0; i < server->output.outputs_size; i++){
        json = json_objOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL);
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "name",
                             server->output.name[server->output.index [i]]);
        json = json_double_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "scale",
                                server->output.scale[i]);
        json = json_int_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "zero_point",
                             server->output.zero_point[i]);
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "datatype",
                             server->output.data_type[i]);
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "shape");
        for (int dim = 0; dim < server->output.shape_size [i]; ++dim) {
            json = json_int_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL,
                                 (int)server->output.shape_data[i] [dim]);
        }
        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);

    }
    json = json_arrClose_flex(&server->json_buffer, &server->json_size, json);

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "layer_count",
                         server->output.num_outputs);
    json = json_uint_flex(&server->json_buffer,
                          &server->json_size,
                          json,
                          "timing",
                          server->run_ns*1e3);
    json = json_arrOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "layers");

    for (int i = 0; i < server->output.num_outputs;  i++){
        json = json_objOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL);
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "name",
                             server->output.name[i]);

        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "type",
                             server->output.type[i]);

        json = json_objOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "tensor");
        json = json_uint_flex(&server->json_buffer,
                              &server->json_size,
                              json,
                              "timing",
                              (int64_t)(server->output.timing[i]/server->inference_count*1e3));
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
        json = json_uint_flex(&server->json_buffer,
                              &server->json_size,
                              json,
                              "avg_timing",
                              (int64_t)(server->output.timing[i]/server->inference_count*1e3));
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
    }

    json =
        json_arrClose_flex(&server->json_buffer, &server->json_size, json);
    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
    json_end(json);

    print_results(server->json_buffer);
    return 0;

json_oom:
    PRINTF("\r\nJson too large\r\n");
    return 1;
}

static int do_cmd_model_run(NNServer* server){

    char*       json;
    int n_outputs = 0;
    int outputs_idx[16];
    char version[30];

    sprintf(version,"TFLiteMicro-%s", TF_VERSION_STRING);
    memset(outputs_idx, 0, sizeof(outputs_idx));

    char *val, *key, *query, *rem;
    rem = server->params;
    for(query = strtok_r(NULL, " ", &rem); query != NULL; query = strtok_r(NULL, " ", &rem)) {
        key = strtok_r(query,"=", &val );

        if (strcmp("run", key) == 0){
            server->inference_count = atoi(val);
        } else if(strcmp("output", key) == 0) {
            char out_tensor_name[512];
            int outind = 0;
            int index  = 0;
            while (outind < strlen(val)) {
                char decoded = url_decode(&val[outind]);
                if (decoded != 0) {
                    out_tensor_name[index++] = decoded;
                    outind += 3;
                } else {
                    out_tensor_name[index++] = val[outind];
                    outind++;
                }
            }
            out_tensor_name[index] = '\0';

            int output = 0;
            for (int i=0; i < server->output.outputs_size; i++) {
                if (strcmp (out_tensor_name, server->output.name[server->output.index[i]]) == 0 ){
                    output = 1;
                    outputs_idx[n_outputs] = i;
                    n_outputs++;
                    break;
                }
            }
            if (!output) {
                PRINTF("Output Tensor Not Found");
                return 1;
            }
        }
    }

    Model_RunInference(server);

    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            return -1;
        }
        server->json_buffer = (char *)ptr;
        server->json_size   = 4096;
    }

    json = server->json_buffer;
    json = json_objOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             NULL);
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "engine",
                         version);
    if (!json) { goto json_oom; }

    json = json_uint_flex(&server->json_buffer,
                          &server->json_size,
                          json,
                          "timing",
                          (server->run_ns*1e3));
    if (!json) { goto json_oom; }

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "index",
                         server->timing.run);

    if (n_outputs > 0) {
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "outputs");
        for (int i = 0; i < n_outputs; i++){
            if (server->output.data[outputs_idx[i]]){
                json = json_objOpen_flex(&server->json_buffer,
                                         &server->json_size,
                                         json,
                                         NULL);
                json = json_str_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "name",
                                     server->output.name[server->output.index[outputs_idx[i]]]);
                json = json_str_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "type",
                                     server->output.type[server->output.index[outputs_idx[i]]]);
                json = json_str_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "datatype",
                                     server->output.data_type [i]);
                json = json_arrOpen_flex(&server->json_buffer,
                                         &server->json_size,
                                         json,
                                         "shape");
                for (int dim = 0; dim < server->output.shape_size [outputs_idx [i]]; ++dim) {
                    json = json_int_flex(&server->json_buffer,
                                         &server->json_size,
                                         json,
                                         NULL,
                                         server->output.shape_data[outputs_idx [i]] [dim]);
                }
                json =
                    json_arrClose_flex(&server->json_buffer, &server->json_size, json);
                json = json_base64_flex(&server->json_buffer,
                                        &server->json_size,
                                        json,
                                        "data",
                                        server->output.data[outputs_idx[i]],
                                        server->output.bytes[outputs_idx[i]]);
                json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
            }
        }
        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
    }

    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);


    json = json_end(json);
    print_results(server->json_buffer);

    return 0;

json_oom:
    PRINTF("\r\nJson too large\r\n");
    return 1;
}

static size_t s_recv(char *buf){
    char* tmp_buf = buf;
    if(tmp_buf){
        PRINTF(" MEM\r\n");
        char c;
        int eof = 0;
        while(1){
            c = GETCHAR();
            switch(c){
            case '%':
                *tmp_buf++ = c;
                while(++eof < 6){
                    c = GETCHAR();
                    *tmp_buf++ = c;
                    if(c != '%'){
                        eof = 0;
                        break;
                    }
                }
                if(eof > 6){
                *(tmp_buf-6) = 0;
                PRINTF("\r\n######### %d bytes received #########\r\n", tmp_buf-buf-7);
                return tmp_buf-buf-7;
                }
                break;
            default:
                *tmp_buf++ = c;
            }
        }
    }else{
        PRINTF(" Flash\r\n");
        buf = (char*)malloc(16384);
        tmp_buf = buf;
        char c;
        int eof = 0;
        int32_t start = FLASH_MODEL_ADDR;

        while(1){
            for(int i = 0; i<16384; i++){
                c = GETCHAR();
                switch(c){
                case '%':
                    *tmp_buf++ = c;
                    if (i == 16383){
                        eof = 0;
                        break;
                    }

                    while(++eof < 6){
                        c = GETCHAR();
                        i++;
                        *tmp_buf++ = c;
                        if(c != '%' ){
                            eof = 0;
                            break;
                        }
                    }
                    if(eof > 6){
                        *(tmp_buf-6) = 0;
                        FlashErase(&config, start, 16384);
                        FlashProgram(&config,start,(uint32_t*)buf,16384);
                        return 0x100000;
                    }
                    break;
                default:
                    *tmp_buf++ = c;
                }
            }
            FlashErase(&config, start, 16384);
            FlashProgram(&config,start,(uint32_t*)buf,16384);
            start += 16384;
            tmp_buf = buf;
        }
    }
}
