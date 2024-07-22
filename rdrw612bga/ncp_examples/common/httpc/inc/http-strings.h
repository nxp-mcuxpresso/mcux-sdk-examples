/*
 * Copyright (c) 2001-2006, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * Adapted by C Michael Sundius, Solico Group LLC for Cozybit Inc
 *
 * Description
 *  SSI processor for simple web server
 *  syntax of a script is
 *   %! <func_name> [optional_arg]
 *   %! : <include html filename>
 *   <standard html>
 *
 */

/*
 *  Copyright 2008-2020 NXP
 */

extern const char http_http[];
extern const char http_200[];
extern const char http_301[];
extern const char http_302[];
extern const char http_get[];
extern const char http_post[];
extern const char http_put[];
extern const char http_delete[];
extern const char http_head[];
extern const char http_10[];
extern const char http_11[];
extern const char http_last_chunk[];
extern const char http_content_type[];
extern const char http_content_len[];
extern const char http_user_agent[];
extern const char http_if_none_match[];
extern const char http_if_modified_since[];
extern const char http_encoding[];
extern const char http_texthtml[];
extern const char http_location[];
extern const char http_host[];
extern const char http_crnl[];
extern const char http_index_html[];
extern const char http_404_html[];
extern const char http_referer[];
extern const char http_header_server[];
extern const char http_header_conn_close[];
extern const char http_header_conn_keep_alive[];
extern const char http_header_type_chunked[];
extern const char http_header_cache_ctrl[];
extern const char http_header_cache_ctrl_no_chk[];
extern const char http_header_pragma_no_cache[];
extern const char http_header_200_keepalive[];
extern const char http_header_200[];
extern const char http_header_304_prologue[];
extern const char http_header_404[];
extern const char http_header_400[];
extern const char http_header_500[];
extern const char http_header_505[];
extern const char http_content_type_plain[];
extern const char http_content_type_xml[];
extern const char http_content_type_html[];
extern const char http_content_type_html_nocache[];
extern const char http_content_type_css[];
extern const char http_content_type_text[];
extern const char http_content_type_png[];
extern const char http_content_type_gif[];
extern const char http_content_type_jpg[];
extern const char http_content_type_js[];
extern const char http_content_type_binary[];
extern const char http_content_type_json[];
extern const char http_content_type_json_nocache[];
extern const char http_content_type_xml_nocache[];
extern const char http_content_type_form[];
extern const char http_content_type_text_cache_manifest[];
extern const char http_content_encoding_gz[];
extern const char http_html[];
extern const char http_shtml[];
extern const char http_htm[];
extern const char http_css[];
extern const char http_png[];
extern const char http_gif[];
extern const char http_jpg[];
extern const char http_text[];
extern const char http_txt[];
extern const char http_xml[];
extern const char http_js[];
extern const char http_gz[];
extern const char http_manifest[];
extern const char http_cache_control[];
