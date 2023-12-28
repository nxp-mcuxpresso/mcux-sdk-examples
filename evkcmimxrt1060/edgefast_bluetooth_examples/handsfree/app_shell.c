/*
 * Copyright 2021 NXP
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
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "app_shell.h"
#include <bluetooth/hfp_hf.h>
#include "app_handsfree.h"
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
                     "  USAGE: bt [dial|aincall|eincall]\r\n"
                     "    dial          dial out call.\r\n" 
                     "    aincall       accept the incoming call.\r\n"
                     "    eincall       end an incoming call.\r\n"
                     "    svr           start voice recognition.\r\n" 
                     "    evr           stop voice recognition.\r\n" 
                     "    clip          enable CLIP notification.\r\n" 
                     "    disclip       disable CLIP notification.\r\n"  
                     "    ccwa          enable call waiting notification.\r\n" 
                     "    disccwa       disable call waiting notification.\r\n"                       
                     "    micVolume     Update mic Volume.\r\n" 
                     "    speakerVolume Update Speaker Volume.\r\n"                           
                     "    lastdial      call the last dial number.\r\n" 
                     "    voicetag      Get Voice-tag Phone Number (BINP).\r\n"   
                     "    multipcall    multiple call option.\r\n",
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

    if (argc < 1)
    {
        PRINTF("the parameter count is wrong\r\n");
    }

    if (strcmp(argv[1], "dial") == 0)
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
        hfp_dial(ch);
    }  
    else if (strcmp(argv[1], "svr") == 0)
    {
        hfp_start_voice_recognition();
    } 
    else if (strcmp(argv[1], "evr") == 0)
    {
        hfp_stop_voice_recognition();
    } 
     else if (strcmp(argv[1], "micVolume") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        hfp_volume_update(hf_volume_type_mic, hfp_get_value_from_str(argv[2]));

    } 
    else if (strcmp(argv[1], "speakerVolume") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        hfp_volume_update(hf_volume_type_speaker, hfp_get_value_from_str(argv[2]));
    }    

   else if (strcmp(argv[1], "lastdial") == 0)
   {
      hfp_last_dial();
    }  
   else if (strcmp(argv[1], "memorydial") == 0)
   {
      if (argc < 2)
      {
          PRINTF("the parameter count is wrong\r\n");
          return kStatus_SHELL_Error;
      }
      dial_memory(hfp_get_value_from_str(argv[2]));
    }  
    else if (strcmp(argv[1], "clip") == 0)
    {
       hfp_enable_clip(1);
    }
    else if (strcmp(argv[1], "disclip") == 0)
    {
       hfp_enable_clip(0);
    }
    else if (strcmp(argv[1], "ccwa") == 0)
    {
       hfp_enable_ccwa(1);
    }
    else if (strcmp(argv[1], "disccwa") == 0)
    {
       hfp_enable_ccwa(0);
    }    
     else if (strcmp(argv[1], "multipcall") == 0)
    {
        if (argc < 2)
        {
            PRINTF("the parameter count is wrong\r\n");
            return kStatus_SHELL_Error;
        }
        hfp_multiparty_call_option(hfp_get_value_from_str(argv[2]));
    }    
    
    else if (strcmp(argv[1], "aincall") == 0)
    {

      hfp_AnswerCall();
    }
    else if (strcmp(argv[1], "voicetag") == 0)
    {

      hfp_hf_get_last_voice_tag_number();
    }
    
    else if (strcmp(argv[1], "eincall") == 0)
    {
      hfp_RejectCall();
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
