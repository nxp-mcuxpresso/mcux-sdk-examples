/*
 *  Copyright 2008-2020 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

/*! \file httpc.h
 *  \brief HTTP Client Module
 *
 * The HTTP Client module is a software component layered atop basic BSD
 * socket API and mbed TLS (a TLS stack). The HTTP Client module
 * (hereafter called \a the \a module) provides a set of API and
 * data structures to provide an easy way of communication with any web
 * server supporting the HTTP protocol.

 * Some examples of the HTTP client users (hereafter called \a the \a
 * users) of this module are: an email client, a websockets library, OTA
 * firmware upgrade module, and all the myriad web services users.
 *
 * The module implements a session based API. The users \a open a \a
 * session with the server which internally sets up a TCP stream with the
 * server. The user gets a handle to this session. All the other operations
 * viz. sending HTTP headers, sending HTTP content, reading HTTP content,
 * etc. are then performed on the same session through the session
 * handle. The server also supports multiple HTTP transactions on the same
 * server (which uses the keep-alive mechanism through persistent
 * connections). A \a transaction is defined as a request and a response
 * activity. Pipelined transactions are also supported. The module also
 * supports chunked encoding transparently to the user.
 *
 * An important feature supported by the module is HTTPS connections. This
 * allows a user to perform his HTTP transactions securely. To help support
 * this the module works with an external TLS library. During session open,
 * the user \a optionally needs to supply a root certificate to verify the
 * server certificate. The user can also provide the module with its own
 * client certificate if the server is expected to do client
 * verification. Apart from session the initialization API, the remaining
 * API's handle secure and non-secure connections transparently to the user.
 *
 * Lastly, the module also provides some API's to deal with HTTP session
 * directly. i.e. it provides users access to the socket directly.
 *
 * \section httpc_usage Usage
 *
 * A typical HTTP Client usage scenario is as follows:
 *
 * -# Initiate a connection with the remote Web Server using a call to
 *    http_open_session().
 * -# Prepare the request with a call the API to http_prepare_req().
 * -# Send one or more (for pipelined mode) HTTP requests to the server
 *    using the APIs http_prepare_req() and http_send_request() pair
 *    together.
 * -# An optional call the API to API http_get_response_hdr() can be made if the
 *    application wishes to know certain fields of the HTTP response.
 * -# Then multiple calls of API http_read_content() are made for reading the
 *    response from the remote Web Server.
 * -# The connection with the remote server is terminated using the API
 *    http_close_session()
 *
 */

#ifndef _HTTPC_H_
#define _HTTPC_H_

#include "osa.h"
#include "fsl_debug_console.h"
#include <wmlog.h>
#define httpc_e(...) wmlog_e("httpc", ##__VA_ARGS__)
#define httpc_w(...) wmlog_w("httpc", ##__VA_ARGS__)

#if CONFIG_HTTPC_DEBUG
#define httpc_d(...) wmlog("httpc", ##__VA_ARGS__)
#else
#define httpc_d(...)
#endif /* ! CONFIG_HTTPC_DEBUG */

#ifdef CONFIG_ENABLE_HTTPC_MODIFY_TIME
#include <wmtime.h>
#endif /* CONFIG_ENABLE_HTTPC_MODIFY_TIME	*/
#include <wm_net.h>
#if CONFIG_ENABLE_TLS
#include <wm_mbedtls_helper_api.h>
#else
typedef void wm_mbedtls_cert_t;
#endif /* CONFIG_ENABLE_TLS */

typedef int http_session_t;

/* Request methods */
typedef enum
{
    HTTP_OPTIONS, /* request to server for communication  options */
    HTTP_GET,     /* retrieve information */
    HTTP_HEAD,    /* get meta-info */
    HTTP_POST,    /* request to accept new sub-ordinate of resource */
    HTTP_PUT,     /* modify or create new resource referred to by URI */
    HTTP_PATCH,   /* modify or create new resource referred
                   * to by URI */
    HTTP_DELETE,  /* delete the resource */
    HTTP_TRACE,   /* echo */
    HTTP_CONNECT, /* do we need this  ? */
} http_method_t;

typedef enum
{
    HTTP_VER_1_0,
    HTTP_VER_1_1,
} http_ver_t;

/**
 * The OR of zero or more flags below is passed to the API
 * http_prepare_req(). If the flag is passed the corresponding HTTP
 * header field is added to the HTTP header. The values added will be
 * default ones.
 */
