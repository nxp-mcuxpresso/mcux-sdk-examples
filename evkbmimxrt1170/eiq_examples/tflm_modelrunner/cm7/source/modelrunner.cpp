/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "modelrunner.h"
#include "tf_benchmark.h"
#ifdef USE_RTOS
#include "http.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CMD_SIZE 350

#ifndef MODEL_SIZE
#define MODEL_SIZE 10 * 1024 * 1024
#endif

static char* model_buf = nullptr;

char cmd[CMD_SIZE + 2];
size_t cmd_pos = 0u;
bool echo = 1;

static size_t s_recv(char* buf, int size, NNServer* server);
static int do_cmd_model_loadb(char* buf, NNServer* server);
static int do_cmd_tensor_loadb(NNServer* server);
static int do_cmd_model_run(NNServer* server);
static int do_cmd_model_print(NNServer* server);
static int do_cmd_echo(NNServer* server);
static int do_cmd_reset();


char
cmd_decode(const char* in)
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

void parse_cmd(void* arg){
    NNServer* server = (NNServer*) arg;
    char c;
    PRINTF("\r\n=> ");
    while(1){
        c = GETCHAR();
        if(echo){
            PRINTF("%c", c);
        }

        if(c == 127){
            if (cmd_pos > 0){
                cmd_pos--;
                PRINTF("\b \b");
            }
            continue;
        }

        if(c == '\r'){
            cmd[cmd_pos] = 0;
            if(cmd_pos > 0)
            {
                cmd_pos = 0;
                cmd_router(cmd, server);
            }
            PRINTF("\r\n=> ");
            continue;
        }

        if (cmd_pos > CMD_SIZE){
            PRINTF("\b \b");
            continue;
        }

        cmd[cmd_pos] = c;
        cmd_pos++;
    }
}

int modelrunner(){
    NNServer* server =(NNServer *)malloc(sizeof(NNServer));
    if (!server )
    {
	    PRINTF("nnserver malloc failed \r\n");
    }
    FlashConfig* config =(FlashConfig *)malloc(sizeof(FlashConfig));
    memset(server,0,sizeof(NNServer));
    memset(config,0,sizeof(FlashConfig));
    FlashInit(config);
    server->flash_config = config;

#ifdef USE_RTOS
    http(server);
#endif
    parse_cmd(server);
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

static int print_results(const char* buf){
    PRINTF("\r\n\r\nResults:");
    if(buf)
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
            server->input.input_data [i] = (char*) malloc(size * server->input.bytes[i] + 8);
            PRINTF("\r\n######### Ready for %s tensor download ", server->input.name[i]);
            s_recv(server->input.input_data[i], size, server);
            return 0;
        }
    }

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
        if(model_buf)
            free(model_buf);
        model_buf = (char*)malloc(size+8);
        server->model_upload = model_buf;
    } else {
#ifndef __XCC__
    server->model_upload = (char*)(FLASH_BASE_ADDR + FLASH_MODEL_ADDR);
    server->model_flash_load = true;
#else
    return -1;
#endif
    }
    s_recv(model_buf, size, server);
    Model_Setup(server);
    return 0;
}

static int do_cmd_model_print(NNServer* server){
    size_t size;
    char* data = model_info(server, &size);
    print_results(data);
    free(data);
    return 0;
}

static int do_cmd_model_run(NNServer* server){
    if (!server->model_upload){
        print_results("{\"error\": \"Model Not Uploaded\"}");
	return 1;
    }

    int n_outputs = 0;
    int outputs_idx[16];

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
                char decoded = cmd_decode(&val[outind]);
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
                print_results("{\"error\": \"Output Tensor Not Found\"}");
                return 1;
            }
        }
    }

    Model_RunInference(server);
    size_t data_len;
    char* data = inference_results(server, &data_len, outputs_idx, n_outputs);
    print_results(data);
    free(data);
    return 0;
}

static size_t s_recv(char *buf, int size, NNServer* server){
    char* tmp_buf = buf;
    if(tmp_buf){
        PRINTF(" MEM\r\n");
        char c;
	for(int j = size; j > 0; j--)
	{
            c = GETCHAR();
            *tmp_buf++ = c;
	}

	for(int i=0; i < 7; i++)
	{
            c = GETCHAR();
            if(c != '%')
                PRINTF("EOF");
        }
        *tmp_buf++ = 0;
        PRINTF("\r\n######### %d bytes received #########\r\n", size);

    }else{
        PRINTF(" Flash\r\n");
        buf = (char*)malloc(16384+8);
        if(!buf){
            PRINTF("malloc failed \r\n");
            return 1;
        }
        tmp_buf = buf;
        char c;
        int i = 0;
        int32_t start = FLASH_MODEL_ADDR;
	for(int j = size; j > 0; j--)
	{
            c = GETCHAR();
            *tmp_buf++ = c;
	    i++;
	    if(i == 16384 || j == 1){
		if(j == 1)
		{
                    *tmp_buf++ = 0;
		}
                FlashErase(server->flash_config, start, 16384);
                FlashProgram(server->flash_config,start,(uint32_t*)buf,16384);
                start += 16384;
                tmp_buf = buf;
		i = 0;
	    }
	}
	for(int i=0; i < 7; i++)
	{
                c = GETCHAR();
		if(c != '%')
			PRINTF("EOF");
        }
                    PRINTF("\r\n######### %d bytes received #########\r\n", size);
        free(buf);
    }
    return 0;
}
