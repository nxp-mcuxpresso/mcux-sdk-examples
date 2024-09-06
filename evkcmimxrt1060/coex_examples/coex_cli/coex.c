/**
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include <string.h>/**/
#include "BT_common.h"
#include "BT_version.h"
#include "BT_hci_api.h"
#include "wlan.h"
#include "cli.h"
#include "coex.h"

extern struct cli_command tests[];
char *wlantemp;
#define WLAN_CMD_BUF_SIZE 512
char wlan_command[WLAN_CMD_BUF_SIZE];
void execute_commnd(char *wlan_command,char** args,int j);
extern int appl_main (int argc, char **argv);
int wlan_connect_success_count = 0;
int wlan_connect_total_count   = 0;
void coex_menuPrint(void);

#define TASK_SCAN_STACK_SIZE      (2* 1024)
#define TASK_CONNECT_STACK_SIZE   (10 * 1024)
int wlan_scan_running=0;
int wlan_connect_running=0;
portSTACK_TYPE *task_scan_stack = NULL;
void scan_loop(void * parameter);
static OSA_TASK_HANDLE_DEFINE(ScanTaskHandle);
static OSA_TASK_DEFINE(scan_loop, OSA_PRIORITY_LOW, 1, TASK_SCAN_STACK_SIZE, 0);

void connect_loop(void * parameter);
static OSA_TASK_HANDLE_DEFINE(ConnectTaskHandle);
static OSA_TASK_DEFINE(connect_loop, OSA_PRIORITY_LOW, 1, TASK_CONNECT_STACK_SIZE, 0);
/*******************************************************************************
 * Code
 ******************************************************************************/

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

/*!
 * @brief Function to menu Separator.
 */
static void menuSeparator(void)
{
    printSeparator();
}

static struct cli_command built_ins[] = {
    {"help", NULL, help_command},
};

int coex_cli_init(void)
{
    if (cli_register_commands(&built_ins[0], (int)(sizeof(built_ins) / sizeof(struct cli_command))) != 0)
    {
        return -WM_FAIL;
    }

     return WM_SUCCESS;
}

/*!
 * @brief Function to start MAIN BT MENU.
 */
#if defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0)
static void bt_menu(void)
{
	appl_main(0 , NULL);
}
#endif

void wifi_cli(void)
{
	char a;
	int i=0;
    char wlan_command[WLAN_CMD_MAX_LEN];
	PRINTF("Enter WiFi commands\n");

	while (1)
	{
		PRINTF("\n>");
		while (1) 
		{
			a=pollChar();
			if(a == '\n') continue;
			if (a== '\r')
			{
				wlan_command[i]='\0';          
				break;
			}
			wlan_command[i]=a;
			i++;
            if (i >= WLAN_CMD_MAX_LEN)
                break;
		} 
 
		i=0;
		char* List[20];
		char *wlantemp=strtok(wlan_command, " ");
		int j=1;
		List[0]=wlan_command;
		while(1)
		{
			wlantemp = strtok(NULL, " ");
			if(wlantemp == NULL)
				break;     	  
			List[j]=wlantemp;
			j++;    	
		}
   
		if (strcmp(wlan_command,"0")==0)
		{	
			break;
		}
		if (strcmp(wlan_command,"")==0)
		{	
			continue;
		}
		execute_commnd(wlan_command,List,j);
	}	
}

void scan_loop(void *parameter)
{
    int j = 0;
    char *args[20];
    const struct cli_command *command = NULL;
    char wlan_command[10];
    const char *p;
    int i   = 0;
    args[0] = "wlan-scan";
    strcpy(wlan_command, "wlan-scan");
    i       = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
    command = lookup_command(wlan_command, i);
    while (1)
    {
        command->function(j, args);
        OSA_TimeDelay(5000);
    }
}

bool is_connected()
{
    enum wlan_connection_state state;
    wlan_get_connection_state(&state);

    if (state == WLAN_CONNECTED)
    {
        return 1;
    }
    else
        return 0;
}

bool is_disconnected()
{
    enum wlan_connection_state state;
    wlan_get_connection_state(&state);

    if (state == WLAN_DISCONNECTED)
    {
        return 1;
    }
    else
        return 0;
}

void connect_loop(void *profile)
{
    int j = 2;
    char *args[ARGS_SIZE];
    const struct cli_command *command = NULL;
    const char *p;
    int i = 0;
    char wlan_command[30];

    char profile_name[PROFILE_NAME_LEN];
    if ((profile != NULL) && (strlen((char *)profile) < PROFILE_NAME_LEN))
    {	    
    	strcpy(profile_name, profile);
    }
    wlan_connect_total_count   = 0;
    wlan_connect_success_count = 0;

    while (1)
    {
    	args[0] = "wlan-connect";
    	args[1] = profile_name;
    	strcpy(wlan_command, "wlan-connect");
    	i       = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
    	command = lookup_command(wlan_command, i);
        command->function(j, args);
        while (1)
        {
            OSA_TimeDelay(500);
            if (is_disconnected())
            {
                PRINTF("wlan connection failed");
                wlan_connect_total_count++;
                break;
            }
            else if (is_connected())
            {
                wlan_connect_success_count++;
                wlan_connect_total_count++;
                args[0] = "wlan-disconnect";
                strcpy(wlan_command, "wlan-disconnect");
                i       = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
                command = lookup_command(wlan_command, i);
                command->function(j, args);
                OSA_TimeDelay(5000);
                break;
            }
            else
            {
                continue;
            }
        }
    }
}