typedef enum
{
    HDR_ADD_DEFAULT_USER_AGENT = 0x0001,
    /* Note: This flag is not necessary to set up persistent
     * connections in HTTP 1.1. However, if you want the server to
     * respond with persistent connection timeout values you may need
     * to add this flag. These timeout values are used to find out how
     * long a persistent connection will be kept alive by the
     * server. */
    HDR_ADD_CONN_KEEP_ALIVE   = 0x0002,
    HDR_ADD_CONN_CLOSE        = 0x0004,
    HDR_ADD_TYPE_CHUNKED      = 0x0008,
    HDR_ADD_CONTENT_TYPE_JSON = 0x0010,
} http_hdr_field_sel_t;

/*
 * We are not sure of the exact header size. Instead of reading one octet
 * at a time we will have prefetch buffer. This buffer will be used to read
 * the either header and then this header is parsed.
 * This prefetch buffer is also used as a buffer to store the request
 * header, before sending an HTTP request.
 */
#define MAX_REQ_RESP_HDR_SIZE 2048

#define STANDARD_HDR_FLAGS (HDR_ADD_DEFAULT_USER_AGENT)

/** The number of times HTTP client tries to create a socket before giving
    up and returning error */
#define DEFAULT_RETRY_COUNT 3

/**
 * If the given URL has a scheme field, and it is https then the http
 * client will auto-switch to SSL mode, irrespective of the TLS_ENABLE
 * flag below.
 */
typedef enum
{
    /** Pass this flag when you want the connection to be SSL based */
    TLS_ENABLE = 0x01,
} http_open_flags_t;

/** HTTP session configuration. Passed to http_open_session() */
typedef struct
{
    /** This is the bitwise OR of the members in \ref
        http_open_flags_t. Set to 0 if unused */
    int flags;
    /** If 0 is passed default value is used */
    int retry_cnt;

    /** Socket timeout value in milliseconds
     *  Set it to zero if timeout is not needed
     */
    unsigned int socket_timeout;

#if CONFIG_ENABLE_HTTPC_SECURE
    /** If TLS connection with specific certificates is to be done,
        this member should be populated using
        wm_mbedtls_ssl_config_new(). If TLS connection without specific
        certificates is required then pass NULL here. If NULL is passed
        here or NULL is passed for entire httpc_cfg_t, HTTP client
        would automatically manage and internally use a temporary
        structure */
    mbedtls_ssl_config *ctx;

    /** Size of internal TLS receive I/O buffer. 0 to use default size
        See \ref wm_mbedtls_ssl_conf_buffsizes() */
    size_t in_buf_len;
    /** Size of internal TLS send I/O buffer. 0 to use default size
        See \ref wm_mbedtls_ssl_conf_buffsizes() */
    size_t out_buf_len;
#endif /* ENABLE_HTTPC_SECURE */
} httpc_cfg_t;

/*
 * Callback function to retrieve content.
 *
 * This function will be called repeatedly till entire content length is
 * retrieved.
 *
 * @param[in] handle Handle to http session.
 * @param[out] content Should be populated with content data of the
 * requested length.
 * @param[in] len Length of the requested content. The 'content' buffer already
 *                has space allocated for this 'len' before this callback
 *                is invoked. This function can return data length less than
 *                requested length if necessary.
 *
 * @return The number of bytes written to 'content' buffer. It should be
 * greater than zero and max equal to 'len'
 * @return Zero or negative value if error condition.
 */
typedef int (*content_fetch_cb_t)(http_session_t handle, void *content, int len);

/*
 * Note 1: A resource is a part of the string immediately after the
 * hostname[:portno] part of the URL. In the URL,
 * [http://]hostname[:portno][/path/to/resource],
 * "/path/to/resource" part is called as a resource. It starts with the
 * character '/'.
 */
/** The HTTP Client's Request structure
 *
 * \par
 * The application should fill up this structure and pass it to the
 * http_prepare_req() API while making an HTTP request.
 * For HTTP POST, the fields content and content_len of this structure
 * have to be set as well. If callback mechanism is required, set
 * content_fetch_cb and content_len fields instead.
 * More name-value fields can be added to HTTP header using http_add_header().
 */
