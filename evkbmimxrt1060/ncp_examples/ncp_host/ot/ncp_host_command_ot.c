/*!\file ncp_host_command_ot.c
 *\brief This file provides API functions to build tlv commands and process tlv responses.
 */
 /*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_OT

/* -------------------------------------------------------------------------- */
/*                                Includes                                    */
/* -------------------------------------------------------------------------- */

#include "ncp_cmd_common.h"
#include "ncp_host_app.h"
#include "ncp_host_command.h"
#include "ncp_debug.h"
#include "otopcode.h"

/* -------------------------------------------------------------------------- */
/*                                Definitions                                 */
/* -------------------------------------------------------------------------- */

#define NCP_15d4_CMD_FORWARD 0x00100000
#define OT_OPCODE_SIZE       1
#define OT_SPACE_SIZE        1

/* -------------------------------------------------------------------------- */
/*                                Variables                                   */
/* -------------------------------------------------------------------------- */

extern uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN];
uint8_t ot_reset_flag = 0;

/* -------------------------------------------------------------------------- */
/*                                Prototypes                                  */
/* -------------------------------------------------------------------------- */

extern int ncp_host_cli_register_commands(const struct ncp_host_cli_command *commands, int num_commands);

/* -------------------------------------------------------------------------- */
/*                                Code                                        */
/* -------------------------------------------------------------------------- */

void *ncp_host_get_cmd_buffer_ot()
{
    return mcu_tlv_command_buff;
}

static int ot_command(int argc, char **argv)
{
    int8_t  opcode;
    uint8_t i;

    NCP_COMMAND *command = ncp_host_get_cmd_buffer_ot();
    memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);

    /* ot using opcode to identify commands, command id bits can be any value */
    command->cmd    = NCP_CMD_15D4 | NCP_15d4_CMD_FORWARD | NCP_MSG_TYPE_CMD | 0x00000001;
    command->size   = NCP_CMD_HEADER_LEN;
    command->result = NCP_CMD_RESULT_OK;
    command->rsvd   = 0;

#if NCP_COEX_OT_DEBUG
    for (i = 1; i < argc; i++)
    {
        PRINTF("ot > argv[%d] = %s, argv[%d].len = %d\n", i, argv[i], i, strlen(argv[i]));
    }
#endif
    
    if (strcmp((char const *)argv[1], "reset") == 0 || strcmp((char const *)argv[1], "factoryreset") == 0)
    {
        /* reset / factoryreset no response, should give command Semaphore */
        mcu_put_command_resp_sem();
#if CONFIG_NCP_SDIO
        ot_reset_flag = 1;
#endif
    }

    opcode = ot_get_opcode((uint8_t *)argv[1], strlen(argv[1]));
    if (opcode == -1)
    {
        PRINTF("\nNot supported command.\n> ");
        return NCP_STATUS_ERROR;
    }

    /* ot command opcode */
    *((uint8_t *)command + command->size) = opcode;
    command->size += OT_OPCODE_SIZE;
    
    /* ot command argurment */
    for (i = 2; i < argc; i++)
    {
        *((uint8_t *)command + command->size) = ' ';
        command->size += OT_SPACE_SIZE;
        memcpy((uint8_t *)command + command->size, argv[i], strlen(argv[i]));
        command->size += strlen(argv[i]);
    }
    
    /* ot command tailed */
    *((uint8_t *)command + command->size) = '\r';
    command->size += OT_SPACE_SIZE;

    return NCP_STATUS_SUCCESS;
}

static struct ncp_host_cli_command ncp_host_app_cli_commands_ot[] = {
    {"ot", "<command name> <command argument>", ot_command},

};

int ncp_host_ot_command_init()
{
    if (ncp_host_cli_register_commands(ncp_host_app_cli_commands_ot, sizeof(ncp_host_app_cli_commands_ot) / sizeof(struct ncp_host_cli_command)) != 0)
    {
        return NCP_STATUS_ERROR;
    }

    return NCP_STATUS_SUCCESS;
}

#endif
