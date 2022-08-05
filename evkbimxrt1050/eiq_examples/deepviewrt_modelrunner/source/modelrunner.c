#include "modelrunner.h"
#include "cr_section_macros.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "deepview_rt.h"

#define STATIC_JSON 1
#define HTTPSRV_CFG_RECEIVE_TIMEOUT (50)
#define HTTP_BUFSIZE 2 * 1024 * 1024
__BSS(BOARD_SDRAM) static char http_buffer[HTTP_BUFSIZE];

#if STATIC_JSON
#define JSON_BUFSIZE 2 * 1024 * 1024
__BSS(BOARD_SDRAM) static char nn_json_buffer[JSON_BUFSIZE];
#endif

void* server = NULL;

/* DeepView mempool and cache size definitions. */
#if defined(CPU_MIMXRT1176DVMAA_cm7)
#define CACHE_SIZE 256 * 1024
#define MEMBLOB_SIZE 32 * 1024 * 1024 //RT1170-EVK has 64MB SDRAM
#define MEMPOOL_SIZE 12 * 1024 * 1024
#else
/* Store model in SDRAM, avoiding re-work board */
#define CACHE_SIZE 128 * 1024
#define MEMBLOB_SIZE 12 * 1024 * 1024
#define MEMPOOL_SIZE 4 * 1024 * 1024

extern
NNError
nn_hyperflash_write_page(uint32_t dstAddr, const uint8_t *src, uint32_t actual_chunk_len);
#endif


#if defined(CPU_MIMXRT1176DVMAA_cm7)
uint8_t *cache = (uint8_t*)(0x20000000); //Cache at SRAM_DTC; use this for noinit of DTC
//__BSS(SRAM_DTC_cm7) uint8_t cache[CACHE_SIZE] __attribute__((aligned(32)));
__BSS(BOARD_SDRAM) uint8_t memblob[MEMBLOB_SIZE] __attribute__((aligned(32)));
#else
uint8_t *cache = (uint8_t*)(0x20000000); //Cache at SRAM_DTC; use this for noinit of DTC
/* Change the default behavior, store model in SDRAM, avoiding re-work board */
__BSS(BOARD_SDRAM) uint8_t memblob[MEMBLOB_SIZE] __attribute__((aligned(32)));
//uint8_t *memblob = (uint8_t*)(0x60000000); //Hyperflash storage of models
#endif

__BSS(BOARD_SDRAM) uint8_t mempool[MEMPOOL_SIZE] __attribute__((aligned(32)));

unsigned int model_validated = 0;

void nn_printf_uart(char * s)
{
	char str[128] = {'0'};
	memcpy(str, s, 127);
	PRINTF("%s\r\n", str);
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


//Function to write model block to model memory(memblob).
NNError
nn_model_memory_write(uint8_t* base,
                      size_t offset,
                      const uint8_t* src,
                      uint32_t actual_chunk_len)
{
    NNError err = NN_SUCCESS;
/* Change the default behavior, store model in SDRAM, avoiding re-work board */
#if 1
    memcpy(base + offset, src, (size_t)actual_chunk_len);
#else
    err = nn_hyperflash_write_page((uint32_t)offset, src, actual_chunk_len);
#endif

    return err;
}


void modelrunner_task(void* arg)
{
    int                 err;
    int                 listener;
    struct sockaddr_in  addr;
    void*               buf    = NULL;
    size_t              bufsiz = HTTP_BUFSIZE;


	sys_msleep(100);

	server = nn_server_init(0, MEMPOOL_SIZE, mempool, CACHE_SIZE, cache, MEMBLOB_SIZE, memblob);
	if (!server) {
		PRINTF("[ERROR] out of memory to start deepview server\r\n");
	}

#if STATIC_JSON
	((NNServer*)server)->json_buffer = nn_json_buffer;
	((NNServer*)server)->json_size = JSON_BUFSIZE;
#endif

    struct http_route routes[] = {{"/", 0, server, nn_server_http_handler},
                                  {NULL, 0, NULL, NULL}};

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

    PRINTF("Initialized DeepView modelrunner server at port %d\r\n",MODELRUNNER_DEFAULT_PORT);

    while (1) {
        size_t offset = 0;
        int    sock   = accept(listener, NULL, NULL);
        if (sock == -1) {
            PRINTF("Invalid socket: %s\r\n",strerror(errno));
            continue;
        }

        err = http_handler(sock, buf, bufsiz, &offset, routes);
        if (err) {
            PRINTF("http handler failed: %s\r\n", strerror(errno));
            closesocket(sock);
            continue;
        }
        //closesocket(sock);
    }

finish:
    vTaskDelete(NULL);

}