typedef struct
{
    /** The Type of HTTP Request */
    http_method_t type;
    /** The target resource for the HTTP Request. A complete URL is also
     * accepted.
     */
    const char *resource;
    /** The HTTP Protocol Version */
    http_ver_t version;
    /** Pointer to data buffer. This field is ignored for HTTP methods
     * like GET where content body is not used.
     *
     * If your content is large and cannot fit
     * in a single buffer set this to NULL and use content fetch
     * callback mechanism by populated callback function below.
     */
    const char *content;
    /** Set this callback if your content buffer is large and you need
     * multiple data callback requests from HTTP client. You can then
     * return partial data in every request callback. The total length
     * needs to be provided in below 'content_len' member. The callback
     * will be invoked repeatedly by HTTP client till 'content_len' is
     * exhausted. Note that chunked encoding is _not_ used here.
     *
     * If you are using the callback mechanism ensure that 'content' member
     * above is NULL.
     */
    content_fetch_cb_t content_fetch_cb;
    /** The length of the data pointed to by \a content above. This is
     * don't-care if the content and content length callback is set to
     * NULL.
     */
    int content_len;
} http_req_t;

/**
 * Structure used to give back http header response fields to the caller.
 */
typedef struct
{
    /** The value of the protocol field in the first line of the HTTP
        header response. e.g. "HTTP". */
    const char *protocol;
    /** HTTP version */
    http_ver_t version;
    /** The status code returned as a part of the first line of the
        HTTP response e.g. 200 if success
    */
    int status_code;
    /** The ASCII string present in the first line of HTTP response. It
        is the verbose representation of status code. e.g. "OK" if
        status_code is 200
    */
    const char *reason_phrase;
    /** HTTP "Server" header field value */
    const char *server;
#ifdef CONFIG_ENABLE_HTTPC_MODIFY_TIME
    /** Last-Modified header field value in POSIX time format */
    time_t modify_time;
#endif /* CONFIG_ENABLE_HTTPC_MODIFY_TIME	*/
    /** The value of "Content-Type" header field. e.g. "text/html" */
    const char *content_type;
    /** The value of "Content-Encoding" header field e.g. "gzip" */
    const char *content_encoding;
    /** If "Keep-Alive" field is present or if the value of
        "Connection" field is "Keep-Alive" then this member is set to
        'true'. It is set to 'false' in other cases
    */
    bool keep_alive_ack;
    /** If "Keep-Alive" field is present in the response, this member
        contains the value of the "timeout" sub-field of this
        field. This is the time the server will allow an idle
        connection to remain open before it is closed.
    */
    int keep_alive_timeout;
    /** If "Keep-Alive" field is present in the response, this member
        contains the value of the "max" sub-field of this field. The
        max parameter indicates the maximum number of requests that a
        client will make, or that a server will allow to be made on the
        persistent connection.
    */
    int keep_alive_max;
    /** This will be 'true' if "Transfer-Encoding" field is set to
        "chunked". Note that this is only for information and the API
        http_read_content() transparently handles chunked reads.
    */
    bool chunked;
    /** Set to 'true' when "Content-Length" header field is present in
        server response.
     */
    bool content_length_field_present;
    /** Value of the "Content-Length" field. If "Transfer-Encoding" is
        set to chunked then this value will be zero. If
        'content_length_present' is 'false' ignore this field value.
    */
    uint32_t content_length;
} http_resp_t;

typedef struct
{
    char *name;
    char *value;
} http_header_pair_t;

typedef struct
{
    const char *scheme;
    const char *hostname;
    unsigned portno;
    const char *resource;
} parsed_url_t;

/** HTTPC Error Codes **/

enum wm_httpc_errno
{
    WM_E_HTTPC_ERRNO_START = MOD_ERROR_START(MOD_HTTPC),
    /** TCP connection failed (maybe due to unreachable server) */
    WM_E_HTTPC_TCP_CONNECT_FAIL,
    /** HTTP File not found */
    WM_E_HTTPC_FILE_NOT_FOUND,
    /** HTTP Bad Request */
    WM_E_HTTPC_BAD_REQUEST,
    /** TLS not enabled */
    WM_E_HTTPC_TLS_NOT_ENABLED,
    /** Socket error. Note to applications: Please check 'errno' for
        more information */
    WM_E_HTTPC_SOCKET_ERROR,
    /** Peer has performed orderly shutdown */
    WM_E_HTTPC_SOCKET_SHUTDOWN,
};

/* Status codes */
#define HTTP_RESP_INFORMATIONAL(x) (x >= 100 && < 200)
#define HTTP_RESP_SUCCESS(x)       (x >= 200 && x < 300)
#define HTTP_RESP_REDIR(x)         (x >= 300 && x < 400)
#define HTTP_RESP_CLIENT_ERR(x)    (x >= 400 && x < 500)
#define HTTP_RESP_SERVER_ERR(x)    (x >= 500 && x < 600)

