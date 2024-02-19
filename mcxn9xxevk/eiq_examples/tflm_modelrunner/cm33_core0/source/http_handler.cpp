/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <lwip/sockets.h>
#include "http_handler.h"
#include "tf_benchmark.h"
//#include "fsl_debug_console.h"
#include "flash_opts.h"


#define HTTP_BUF_SIZE 10670

int http_mem_write(NNServer* server, char* buf, int buf_len,
                   char* dst, size_t offset, bool flash_write){
    if(flash_write){
        offset += FLASH_MODEL_ADDR;
        FlashErase(server->flash_config, offset, 16384);
        FlashProgram(server->flash_config, offset,(uint32_t*)buf,buf_len);
    }else{
        memcpy(dst+offset, buf, buf_len);
    }
    return 0;
}

size_t http_mem_write_size(){
    return 16384;
}

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

int http_response(int sock, int status_code, char** headers,
                  char* data, size_t size){
    size_t offset = 0;
    size_t bytes_sent;
    char h[150];
    offset += sprintf(h+offset, "HTTP/1.1 %d OK\r\n", status_code);
    offset += sprintf(h+offset, "Content-Type: text/html\r\n");
    offset += sprintf(h+offset, "Content-Length: %d\r\n\r\n", size);
    write(sock, h, offset);
    while (size > 0) {
        bytes_sent = write(sock, data, size);
        if (bytes_sent < 0)
            return -1;
        size -= bytes_sent;
        data += bytes_sent;
    }
    return 0;
}

int http_readline(int sock, char* buf, size_t* len,
                  size_t* content_length) {
    char c[1];
    int rret;
    int n = 0;
    *len = 0;
    for(int i=0; ;i++){
        rret = recv(sock, c, 1, 0);
        if(rret<=0)
            return -1;
        *content_length -= 1;
        switch (c[0]){
        case '\012':
            if (n == '\012'){
                buf[i-1] = '\0';
                *len+=1;
                return 0;}
        case '\015':
            n = '\012';
            break;
        default:
            buf[i] = c[0];
            *len+=1;
        }
    }
}

int http_parse_mime(NNServer* server, int sock, char* boundary,
                    size_t boundary_len, char* name, char* filename,
                    char* data_buf, size_t* data_len,size_t*
                    content_length, bool flash_write) {

    size_t len;
    char* tmp_buf = (char*)malloc(16384);
    if (!tmp_buf){
        PRINTF("http buf malloc failed \r\n", tmp_buf);
    }
    char* buf = (char*)malloc(1024);
    if (!buf){
        PRINTF("http buf malloc failed \r\n", tmp_buf);
    }
    memset(buf, 0, 1024);
    memset(tmp_buf, 0, 16384);
    http_readline(sock, buf, &len, content_length);
    if(strncmp(buf, "Conntent-Disposition", 20) > 0){
        char *val, *key, *query, *rem;
        strtok_r(buf, ":", &rem);
        for(query = strtok_r(NULL, "; ", &rem); query != NULL; query = strtok_r(NULL, "; ", &rem)){
            key = strtok_r(query, "=\"", &val);
            val = strtok(val,"\"");
            if (strcmp(key, "name") == 0){
                strcpy(name, val);
            }else if(strcmp(key, "filename") == 0){
                strcpy(filename, val);
            }
        }
    }
    http_readline(sock, buf, &len, content_length);

    if(strncmp(buf, "Conntent-Disposition", 20) > 0){
        char *val, *key, *query, *rem;
        strtok_r(buf, ":", &rem);
        for(query = strtok_r(NULL, "; ", &rem); query != NULL; query = strtok_r(NULL, "; ", &rem)){
            key = strtok_r(query, "=\"", &val);
            val = strtok(val,"\"");
            if (strcmp(key, "name") == 0){
                strcpy(name, val);
            }else if(strcmp(key, "filename") == 0){
                strcpy(filename, val);
            }
        }
    }

    while(len>1 && *content_length >0){
        http_readline(sock, buf, &len, content_length);
    }
    int rret =0;
    *data_len = 0;
    char n = '\015';
    int end = 0;
    size_t offset = 0;

    for(int i = 0; *content_length > 0;)
    {
        rret = recv(sock, buf, 1, 0);
        if(rret<=0){
            free(tmp_buf);
            free(buf);
            return -1;
        }
        *content_length -= rret;
        tmp_buf[*data_len] = buf[0];
        *data_len +=1;
        if (*data_len == 16384){
            http_mem_write(server, tmp_buf, 16384, data_buf, offset,
                           flash_write);
            *data_len = 0;
            offset += 16384;
            continue;
        }
        switch (buf[0]){
        case '\015':
            if(n==0){
                break;
            }
            i = 1;
            n = '\012';
            break;
        case '\012':
            if(n==0){
                *data_len -= boundary_len + 6 + end;
                http_mem_write(server, tmp_buf, *data_len,
                               data_buf, offset, flash_write);
                *data_len += offset;
                free(tmp_buf);
                free(buf);
                return end;
            }
            if (n!='\012'){
                i = 0;
                break;
            }
            i ++;
            n = '\055';
            break;
        case '\055':
            if(n == 0){
                end++;
                break;
            }
            if (n!='\055'){
                i = 0;
                break;
            }
            if (i < 4){
                i ++;
                break;
            }
        default:
            if (n==0)
                break;
            if(i > 3){
                if(boundary[i-4] == buf[0] && i < (boundary_len + 4) ){
                    if(i == boundary_len+3){
                        n=0;
                    }else {
                        n = boundary[i-3];
                    }
                    i++;
                }else{
                    i=0;
                    n = '\015';
                }
            }else if (n!='\015'){
                i=0;
                n = '\015';
            }
            break;
        }
    }
    free(tmp_buf);
    free(buf);
    return -1;
}

