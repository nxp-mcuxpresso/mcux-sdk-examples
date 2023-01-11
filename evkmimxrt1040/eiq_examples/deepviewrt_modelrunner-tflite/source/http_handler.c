/**
 * Copyright 2018 by Au-Zone Technologies.  All Rights Reserved.
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 *
 * Authorization of this file is not implied by any DeepViewRT license
 * agreements unless explicitly stated.
 */

//#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stb_image.h"
#include "stb_image_resize.h"
//#include "tensorflow/core/public/version.h"

#include "modelrunner.h"
#include "json/flex.h"

#if !(defined(__ARMCC_VERSION) || defined(__ICCARM__))
#include <strings.h>
#endif

#ifndef __XCC__
#include <cmsis_compiler.h>
#else
#define __ALIGNED(x) __attribute__((aligned(x)))
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#ifndef TF_VERSION_STRING
#define TF_VERSION_STRING "2.6.0"
#endif

extern char*
strtok_r(char* str, const char* delim, char** saveptr);

extern size_t
strnlen(const char* s, size_t maxlen);

#define HTTP_FLAGS HTTP_CONNECTION_CLOSE

static const int32_t block_size = 1048576;

#define BLOB_SIZE 15 * 1024 * 1024

__attribute__ ((section(".bss" ".$BOARD_SDRAM" ))) uint8_t blob[BLOB_SIZE] __attribute__((aligned(16)));

static int data_t_count = 1;
static int data_t_max = 1;

static void
parse_out_data(char* src, char* dest, char* line_end, size_t dest_sz)
{
    char* end;
    char  delim         = '\0';
    bool  escaped_delim = false;
    /* if there is a delimeter, save it */

    /* not sure about escape chars yet, just skip them for now */
    if (*src == '\\') {
        src++;
        escaped_delim = true;
    }
    if (*src == '"' || *src == '\'') {
        delim = *src;
        src++;
    }
    end = src;
    if (escaped_delim) {
        do {
            end++;
        } while (*end != delim && end[-1] != '\\' && end < line_end);
        end--;
    }
    if (delim) {
        do {
            end++;
        } while (*end != delim && end < line_end);
    } else {
        do {
            end++;
        } while ((*end != ' ' && *end != ';' && *end != '\'') &&
                 end < line_end);
    }

    strncpy(dest, src, end - src);
    dest[MIN((size_t)(end - src), dest_sz - 1)] = '\0';
}

#ifndef __APPLE__
static char*
strnstr(const char* haystack, const char* needle, size_t len)
{
    int    i;
    size_t needle_len;

    if (0 == (needle_len = strnlen(needle, len))) return (char*) haystack;

    for (i = 0; i <= (int) (len - needle_len) && *haystack; i++) {
        if ((haystack[0] == needle[0]) &&
            (0 == strncmp(haystack, needle, needle_len)))
            return (char*) haystack;
        haystack++;
    }

    return NULL;
}
#endif

