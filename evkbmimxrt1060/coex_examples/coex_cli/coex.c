/** @file coex.c
 *
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

extern struct cli_command tests[];
char *wlantemp;
char wlan_command[250];
void execute_commnd(char *wlan_command,char** args,int j);
extern int appl_main (int argc, char **argv);

void coex_menuPrint(void);

const int TASK_SCAN_PRIO       = OS_PRIO_4;
const int TASK_SCAN_STACK_SIZE = (2* 1024);
const int TASK_CONNECT_PRIO       = OS_PRIO_4;
const int TASK_CONNECT_STACK_SIZE = (10 * 1024);
int wlan_scan_running=0;
int wlan_connect_running=0;
portSTACK_TYPE *task_scan_stack = NULL;
TaskHandle_t task_scan_task_handler;
TaskHandle_t task_connect_task_handler;
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


int coex_cli_init()
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
static void bt_menu(void)
{
	appl_main(0 , NULL);

}

int pollChar();


void wifi_cli(void)
{
	char a;
	int i=0;
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


void scan_loop(void * parameter)
{ 
	int j=0;
	char* args[20];
	const struct cli_command *command = NULL;
	const char *p;
	int i=0;
	args[0]="wlan-scan";
	strcpy(wlan_command,"wlan-scan");
	i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
    command = lookup_command(wlan_command, i);
	while(1){
		command->function(j, args); 
		os_thread_sleep(os_msec_to_ticks(5000)); 
 	}
}

bool is_connected()
{
	enum wlan_connection_state state;
	wlan_get_connection_state(&state);
//if (wlan_get_connection_state(&state)!=0){
	if (state==WLAN_CONNECTED)
	{
		
		return 1;
	}
	else
		return 0;
//}
}

void connect_loop(void * profile)
{ 
    int j=2;
	char* args[20];
	const struct cli_command *command = NULL;
	const char *p;
	int i=0;
	args[0]="wlan-connect";
	args[1]="test";
	strcpy(wlan_command,"wlan-connect");
	i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
    command = lookup_command(wlan_command, i);
	while(1){
		strcpy(wlan_command,"wlan-connect");
		i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
		command = lookup_command(wlan_command, i);
	    command->function(j, args);
        while(1){		   
			if (!is_connected()){
				os_thread_sleep(os_msec_to_ticks(500));
			}
			else
			{
				break;
			}			 
		}
		args[0]="wlan-disconnect";
		strcpy(wlan_command,"wlan-disconnect");
		i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
		command = lookup_command(wlan_command, i);
		command->function(j, args);
		os_thread_sleep(os_msec_to_ticks(5000));  
	}		   
}


	

void execute_commnd(char *wlan_command,char** args,int j)

{
	
	int32_t result = 0;
    (void)result;
	const struct cli_command *command = NULL;
	const char *p;
	int i=0;
	if(strcmp(wlan_command,"wlan-scan-loop")==0){
		wlan_scan_running=1;
		result =
        xTaskCreate(scan_loop, "scan_loop", TASK_SCAN_STACK_SIZE, task_scan_stack, TASK_SCAN_PRIO, &task_scan_task_handler);
        assert(pdPASS == result);
	}   
    else if(strcmp(wlan_command,"wlan-scan-stop")==0){
		if (wlan_scan_running==1)
		{
		 	vTaskDelete(task_scan_task_handler);
			wlan_scan_running=0;
		}
	    else{
			PRINTF("Wifi scan is not running");
		}
	}
	else if(strcmp(wlan_command,"wlan-connect-loop")==0){
		wlan_connect_running=1;
		wlan_command="wlan-connect";
		result =xTaskCreate(connect_loop, "connect_loop", TASK_CONNECT_STACK_SIZE, args[1], TASK_CONNECT_PRIO, &task_connect_task_handler);
        assert(pdPASS == result);
	   
	}
	else if(strcmp(wlan_command,"wlan-connect-stop")==0){
		if (wlan_connect_running==1)
		{
			vTaskDelete(task_connect_task_handler);
			wlan_connect_running=0;
		}
	    else{
			PRINTF("Wifi connect-disconnect is not running");
	    }
	   
    }
	else{   
		i = ((p = strchr(wlan_command, '.')) == NULL) ? 0 : (p - wlan_command);
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
    //int key;
    void (*func)(void);
    const char *text;
} menu_item_t;



menu_item_t menuItems[] = {
	{'0',coex_menuPrint, "Coex menu print"},
	{'b', bt_menu, "BT MAIN MENU-Call"},
	{'w',wifi_cli, "Wifi Cli"},
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
int pollChar()
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

