/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

void cgi_urldecode(char *url);
bool cgi_get_varval(char *var_str, char *var_name, char *var_val, uint32_t length);

void format_post_data(char *string);

int http_srv_init(void);
void http_srv_deinit(void);

#endif