static size_t
parse_multipart_mime(const char*  buf,
                     size_t       buf_len,
                     char*        variable,
                     size_t       variable_len,
                     char*        file_name,
                     size_t       file_name_len,
                     char*        mime_type,
                     size_t       mime_type_len,
                     const char** data,
                     size_t*      data_len)
{
    char* header_end   = NULL;
    char* boundary_end = NULL;
    char* line_start;
    char* line_end;
    char* found = NULL;
    char* name  = NULL;

    /* param checks */
    if (!buf || !buf_len || !variable || !file_name) { return 0; }

    /* find the request length */
    if ((header_end = (strnstr(buf, "\n\r\n", buf_len)))) { header_end += 3; }

    /* make sure we have a boundary */
    if (!header_end || buf[0] != '-' || buf[1] != '-' || buf[2] == '\n') {
        return 0;
    }

    /* get the boundary end */
    boundary_end = strnstr(buf, "\n", buf_len);
    if (!boundary_end) { return 0; }
    boundary_end++;

    /* search for the variable and file_name */
    *variable  = '\0';
    *file_name = '\0';

    /* starting after the boundary, search for stuff, one line at a time*/
    line_start = boundary_end;

    /* a blank line within the buffer is where parsing will stop */
    line_end = strstr(line_start, "\n");
    while (line_end && (line_end < header_end)) {
        line_end++;
        if ((found = strnstr(line_start,
                             "Content-Disposition: ",
                             line_end - line_start))) {
            found += 21; /**@todo clean this up */
            /* now search for name */
            name = strnstr(found, "name=", line_end - found);
            if (name) {
                name += 5;
                parse_out_data(name, variable, line_end, variable_len);
            }
            name = strnstr(found, "filename=", line_end - found);
            if (name) {
                name += 9;
                parse_out_data(name, file_name, line_end, file_name_len);
            }
        }
        if ((found = strnstr(line_start,
                             "Content-Type: ",
                             line_end - line_start))) {
            found += 14; /**@todo clean this up */
            strncpy(mime_type,
                    found,
                    MIN((size_t)(line_end - found - 2), mime_type_len));
            mime_type[MIN((size_t)(line_end - found - 2), mime_type_len - 1)] =
                '\0';
        }
        line_start = line_end;
        line_end   = strstr(line_start, "\n");
    }

    /* find the end boundary */
    size_t hl = header_end - buf;
    size_t bl = boundary_end - buf;

    for (size_t pos = hl; pos + (bl - 2) < buf_len; pos++) {
        if (buf[pos] == '-' && !strncmp(buf, &buf[pos], bl - 2)) {
            if (data_len != NULL) { *data_len = (pos - 2) - hl; }
            if (data != NULL) { *data = buf + hl; }
            return pos;
        }
    }

    return 0;
}

//Decode URL special characters in tensor names
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


/**
 * @brief v1_post implements how the server handles a post request
 * that runs the model with given inputs.
 *
 * This function is called by ev_handler in the case that a post
 * request was received by the server. It then handles a multipart
 * message that contains the tensors to be used as inputs
 * to the model used by the server. Given that no errors arose
 * the server generates a message that contains the requested
 * output tensors, which is then used by v1_response to respond
 * to the requester.
 */

int
model_handler(SOCKET             sock,
              char*              method,
              char*              path,
              struct phr_header* headers,
              size_t             n_headers,
              char*              content,
              size_t             content_length,
              void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    char*       json;
    const char* errmsg;
    int         errlen;
    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            errno = ENOMEM;
            return -1;
        }
        server->json_buffer = ptr;
        server->json_size   = 4096;
    }

    for (size_t i = 0; i < n_headers; ++i) {
        struct phr_header* hdr = &headers[i];
        if (strncasecmp("Accept", hdr->name, hdr->name_len) == 0 &&
            strncasecmp("application/vnd.deepview.rtm",
                        hdr->value,
                        hdr->value_len) == 0) {
            return http_response(sock,
                                 200,
                                 "application/vnd.deepview.rtm",
                                 server->model_size,
                                 server->model_upload,
                                 0,
                                 NULL);
        }
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
    json = json_uint64_flex(&server->json_buffer,
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
        json = json_uint64_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "timing",
                                (int64_t)(server->output.timing[i]/server->inference_count*1e3));
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
        json = json_uint64_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "avg_timing",
                                (int64_t)(server->output.timing[i]/server->inference_count*1e3));
        json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
    }

    json =
        json_arrClose_flex(&server->json_buffer, &server->json_size, json);
    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);

    if (!json) { goto json_oom; }

    json = json_end(json);
    return http_response(sock,
                         200,
                         "application/json",
                         json - server->json_buffer,
                         server->json_buffer,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);


json_oom:
    errmsg = "JSON Response Too Large";
    errlen = strlen(errmsg);
    return http_response(sock,
                         500,
                         "text/plain",
                         errlen,
                         errmsg,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);

}

