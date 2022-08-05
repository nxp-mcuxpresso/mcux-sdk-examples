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

#include "modelrunner.h"
#include "json/flex.h"

#if !(defined(__ARMCC_VERSION) || defined(__ICCARM__))
#include <strings.h>
#endif

#ifndef MIN
#define MIN(a, b) (a < b ? a : b)
#endif

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)
#endif

#define MEMORY_ALIGNMENT 64


extern char*
strtok_r(char* str, const char* delim, char** saveptr);

extern size_t
strnlen(const char* s, size_t maxlen);

#define HTTP_FLAGS HTTP_CONNECTION_CLOSE

static const int32_t block_size = 1048576;

//static const size_t mime_data_limit = 4 * 1024 * 1024;
//static const char*  mime_tensor     = "application/vnd.deepview.tensor";

static char* uuid_null()
{
    return NULL;
}

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

static const char*
tensor_datatype(int32_t glow_type)
{
    switch (glow_type) {
    case 0:
        return "INT8";
    case 1:
        return "FLOAT32";
    }

    return "NONE";
}

static char*
error_json(char* json, NNServer* server, const char* errmsg, ...)
{
    va_list ap;
    int     err;
    char    msg[1024];

    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { return NULL; }

    va_start(ap, errmsg);
    err = vsnprintf(msg, sizeof(msg), errmsg, ap);
    va_end(ap);

    if (err == -1) {
        fprintf(stderr, "error_json: encoding error\n");
        return NULL;
    }

    if (err > sizeof(msg)) {
        fprintf(stderr, "error_json: error truncated from length %d\n", err);
    }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "error",
                         msg);
    if (!json) { return NULL; }

    return json_objClose_flex(&server->json_buffer, &server->json_size, json);
}

static char*
layer_json(char*       json,
           NNServer*   server,
           const char* key,
           size_t      index,
           int         data,
           int         tensor_is_child)
{
    const char* layer_name = server->output_name[index];
    const char* data_type = tensor_datatype(server->output_type[index]);

    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, key);
    if (!json) { return NULL; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "name",
                         layer_name ? layer_name : "");
    if (!json) { return NULL; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "type",
                         "output");
    if (!json) { return NULL; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "avg_timing",
                           0);
    if (!json) { return NULL; }

    if (1) {
        int32_t* shape = server->output_shape[index];
        int32_t num_dims = server->output_dims[index];
        if (tensor_is_child) {
            json = json_objOpen_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "tensor");
            if (!json) { return NULL; }
        }

        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "datatype",
                             data_type);
        if (!json) { return NULL; }

        json = json_int64_flex(&server->json_buffer,
                               &server->json_size,
                               json,
                               "timing",
                               0);
        if (!json) { return NULL; }

        json = json_int64_flex(&server->json_buffer,
                               &server->json_size,
                               json,
                               "io_timing",
                               0);
        if (!json) { return NULL; }

        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "shape");
        if (!json) { return NULL; }

        for (int32_t dim = 0; dim < num_dims; ++dim) {
            json = json_int_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL,
                                 shape[dim]);
            if (!json) { return NULL; }
        }

        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { return NULL; }

        if (data) {
            const void* map = (const void*)server->output[index];
            if (map) {
                json = json_base64_flex(&server->json_buffer,
                                        &server->json_size,
                                        json,
                                        "data",
                                        map,
                                        tensor_size(shape,num_dims,server->output_type[index]));
                if (!json) { return NULL; }
            }
        }

        if (tensor_is_child) {
            json = json_objClose_flex(&server->json_buffer,
                                      &server->json_size,
                                      json);
            if (!json) { return NULL; }
        }
    }

    return json_objClose_flex(&server->json_buffer, &server->json_size, json);
}

/**
 * @brief v1_hostinfo implements how the server handles a request
 * for the server information.
 *
 * This function is called by ev_handler in the case that the
 * server information was requested. This function then gathers
 * all of the relevant information placing it within a builder,
 * that is then used by v1_response to generate the correct message
 * before being sent to the requester.
 */
