/*
 * Copyright 2023-2024 NXP
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

#include "flash_opts.h"
#include "cJSON/cJSON.h"
#include "app.h"
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
unsigned int model_validated = 0;
static const int32_t block_size = 1048576;
static const int max_model_size = 20971520;
//number of the block that is currently received by modelrunner-client when sending model to this server
int receiving_block_number = 0;

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
    NNServer* server = (NNServer*)arg;
    char c;
    PRINTF("\r\n=> ");
    while (1) {
        c = GETCHAR();
        if (echo) {
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
            if (cmd_pos > 0)
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
    NNServer* server = (NNServer*)malloc(sizeof(NNServer));
    if (!server)
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
    } else if(strcmp(cmd, "run_profiling") == 0) {
        run_profiling(server);
    } else if(strcmp(cmd, "run_inference") == 0) {
        run_inference(server);
    } else if( strcmp(cmd, "post_model") == 0) {
        post_model(server);
    } else if(strcmp(cmd, "get_hostinfo") == 0) {
    	get_hostinfo();
    } else if(strcmp(cmd, "post_input") == 0) {
    	post_input(server);
    }  else if(strcmp(cmd, "post_inference_params") == 0) {
    	post_inference_params(server);
    } else if(strcmp(cmd, "get_modelinfo") == 0) {
    	get_modelinfo(server);
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
        model_buf = (char*)malloc(size + 8);
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
    if (tmp_buf) {
        PRINTF(" MEM\r\n");
        char c;
        for (int j = size; j > 0; j--)
        {
            c = GETCHAR();
            *tmp_buf++ = c;
        }

        for (int i = 0; i < 7; i++)
        {
            c = GETCHAR();
            if (c != '%')
                PRINTF("EOF");
        }
        *tmp_buf++ = 0;
        PRINTF("\r\n######### %d bytes received #########\r\n", size);

    }
    else {
        PRINTF(" Flash\r\n");
        buf = (char*)malloc(16384 + 8);
        if (!buf) {
            PRINTF("malloc failed \r\n");
            return 1;
        }
        tmp_buf = buf;
        char c;
        int i = 0;
        int32_t start = FLASH_MODEL_ADDR;
        for (int j = size; j > 0; j--)
        {
            c = GETCHAR();
            *tmp_buf++ = c;
            i++;
            if (i == 16384 || j == 1) {
                if (j == 1)
                {
                    *tmp_buf++ = 0;
                }
                FlashErase(server->flash_config, start, 16384);
                FlashProgram(server->flash_config, start, (uint32_t*)buf, 16384);
                start += 16384;
                tmp_buf = buf;
                i = 0;
            }
        }
        for (int i = 0; i < 7; i++)
        {
            c = GETCHAR();
            if (c != '%')
                PRINTF("EOF");
        }
        PRINTF("\r\n######### %d bytes received #########\r\n", size);
        free(buf);
    }
    return 0;
}

/************************************************/
/*************TOOKLIT's INTERFACE****************/
/************************************************/

//functions implementation


void response_end(){
	PRINTF("uart response end");
}

void error_message(const char* message){
	PRINTF("ERROR: %s\r\n", message);
	response_end();
}


void send_ok_response(NNServer* server, const char* response_message){
	cJSON *ok_response = cJSON_CreateObject();
	cJSON_AddStringToObject(ok_response, "status", "OK");
	if(response_message != nullptr) {
		cJSON_AddStringToObject(ok_response, "message", response_message);
	}
	char* string = cJSON_PrintUnformatted(ok_response);
	PRINTF("%s\n", string);
	cJSON_free(string);
	response_end();
}

void send_error_response(const char* message) {
	cJSON* error_response = cJSON_CreateObject();
	cJSON_AddStringToObject(error_response, "status", "error");
	cJSON_AddStringToObject(error_response, "message", message);
	char* string = cJSON_PrintUnformatted(error_response);
	PRINTF("%s\n", string);
	cJSON_free(string);
	response_end();
}

void memory_allocation_error(int bytes_n) {
	send_error_response("Not enough heap to allocate memory!");
}

void unset_request_size_error() {
	send_error_response("The 'request_size' parameter was not set!");
}

void receive_UART_command(NNServer* server, bool break_when_finished, void (*exec_command)(const char* received_command, NNServer* server)){
    while(1){
        char c = GETCHAR();
        if(c == '\r'){
            cmd[cmd_pos] = 0;
            (*exec_command)(cmd, server);
            if(cmd_pos > 0)
            {
                cmd_pos = 0;
                if(break_when_finished)
                    break;
            }
        }
        else if(c == 127){
            if (cmd_pos > 0){
                cmd_pos--;
                PRINTF("\b \b");
            }
        }
        else{
            if (cmd_pos > CMD_SIZE){
                PRINTF("\b \b");
            }
            else{
                cmd[cmd_pos] = c;
                cmd_pos++;
            }
        }
    }
}


