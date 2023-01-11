/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ff.h"
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/hfp_ag.h>
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "app_shell.h"
#include "app_discover.h"
#include "app_connect.h"
#include "app_handsfree_ag.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static shell_status_t shellBt(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/

SHELL_COMMAND_DEFINE(bt,
                     "\r\n\"bt\": BT related function\r\n"
                     "  USAGE: bt [discover|connect|disconnect|delete]\r\n"
                     "    discover             start to find BT devices\r\n"
                     "    connect              connect to the device that is found, for example: bt connect n (from 1)\r\n"
                     "    openaudio            open audio connection without calls\r\n"
                     "    closeaudio           close audio connection without calls\r\n"  
                     "    sincall              start an incoming call\r\n"
                     "    aincall              accept the call.\r\n"
                     "    eincall              end an call.\r\n"
                     "    set_tag              set phone num tag, for example: bt set_tag 123456789\r\n" 
                     "    select_codec         codec select for codec Negotiation, for example: bt select_codec 2, it will select the codec 2 as codec.\r\n" 
                     "    set_mic_volume       update mic Volume, for example: bt set_mic_volume 14\r\n" 
                     "    set_speaker_volume   update Speaker Volume, for example: bt set_speaker_volume 14\r\n"
                     "    stwcincall           start multiple an incoming call\r\n"   
                     "    disconnect           disconnect current connection\r\n"
                     "    delete               delete all devices. Ensure to disconnect the HCI link connection with the peer "
                     "device before attempting to delete the bonding information.\r\n",
                     shellBt,
                     SHELL_IGNORE_PARAMETER_COUNT);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint32_t hfp_get_value_from_str(char *ch)
{
      uint8_t selectIndex = 0;
      uint8_t value = 0;

      for (selectIndex = 0; selectIndex < strlen(ch); ++selectIndex)
      {
          if ((ch[selectIndex] < '0') || (ch[selectIndex] > '9'))
          {
              PRINTF("The Dial parameter is wrong\r\n");
              return kStatus_SHELL_Error;
          }
      }

      if (selectIndex == 0U)
      {
          PRINTF("The Dial parameter is wrong\r\n");
      }  
      else if(selectIndex == 1U)
      {
        value = (ch[0] - '0');
      }
      else if(selectIndex == 2U)
      {
        value = (ch[0] - '0')*10 + (ch[1] - '0');
      }
      return value;
}
static shell_status_t shellBt(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint8_t *addr;

    if (argc < 1)
    {
        PRINTF("the parameter count is wrong\r\n");
    }

    if (strcmp(argv[1], "discover") == 0)
    {
        app_discover();
    }
    else if (strcmp(argv[1], "connect") == 0)
    {
        uint8_t selectIndex = 0;
        char *ch            = argv[2];

        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }

        for (selectIndex = 0; selectIndex < strlen(ch); ++selectIndex)
        {
            if ((ch[selectIndex] < '0') || (ch[selectIndex] > '9'))
            {
                PRINTF("the parameter is wrong\r\n");
                return kStatus_SHELL_Error;
            }
        }

        switch (strlen(ch))
        {
            case 1:
                selectIndex = ch[0] - '0';
                break;
            case 2:
                selectIndex = (ch[0] - '0') * 10 + (ch[1] - '0');
                break;
            default:
                PRINTF("the parameter is wrong\r\n");
                break;
        }

        if (selectIndex == 0U)
        {
            PRINTF("the parameter is wrong\r\n");
        }
        addr = app_get_addr(selectIndex - 1);
        app_connect(addr);
    }
    else if (strcmp(argv[1], "disconnect") == 0)
    {
        app_disconnect();
    }
    else if (strcmp(argv[1], "openaudio") == 0)
    {
        app_hfp_ag_open_audio();
    }
    else if (strcmp(argv[1], "closeaudio") == 0)
    {
        app_hfp_ag_close_audio();
    }
    else if (strcmp(argv[1], "sincall") == 0)
    {
        app_hfp_ag_start_incoming_call();
    }
    else if (strcmp(argv[1], "stwcincall") == 0)
    {
        app_hfp_ag_start_twc_incoming_call();
    }
    else if (strcmp(argv[1], "aincall") == 0)
    {
        app_hfp_ag_accept_incoming_call();
    }
    else if (strcmp(argv[1], "eincall") == 0)
    {
        app_hfp_ag_stop_incoming_call();
    }
    else if (strcmp(argv[1], "select_codec") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        app_hfp_ag_codec_select(hfp_get_value_from_str(argv[2]));
    }
    else if (strcmp(argv[1], "set_mic_volume") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        app_hfp_ag_volume_update(hf_volume_type_mic, hfp_get_value_from_str(argv[2]));

    } 
    else if (strcmp(argv[1], "set_speaker_volume") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        app_hfp_ag_volume_update(hf_volume_type_speaker, hfp_get_value_from_str(argv[2]));
    } 
    else if (strcmp(argv[1], "set_tag") == 0)
    {
        uint8_t selectIndex = 0;
        char *ch            = argv[2];

        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }

        for (selectIndex = 0; selectIndex < strlen(ch); ++selectIndex)
        {
            if ((ch[selectIndex] < '0') || (ch[selectIndex] > '9'))
            {
                PRINTF("The Dial parameter is wrong\r\n");
                return kStatus_SHELL_Error;
            }
        }

        if (selectIndex == 0U)
        {
            PRINTF("The Dial parameter is wrong\r\n");
        } 
        app_hfp_ag_set_phnum_tag(ch);
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        int err = 0;
        err     = bt_unpair(BT_ID_DEFAULT, NULL);
        if (err != 0)
        {
            PRINTF("failed reason = %d\r\n", err);
        }
        else
        {
            PRINTF("success\r\n");
        }
    }
    else
    {
    }

    return kStatus_SHELL_Success;
}

void app_shell_init(void)
{
    DbgConsole_Flush();
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");
    PRINTF("\r\n");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(bt));
}
