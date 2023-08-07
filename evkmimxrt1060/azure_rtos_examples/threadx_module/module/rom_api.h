#ifndef _ROM_API_H__
#define _ROM_API_H__

#include "tx_port.h"

typedef struct ROM_API_STRUCT
{
    int (*Printf)(const char *fmt_s, ...);
} S_ROM_API;

extern const S_ROM_API rom_apis;

#define ROM_API_ADDR 0x6000201C //0x6004041C
#define g_romapiTree (*(S_ROM_API **)ROM_API_ADDR)

#endif