int
v1_handler_put(SOCKET             sock,
               char*              method,
               char*              path,
               struct phr_header* headers,
               size_t             n_headers,
               char*              content,
               size_t             content_length,
               void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    const char* errmsg = "";
    int         errlen;

    size_t      data_len, n1 = 0, n2 = 0;
    const char* data;
    char        name[64];
    char        filename[64];
    char        mime[64];
    char*       json;
    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            errno = ENOMEM;
            return -1;
        }
        server->json_buffer = ptr;
        server->json_size   = 4096;
    }

    while ((n2 = parse_multipart_mime(content + n1,
                                      content_length - n1,
                                      name,
                                      sizeof(name),
                                      filename,
                                      sizeof(filename),
                                      mime,
                                      sizeof(mime),
                                      &data,
                                      &data_len))) {

        if (strcmp(name, "block_count") == 0) {
            data_t_max = atoi(data);
            data_t_count = 0;
            goto json_success;
        }

        if (data_t_count < data_t_max && strcmp(name, "block_content") == 0){
            memcpy(blob + block_size * data_t_count, data, data_len);
            data_t_count++;
        }
        if (data_t_count == data_t_max){
            server->model_upload = (char*)blob;
            server->model_size = block_size * data_t_count + data_len;
            Model_Setup(server);
            goto json_success;
        }
        n1 += n2;
    }
    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "block",
                         data_t_count);


    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);

    if (!json) { goto json_oom; }

    json = json_end(json);
    return http_response(sock,
                         200,
                         "application/json",
                         json - server->json_buffer,
                         server->json_buffer,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);

json_success:
    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "reply",
                         "success");


    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);

    if (!json) { goto json_oom; }

    json = json_end(json);

    return http_response(sock,
                         200,
                         "application/json",
                         json - server->json_buffer,
                         server->json_buffer,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);


json_oom:
    errmsg = "JSON Response Too Large";
    errlen = strlen(errmsg);
    return http_response(sock,
                         500,
                         "text/plain",
                         errlen,
                         errmsg,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);

}
int
v1_handler(SOCKET             sock,
           char*              method,
           char*              path,
           struct phr_header* headers,
           size_t             n_headers,
           char*              content,
           size_t             content_length,
           void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    const char* errmsg = "error";
    int         errlen;

    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            errno = ENOMEM;
            return -1;
        }
        server->json_buffer = ptr;
        server->json_size   = 4096;
    }

    char*       json;
    char version[30];
    sprintf(version,"TFLiteMicro-%s", TF_VERSION_STRING);

    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "engine",
                         "TensorFlow Lite");
    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "version",
                         version);
    json = json_objOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "model_limits");
    if (!json) { goto json_oom; }

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "block_size",
                         block_size);
    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "max_layers",
                         1024);
    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "max_input_size",
                         607500);
    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "max_model_size",
                         20971520);
    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);
    if (!json) { goto json_oom; }

    json = json_objClose_flex(&server->json_buffer, &server->json_size, json);

    if (!json) { goto json_oom; }

    json = json_end(json);
    return http_response(sock,
                         200,
                         "application/json",
                         json - server->json_buffer,
                         server->json_buffer,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);
json_oom:
    errmsg = "JSON Response Too Large";
    errlen = strlen(errmsg);
    return http_response(sock,
                         500,
                         "text/plain",
                         errlen,
                         errmsg,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);

}