/*
 * These macros are not of any use to the HTTP client itself. They are used
 * by the users of the HTTP client. This list may be extended if required
 */
#define HTTP_OK           200
#define HTTP_CREATED      201
#define HTTP_ACCEPTED     202
#define HTTP_NO_CONTENT   204
#define HTTP_FOUND        302
#define HTTP_NOT_MODIFIED 304

#define HTTP_BAD_REQUEST 400
#define HTTP_NOT_AUTH    401
#define HTTP_FORBIDDEN   403
#define HTTP_NOT_FOUND   404

/* Address types for IPv4 and IPv6 */
#define ADDR_TYPE_IPV4 1
#define ADDR_TYPE_IPV6 2

/**
 * Start an HTTP session with proxy server.
 *
 * @par
 * This API starts a new HTTP session with proxy server. It will create a http
 * link with proxy server's address and port number, and call
 * http_open_session() to create a socket with the proxy server.
 *
 * @param[in,out] handle Pointer to a handle for the session. Will be
 * assigned by the callee.
 * @param[in] proxy_addr Proxy server's address.
 * @param[in] proxy_port Proxy server's port id.
 * @param[in] httpc_cfg HTTP configuration. Pass NULL to allow HTTPC client
 * to use default parameters.
 *
 * @note Connection to HTTPS proxy server is not supported as of now.
 *
 * @return WM_SUCCESS on success
 * @return -WM_E_INVAL if any of the arguments were invalid
 * @return -WM_E_NOMEM if memory allocation fails
 * @return -WM_FAIL if the API was unable to initiate an HTTP session with
 * the proxy server.
 */
int http_open_proxy_session(http_session_t *handle,
                            const char *proxy_addr,
                            const char *proxy_port,
                            const httpc_cfg_t *httpc_cfg);

/**
 * Start an HTTP session.
 *
 * @par
 * This API starts a new HTTP session. It will create a socket and
 * then connect to the server given as a parameter.
 *
 * @param[in,out] handle Pointer to a handle for the session. Will be
 * assigned by the callee.
 * @param[in] hostname Host name or IP address.  The hostname should be in
 * the format [http://]hostname[:portno][/path/to/resource]. The fields
 * between the square brackets above are optional. The part specifying
 * the path to resource will be ignored as it is of no use during session
 * setup.
 * @param[in] httpc_cfg HTTP configuration. Pass NULL to allow HTTPC client
 * to use default parameters.
 *
 * @return WM_SUCCESS on success
 * @return -WM_E_INVAL if any of the arguments were invalid
 * @return -WM_FAIL if the API was unable to initiate an HTTP session with
 * the server.
 */
int http_open_session(http_session_t *handle, const char *hostname, const httpc_cfg_t *httpc_cfg);

/**
 * Prepare the HTTP header for sending to the server.
 *
 * This API will prepare an HTTP request for sending it out through
 * http_send_request(). Calling this API is mandatory before calling
 * http_send_request().
 *
 * @pre http_open_session()
 *
 * @param[in] handle Handle returned from the call to http_open_session()
 * @param[in] req Allocated and initialized \ref http_req_t structure.
 * @param[in] field_flags Bitwise OR of flags of enum \ref
 * http_hdr_field_sel_t as required by the caller.
 *
 * @note The request structure \ref http_req_t needs to be filled before
 * invoking this API.
 * @note The field_flags parameter is a way to request the http client to
 * use default field values for standard fields. If the corresponding bits
 * are not set then the respective fields are not added. The user can add
 * any custom fields including the standard ones using the API
 * http_add_header()
 *
 * @return Standard wmsdk return codes.
 */
int http_prepare_req(http_session_t handle, const http_req_t *req, http_hdr_field_sel_t field_flags);

/**
 * Add custom http headers to the partially generated header.
 *
 * This API allows the caller to add custom headers to the partial
 * http header generated by earlier call to http_prepare_req()
 *
 * @note Calling this API is optional and needs to be called only if custom
 * headers (or standard headers with non-default values) are to be added.
 *
 * @pre http_prepare_req()
 *
 * @param[in] handle Handle returned from the call to http_open_session()
 * @param[in] req Structure of the type \ref http_req_t passed earlier to
 * http_prepare_req()
 * @param[in] name The name of the field. e.g User-Agent or
 * If-Modified-Since
 * @param[in] value Value associated with the name given above.
 *
 * @return Standard wmsdk value.
 */
