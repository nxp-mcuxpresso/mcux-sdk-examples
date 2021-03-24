/* Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "app_flash.h"
#include "fsl_debug_console.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SHELL_Printf        PRINTF
#define APP_BYTES_EACH_LINE (16)

typedef enum
{
    APP_MemAccess8Bit  = 0U,
    APP_MemAccess16Bit = 1U,
    APP_MemAccess32Bit = 2U
} app_mem_access_unit_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t MemGet8Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemGet16Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemGet32Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemSet8Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemSet16Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemSet32Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCmp8Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCmp16Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCmp32Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCpy8Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCpy16Control(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t MemCpy32Control(shell_handle_t shellHandle, int32_t argc, char **argv);

static shell_status_t FlashEraseControl(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t FlashWriteControl(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
SHELL_COMMAND_DEFINE(memget8,
                     "\r\n\"memget8 arg1 arg2\":  read memory in 8bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: count           memory read count\r\n",
                     MemGet8Control,
                     2);

SHELL_COMMAND_DEFINE(memget16,
                     "\r\n\"memget16 arg1 arg2\": read memory in 16bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: count           memory read count\r\n",
                     MemGet16Control,
                     2);

SHELL_COMMAND_DEFINE(memget32,
                     "\r\n\"memget32 arg1 arg2\": read memory in 32bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: count           memory unit read count\r\n",
                     MemGet32Control,
                     2);

SHELL_COMMAND_DEFINE(memset8,
                     "\r\n\"memset8 arg1 arg2 arg3\":  write memory in 8bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: value           memory write value\r\n"
                     "    arg3: count           memory unit write count\r\n",
                     MemSet8Control,
                     3);

SHELL_COMMAND_DEFINE(memset16,
                     "\r\n\"memset16 arg1 arg2 arg3\": write memory in 16bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: value           memory write value\r\n"
                     "    arg3: count           memory unit write count\r\n",
                     MemSet16Control,
                     3);

SHELL_COMMAND_DEFINE(memset32,
                     "\r\n\"memset32 arg1 arg2 arg3\": write memory in 32bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            memory address\r\n"
                     "    arg2: value           memory write value\r\n"
                     "    arg3: count           memory unit write count\r\n",
                     MemSet32Control,
                     3);

SHELL_COMMAND_DEFINE(memcmp8,
                     "\r\n\"memcmp8 arg1 arg2 arg3\":  compare memory in 8bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr1           memory address1\r\n"
                     "    arg2: addr2           memory address2\r\n"
                     "    arg3: count           memory unit compare count\r\n",
                     MemCmp8Control,
                     3);

SHELL_COMMAND_DEFINE(memcmp16,
                     "\r\n\"memcmp16 arg1 arg2 arg3\": compare memory in 16bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr1           memory address1\r\n"
                     "    arg2: addr2           memory address2\r\n"
                     "    arg3: count           memory unit compare count\r\n",
                     MemCmp16Control,
                     3);

SHELL_COMMAND_DEFINE(memcmp32,
                     "\r\n\"memcmp32 arg1 arg2 arg3\": compare memory in 32bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: addr1           memory address1\r\n"
                     "    arg2: addr2           memory address2\r\n"
                     "    arg3: count           memory unit compare count\r\n",
                     MemCmp32Control,
                     3);

SHELL_COMMAND_DEFINE(memcpy8,
                     "\r\n\"memcpy8 arg1 arg2 arg3\":  copy memory in 8bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: dst             destination memory address\r\n"
                     "    arg2: src             source memory address\r\n"
                     "    arg3: count           memory unit copy count\r\n",
                     MemCpy8Control,
                     3);

SHELL_COMMAND_DEFINE(memcpy16,
                     "\r\n\"memcpy16 arg1 arg2 arg3\": copy memory in 16bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: dst             destination memory address\r\n"
                     "    arg2: src             source memory address\r\n"
                     "    arg3: count           memory unit copy count\r\n",
                     MemCpy16Control,
                     3);

SHELL_COMMAND_DEFINE(memcpy32,
                     "\r\n\"memcpy32 arg1 arg2 arg3\": copy memory in 32bit unit\r\n"
                     " Usage:\r\n"
                     "    arg1: dst             destination memory address\r\n"
                     "    arg2: src             source memory address\r\n"
                     "    arg3: count           memory unit copy count\r\n",
                     MemCpy32Control,
                     3);

SHELL_COMMAND_DEFINE(flasherase,
                     "\r\n\"flasherase arg1 arg2\": erase QSPI flash\r\n"
                     " Usage:\r\n"
                     "    arg1: offset          flash memory offset in bytes\r\n"
                     "    arg2: bytes           flash memory size in bytes\r\n",
                     FlashEraseControl,
                     2);

SHELL_COMMAND_DEFINE(flashwrite,
                     "\r\n\"flashwrite arg1 arg2 arg3\": write QSPI flash\r\n"
                     " Usage:\r\n"
                     "    arg1: addr            source memory address\r\n"
                     "    arg2: offset          flash memory offset in bytes\r\n"
                     "    arg3: bytes           flash memory size in bytes\r\n",
                     FlashWriteControl,
                     3);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

static shell_status_t MemGetControl(shell_handle_t shellHandle, int32_t argc, char **argv, app_mem_access_unit_t unit)
{
    uint32_t addr  = strtoul(argv[1], NULL, 0);
    uint32_t count = strtoul(argv[2], NULL, 0);
    uint8_t *p8;
    uint16_t *p16;
    uint32_t *p32;
    uint32_t pos;

    switch (unit)
    {
        case APP_MemAccess8Bit:
            p8 = (uint8_t *)addr;
            while (count)
            {
                PRINTF("0x%08x: ", (uint32_t)p8);
                for (pos = 0; (count > 0) && (pos < APP_BYTES_EACH_LINE); pos++)
                {
                    PRINTF("%02x ", *p8++);
                    count--;
                }
                PRINTF("\r\n");
            }
            break;

        case APP_MemAccess16Bit:
            p16 = (uint16_t *)(addr & 0xFFFFFFFEU);
            while (count)
            {
                PRINTF("0x%08x: ", (uint32_t)p16);
                for (pos = 0; (count > 0) && (pos < APP_BYTES_EACH_LINE); pos += 2)
                {
                    PRINTF("%04x ", *p16++);
                    count--;
                }
                PRINTF("\r\n");
            }
            break;

        case APP_MemAccess32Bit:
            p32 = (uint32_t *)(addr & 0xFFFFFFFCU);
            while (count)
            {
                PRINTF("0x%08x: ", (uint32_t)p32);
                for (pos = 0; (count > 0) && (pos < APP_BYTES_EACH_LINE); pos += 4)
                {
                    PRINTF("%08x ", *p32++);
                    count--;
                }
                PRINTF("\r\n");
            }
            break;

        default:
            break;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t MemSetControl(shell_handle_t shellHandle, int32_t argc, char **argv, app_mem_access_unit_t unit)
{
    uint32_t addr  = strtoul(argv[1], NULL, 0);
    uint32_t value = strtoul(argv[2], NULL, 0);
    uint32_t count = strtoul(argv[3], NULL, 0);
    uint8_t *p8;
    uint16_t *p16;
    uint32_t *p32;

    switch (unit)
    {
        case APP_MemAccess8Bit:
            p8 = (uint8_t *)addr;
            while (count--)
            {
                *p8++ = value & 0xFFU;
            }
            break;

        case APP_MemAccess16Bit:
            p16 = (uint16_t *)(addr & 0xFFFFFFFEU);
            while (count--)
            {
                *p16++ = value & 0xFFFFU;
            }
            break;

        case APP_MemAccess32Bit:
            p32 = (uint32_t *)(addr & 0xFFFFFFFCU);
            while (count--)
            {
                *p32++ = value;
            }
            break;

        default:
            break;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t MemCmpControl(shell_handle_t shellHandle, int32_t argc, char **argv, app_mem_access_unit_t unit)
{
    uint32_t addr1 = strtoul(argv[1], NULL, 0);
    uint32_t addr2 = strtoul(argv[2], NULL, 0);
    uint32_t count = strtoul(argv[3], NULL, 0);
    uint8_t *p8[2];
    uint16_t *p16[2];
    uint32_t *p32[2];
    bool diff = false;

    switch (unit)
    {
        case APP_MemAccess8Bit:
            p8[0] = (uint8_t *)addr1;
            p8[1] = (uint8_t *)addr2;
            while (count--)
            {
                if (*(p8[0]) != *(p8[1]))
                {
                    PRINTF("DIFF at [0x%08x]:%02x and [0x%08x]:%02x\r\n", p8[0], *(p8[0]), p8[1], *(p8[1]));
                    diff = true;
                }
                p8[0]++;
                p8[1]++;
            }
            break;

        case APP_MemAccess16Bit:
            p16[0] = (uint16_t *)(addr1 & 0xFFFFFFFEU);
            p16[1] = (uint16_t *)(addr2 & 0xFFFFFFFEU);
            while (count--)
            {
                if (*(p16[0]) != *(p16[1]))
                {
                    PRINTF("DIFF at [0x%08x]:%04x and [0x%08x]:%04x\r\n", p16[0], *(p16[0]), p16[1], *(p16[1]));
                    diff = true;
                }
                p16[0]++;
                p16[1]++;
            }
            break;

        case APP_MemAccess32Bit:
            p32[0] = (uint32_t *)(addr1 & 0xFFFFFFFCU);
            p32[1] = (uint32_t *)(addr2 & 0xFFFFFFFCU);
            while (count--)
            {
                if (*(p32[0]) != *(p32[1]))
                {
                    PRINTF("DIFF at [0x%08x]:%08x and [0x%08x]:%08x\r\n", p32[0], *(p32[0]), p32[1], *(p32[1]));
                    diff = true;
                }
                p32[0]++;
                p32[1]++;
            }
            break;

        default:
            break;
    }

    if (!diff)
    {
        PRINTF("Same\r\n");
    }

    return kStatus_SHELL_Success;
}

static shell_status_t MemCpyControl(shell_handle_t shellHandle, int32_t argc, char **argv, app_mem_access_unit_t unit)
{
    uint32_t dst   = strtoul(argv[1], NULL, 0);
    uint32_t src   = strtoul(argv[2], NULL, 0);
    uint32_t count = strtoul(argv[3], NULL, 0);
    uint8_t *p8Dst, *p8Src;
    uint16_t *p16Dst, *p16Src;
    uint32_t *p32Dst, *p32Src;

    switch (unit)
    {
        case APP_MemAccess8Bit:
            p8Dst = (uint8_t *)dst;
            p8Src = (uint8_t *)src;
            while (count--)
            {
                *p8Dst++ = *p8Src++;
            }
            break;

        case APP_MemAccess16Bit:
            p16Dst = (uint16_t *)(dst & 0xFFFFFFFEU);
            p16Src = (uint16_t *)(src & 0xFFFFFFFEU);
            while (count--)
            {
                *p16Dst++ = *p16Src++;
            }
            break;

        case APP_MemAccess32Bit:
            p32Dst = (uint32_t *)(dst & 0xFFFFFFFCU);
            p32Src = (uint32_t *)(src & 0xFFFFFFFCU);
            while (count--)
            {
                *p32Dst++ = *p32Src++;
            }
            break;

        default:
            break;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t MemGet8Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemGetControl(shellHandle, argc, argv, APP_MemAccess8Bit);
}

static shell_status_t MemGet16Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemGetControl(shellHandle, argc, argv, APP_MemAccess16Bit);
}

static shell_status_t MemGet32Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemGetControl(shellHandle, argc, argv, APP_MemAccess32Bit);
}

static shell_status_t MemSet8Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemSetControl(shellHandle, argc, argv, APP_MemAccess8Bit);
}

static shell_status_t MemSet16Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemSetControl(shellHandle, argc, argv, APP_MemAccess16Bit);
}

static shell_status_t MemSet32Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemSetControl(shellHandle, argc, argv, APP_MemAccess32Bit);
}

static shell_status_t MemCmp8Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCmpControl(shellHandle, argc, argv, APP_MemAccess8Bit);
}

static shell_status_t MemCmp16Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCmpControl(shellHandle, argc, argv, APP_MemAccess16Bit);
}

static shell_status_t MemCmp32Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCmpControl(shellHandle, argc, argv, APP_MemAccess32Bit);
}

static shell_status_t MemCpy8Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCpyControl(shellHandle, argc, argv, APP_MemAccess8Bit);
}

static shell_status_t MemCpy16Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCpyControl(shellHandle, argc, argv, APP_MemAccess16Bit);
}

static shell_status_t MemCpy32Control(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    return MemCpyControl(shellHandle, argc, argv, APP_MemAccess32Bit);
}

static shell_status_t FlashEraseControl(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint32_t offset = strtoul(argv[1], NULL, 0);
    uint32_t bytes  = strtoul(argv[2], NULL, 0);

    return (shell_status_t)APP_EraseFlash(offset, bytes);
}

static shell_status_t FlashWriteControl(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint32_t addr   = strtoul(argv[1], NULL, 0);
    uint32_t offset = strtoul(argv[2], NULL, 0);
    uint32_t bytes  = strtoul(argv[3], NULL, 0);

    return (shell_status_t)APP_WriteFlash((uint32_t *)addr, offset, bytes);
}

/*! @brief Main function */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrcDiv(kCLOCK_Qspi, kCLOCK_IpSrcSircAsync, 0, 0);

    APP_InitFlash();

    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, "SHELL>> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memget8));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memget16));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memget32));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memset8));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memset16));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memset32));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcmp8));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcmp16));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcmp32));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcpy8));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcpy16));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(memcpy32));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(flasherase));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(flashwrite));

    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }
}