void run_inference(NNServer* server){
	//Run inference without profiling data
	Model_RunInference(server);
	char *data;
	size_t data_len;
	data = inference_results(server, &data_len, server->outputs_idx, server->n_outputs);
	PRINTF("%s\n", data);
	free(data);
	response_end();
}

void get_modelinfo(NNServer* server){
	char *data;
	size_t data_len;
	data = model_info(server, &data_len);
	PRINTF("%s\n", data);
	free(data);
	response_end();
}

void post_request_params(NNServer* server) {
	char params_buffer[1024];
	memset(params_buffer, '\0', 1024);
	char ch;
	int buffer_position = 0;

	while(1) {
		ch = GETCHAR();
		if(ch == '\r') {
			break;
		}
		params_buffer[buffer_position++] = ch;
	}
	cJSON *parsed_params = cJSON_Parse(params_buffer);

	// parse parameters
	cJSON *parameters_it = parsed_params->child;
	// if not parameters present, set request_size to value indicating that any should be awaited
	if(parameters_it == nullptr) {
		server->request_size = -1;
		return;
	}

	while(parameters_it != nullptr) {
		if(strcmp(parameters_it->string, "request_size") == 0) {
			server->request_size = parameters_it->valueint;
		} else if(strcmp(parameters_it->string, "tensor_name_to_load") == 0) {
			if(server->tensor_name_to_load != nullptr) {
				free(server->tensor_name_to_load);
			}
			server->tensor_name_to_load = (char*)malloc(strlen(parameters_it->valuestring) + 1);
			if(server->tensor_name_to_load == nullptr) {
				error_message("Unable to allocate memory!");
			}
			strcpy(server->tensor_name_to_load, parameters_it->valuestring);
		} else if (strcmp(parameters_it->string, "profiling_type") == 0) {
			server->profiling_type = parameters_it->valueint;
		}
		parameters_it = parameters_it->next;
	}

	cJSON_Delete(parsed_params);
}

void post_model(NNServer* server) {
	// call receive request_params
	post_request_params(server);
	if(server->request_size == -1) {
		unset_request_size_error();
		return;
	}

    if (server->request_size <= MODEL_SIZE) {
    	if(server->model_upload) {
    	    free(server->model_upload);
    	    server->model_upload = nullptr;
    	    server->model_flash_load = false;
    	}
        server->model_upload = (char*)malloc(server->request_size + 1);
        if(server->model_upload == nullptr) {
        	memory_allocation_error(server->request_size + 1);
        	return;
        }
    } else {
#ifndef __XCC__
        server->model_upload = (char*)(FLASH_BASE_ADDR + FLASH_MODEL_ADDR);
        server->model_flash_load = true;
#else
        return;
#endif
    }
    // load to RAM memory
	if(!server->model_flash_load) {
		for(int buffer_position = 0; buffer_position < server->request_size; buffer_position++) {
			char ch = GETCHAR();
			server->model_upload[buffer_position] = ch;
		}
		server->model_upload[server->request_size] = '\0';
	} else {
		// load model to flash memory
		int unknown_constant = server->request_size > block_size ? block_size : server->request_size;
		char *tmp_buf = (char*)malloc(unknown_constant+8);
		if(tmp_buf == nullptr) {
			memory_allocation_error(unknown_constant + 8);
			return;
		}
		int32_t start = FLASH_MODEL_ADDR;
		for(int char_counter = 0, buffer_position = 0; char_counter < server->request_size; char_counter++) {
			char c = GETCHAR();
			tmp_buf[buffer_position] = c;
			if(char_counter == server->request_size - 1 || buffer_position == unknown_constant - 1) {
				uint32_t memory_to_flash_size = buffer_position;
				if(char_counter == server->request_size - 1) {
					tmp_buf[buffer_position + 1] = '\0';
					memory_to_flash_size += 1;
				}
				FlashErase(server->flash_config, start, memory_to_flash_size);
				FlashProgram(server->flash_config, start, (uint32_t*)tmp_buf, memory_to_flash_size);
				start += memory_to_flash_size;
				// reset temporal buffer
				memset(tmp_buf, '\0', unknown_constant);
				buffer_position = 0;
			} else {
				buffer_position++;
			}
		}

		free(tmp_buf);
	}


    int setup_status = Model_Setup(server);
    if(setup_status == kStatus_Fail) {
        // handle model setup failure
    }
    const char success_message[] = "Model was successfully uploaded and set up.";
    send_ok_response(server, success_message);
    return;
}