int http_add_header(http_session_t handle, const http_req_t *req, const char *name, const char *value);

/**
 * Perform an HTTP request.
 *
 * This API sends the prepared request header and content if
 * applicable.
 *
 * @note If this API fails and the caller decides to abandon this
 * session altogether it needs to call the API  http_close_session()
 * explicitly. Failure to do this will cause undefined behaviour.
 *
 * @pre http_prepare_req() is mandatory. http_add_header() is optional.
 *
 * @param[in] handle Handle returned from the call to http_open_session()
 * @param[in] req The \ref http_req_t structure filled up with appropriate
 * parameters.
 *
 * @return WM_SUCCESS on success
 * @return -WM_E_IO if failed to send data to network
 * @return -WM_E_INVAL for an invalid arguments.
 */
int http_send_request(http_session_t handle, const http_req_t *req);

/**
 * Get the HTTP response header
 *
 * This API receives the response header from the server and parses
 * it. It then populates the \ref http_resp_t structure and returns the
 * pointer to this structure. The structure \ref http_resp_t is allocated
 * and managed by the callee and callers need not care about freeing it.
 *
 * @par
 * The caller is expected to analyse this information to find out the
 * result of the earlier request sent through API http_send_request(). If
 * the earlier request was HTTP_GET then the caller will need to call the
 * API http_read_content() to read the data sent by the server.
 *
 * @note Calling this API is \a optional. If the caller does not want
 * to read/analyse the header and knows beforehand what to expect, it can
 * directly call http_read_content(). The API http_read_content()
 * will internally read the HTTP response header. However, the API
 * http_read_content() will return an error if the HTTP status code
 * returned is not 200.  If you need to read content in cases where the
 * return status code is other than 200 you need to call
 * http_get_response_hdr() first.
 *
 * @note After invoking this API, it is mandatory to read the entire
 * content using the http_read_content() API, especially in the case
 * of repeat http transaction on the same session handle (pipelined mode).
 * @note If this API fails and the caller decides to abandon this
 * session altogether it needs to call http_close_session()
 * explicitly. Failure to do this will cause undefined behaviour.
 *
 * @pre http_send_request() is called before.
 *
 * @param[in] handle Handle returned from the call to http_new_session()
 * @param[in,out] resp Pointer to a pointer of type \ref http_resp_t .The
 * structure will be allocated by the callee. Note that the caller does
 * \b not need to free the structure allocated and returned from this
 * API. The allocation and free is done by the callee automatically.
 *
 * @return WM_SUCCESS on success
 * @return -WM_FAIL on error
 */
int http_get_response_hdr(http_session_t handle, http_resp_t **resp);

/**
 * Get a particular HTTP response header value.
 *
 * This API parses the response header received from the server and
 * returns the value corresponding to a given name. This can help the
 * caller get value of a particular field not parsed automatically in
 * \ref http_resp_t
 *
 * @par
 * The caller is not expected to call http_get_response_hdr() before
 * this. However, calling http_get_response_hdr() is recommended so that
 * the user gets to know the exact status of the HTTP operation before he
 * tries to extract any field not present in structure \ref http_resp_t
 *
 * @note Calling this API is \b optional. If the caller does not want to
 * read/analyse the header and knows beforehand what to expect, it can
 * directly call http_read_content(). The API http_read_content()
 * will internally read the HTTP response header. However, the API
 * http_read_content() will return an error if the HTTP status code
 * returned is not 200. If you need to read content in cases where the
 * return status code is other than 200 you need to call
 * http_get_response_hdr() or the current API first.
 *
 * @note If this API fails and the caller decides to abandon this
 * session altogether it needs to call http_close_session()
 * explicitly. Failure to do this will cause undefined behaviour.
 *
 * @pre \a http_send_request is called before this call. Calling
 * http_get_response_hdr() is \b not mandatory but recommended.
 *
 * @param[in] handle Handle returned from the call to \a http_new_session
 * @param[in] header_name The name of the field whose value you wish to
 * know
 * @param[in,out] value A pointer which will point to the value
 * corresponding to the name passed as second parameter.
 *
 * @return WM_SUCCESS on success
 * @return -WM_FAIL on error
 */