static int
v1_hostinfo(SOCKET             sock,
            char*              method,
            char*              path,
            struct phr_header* headers,
            size_t             n_headers,
            char*              content,
            size_t             content_length,
            void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    const char* errmsg;
    size_t      errlen;
    char*       json;

    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "engine",
                         "GLOW");
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "version",
                         "1.1.0");
    if (!json) { goto json_oom; }

    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "batch_size",
                         1);
    if (!json) { goto json_oom; }


    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "hostname",
                         "RT1170-EVK");
	if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "CPU",
                         "cortex-m7");
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "arch",
                         "arm");
    if (!json) { goto json_oom; }

    if (1) {
        const char* model_uuid = uuid_null();
        const char* model_name = server->model_name;

        json = json_objOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "model");
        if (!json) { goto json_oom; }

        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "uuid",
                             model_uuid ? model_uuid : "");
        if (!json) { goto json_oom; }

        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "name",
                             model_name ? model_name : "");
        if (!json) { goto json_oom; }

        json = json_uint64_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "size",
                                server->model_size);
        if (!json) { goto json_oom; }

        json = json_uint64_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "activation_size",
								server->mem_size);
        if (!json) { goto json_oom; }

/*
        json = json_int_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "label_count",
                             nn_model_label_count(server->model));
        if (!json) { goto json_oom; }

        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "labels");
        if (!json) { goto json_oom; }

        for (int i = 0; i < nn_model_label_count(server->model); ++i) {
            const char* label = nn_model_label(server->model, i);
            json              = json_str_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 NULL,
                                 label ? label : "");
            if (!json) { goto json_oom; }
        }

        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { goto json_oom; }
*/

        json =
            json_objClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { goto json_oom; }
    } else {
        json = json_null_flex(&server->json_buffer,
                              &server->json_size,
                              json,
                              "model");
        if (!json) { goto json_oom; }
    }


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
    if (!json) { goto json_oom; }

    //16MB of flash max
    json = json_uint64_flex(&server->json_buffer,
                            &server->json_size,
                            json,
                            "max_model_size",
                            12582912);
    if (!json) { goto json_oom; }


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


/**
 * @brief v1_modelinfo implements how the server handles a request
 * for the model information.
 *
 * This function is called by ev_handler in the case that the
 * model information was requested. This function then gathers
 * all of the relevant information placing it within a builder,
 * that is then used by v1_response to generate the correct message
 * before being sent to the requester.
 */
static int
v1_modelinfo(SOCKET             sock,
             char*              method,
             char*              path,
             struct phr_header* headers,
             size_t             n_headers,
             char*              content,
             size_t             content_length,
             void*              user_data)
{
    size_t      errlen;
    const char* errmsg;
    char*       json;
    const char* model_name;
    const char* model_uuid;
    NNServer*   server = (NNServer *)user_data;

    model_uuid = uuid_null();
    model_name = server->model_name;

    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "model_name",
                         model_name ? model_name : "");
    if (!json) { goto json_oom; }

    json = json_str_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "uuid",
                         model_uuid ? model_uuid : "");
    if (!json) { goto json_oom; }

    json = json_uint64_flex(&server->json_buffer,
                            &server->json_size,
                            json,
                            "size",
                            server->model_size);
    if (!json) { goto json_oom; }

    json = json_uint64_flex(&server->json_buffer,
                            &server->json_size,
                            json,
                            "memory_size",
							server->mem_size);
    if (!json) { goto json_oom; }

    if (1) {
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "inputs");
        if (!json) { goto json_oom; }

        for (size_t i = 0; i < 1; i++) {
            json = json_objOpen_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     NULL);
            if (!json) { goto json_oom; }


            json = json_str_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "name",
                                 server->input_name);
            if (!json) { goto json_oom; }


            json = json_str_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "datatype",
                                 tensor_datatype(server->input_type));
            if (!json) { goto json_oom; }

            double scale_val = (double)server->input_scale;
            json = json_double_flex(&server->json_buffer,
                                    &server->json_size,
                                    json,
                                    "scale",
                                    scale_val);
            if (!json) { goto json_oom; }

            int32_t zero_val = server->input_zero;
            json =
                json_int_flex(&server->json_buffer,
                              &server->json_size,
                              json,
                              "zero_point",
                              zero_val);
            if (!json) { goto json_oom; }

            json = json_arrOpen_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "shape");
            if (!json) { goto json_oom; }
            int      n_dims = server->input_dims;
            int32_t* inshape = server->input_shape;

            for (int dim = 0; dim < n_dims; ++dim) {
                json = json_int_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     NULL,
                                     inshape[dim]);
                if (!json) { goto json_oom; }
            }

            json = json_arrClose_flex(&server->json_buffer,
                                      &server->json_size,
                                      json);
            if (!json) { goto json_oom; }

            json = json_objClose_flex(&server->json_buffer,
                                      &server->json_size,
                                      json);
            if (!json) { goto json_oom; }
        }

        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { goto json_oom; }
    }

    if (1) {
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "outputs");
        if (!json) { goto json_oom; }

        for (int i = 0; i < server->num_outputs; i++) {
            json = json_objOpen_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     NULL);
            if (!json) { goto json_oom; }

            json = json_str_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "name",
                                 server->output_name[i]);
            if (!json) { goto json_oom; }

            json = json_str_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "datatype",
                                 tensor_datatype(server->output_type[i]));
            if (!json) { goto json_oom; }

            /*
            double scale_val = (double)server->output_scale[i];
            json = json_double_flex(&server->json_buffer,
                                    &server->json_size,
                                    json,
                                    "scale",
                                    scale_val);
            if (!json) { goto json_oom; }

            int zero_val = server->output_zero[i];
            json =
                  json_int_flex(&server->json_buffer,
                                &server->json_size,
                                json,
                                "zero_point",
                                zero_val);
            if (!json) { goto json_oom; }
            */

            json = json_arrOpen_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "shape");
            if (!json) { goto json_oom; }
            int32_t  n_dims = server->output_dims[i];
            int32_t* outshape = server->output_shape[i];

            for (int dim = 0; dim < n_dims; ++dim) {
                json = json_int_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     NULL,
                                     outshape[dim]);
                if (!json) { goto json_oom; }
            }

            json = json_arrClose_flex(&server->json_buffer,
                                      &server->json_size,
                                      json);
            if (!json) { goto json_oom; }

            json = json_objClose_flex(&server->json_buffer,
                                      &server->json_size,
                                      json);
            if (!json) { goto json_oom; }
        }

        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { goto json_oom; }
    }