int v1_handler_get(int                sock,
                   size_t*           content_length,
                   const char*        path,
                   struct phr_header* headers,
                   size_t             n_headers,
                   NNServer*          server){
    char data[50] = "{\"model_limits\": {\"block_size\": 10000000}}";
    int size = strlen(data);

    http_response(sock, 200, NULL, data, size);
    return 0;
}

int v1_handler_put(int                sock,
                   size_t*           content_length,
                   const char*        path,
                   struct phr_header* headers,
                   size_t             n_headers,
                   NNServer*          server){
    char* boundary=nullptr;
    int boundary_len=0;
    for (int i = 0; i != n_headers; ++i) {
        if (strncmp(headers[i].name, "Content-Length", (int)headers[i].name_len)==0){
            *content_length = atoi(headers[i].value);
        } else if (strncmp(headers[i].name, "Content-Type", (int)headers[i].name_len)==0){
            char* c;
            for (c = (char* )headers[i].value; *c != '\075'; c++);
            boundary = c+1;
            for (c++; c != NULL; c++)
            {
                if(*c == '\012' || *c == '\015')
                {
                    break;
                }
                boundary_len++;
            }
        }
    }

    char buf[2];
    size_t rret=0;

    rret = recv(sock, buf, 2, 0);
    if(rret<=0)
        return -1;
    if (strncmp(buf, "--", 2)){
        PRINTF("no boundary found %s len: %d\r\n", buf, boundary_len);
    }
    *content_length -=2;

    for (int i = 0; i < boundary_len ; i++){
        rret = recv(sock, buf, 1, 0);
        if(rret<=0)
            return -1;
        if (*buf != boundary[i])
            return -1;
        *content_length -=1 ;
    }

    size_t data_buf_len;
    if(*content_length > MODEL_SIZE){
        server->model_flash_load = true;
        if(server->model_upload && !server->model_flash_load)
            free(server->model_upload);
        server->model_upload = (char*)(FLASH_BASE_ADDR + FLASH_MODEL_ADDR);
    }else{
        if(server->model_upload && !server->model_flash_load){
            free(server->model_upload);
        }
        server->model_flash_load = false;
        server->model_upload = (char*) malloc(*content_length);
    }

    char name[20], filename[20];
    http_parse_mime(server, sock, boundary, boundary_len, name,
                    filename, server->model_upload, &data_buf_len,
                    content_length, server->model_flash_load);

    char msg[50] = "{\"reply\": \"success\"}";
    size_t size = strlen(msg);

    if (strcmp(name, "block_count") == 0){
        http_response(sock, 200, NULL, msg, size);
        return 0;

    }else if (strcmp(name, "block_content") == 0){
        Model_Setup(server);
        http_response(sock, 200, NULL, msg, size);
        return 0;
    }
    return 0;
}