int http_get_response_hdr_value(http_session_t handle, const char *header_name, char **value);
/**
 * Get requested number of HTTP response header name-value pairs.
 *
 * @par
 * This API parses the response header received from the server and
 * returns requested number of HTTP response header name-value pairs.
 *
 * @par
 * The caller is not expected to call API http_get_response_hdr() before
 * this. However, calling \ref http_get_response_hdr is recommended so that
 * the user gets to know the exact status of the HTTP operation before he
 * tries to extract any field not present in structure \ref http_resp_t
 *
 * @note Calling this API is \b optional. If the caller does not want to
 * read/analyse the header and knows beforehand what to expect, s/he can
 * directly call http_read_content(). The API http_read_content()
 * will internally read the HTTP response header. However, the API
 * http_read_content() will return an error if the HTTP status code
 * returned is not 200. If you need to read content in cases where the
 * return status code is other than 200 you need to call
 * http_get_response_hdr() or the current API first.
 *
 * @note If this API fails and the caller decides to abandon this
 * session altogether it needs to call http_close_session()
 * explicitly. Failure to do this will cause undefined behaviour.
 *
 * @pre http_send_request() is called before this call. Calling
 * http_get_response_hdr() is \b not mandatory but recommended.
 *
 * @param[in] handle Handle returned from the call to http_new_session()
 * @param[in,out] arr A pointer that points to an array of the
 * structure to hold the response header name-value pairs.
 * This array needs to be pre-allocated and its pointer is to be passed,
 * this pointer shall point to the array of requested number of
 * response header name-value pairs.
 * @param[in,out] count An integer pointer that points to the number of
 * response header name-value pairs that the user wants.
 * This has to be passed by the user and it is later updated to the
 * actual number of response header name-value pairs that were filled
 * in the array mentioned above.
 *
 * @return WM_SUCCESS on success
 * @return -WM_FAIL on error
 */

int http_get_response_hdr_all(http_session_t handle, http_header_pair_t *arr, int *count);

/**
 * Read data content from the stream.
 *
 * This API will read the data sent by the server and pass it back to the
 * caller.
 *
 * @par
 * The data transfer from the server to the client may be chunked. This
 * fact will be reported in the response structure \ref http_resp_t returned
 * in a previous call to http_get_response_hdr(). Notwithstanding this
 * information, chunked data handling is transparent to the
 * caller. Thus, irrespective of the transfer type, this API should be
 * called repeatedly till the return value of the API is zero.
 *
 * @note For persistent HTTP connections it is mandatory to read the entire
 * content before you begin the next HTTP transaction.
 *
 * @note If this API fails and the caller decides to abandon this
 * session altogether it needs to call http_close_session()
 * explicitly. Failure to do this will cause undefined behaviour.
 *
 * @param[in] handle Handle returned from the call to http_new_session()
 * @param[in, out] buf Caller allocated buffer of size max_len
 * @param[in] max_len Size of the buffer. The data read will be less than
 * or equal
 * to this size, depending on the size of the resource.
 *
 * @return On success, the number of bytes read is returned. Return value
 * of zero is \a not an error case  and should be treated as a signal that
 * server is done with sending the data.
 * @return -WM_FAIL if this API is called again after the previous call
 * retuned 0 or for any other error.
 */

int http_read_content(http_session_t handle, void *buf, uint32_t max_len);

/**
 * Parse the given string into URL subcomponents.
 *
 * The hostname string syntax is:
 * [scheme://]hostname[:port][/path_to_resource?query_string#fragment_id]
 * The fields between the square brackets above are optional.
 *
 * @note <b>Buffer management</b>: To free the caller of the task of
 * allocating multiple buffers of arbitrary size to store each of the
 * subcomponents we use the following strategy. This API takes a temporary
 * buffer, allocated (static or dynamic) by the caller to store the
 * subcomponents. The caller needs to allocate this buffer with size equal
 * to (strlen(URL) + 10). When this API returns, the pointers in the
 * parsed_url structure will point to appropriate address in this
 * buffer. The caller may free this buffer after usage of the
 * sub-components is complete. Freeing before that will cause undefined
 * behavior.
 *
 * @param[in] URL Pointer to the string containing the URL. This API
 * will not modify this argument.
 * @param[in] tmp_buf A buffer where the strings will be stored after
 * parsing. Needs to be allocated by the caller and should be of size
 * atleast 10 greater than size of the URL string passed as first
 * argument.
 * @param[in] tmp_buf_len Size of the caller allocated, temporary buffer
 * sent as second argument.
 * @param[in, out] parsed_url Pointer to an allocated structure. The member
 * pointers will point to the appropriate address in the temporary buffer
 * holding the NULL terminated string corresponding to the subcomponent.
 *
 * @return Standard WMSDK return codes.
 */