/*
    json = json_int_flex(&server->json_buffer,
                         &server->json_size,
                         json,
                         "label_count",
                         nn_model_label_count(server->model));
    if (!json) { goto json_oom; }

    json = json_arrOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "labels");
    if (!json) { goto json_oom; }

    for (int i = 0; i < nn_model_label_count(server->model); ++i) {
        const char* label = nn_model_label(server->model, i);
        json              = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             NULL,
                             label ? label : "");
        if (!json) { goto json_oom; }
    }

    json = json_arrClose_flex(&server->json_buffer, &server->json_size, json);
    if (!json) { goto json_oom; }
*/
    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "timing",
                           server->timing.run);
    if (!json) { goto json_oom; }

    json = json_objOpen_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "timings");
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "decode",
                           server->timing.decode);
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "input",
                           server->timing.input);
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "output",
                           server->timing.output);
    if (!json) { goto json_oom; }

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
                         NULL);

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

/**
 * @brief v1_modelinfo implements how the server handles a request
 * for the model buffer.
 * <p>
 * This function is called by ev_handler in the case that the
 * model buffer was requested. This function is called when a
 * GET request has application/vnd.deepview.rtm in the Accept header
 * </p>
 */
static int
v1_modelbuffer(SOCKET             sock,
               char*              method,
               char*              path,
               struct phr_header* headers,
               size_t             n_headers,
               char*              content,
               size_t             content_length,
               void*              user_data)
{
    NNServer* server = (NNServer *)user_data;

    return http_response(sock,
                         200,
                         "application/vnd.deepview.rtm",
                         server->model_size,
                         server->weights,
                         HTTP_FLAGS,
                         NULL);
}