int
v1_handler_post(SOCKET             sock,
                char*              method,
                char*              path,
                struct phr_header* headers,
                size_t             n_headers,
                char*              content,
                size_t             content_length,
                void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    const char* errmsg = "no model uploaded";
    int         errlen;
    int n_outputs = 0;
    int outputs_idx[16];
    char*       json;
    char* accepts="";
    char runtime [32];
    char version[30];
    sprintf(version,"TFLiteMicro-%s", TF_VERSION_STRING);
    memset(outputs_idx, 0, sizeof(outputs_idx));

    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            errno = ENOMEM;
            return -1;
        }
        server->json_buffer = ptr;
        server->json_size   = 4096;
    }

    char *val, *key, *query, *rem;
    strtok_r(path, "?", &rem);
    for(query = strtok_r(NULL, "&", &rem); query != NULL; query = strtok_r(NULL, "&", &rem)) {
        key = strtok_r(query,"=", &val );
#ifdef DEBUG
        PRINTF("param: %s value %s\r\n",key, val);
#endif
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
                errmsg = "Output Tensor Not Found";
                errlen = strlen(errmsg);
                return http_response(sock,
                                     404,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }
        }
    }

    char mime [50];
    char name [30];
    char filename [30];
    size_t      data_len_, n1 = 0, n2 = 0;
    const char* data             = NULL;

    while ((n2 = parse_multipart_mime(content + n1,
                                      content_length - n1,
                                      name,
                                      sizeof(name),
                                      filename,
                                      sizeof(filename),
                                      mime,
                                      sizeof(mime),
                                      &data,
                                      &data_len_))) {
        if (strcmp("application/vnd.deepview.tensor.float32", mime) == 0) {
            for (int i=0; i<server->input.inputs_size; i++){
                if (strcmp (name, server->input.name [i] ) == 0){
                    server->input.input_data [i] = (char*) malloc(data_len_);
                    memcpy(server->input.input_data [i], data, data_len_);
                    break;
                }
            }
        }
        n1 += n2;
    }


    for (size_t i = 1; i < n_headers; ++i) {
        const struct phr_header* hdr = &headers[i];
        if (strncmp("Content-Type", hdr->name, hdr->name_len) == 0 &&
            strncmp("image/", hdr->value, MIN(hdr->value_len, 6)) == 0) {

            int w, h, c;
            uint8_t* source = stbi_load_from_memory((const void*)content,
                                                    (int)content_length,
                                                    &w,
                                                    &h,
                                                    &c,
                                                    3);
            if (!source){
                printf("image load failed\r\n");
                errmsg = "Invalid or Unsupported Image Format";
                errlen = strlen(errmsg);

                return http_response(sock,
                                     415,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }
            int height = server->input_dims_data[1];
            int width = server->input_dims_data[2];
            int channels = server->input_dims_data[3];
			if (server->image_upload_data){
                free(server->image_upload_data);
			}
            server->image_upload_data = (uint8_t*) malloc(height * width * channels);
            if (h != height || w != width) {
                stbir_resize_uint8(source,
                                   w,
                                   h,
                                   0,
                                   server->image_upload_data,
                                   width,
                                   height,
                                   0,
                                   channels);
            } else {
				memcpy((void*) server->image_upload_data, (const void*) source, height * width * channels);
            }
            stbi_image_free(source);

        }
    }

    for (size_t i = 0; i < n_headers; ++i) {
        const struct phr_header* hdr = &headers[i];
        if (strncmp("Accept", hdr->name, hdr->name_len) == 0) {
            accepts                 = (char*) hdr->value;
            accepts[hdr->value_len] = '\0';
        }
    }
    if (n_outputs > 1 && strcmp(accepts, "application/vnd.deepview.tensor") == 0){
        errmsg = "Only a single tensor can be returned using "
            "application/vnd.deepview.tensor";

        errlen = strlen(errmsg);
        return http_response(sock,
                             400,
                             "text/plain",
                             errlen,
                             errmsg,
                             HTTP_FLAGS,
                             HTTP_HEADERS_END);
    }

    Model_RunInference(server);

    if (strcmp(accepts, "application/vnd.deepview.tensor") == 0) {
        snprintf(runtime, sizeof(runtime), "%lld", server->run_ns);
        if (!server->output.data[outputs_idx[0]]) {
            errmsg = "Output Tensor Empty";
            errlen = strlen(errmsg);
            return http_response(sock,
                                 500,
                                 "text/plain",
                                 errlen,
                                 errmsg,
                                 HTTP_FLAGS,
                                 HTTP_HEADERS_END);
        }

        char shape[30];
        int offset=0;
        for (int i=0; i< server->output.shape_size [outputs_idx [0]]; i++) {
            offset += sprintf (shape+offset, "%d ",server->output.shape_data[outputs_idx [0]] [i]);
        }

        if (http_response(sock,
                          200,
                          "application/vnd.deepview.tensor",
                          server->output.bytes[outputs_idx[0]],
                          server->output.data[outputs_idx[0]],
                          HTTP_FLAGS,
                          "X-Tensor-Type",
                          server->output.data_type [outputs_idx [0]],
                          "X-Tensor-Shape",
                          shape,
                          "X-Model-Timing",
                          runtime,
                          HTTP_HEADERS_END)) {
            return -1;
        }
        return 0;
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

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "timing",
                         (float)(server->run_ns*1e3));
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

    if (!json) { goto json_oom; }

    json = json_end(json);
    return http_response(sock,
                         200,
                         "application/json",
                         json - server->json_buffer,
                         server->json_buffer,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);
json_oom:
    errmsg = "JSON Response Too Large";
    errlen = strlen(errmsg);
    return http_response(sock,
                         500,
                         "text/plain",
                         errlen,
                         errmsg,
                         HTTP_FLAGS,
                         HTTP_HEADERS_END);

}