int http_parse_URL(const char *URL, char *tmp_buf, int tmp_buf_len, parsed_url_t *parsed_url);
/**
 * HTTP lowlevel read
 *
 * This API is present to serve clients which want to handle the
 * HTTP stream directly. This is useful for handling websockets for e.g.
 *
 * @warning This is not for general users of HTTP client API. Normal HTTP
 * transactions do not need this API.
 *
 * @note Once this API used no other API from the HTTP API set, except
 * http_close_session(), can be used. This is because once this API is used
 * the HTTP client loses track of the stream and only the caller using the
 * API can know the state of the stream. The only advantage of using this
 * API over normal socket read/write is that this API transparently takes
 * care of TLS and non-TLS interface to read/write.
 *
 * @param[in] handle Handle returned from the call to http_new_session()
 * @param[in, out] buf Pointer to an allocated buffer of size equal to or
 * more than the value of the third parameter maxlen
 * @param[in] maxlen The maximum number of bytes to be read from the
 * network. Note that the actual read bytes can be less than this.
 *
 * @return Number of bytes read. -WM_E_INVAL is returned in case of invalid
 * parameters. Standard libc error codes are returned in case when there
 * other problems.
 */

int http_lowlevel_read(http_session_t handle, void *buf, unsigned maxlen);

/**
 * HTTP lowlevel write
 *
 * This API is present to serve clients which want to handle the
 * HTTP stream directly. This is useful for handling websockets for e.g.
 *
 * @warning This is not for general users of HTTP client API. Normal HTTP
 * transactions do not need this API.
 *
 * @note Once this API used no other API from the HTTP API set, except \a
 * http_close_session, can be used. This is because once this API is used
 * the HTTP client loses track of the stream and only the caller using the
 * API can know the state of the stream. The only advantage of using this
 * API over normal socket read/write is that this API transparently takes
 * care of TLS and non-TLS interface to read/write.
 *
 * @param[in] handle Handle returned from the call to http_new_session()
 * @param[in, out] buf Pointer to a buffer which has the data to be written
 * out.
 * @param[in] len The number of bytes to be written to the
 * network.
 *
 * @return Number of bytes written. -WM_E_INVAL is returned in case of
 * invalid parameters. Standard libc error codes are returned in case when
 * there other problems.
 */

int http_lowlevel_write(http_session_t handle, const void *buf, unsigned len);

/**
 * HTTP write chunked
 *
 * This API is used for chunked transfer encoding
 *
 * @note To end the chunked data transfer, this API should be called with
 * len=0, buf may be NULL
 *
 * @param[in] handle Handle retrieved from the call to http_open_session()
 * @param[in] data Pointer to the buffer to be written.
 * @param[in] len The number of bytes to be written.
 *
 * @return WM_SUCCESS is returned on success.
 * @return -WM_E_INVAL is returned in case of invalid parameters.
 * @return -WM_FAIL is returned if session is invalid.
 * @return Standard libc error codes are returned when there are other
 * problems.
 */

int httpc_write_chunked(http_session_t handle, const char *data, int len);

/**
 * Close the proxy session.
 *
 * This API will close the proxy session represented by the given
 * handle. The socket associated with this session will be closed and thus
 * the TCP connection with the proxy server will be terminated. No requests on
 * this same session will succeed after this API is called.
 *
 * @note <b>The response structure \ref http_resp_t returned earlier from
 *  http_get_response_hdr() will be invalid after this call.</b>
 *
 * @param[in] handle Handle returned from the previous call to
 * http_new_session(). The handle will be set to NULL by the callee thus
 * effectively blocking further requests on the same session handle.
 *
 */
void http_close_proxy_session(http_session_t *handle);

/**
 * Close the session.
 *
 * This API will close the session represented by the given
 * handle. The socket associated with this session will be closed and thus
 * the TCP connection with the server will be terminated. No requests on
 * this same session will succeed after this API is called.
 *
 * @note <b>The response structure \ref http_resp_t returned earlier from
 *  http_get_response_hdr() will be invalid after this call.</b>
 *
 * @param[in] handle Handle returned from the previous call to
 * http_new_session(). The handle will be set to NULL by the callee thus
 * effectively blocking further requests on the same session handle.
 *
 */
