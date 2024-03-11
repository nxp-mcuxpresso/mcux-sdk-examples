/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 *  Copyright 2008-2020 NXP
 */

/*! \file websockets.h
 * \brief Websocket Implementation in WMSDK using the WSLAY websocket library.
 *
 * This file exposes the API's to be used for websocket communication and which
 * internally call the WSLAY library functions
 *
 */

#ifndef _WEBSOCKETS_H_
#define _WEBSOCKETS_H_

#include <httpc.h>

/** Websocket events **/
typedef enum
{
    /** Websocket send data */
    WS_SEND_DATA = 1,
    /** Websocket receive data */
    WS_RECV_DATA,
    /** Websocket stop transaction */
    WS_STOP_TRANSACTION,
} ws_event_t;

/** Websocket opcodes */
typedef enum
{
    /** Websocket continuation frame */
    WS_CONT_FRAME = 0x0,
    /** Websocket text frame */
    WS_TEXT_FRAME = 0x1,
    /* Websocket binary frame */
    WS_BIN_FRAME = 0x2,
    /* Websocket connection close frame */
    WS_CONN_CLOSE_FRAME = 0x8,
    /* Websocket ping frame */
    WS_PING_FRAME = 0x9,
    /* Websocket pong frame */
    WS_PONG_FRAME = 0xa,

} ws_opcode_t;

typedef int ws_context_t;

/** Web Sockets Transfer Frame
 *
 */
typedef struct wslay_frame
{
    /** Frame Finish. Set to 1 if this is the final fragment of the frame */
    uint8_t fin;
    /** The Opcode of data of type \ref ws_opcode_t */
    uint8_t opcode;
    /** Pointer to data */
    const uint8_t *data;
    /** Length of data */
    size_t data_len;
} ws_frame_t;

/** Upgrade a socket to websocket
 *
 * This function upgrades an already created socket to websocket.
 *
 * @param[in] handle Pointer to the handle retrieved from the
 * call to http_open_session()
 * @param[in] req Pointer to the HTTP request retrieved from the
 * call to http_prepare_req()
 * @param[in] protocol Websocket subprotocol to be used.
 * @param[out] ws_ctx The Web Socket context to be used from here onwards
 * @return WM_SUCCESS is returned on success. -WM_FAIL in case of an error.
 *
 */
int ws_upgrade_socket(http_session_t *handle, http_req_t *req, const char *protocol, ws_context_t *ws_ctx);

/** Send a websocket frame
 *
 * @param[in] ws_ctx WS Context returned from a call to ws_upgrade_socket()
 * @param[in] f The frame (or fragment) to send out
 * @return Number of bytes written. -WM_FAIL in case of an error.
 *
 */
int ws_frame_send(ws_context_t *ws_ctx, ws_frame_t *f);

/** Receive a websocket frame
 * This is a blocking call.
 *
 * @param[in] ws_ctx WS Context returned from a call to ws_upgrade_socket()
 * @param[out] f The frame (or fragment) that is received. After this call,
 * f->data contains the data that is received, while f->data_len is the length
 * of this data.
 *
 * @return Number of bytes read. -WM_FAIL in case of an error
 *
 */
int ws_frame_recv(ws_context_t *ws_ctx, ws_frame_t *f);

/** Close Web Socket context
 *
 * @param[in] ws_ctx WS Context returned from a call to ws_upgrade_socket()
 */
void ws_close_ctx(ws_context_t *ws_ctx);

/** Set websocket mask callback to generate mask key
 *
 * The SDK has a default masking function that uses \ref get_random_sequence()
 * to generate mask key. This function can be used to override it. It is
 * executed while sending masked data.
 *
 * @param[in] cb A user defined callback to generate mask key. The
 * implementation of this function must write exactly len bytes of mask key to
 * buf. This function should return WM_SUCCESS if successful, -WM_FAIL in case
 * of an error. Note that user_data is not required in this case.
 *
 */
void ws_set_genmask_callback(int (*cb)(uint8_t *buf, size_t len, void *user_data));

int websocket_cli_init(void);
#endif /* _WEBSOCKETS_H_ */
