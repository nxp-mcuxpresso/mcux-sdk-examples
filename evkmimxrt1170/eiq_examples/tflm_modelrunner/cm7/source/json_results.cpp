/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "modelrunner.h"
#include "cJSON/cJSON.h"
#include <string.h>
#include "base64/base64.h"

char* inference_results(NNServer* server, size_t* data_len, int outputs_idx[], int n_outputs){
    cJSON *results = cJSON_CreateObject();
    cJSON *timing = cJSON_CreateNumber(server->run_ns*1e3);
    if (n_outputs > 0) {
        cJSON *outputs = cJSON_AddArrayToObject(results, "outputs");
        for (int i = 0; i < n_outputs; i++){
            if (server->output.data[outputs_idx[i]]){
                cJSON *output = cJSON_CreateObject();
                cJSON_AddStringToObject(output, "name", server->output.name[server->output.index[outputs_idx[i]]]);
                cJSON_AddStringToObject(output, "type", server->output.type[server->output.index[outputs_idx[i]]]);
                cJSON_AddStringToObject(output, "datatype", server->output.data_type [i]);
                cJSON *shape = cJSON_AddArrayToObject(output, "shape");
                for (int dim = 0; dim < server->output.shape_size [outputs_idx [i]]; ++dim) {
                  cJSON* s = cJSON_CreateNumber(server->output.shape_data[outputs_idx [i]] [dim]);
                  cJSON_AddItemToArray(shape, s);
                }
                char* data = (char*)base64_encode((const unsigned char*)server->output.data[outputs_idx[i]], server->output.bytes[outputs_idx[i]], NULL);
                cJSON_AddStringToObject(output, "data", data);
                free(data);
                cJSON_AddItemToArray(outputs, output);
            }
        }
    }

    cJSON_AddItemToObject(results, "timing", timing);

    char* string = cJSON_PrintUnformatted(results);
    *data_len = strlen(string);
    cJSON_Delete(results);
    return string;
}

char* model_info(NNServer* server, size_t* data_len){
    cJSON *model = cJSON_CreateObject();
    cJSON_AddNumberToObject(model, "timing", server->run_ns*1e3);
    cJSON_AddNumberToObject(model, "ktensor_arena_size", server->kTensorArenaSize);
    cJSON *inputs = cJSON_AddArrayToObject(model, "inputs");
    for (int i=0; i < server->input.inputs_size; i++){
        cJSON *input = cJSON_CreateObject();
        cJSON_AddStringToObject(input, "name", server->input.name[i]);
        cJSON_AddNumberToObject(input, "scale", server->input.scale[i]);
        cJSON_AddNumberToObject(input, "zero_points", server->input.zero_point[i]);
        cJSON_AddStringToObject(input, "datatype", server->input.data_type[i]);
        cJSON *shape = cJSON_AddArrayToObject(input, "shape");
        for (int dim = 0; dim < server->input.shape_size [i]; ++dim) {
            cJSON* s = cJSON_CreateNumber(server->input.shape_data[i][dim]);
            cJSON_AddItemToArray(shape, s);
        }
        cJSON_AddItemToArray(inputs, input);
    }

    cJSON *outputs = cJSON_AddArrayToObject(model, "outputs");
    for (int i=0; i < server->input.inputs_size; i++){
        cJSON *output = cJSON_CreateObject();
        cJSON_AddStringToObject(output, "name", server->output.name[server->output.index[i]]);
        cJSON_AddNumberToObject(output, "scale", server->output.scale[i]);
        cJSON_AddNumberToObject(output, "zero_points", server->output.zero_point[i]);
        cJSON_AddStringToObject(output, "datatype", server->output.data_type[i]);
        cJSON *shape = cJSON_AddArrayToObject(output, "shape");
        for (int dim = 0; dim < server->output.shape_size [i]; ++dim) {
            cJSON* s = cJSON_CreateNumber(server->output.shape_data[i][dim]);
            cJSON_AddItemToArray(shape, s);
        }
        cJSON_AddItemToArray(outputs, output);
    }


    cJSON_AddNumberToObject(model, "layer_count", server->output.num_outputs);
    cJSON *layers = cJSON_AddArrayToObject(model, "layers");

    for (int i = 0; i < server->output.num_outputs;  i++){
        cJSON *layer = cJSON_CreateObject();
        cJSON_AddStringToObject(layer, "name", server->output.name[i]);
        cJSON_AddStringToObject(layer, "type", server->output.type[i]);
        cJSON_AddNumberToObject(layer, "avg_timing", server->output.timing[i]/server->inference_count*1e3);
        cJSON *tensor = cJSON_AddObjectToObject(layer, "tensor");
        cJSON_AddNumberToObject(tensor, "timing", server->output.timing[i]/server->inference_count*1e3);
        cJSON_AddItemToArray(layers, layer);
    } 
    char *string = cJSON_PrintUnformatted(model);
    *data_len = strlen(string);
    cJSON_Delete(model);
    return string;
}