void get_hostinfo(){
    char name[15];
    memset(name, 0, 15);
	name[14] = '\0';
	gethostname(name);

    cJSON *hostinfo_object = cJSON_CreateObject();
    cJSON_AddStringToObject(hostinfo_object, "target", name);
    cJSON_AddStringToObject(hostinfo_object, "engine", "TensorFlow Lite");
    cJSON *model_limits = cJSON_CreateObject();
    cJSON_AddNumberToObject(model_limits, "block_size", block_size);
    cJSON_AddNumberToObject(model_limits, "max_layers", 1024);
    cJSON_AddNumberToObject(model_limits, "max_input_size", 607500);
    cJSON_AddNumberToObject(model_limits, "max_model_size", MODEL_SIZE);
    cJSON_AddItemToObject(hostinfo_object, "model_limits", model_limits);

    char* response_json_string = cJSON_PrintUnformatted(hostinfo_object);
    PRINTF("%s\n", response_json_string);
    cJSON_free(response_json_string);
    response_end();
}


void post_input(NNServer *server) {
    post_request_params(server);
    // check if request_size param was set
    if(server->request_size == -1) {
    	unset_request_size_error();
    	return;
    }

    // load json with input names
    char* input_content = (char*)malloc(sizeof(char)*(server->request_size+1));
    if(input_content == nullptr) {
    	memory_allocation_error(sizeof(char)*(server->request_size+1));
        return;
    }
    // load input content
    size_t counter = 0;
    while(counter < server->request_size) {
    	char ch = GETCHAR();
    	input_content[counter++] = ch;
    }
    input_content[server->request_size] = '\0';
    // find input structure according its name
    for (int i=0; i<server->input.inputs_size; i++) {
    	if (strcmp(server->tensor_name_to_load, server->input.name[i]) == 0) {
    		// check given input data pointer is free
    		if(server->input.input_data[i] != nullptr) {
    			free(server->input.input_data[i]);
    		}
    		// redirect input_data
    		server->input.input_data[i] = input_content;
            // free memory for the tensor_name_to_load allocated in post_request_params func
            free(server->tensor_name_to_load);
            server->tensor_name_to_load = nullptr;
            break;
    	}
    }

    send_ok_response(server, "Input tensor was successfully loaded.");
}

void post_inference_params(NNServer* server){
	post_request_params(server);
	if(server->request_size == -1) {
		unset_request_size_error();
		return;
	}
    int outputs_idx[16];
    int n_outputs = 0;
    memset(outputs_idx, 0, sizeof(outputs_idx));
    char* inference_params_raw = (char*)malloc(sizeof(char)*(server->request_size + 1));
    if(inference_params_raw == nullptr) {
    	memory_allocation_error(sizeof(char)*(server->request_size + 1));
    	return;
    }
    size_t counter = 0;
    while(counter < server->request_size){
        char c = GETCHAR();
        inference_params_raw[counter++] = c;
    }

    inference_params_raw[counter] = '\0';


    cJSON* inference_params = cJSON_Parse(inference_params_raw);
    cJSON* inference_count_object = cJSON_GetObjectItem(inference_params, "inference_count");
    server->inference_count = inference_count_object->valueint;
    cJSON* output_tensors = cJSON_GetObjectItem(inference_params, "output_tensors");
    cJSON *child = output_tensors->child;
    while (child != nullptr) {
        char* output_name = child->valuestring;
        for (int i=0; i < server->output.outputs_size; i++) {
            if (strcmp(output_name, server->output.name[server->output.index[i]]) == 0) {
                outputs_idx[n_outputs] = i;
                n_outputs++;
                break;
            }
        }
        server->n_outputs = n_outputs;
        memcpy(server->outputs_idx, outputs_idx, 16);
        child = child->next;
    }

    cJSON_Delete(inference_params);
    free(inference_params_raw);
    send_ok_response(server, "Inference parameters were successfully set.");
}


void run_profiling(NNServer* server) {
	// post profiling_type parameter
	post_request_params(server);
	int profiling_status = Model_RunInference(server);
	if(profiling_status != 0) {
		PRINTF("ERROR, profiling execution failed!\n");
	}
	// reseting profiling indicator
	server->profiling_type = 0;
	response_end();
}