static int
v1_options(SOCKET             sock,
           char*              method,
           char*              path,
           struct phr_header* headers,
           size_t             n_headers,
           char*              content,
           size_t             content_length,
           void*              user_data)
{
    return http_response(sock, 204, "", 0, NULL, HTTP_FLAGS, HTTP_HEADERS_END);
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
static int
v1_post(SOCKET             sock,
        char*              method,
        char*              path,
        struct phr_header* headers,
        size_t             n_headers,
        char*              content,
        size_t             content_length,
        void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    char*       state  = NULL;
    char*       query  = NULL;
    char*       amp    = NULL;
    size_t      data_len, n1 = 0, n2 = 0;
    const char* data;
    char        name[64];
    char        filename[64];
    char        mime[64];
    char        out_tensor_name[64];
    int         outputs[4];
    int         status;
    int         n_outputs  = 0;
    char*       runid      = NULL;
    int         run_model  = 0;
    int         imgproc    = 0;
    char*       accepts    = "";
    char*       json;
    const char* errmsg;
    size_t      errlen;

    strtok_r(path, "?", &state);
    query = strtok_r(NULL, "?", &state);

    if (query) { amp = strtok_r(query, "&", &state); }

    while (amp) {
        char* sub = NULL;
        char* key = strtok_r(amp, "=", &sub);

        if (!key) {
            errmsg = "Badly Formatted URL Query";
            errlen = strlen(errmsg);

            return http_response(sock,
                                 400,
                                 "text/plain",
                                 errlen,
                                 errmsg,
                                 HTTP_FLAGS,
                                 HTTP_HEADERS_END);
        }
        char* val = strtok_r(NULL, "=", &sub);

        if (strcasecmp("output", key) == 0) {
            if (n_outputs >= 4) {
                errmsg = "Too Many Outputs Requested";
                errlen = strlen(errmsg);

                return http_response(sock,
                                     400,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }
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
            int output = server->num_outputs;
            for(int ind = 0; ind < server->num_outputs; ind++){
                if(strcmp(server->output_name[ind],out_tensor_name)==0){
                	output = ind;
                	break;
                }
            }

            if (output == server->num_outputs) {
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

            //output_names[n_outputs] = val;
            outputs[n_outputs]      = output;
            n_outputs++;
        } else if (strcasecmp("run", key) == 0) {
            run_model = atoi(val);
        } else if (strcasecmp("step", key) == 0) {
            errmsg = "Step unsupported";
            errlen = strlen(errmsg);

            return http_response(sock,
                                 404,
                                 "text/plain",
                                 errlen,
                                 errmsg,
                                 HTTP_FLAGS,
                                 HTTP_HEADERS_END);
        } else if (strcasecmp("id", key) == 0) {
            runid = val;
        } else if (strcasecmp("imgproc", key) == 0) {
            if (strcasecmp("none", val) == 0) {
                imgproc = 0;
            } else if (strcasecmp("normalize", val) == 0) {
                imgproc = NN_IMAGE_PROC_UNSIGNED_NORM;
            } else if (strcasecmp("whitening", val) == 0) {
                imgproc = NN_IMAGE_PROC_WHITENING;
            } else if (strcasecmp("signed_normalize", val) == 0) {
                imgproc = NN_IMAGE_PROC_SIGNED_NORM;
            } else {
                errmsg = "Unsupported Image Processing Request";
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

        amp = strtok_r(NULL, "&", &state);
    }

    for (size_t i = 0; i < n_headers; ++i) {
        const struct phr_header* hdr = &headers[i];

        if (strncmp("Content-Type", hdr->name, hdr->name_len) == 0 &&
            strncmp("image/", hdr->value, MIN(hdr->value_len, 6)) == 0) {
            int64_t   decode_start;

            decode_start = os_clock_now();
            status   = tensor_load_image(server,
                                         content,
                                         content_length,
                                         imgproc);
            server->timing.decode = os_clock_now() - decode_start;

            if (status) {
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
        }
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
        if (strcmp("application/vnd.deepview.tensor.float32", mime) == 0) {
            int32_t* shape    = server->input_shape;
            int32_t  num_dims = server->input_dims;

            if (data_len != tensor_size(shape,num_dims,server->input_type)) {
                errmsg = "Input Tensor Size Mismatch";
                errlen = strlen(errmsg);

                return http_response(sock,
                                     400,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }

            char* dim = strtok_r(filename, ",", &state);
            int n_dims = 0;
            int inshape[4];
            while (dim) {
                if (n_dims == num_dims) {
                    errmsg = "Input Tensor Rank Too High";
                    errlen = strlen(errmsg);

                    return http_response(sock,
                                         400,
                                         "text/plain",
                                         errlen,
                                         errmsg,
                                         HTTP_FLAGS,
                                         HTTP_HEADERS_END);
                }

                inshape[n_dims++] = atoi(dim);
                dim             = strtok_r(NULL, ",", &state);
            }
            bool shape_equal = true;
            if(n_dims != num_dims) {
            	shape_equal = false;
            } else{
                for(int d = 0; d < num_dims; d++){
                    if(inshape[d]!=shape[d]) shape_equal = true;
                }
            }
            if (!shape_equal) {
                errmsg = "POSTed Input Tensor Has Incorrect Shape";
                errlen = strlen(errmsg);

                return http_response(sock,
                                     400,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }

            void* map = (void *)server->input;
            if (!map) {
                errmsg = "Failed Map Input Tensor";
                errlen = strlen(errmsg);

                return http_response(sock,
                                     500,
                                     "text/plain",
                                     errlen,
                                     errmsg,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }

            memcpy(map, data, data_len);
        }

        n1 += n2;
    }

    if (run_model) {
    	server->timing.run = 0;
        for (int i = 0; i < run_model; ++i) {
            int64_t start = os_clock_now();
            status = server->inference(server->weights, server->mutable, server->activation);
            server->timing.run += os_clock_now() - start;

            if (status) {
                json = error_json(server->json_buffer,
                                  server,
                                  "inference error: %d",
                                  status);
                if (!json) { goto json_oom; }

                return http_response(sock,
                                     500,
                                     "application/json",
                                     json - server->json_buffer,
                                     server->json_buffer,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }
        }
        server->timing.run /= run_model;
    }

    for (size_t i = 0; i < n_headers; ++i) {
        const struct phr_header* hdr = &headers[i];
        if (strncmp("Accept", hdr->name, hdr->name_len) == 0) {
            accepts                 = (char*) hdr->value;
            accepts[hdr->value_len] = '\0';
        }
    }
/*
    if (strcmp(accepts, "application/vnd.deepview.tensor") == 0) {
        NNTensor*      output;
        const int32_t* output_shape;
        int            shape_offset = 0;
        char           tensor_shape[32];
        char           runtime[32];
        const void*    map;

        if (n_outputs == 0) {
            return http_response(sock,
                                 204,
                                 "application/vnd.deepview.tensor",
                                 0,
                                 NULL,
                                 HTTP_FLAGS,
                                 HTTP_HEADERS_END);
        } else if (n_outputs > 1) {
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

        output = nn_context_tensor_index(server->context, outputs[0]);
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

        nn_tensor_sync(output);
        server->timing.output = nn_tensor_io_time(output);

        map = nn_tensor_mapro(output);
        if (!map) {
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

        output_shape = nn_tensor_shape(output);
        for (int i = 0; i < nn_tensor_dims(output); i++) {
            shape_offset = snprintf(tensor_shape + shape_offset,
                                    sizeof(tensor_shape) - shape_offset,
                                    "%d ",
                                    output_shape[i]);
            if (shape_offset == -1 || shape_offset >= sizeof(tensor_shape)) {
                tensor_shape[0] = '\0';
                break;
            }
        }

        snprintf(runtime, sizeof(runtime), "%lld", server->timing.run);

        if (http_response(sock,
                          200,
                          "application/vnd.deepview.tensor",
                          nn_tensor_size(output),
                          map,
                          HTTP_FLAGS,
                          "X-Tensor-Type",
                          tensor_datatype(output),
                          "X-Tensor-Shape",
                          tensor_shape,
                          "X-Model-Timing",
                          runtime,
                          HTTP_HEADERS_END)) {
            nn_tensor_unmap(output);
            return -1;
        }

        nn_tensor_unmap(output);
        return 0;
    } else
*/
    if (strcmp(accepts, "text/plain") == 0) {
        //const char* label    = NULL;
        float       softmax_ = 0.0f;
        int         argmax   = 0;
        char        runtime[32];
        char        softmax[32];
        char        label[8];

        uint8_t* output = server->output[0];
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
        status = glow_argmax(server, &argmax, &softmax_, sizeof(softmax_));
        if (status) {
            errmsg = server->json_buffer;
            errlen = snprintf(server->json_buffer,
                              server->json_size,
                              "ArgMax Error \n");

            if (errlen == -1 || errlen > server->json_size) {
                errmsg = "Out of Memory";
                errlen = strlen(errmsg);
            }

            return http_response(sock,
                                 500,
                                 "text/plain",
                                 errlen,
                                 errmsg,
                                 HTTP_FLAGS,
                                 HTTP_HEADERS_END);
        }

        //label = nn_model_label(server->model, argmax);
        snprintf(label,  sizeof(label), "%d", argmax);
        snprintf(runtime, sizeof(runtime), "%lld", server->timing.run);
        snprintf(softmax, sizeof(softmax), "%f", softmax_);

        return http_response(sock,
                             200,
                             "text/plain",
                             strlen(label),
                             label,
                             HTTP_FLAGS,
                             "X-Softmax",
                             softmax,
                             "X-Model-Timing",
                             runtime,
                             HTTP_HEADERS_END);
    }

    json = server->json_buffer;
    json =
        json_objOpen_flex(&server->json_buffer, &server->json_size, json, NULL);
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "timing",
                           server->timing.run);
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "decode",
                           server->timing.decode);
    if (!json) { goto json_oom; }

    json = json_int64_flex(&server->json_buffer,
                           &server->json_size,
                           json,
                           "input",
                           server->timing.input);
    if (!json) { goto json_oom; }

    if (runid) {
        json = json_str_flex(&server->json_buffer,
                             &server->json_size,
                             json,
                             "id",
                             runid);
        if (!json) { goto json_oom; }
    }

    //JSON Response with actual tensor or argmax
    if (n_outputs) {
        json = json_arrOpen_flex(&server->json_buffer,
                                 &server->json_size,
                                 json,
                                 "outputs");
        if (!json) { goto json_oom; }

        for (size_t i = 0; i < n_outputs; i++) {
            json = layer_json(json, server, NULL, outputs[i], 1, 0);
            if (!json) { goto json_oom; }
        }

        json =
            json_arrClose_flex(&server->json_buffer, &server->json_size, json);
        if (!json) { goto json_oom; }
    } else {
        uint8_t* output = server->output[0];
        if (output) {
            float   softmax = 0.0f;
            int     argmax  = 0;
            status = glow_argmax(server, &argmax, &softmax, sizeof(softmax));;
            if (status) {
                json = error_json(server->json_buffer,
                                  server,
                                  "argmax failed");
                if (!json) { goto json_oom; }

                return http_response(sock,
                                     500,
                                     "application/json",
                                     json - server->json_buffer,
                                     server->json_buffer,
                                     HTTP_FLAGS,
                                     HTTP_HEADERS_END);
            }
            //const char* label = nn_model_label(server->model, argmax);
            if (1) {
                json = json_int_flex(&server->json_buffer,
                                     &server->json_size,
                                     json,
                                     "label",
                                     argmax);
                if (!json) { goto json_oom; }

                json = json_double_flex(&server->json_buffer,
                                        &server->json_size,
                                        json,
                                        "softmax",
                                        softmax);
                if (!json) { goto json_oom; }
            }
        }
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


int
nn_server_http_handler(SOCKET             sock,
                       char*              method,
                       char*              path,
                       struct phr_header* headers,
                       size_t             n_headers,
                       char*              content,
                       size_t             content_length,
                       void*              user_data)
{
    NNServer*   server = (NNServer *)user_data;
    const char* errmsg = "Not Found";
    int         errlen = (int) strlen(errmsg);

    if (server->json_size < 4096) {
        void* ptr = realloc(server->json_buffer, 4096);
        if (!ptr) {
            errno = ENOMEM;
            return -1;
        }
        server->json_buffer = ptr;
        server->json_size   = 4096;
    }

    if (strcmp(method, "GET") == 0) {
        if (strncmp(path, "/v1/model", strlen("/v1/model")) == 0) {
            for (size_t i = 0; i < n_headers; ++i) {
                struct phr_header* hdr = &headers[i];

                if (strncasecmp("Accept", hdr->name, hdr->name_len) == 0 &&
                    strncasecmp("application/vnd.deepview.rtm",
                                hdr->value,
                                hdr->value_len) == 0) {
                    return v1_modelbuffer(sock,
                                          method,
                                          path,
                                          headers,
                                          n_headers,
                                          content,
                                          content_length,
                                          user_data);
                }
            }

            return v1_modelinfo(sock,
                                method,
                                path,
                                headers,
                                n_headers,
                                content,
                                content_length,
                                user_data);
        } else {
            return v1_hostinfo(sock,
                               method,
                               path,
                               headers,
                               n_headers,
                               content,
                               content_length,
                               user_data);
        }
    }
    else if (strcmp(method, "POST") == 0) {
        return v1_post(sock,
                       method,
                       path,
                       headers,
                       n_headers,
                       content,
                       content_length,
                       user_data);
    }
    else if (strcmp(method, "OPTIONS") == 0) {
        return v1_options(sock,
                          method,
                          path,
                          headers,
                          n_headers,
                          content,
                          content_length,
                          user_data);
    }

    return http_response(sock, 404, "text/plain", errlen, errmsg, HTTP_FLAGS, NULL);
}