void execute_commnd(char *wlan_command, char **args, int j)

{
	osa_status_t status;
	(void)status;
	const struct cli_command *command = NULL;
	const char *p;
	int i=0;
	if(strcmp(wlan_command,"wlan-scan-loop")==0)
    {
        if (wlan_scan_running == 0)
        {
		    wlan_scan_running=1;
		    status =
                OSA_TaskCreate((osa_task_handle_t)ScanTaskHandle, OSA_TASK(scan_loop), NULL);
            assert(KOSA_StatusSuccess == status);
        }
        {
            PRINTF("wlan scan loop is already running");
        }
	}   
    else if(strcmp(wlan_command,"wlan-scan-stop")==0)
    {
		if (wlan_scan_running==1)
		{
			OSA_TaskDestroy(ScanTaskHandle);
			wlan_scan_running=0;
            PRINTF("wlan scan loop stopped");
		}
	    else{
			PRINTF("wlan scan loop is not running");
		}
	}
	else if(strcmp(wlan_command,"wlan-connect-loop")==0)
    {
        if (j == 1)
        {
            printf("Error Usage: wlan-connect-loop <profile_name>");
        }
        else if (wlan_connect_running == 0)
        {
		    wlan_connect_running=1;
		    wlan_command="wlan-connect";
		    status =OSA_TaskCreate((osa_task_handle_t)ConnectTaskHandle, OSA_TASK(connect_loop), args[1]);
            assert(KOSA_StatusSuccess == status);
        }
        else
        {
            PRINTF("wlan connect-disconnect loop is already running");
        }
	}
	else if(strcmp(wlan_command,"wlan-connect-stop")==0)
    {
        if (wlan_connect_running==1)
        {
            OSA_TaskDestroy(ConnectTaskHandle);

            PRINTF("Pass count is %d\n", wlan_connect_success_count);
            PRINTF("Total count is %d\n", wlan_connect_total_count);
            wlan_connect_running = 0;
        }
        else
        {
            PRINTF("wlan connect-disconnect loop is not running");
        }
    }
    else
    {
        i       = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
        command = lookup_command(wlan_command, i);
        if (command == NULL)
            PRINTF("Invalid Command");
        else
            command->function(j, args);
    }
}

/*!
 * @brief struct for coex menu.
 */
typedef struct
{
    char key;
    // int key;
    void (*func)(void);
    const char *text;
} menu_item_t;

menu_item_t menuItems[] = {
	{'0',coex_menuPrint, "Coex menu print"},
#if defined(CONFIG_DISABLE_BLE) && (CONFIG_DISABLE_BLE == 0)
	{'b', bt_menu, "BT MAIN MENU-Call"},
#endif
#if defined(CONFIG_WIFI_BLE_COEX_APP) && (CONFIG_WIFI_BLE_COEX_APP == 1)
	{'w',wifi_cli, "Wifi Cli"},
#endif
	{0, NULL, NULL},
};

/*!
 * @brief Function to coex menu print.
 */
void coex_menuPrint(void)
{
    for (int i = 0; menuItems[i].text != NULL; i++)
    {
        if (menuItems[i].key)
            PRINTF("  %c  %s\r\n", menuItems[i].key, menuItems[i].text);
        else
            PRINTF("  %d  %s\r\n", i, menuItems[i].text);
    }
}

/*!
 * @brief Function to coex menu action.
 */
void coex_menuAction(int ch)
{
    if (ch == '\r' || ch == ' ' || ch == '\n')
    {
        menuSeparator();
        return;
    }

    for (int i = 0; menuItems[i].func != NULL; i++)
    {
        if (menuItems[i].key == ch)
        {
            PRINTF("Key '%c': %s\r\n", ch, menuItems[i].text);
            menuItems[i].func();
            return;
        }
    }
    PRINTF("ERROR: No action bound to '%c'\r\n", ch);
}

/*!
 * @brief Function to Poll char.
 */
int pollChar(void)
{
    //    if (!UART_HAL_GetStatusFlag(BOARD_DEBUG_UART_BASEADDR, kUartRxDataRegFull))
    //        return -1;
    //
    //    uint8_t ch;
    //    UART_HAL_Getchar(BOARD_DEBUG_UART_BASEADDR, &ch);
    //    // Flush the UART to avoid the "OpenSDA UART infinite RX FIFO" bug:
    //    // If user presses keys very quickly then after a while, the UART enters
    //    // a mode where the same sequence of characters are repeatedly returned
    //    // by UART_HAL_Getchar(). Even a hard reset does not fix it.
    //    // Only way to recover is by unplugging OpenSDA USB.
    //    UART_HAL_FlushRxFifo(BOARD_DEBUG_UART_BASEADDR);

    int tmp;
    // TODO: verify the message above. Which board has this issue ?
    tmp = GETCHAR();
    // scanf("%d,", &tmp);
    return tmp;
}

//*********************************************************************************************