int v1_handler_post(int                sock,
                    size_t*           content_length,
                    const char*        path,
                    struct phr_header* headers,
                    size_t             n_headers,
                    NNServer*          server){

    char* boundary;
    int boundary_len=0;
    int ret;

    for (int i = 0; i != n_headers; ++i) {
        if (strncmp(headers[i].name, "Content-Length", (int)headers[i].name_len)==0){
            *content_length = atoi(headers[i].value);
        } else if (strncmp(headers[i].name, "Content-Type", (int)headers[i].name_len)==0){
            char* c;
            for (c = (char* )headers[i].value; *c != '\075'; c++);
            boundary = c+1;
            for (c++; c != NULL; c++)
            {
                if(*c == '\012' || *c == '\015')
                {
                    break;
                }
                boundary_len++;
            }
        }
    }

    if (boundary_len>0){
        char *buf = (char*) malloc(HTTP_BUF_SIZE);
        size_t rret=0;

        rret = recv(sock, buf, 2, 0);
        if(rret<=0)
            return -1;
        if (strncmp(buf, "--", 2)){
            PRINTF("no bund found %s len: %d\r\n", buf, boundary_len);
        }
        *content_length -=2;
        for (int i = 0; i < boundary_len ; i++){
            rret = recv(sock, buf, 1, 0);
            if(rret<=0)
                return -1;
            if (*buf != boundary[i])
                return -1;
            *content_length -=1 ;
        }
        free(buf);

        char name[50];
        char filename[50];
        char*  tmp = (char*)malloc(*content_length + 100);
        char*  tensor = (char*)malloc(*content_length);
		free(tmp);
        size_t tensor_len, size=1;

        do{
            ret = http_parse_mime(server, sock, boundary, boundary_len,
                                  name, filename, tensor, &tensor_len,
                                  content_length, false);
            for (int i=0; i<server->input.inputs_size; i++){
                size=1;
                if (strcmp (name, server->input.name [i] ) == 0){
                    for(int j=0; j<server->input.shape_size[i]; j++){
                        size *= server->input.shape_data[i][j];
                    }
                    size *= server->input.bytes[i];
                    server->input.input_data [i] = (char*) malloc(size);
                    if(size == tensor_len){
                        memcpy(server->input.input_data[i], tensor, size);
                    }else{
                        free(tensor);
                        char errmsg[100];
                        sprintf(errmsg, "\"error\": \"invalid tensor size %d %d\"", size, tensor_len);
                        http_response(sock, 200, NULL, errmsg, strlen(errmsg));
                        return 0;
                    }
                }
            }
        }while(ret!=2);
        free(tensor);
    }

    int n_outputs = 0;
    int outputs_idx[16];

    memset(outputs_idx, 0, sizeof(outputs_idx));

    char *val, *key, *query, *rem;
    rem = (char*)path;
    for(query = strtok_r(NULL, "&", &rem); query != NULL; query = strtok_r(NULL, "&", &rem)) {
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

    ret = Model_RunInference(server);
    if(ret){
        char msg[40] = "\"error\": \"model inference failed\"";
        ret = http_response(sock, 200, NULL, msg, strlen(msg));
        return 0;
    }

    size_t data_len;
    char* data = inference_results(server, &data_len, outputs_idx, n_outputs);
	if (!data){
        char msg[40] = "\"error\": \"json results to large\"";
        ret = http_response(sock, 200, NULL, msg, strlen(msg));
		return 0;
	}
    ret = http_response(sock, 200, NULL, data, data_len);
    free(data);
    return 0;
}

int v1_model_handler_get(int                sock,
                         size_t*           content_length,
                         const char*        path,
                         struct phr_header* headers,
                         size_t             n_headers,
                         NNServer*          server){

    size_t size;
    char* data = model_info(server, &size);
    http_response(sock, 200, NULL, data, size);
    free(data);
    return 0;
}


int handle_client(int sock, http_router* routes) {
    char hdr[4096] ;
    char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t hdr_len = 0, prevhdr_len = 0, method_len, path_len, num_headers;
    size_t rret;
    while (1) {
        rret = recv (sock, hdr, sizeof(headers), MSG_PEEK);
        if (rret == -1)
            return -1;
        prevhdr_len = hdr_len;
        hdr_len += rret;
        /*  parse the request */
        num_headers = sizeof(headers) / sizeof(headers[0]);
        pret = phr_parse_request(hdr, hdr_len, (const char**) &method, &method_len, (const char**) &path, &path_len,
                                 &minor_version, headers, &num_headers, prevhdr_len);
        if (pret > 0)
            break; /*  successfully parsed the request */
        else if (pret == -1)
            return -1;
        /*  request is incomplete, continue the loop */
        if(pret == -2)
            continue;
        if (hdr_len == sizeof(hdr))
            return -1;
    }
    rret = recv (sock, hdr, pret, 0);


    char* route;
    char* params;
    *(path + path_len) = '\0';
    route = strtok_r(path, "?", &params);

    size_t content_length = 0;

    for (const struct http_router* r = routes; r->path != NULL;
         ++r) {
        if (
            strcmp(r->path, route) == 0 &&
            strncmp(method, r->method, method_len) == 0) {

            r->handler(sock,
                       &content_length,
                       params,
                       headers,
                       num_headers,
                       r->server);
            break;
        }
    }
    return 0;
}

