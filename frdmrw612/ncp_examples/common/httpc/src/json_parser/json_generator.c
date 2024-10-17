/*
 *  Copyright 2008-2020 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*
 * Simple JSON Generator
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <json_generator.h>
#include <json_parser.h>
#include <wmerrno.h>

#include <wmlog.h>

#define json_e(...) wmlog_e("json", ##__VA_ARGS__)
#define json_w(...) wmlog_w("json", ##__VA_ARGS__)

#ifdef CONFIG_JSON_DEBUG
#define json_d(...) wmlog("json", ##__VA_ARGS__)
#else
#define json_d(...)
#endif /* ! CONFIG_JSON_DEBUG */

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
    while (in && (unsigned char)*in <= 32)
        in++;
    return in;
}

static const char *rev_skip(const char *in)
{
    while (in && (unsigned char)*in <= 32)
        in--;
    return in;
}

const char *verify_json_start(const char *buff)
{
    buff = skip(buff);
    if (*buff != '{' && *buff != '[')
    {
        json_e("Invalid JSON document");
        return NULL;
    }
    else
    {
        return ++buff;
    }
}

static int verify_buffer_limit(struct json_str *jptr)
{
    /*
     * Check for buffer overflow condition here, and then copy remaining
     * data using snprintf. This makes sure there is no mem corruption in
     * json set operations.
     */
    if (jptr->free_ptr >= (jptr->len - 1))
    {
        json_e("buffer maximum limit reached");
        return -1;
    }
    else
        return WM_SUCCESS;
}

void json_str_init(struct json_str *jptr, char *buff, int len)
{
    jptr->buff = buff;
    memset(jptr->buff, 0, len);
    jptr->free_ptr = 0;
    jptr->len      = len;
}

void json_str_init_no_clear(struct json_str *jptr, char *buff, int len)
{
    jptr->buff     = buff;
    jptr->free_ptr = 0;
    jptr->len      = len;
}

void json_str_finish(struct json_str *jptr)
{
    jptr->buff[jptr->free_ptr] = 0;
}

int json_push_object(struct json_str *jptr, const char *name)
{
    char *buff;

    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    /* From last skip cr/lf */
    buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);
    if (*buff != '{') /* Element in object */
        jptr->buff[jptr->free_ptr++] = ',';

    snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":{", name);

    jptr->free_ptr = strlen(jptr->buff);
    return WM_SUCCESS;
}

int json_push_array_object(struct json_str *jptr, const char *name)
{
    char *buff;

    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    /* From last skip cr/lf */
    buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);
    if (*buff != '{') /* Element in object */
        jptr->buff[jptr->free_ptr++] = ',';

    snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":[", name);

    jptr->free_ptr = strlen(jptr->buff);
    return WM_SUCCESS;
}

#if 0
int json_start_object(struct json_str *jptr)
{
    char *buff;

    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    if (jptr->free_ptr)
    {
        /* This should be first call after json_str_init so free_ptr
         * should be 0 but if it is not then we add ',' before
         * starting object as there could have been earlier object
         * already present as case in array.
         */
        /* From last skip cr/lf */
        buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

        if (*buff == '}')
            jptr->buff[jptr->free_ptr++] = ',';
    }
    jptr->buff[jptr->free_ptr++] = '{';
    return WM_SUCCESS;
}
#endif

int json_close_object(struct json_str *jptr)
{
    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    jptr->buff[jptr->free_ptr++] = '}';

    return WM_SUCCESS;
}

int json_pop_array_object(struct json_str *jptr)
{
    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    jptr->buff[jptr->free_ptr++] = ']';

    return WM_SUCCESS;
}

#if 0
int json_start_array(struct json_str *jptr)
{
    char *buff;
    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    /* From last skip cr/lf */
    buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

    if (*buff == ']')
        jptr->buff[jptr->free_ptr++] = ',';

    jptr->buff[jptr->free_ptr++] = '[';
    return WM_SUCCESS;
}
#endif

int json_close_array(struct json_str *jptr)
{
    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    jptr->buff[jptr->free_ptr++] = ']';
    return WM_SUCCESS;
}

int json_set_array_value(struct json_str *jptr, char *str, int value, float val, json_data_types data)
{
    char *buff;

    if (!verify_json_start(jptr->buff))
        return WM_E_JSON_INVSTART;

    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    /* From last skip cr/lf */
    buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

    if (*buff != '[') /* Element in object */
        jptr->buff[jptr->free_ptr++] = ',';

    switch (data)
    {
        case JSON_VAL_STR:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\"", str);
            break;
        case JSON_VAL_INT:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "%d", value);
            break;
        case JSON_VAL_FLOAT:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "%lf", (double)val);
            break;
        case JSON_VAL_BOOL:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "%s", (value == 1) ? "true" : "false");
            break;
        default:
            json_e("Invalid case in array set");
    }

    jptr->free_ptr = strlen(jptr->buff);
    return WM_SUCCESS;
}

int json_set_object_value(struct json_str *jptr,
                          const char *name,
                          const char *str,
                          int64_t value,
                          float val,
                          short precision,
                          json_data_types data)
{
    char *buff;

    if (!verify_json_start(jptr->buff))
        return -WM_E_JSON_INVSTART;

    if (verify_buffer_limit(jptr) < 0)
        return -WM_E_JSON_OBUF;

    /* From last skip cr/lf */
    buff = (char *)rev_skip(&jptr->buff[jptr->free_ptr - 1]);

    if (*buff != '{') /* Element in object */
        jptr->buff[jptr->free_ptr++] = ',';

    switch (data)
    {
        case JSON_VAL_STR:
            /* First, check if the string can fit into the buffer.
             * The + 6 is used to account for "":"" and NULL termintaion
             */
            if ((strlen(str) + strlen(name) + 6) > (jptr->len - jptr->free_ptr))
                return -WM_E_JSON_OBUF;

            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":\"", name);
            jptr->free_ptr = strlen(jptr->buff);
            /* We use memmove in order to allow the source and destination
             * strings to overlap
             */
            memmove(&jptr->buff[jptr->free_ptr], str, strlen(str) + 1);
            jptr->free_ptr = strlen(jptr->buff);
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"");
            break;

        case JSON_VAL_INT:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":%d", name, (int)value);
            break;

        case JSON_VAL_UINT:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":%u", name, (unsigned)value);
            break;

        case JSON_VAL_UINT_64:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":%llu", name, value);
            break;

        case JSON_VAL_FLOAT:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":", name);
            jptr->free_ptr = strlen(jptr->buff);
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "%lf", (double)val);
            break;
        case JSON_VAL_BOOL:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":%s", name,
                     (value == 1) ? "true" : "false");
            break;
        case JSON_VAL_NULL:
            snprintf(&jptr->buff[jptr->free_ptr], jptr->len - jptr->free_ptr, "\"%s\":null", name);
            break;
        default:
            json_e("Invalid case in object set");
    }

    jptr->free_ptr = strlen(jptr->buff);
    return WM_SUCCESS;
}
