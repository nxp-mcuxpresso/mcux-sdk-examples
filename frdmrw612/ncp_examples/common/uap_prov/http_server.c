/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include "http_server.h"
#include "httpsrv.h"

#include <string.h>
#include <wmerrno.h>

/* FS data.*/
extern const HTTPSRV_FS_DIR_ENTRY httpsrv_fs_data[];
extern const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[];

static uint32_t s_httpsrv_handle = 0;

bool cgi_get_varval(char *src, char *var_name, char *dst, uint32_t length)
{
    char *name;
    bool result;
    uint32_t index;
    uint32_t n_length;

    result = false;
    dst[0] = 0;
    name   = src;

    n_length = strlen(var_name);

    while ((name != NULL) && ((name = strstr(name, var_name)) != NULL))
    {
        if (name[n_length] == '=')
        {
            name += n_length + 1;

            index = strcspn(name, "&");
            if (index >= length)
            {
                index = length - 1;
            }
            strncpy(dst, name, index);
            dst[index] = '\0';
            result     = true;
            break;
        }
        else
        {
            name = strchr(name, '&');
        }
    }

    return (result);
}

void format_post_data(char *string)
{
    /* Replace '+' with spaces. */
    char *c;
    while ((c = strchr(string, '+')) != NULL)
    {
        *c = ' ';
    }
}

/* Decode URL encoded string in place. */
void cgi_urldecode(char *url)
{
    char *src = url;
    char *dst = url;

    while (*src != '\0')
    {
        if ((*src == '%') && (isxdigit((unsigned char)*(src + 1))) && (isxdigit((unsigned char)*(src + 2))))
        {
            *src       = *(src + 1);
            *(src + 1) = *(src + 2);
            *(src + 2) = '\0';
            *dst++     = strtol(src, NULL, 16);
            src += 3;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

int http_srv_init(void)
{
    HTTPSRV_PARAM_STRUCT params;

    /* Init Fs*/
    HTTPSRV_FS_init(httpsrv_fs_data);

    /* Init HTTPSRV parameters.*/
    memset(&params, 0, sizeof(params));
    params.root_dir    = "";
    params.index_page  = "/index.html";
    params.cgi_lnk_tbl = cgi_lnk_tbl;

    /* Init HTTP Server.*/
    if (s_httpsrv_handle == 0)
    {
        s_httpsrv_handle = HTTPSRV_init(&params);
        if (s_httpsrv_handle == 0)
        {
            LWIP_PLATFORM_DIAG(("HTTPSRV_init() is Failed"));
            return WM_FAIL;
        }
    }
    return WM_SUCCESS;
}

void http_srv_deinit(void)
{
    if (s_httpsrv_handle)
    {
        HTTPSRV_release(s_httpsrv_handle);
        s_httpsrv_handle = 0;
    }
}
