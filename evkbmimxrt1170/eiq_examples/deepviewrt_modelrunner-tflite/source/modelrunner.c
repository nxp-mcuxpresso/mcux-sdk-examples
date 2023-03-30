#include "modelrunner.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "deepview_rt.h"

#define STATIC_JSON 1
#define HTTPSRV_CFG_RECEIVE_TIMEOUT (50)
#define HTTP_BUFSIZE 2 * 1024 * 1024
__attribute__ ((section(".bss" ".$BOARD_SDRAM" ))) static char http_buffer[HTTP_BUFSIZE];

#if STATIC_JSON
#define JSON_BUFSIZE 1 * 1024 * 1024
__attribute__ ((section(".bss" ".$BOARD_SDRAM" ))) static char nn_json_buffer[JSON_BUFSIZE];
#endif


unsigned int model_validated = 0;

void nn_printf_uart(char * s)
{
    char str[128] = {'0'};
    memcpy(str, s, 127);
    PRINTF("%s \r\n", str);
}

//provide only for keil mdk
#if defined(__ARMCC_VERSION)
__attribute__((weak))
size_t __aeabi_read_tp(void)
{
    return 0;
}
#endif

extern u32_t sys_now(void);
int64_t os_clock_now()
{
    int64_t us = ((SystemCoreClock / 1000) - SysTick->VAL) / (SystemCoreClock / 1000000);
    us += (int64_t)(sys_now()*1e3);
    return us;
}

#if defined(CPU_MIMXRT1176DVMAA_cm7)
int gethostname(char *name, size_t len)
{
    char host[8]="1170EVK";

    memcpy(name, host, len < 7?len:7);

    return 0;
}
#elif defined(CPU_MIMXRT1064DVL6A)
int gethostname(char *name, size_t len)
{
    char host[8]="1064EVK";

    memcpy(name, host, len < 7?len:7);

    return 0;
}
#else
int gethostname(char *name, size_t len)
{
    char host[6]="IMXRT";

    memcpy(name, host, len < 5?len:5);

    return 0;
}
#endif

//Provide weak defintions for strdup and strnlen
#if (defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__ICCARM__))
/* Copies a string to a new allocated string  */
__attribute__((weak))
char* strdup (const char *s)
{
    size_t len = strlen (s) + 1;
    void *new = malloc (len);
    if (new == NULL)
        return NULL;
    return (char *) memcpy (new, s, len);
}

/* Returns the amount of characters in a string without terminating zero.  */
__attribute__((weak))
size_t strnlen (const char *s, size_t maxlen)
{
    const char *p = s;
    /* We don't check here for NULL pointers.  */
    for (;*p != 0 && maxlen > 0; p++, maxlen--)
        ;
    return (size_t) (p - s);
}

#ifdef DEBUG
__attribute__((weak))
void abort(void) {
    for (;;);
}

__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line) {
    char str[12], *p;

    fputs("*** assertion failed: ", stderr);
    fputs(expr, stderr);
    fputs(", file ", stderr);
    fputs(file, stderr);
    fputs(", line ", stderr);

    p = str + sizeof(str);
    *--p = '\0';
    *--p = '\n';
    while (line > 0) {
        *--p = '0' + (line % 10);
        line /= 10;
    }
    fputs(p, stderr);

    abort();
}
#endif

#endif

void modelrunner_task(void* arg)
{

    NNServer server;
    memset(&server,0,sizeof(NNServer));

#if STATIC_JSON
    server.json_buffer = nn_json_buffer;
    server.json_size = JSON_BUFSIZE;
#endif


    int                 err;
    int                 listener;
    struct sockaddr_in  addr;
    void*               buf    = NULL;
    size_t              bufsiz = HTTP_BUFSIZE;

    struct http_route routes[] = {{"/", "GET", 0, (void*)(&server), v1_handler},
                                  {"/v1", "GET", 0, (void*)(&server), v1_handler},
                                  {"/v1/model", "GET", 0, (void*)(&server), model_handler},
                                  {"/v1", "PUT", 0, (void*)(&server), v1_handler_put},
                                  {"/v1", "POST", 0, (void*)(&server), v1_handler_post},
                                  {NULL, NULL, 0, NULL, NULL}};

    for (struct http_route* route = routes; route->path != NULL; ++route) {
        route->path_len = strlen(route->path);
    }
    //Static http buffer allocation - 1.5MB
    buf = (void *)http_buffer;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        fprintf(stderr,
                "failed to create listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(MODELRUNNER_DEFAULT_PORT);

    err = bind(listener, (struct sockaddr*) &addr, sizeof(addr));
    if (err) {
        fprintf(stderr,
                "failed to bind listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    err = listen(listener, 16);
    if (err) {
        fprintf(stderr,
                "failed to listen on listener socket: %s\r\n",
                strerror(errno));
        goto finish;
    }

    PRINTF("Initialized TFLiteMicro modelrunner server at port %d\r\n",MODELRUNNER_DEFAULT_PORT);

    while (1) {
        size_t offset = 0;
        int    sock   = accept(listener, NULL, NULL);
        if (sock == -1) {
            PRINTF("Invalid socket: %s\r\n",strerror(errno));
            continue;
        }

        err = http_handler(sock, buf, bufsiz, &offset, routes);
        if (err) {
            closesocket(sock);
            continue;
        }
        //closesocket(sock);
    }
finish:
    vTaskDelete(NULL);
}