void http_close_session(http_session_t *handle);

/**
 * Set socket options for the socket associated with the session
 *
 * @param[in] handle Handle returned from the previous call to
 * http_open_session().
 * @param[in] level Refer the man page for setsockopt.
 * @param[in] optname Refer the man page for setsockopt.
 * @param[in] optval Refer the man page for setsockopt.
 * @param[in] optlen Refer the man page for setsockopt.
 *
 * @note For the description and usage of the parameters,
 * please refer to the man page for setsockopt. Their semantics are exactly
 * same as mentioned in the man page.
 *
 * @return WM_SUCCESS is returned on success.
 * @return -WM_E_INVAL is returned in case of invalid parameters.
 * @return -WM_FAIL is returned if session is invalid or if setsockopt
 * fails.
 */
int http_setsockopt(http_session_t handle, int level, int optname, const void *optval, socklen_t optlen);

/**
 * Get socket options for the socket associated with the session
 *
 * @param[in] handle Handle returned from the previous call to
 * http_open_session().
 * @param[in] level Refer the man page for setsockopt.
 * @param[in] optname Refer the man page for setsockopt.
 * @param[in] optval Refer the man page for setsockopt.
 * @param[in] optlen Refer the man page for setsockopt.
 *
 * @note For the description and usage of the parameters,
 * please refer to the man page for setsockopt. Their semantics are exactly
 * same as mentioned in the man page.
 *
 * @return WM_SUCCESS is returned on success.
 * @return -WM_E_INVAL is returned in case of invalid parameters.
 * @return -WM_FAIL is returned if session is invalid or if setsockopt
 * fails.
 */
int http_getsockopt(http_session_t handle, int level, int optname, void *optval, socklen_t *optlen);

/**
 * Get the socket fd from the http session handle
 *
 * @param[in] handle Handle returned from the previous call to
 * http_open_session().
 *
 * @return socket fd is returned on success.
 * @return -WM_E_INVAL is returned in case of invalid parameters.
 */
int http_get_sockfd_from_handle(http_session_t handle);

/** Validate URL for outgoing HTTP request
 *
 * This function is used to check if the given URL is
 * valid or not
 *
 * @param[in] url_str URL to be validated
 *
 * @return WM_SUCCESS if URL is valid
 * @return error if URL is invalid
 */
int httpc_validate_url(const char *url_str);

/** Perform HTTP GET operation
 *
 * This is a helper function which calls different functions of httpc
 * module - httpc_validate_url(), http_open_session(),
 * http_prepare_req(), http_send_request(), http_get_response_hdr()
 * in sequence to perform HTTP GET operation.
 * STANDARD_HDR_FLAGS are passed to http_prepare_req() internally.
 *
 *
 * This function reads only the HTTP response header. The caller of
 * this function can use http_read_content() function to read the
 * HTTP response. It is responsibility of the caller to close the HTTP
 * session after the required work is done. However, the
 * http_close_session() function needs to be called only if httpc_get()
 * returns WM_SUCCESS.
 *
 * @param[in] url_str Target URL for HTTP request
 * @param[out] handle Pointer to a handle for the session. The handle
 * will be populated by this function, on success
 * @param[out] http_resp Structure used to give back header fields of
 *  http response to the caller.
 * @param[in] httpc_cfg  HTTP configuration. Pass NULL to allow HTTPC
 * client to use default parameters.
 *
 * @return WM_SUCCESS is returned on success
 * @return other if there is any error returned by httpc_validate_url(),
 *  http_open_session(), or http_prepare_req(), or http_send_request(),
 *  or http_get_response_hdr() functions which are called by this function
 */
int httpc_get(const char *url_str, http_session_t *handle, http_resp_t **http_resp, const httpc_cfg_t *httpc_cfg);

#if CONFIG_ENABLE_HTTPC_SECURE
/**
 * Get TLS context for the given session.
 *
 * @param[in] handle Handle returned from the previous call to
 * http_open_session().
 *
 * @return Returns the same mbedtls_ssl_config parameter from httpc_cfg passed in call
 * to http_open_session().
 */
mbedtls_ssl_config *http_get_tls_context_from_handle(http_session_t handle);
#endif /* ENABLE_HTTPC_SECURE */
#define wmprintf(...) PRINTF(__VA_ARGS__)

int httpc_test_cli_init(void);

#endif /* _HTTPC_H_ */
