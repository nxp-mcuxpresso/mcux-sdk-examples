/**@file ncp_host_command.h
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __NCP_HOST_COMMAND_H__
#define __NCP_HOST_COMMAND_H__

#define MCU_CLI_STRING_SIZE          500
#define NCP_HOST_MAX_COMMANDS        500
#define NCP_HOST_INPUT_UART_BUF_SIZE 32
#define NCP_HOST_INPUT_UART_SIZE     1

#define NCP_HOST_COMMAND_LEN             4096 // The max number bytes which UART can receive.

#define WM_LOOKUP_FAIL         1
#define WM_INVAILD_FAIL        2
#define WM_INVAILD_STRING_FAIL 3

#define MCU_DEVICE_STATUS_ACTIVE 1
#define MCU_DEVICE_STATUS_SLEEP  2

/** Structure for registering CLI commands */
struct ncp_host_cli_command
{
    /** The name of the CLI command */
    const char *name;
    /** The help text associated with the command */
    const char *help;
    /** The function that should be invoked for this command. */
    int (*function)(int argc, char **argv);
};

#endif /* __NCP_HOST_COMMAND_H__ */
