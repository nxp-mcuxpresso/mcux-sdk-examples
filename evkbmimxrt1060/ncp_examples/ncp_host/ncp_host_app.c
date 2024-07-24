/** @file ncp_host_app.c
 *
 *  @brief  This file provides interface for receiving tlv responses and processing tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <ctype.h>
#include "fsl_debug_console.h"
#include "board.h"
#include "task.h"
#include "ncp_tlv_adapter.h"
#include "fsl_lpuart_freertos.h"
#include "fsl_lpuart.h"
#include "fsl_gpio.h"
#include "fsl_adapter_gpio.h"
#include "ncp_cmd_common.h"
#include "ncp_host_command.h"
#include "ncp_host_app.h"

#if CONFIG_NCP_UART
extern lpuart_rtos_handle_t ncp_host_tlv_uart_handle;
#endif

#define MAX_CUSTOM_HOOKS 4
#define CONFIG_TLV_STACK_SIZE 4096

OSA_SEMAPHORE_HANDLE_DEFINE(mcu_cmd_resp_sem);
OSA_MUTEX_HANDLE_DEFINE(mcu_command_lock);

#define END_CHAR     '\r'
#define PROMPT       "\r\n# "
#define HALT_MSG     "CLI_HALT"
#define NUM_BUFFERS  1
#define MAX_COMMANDS 200

#define CONFIG_CLI_STACK_SIZE 4096
uint16_t g_cmd_seqno = 0;
unsigned int crc32_table[256];

/*ID number of command sent to ncp*/
uint32_t mcu_last_cmd_sent;

static struct
{
    int initialized;

    unsigned int bp; /* buffer pointer */
    char *inbuf;

    const struct ncp_host_cli_command *commands[MAX_COMMANDS];
    unsigned int num_commands;
    bool echo_disabled;

} ncp_host_cli;

static char mcu_string_command_buff[MCU_CLI_STRING_SIZE];

#if CONFIG_NCP_SPI
AT_NONCACHEABLE_SECTION_INIT(uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN]) = {0};
#else
uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN] = {0};
#endif

/* LPUART1: NCP Host input uart */
#define NCP_HOST_INPUT_UART_CLK_FREQ  BOARD_DebugConsoleSrcFreq()
#define NCP_HOST_INPUT_UART           LPUART1
#define NCP_HOST_INPUT_UART_IRQ       LPUART1_IRQn
#define NCP_HOST_INPUT_UART_NVIC_PRIO 5

lpuart_rtos_handle_t ncp_host_input_uart_handle;
struct _lpuart_handle t_ncp_host_input_uart_handle;

static uint8_t background_buffer[NCP_HOST_INPUT_UART_BUF_SIZE];

lpuart_rtos_config_t ncp_host_input_uart_config = {
    .baudrate    = BOARD_DEBUG_UART_BAUDRATE,
    .parity      = kLPUART_ParityDisabled,
    .stopbits    = kLPUART_OneStopBit,
    .buffer      = background_buffer,
    .buffer_size = sizeof(background_buffer),
};
uint8_t recv_buffer[NCP_HOST_INPUT_UART_SIZE];

extern power_cfg_t global_power_config;
GPIO_HANDLE_DEFINE(ncp_mcu_host_wakeup_handle);
extern uint8_t mcu_device_status;
OSA_SEMAPHORE_HANDLE_DEFINE(gpio_wakelock);

void ncp_host_input_task(void *param);

#define NCP_HOST_INPUT_TASK_PRIO   2
/* NCP host input handle task */
static OSA_TASK_HANDLE_DEFINE(ncp_host_input_thread);
static OSA_TASK_DEFINE(ncp_host_input_task, NCP_HOST_INPUT_TASK_PRIO, 1, 1024, 0);

extern int ncp_host_system_command_init();
#if CONFIG_NCP_WIFI
extern int ncp_host_wifi_command_init();
#endif
#if CONFIG_NCP_BLE
extern int ncp_host_ble_command_init();
#endif

int mcu_get_command_resp_sem()
{
    return OSA_SemaphoreWait(mcu_cmd_resp_sem, osaWaitForever_c);
}

int mcu_put_command_resp_sem()
{
    return OSA_SemaphorePost(&mcu_cmd_resp_sem);
}

int mcu_get_command_lock()
{
    return OSA_MutexLock((osa_mutex_handle_t)mcu_command_lock, osaWaitForever_c);
}

int mcu_put_command_lock()
{
    return OSA_MutexUnlock(mcu_command_lock);
}

void (*g_os_tick_hooks[MAX_CUSTOM_HOOKS])(void);
void (*g_os_idle_hooks[MAX_CUSTOM_HOOKS])(void);
/** The FreeRTOS Tick hook function. */
void vApplicationTickHook(void)
{
    int i;

    for (i = 0; i < MAX_CUSTOM_HOOKS; i++)
    {
        if (g_os_tick_hooks[i] != NULL)
            g_os_tick_hooks[i]();
    }
}
void vApplicationIdleHook(void)
{
    int i;
    for (i = 0; i < MAX_CUSTOM_HOOKS; i++)
    {
        if (g_os_idle_hooks[i] != NULL)
            g_os_idle_hooks[i]();
    }
}

/* Find the command 'name' in the ncp_host_cli commands table.
 * If len is 0 then full match will be performed else upto len bytes.
 * Returns: a pointer to the corresponding ncp_host_cli_command struct or NULL.
 */
const struct ncp_host_cli_command *lookup_command(char *name, int len)
{
    int i = 0;
    int n = 0;

    while (i < MAX_COMMANDS && n < ncp_host_cli.num_commands)
    {
        if (ncp_host_cli.commands[i]->name == NULL)
        {
            i++;
            continue;
        }
        /* See if partial or full match is expected */
        if (len != 0)
        {
            if (!strncmp(ncp_host_cli.commands[i]->name, name, len))
                return ncp_host_cli.commands[i];
        }
        else
        {
            if (!strcmp(ncp_host_cli.commands[i]->name, name))
                return ncp_host_cli.commands[i];
        }

        i++;
        n++;
    }

    return NULL;
}

/* Parse input line and locate arguments (if any), keeping count of the number
 * of arguments and their locations.  Look up and call the corresponding ncp_host_cli
 * function if one is found and pass it the argv array.
 *
 * Returns: WM_SUCCESS on success: the input line contained at least a function name and
 *          that function exists and command is processed successfully.
 *          -WM_FAIL on failuer: the input line contained at least a function name and
 *          that function exists and command is processed failed.
 *          WM_LOOKUP_FAIL on lookup failure: there is no corresponding function for the
 *          input line.
 *          WM_INVAILD_FAIL on invalid syntax: the arguments list couldn't be parsed
 *          WM_INVAILD_STRING_FAIL on invalid string command input.
 */
static int handle_input(char *inbuf)
{
    struct
    {
        unsigned inArg : 1;
        unsigned inQuote : 1;
        unsigned done : 1;
    } stat;
    static char *argv[32];
    int argc                                   = 0;
    int i                                      = 0;
    int j                                      = 0;
    const struct ncp_host_cli_command *command = NULL;
    const char *p;

    (void)memset((void *)&argv, 0, sizeof(argv));
    (void)memset(&stat, 0, sizeof(stat));

    /*
     * Some terminals add CRLF to the input buffer.
     * Sometimes the CR and LF characters maybe misplaced (it maybe added at the
     * start or at the end of the buffer). Therefore, strip all CRLF (0x0d, 0x0a).
     */
    for (j = 0; j < MCU_CLI_STRING_SIZE; j++)
    {
        if (inbuf[j] == 0x0D || inbuf[j] == 0x0A)
        {
            if (j < (MCU_CLI_STRING_SIZE - 1))
                (void)memmove((inbuf + j), inbuf + j + 1, (MCU_CLI_STRING_SIZE - j));
            inbuf[MCU_CLI_STRING_SIZE] = 0x00;
        }
    }

    do
    {
        switch (inbuf[i])
        {
            case '\0':
                if (stat.inQuote != 0U)
                    return WM_INVAILD_FAIL;
                stat.done = 1;
                break;

            case '"':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
                {
                    (void)memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg)
                    break;
                if (stat.inQuote && !stat.inArg)
                    return WM_INVAILD_FAIL;

                if (!stat.inQuote && !stat.inArg)
                {
                    stat.inArg   = 1;
                    stat.inQuote = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i + 1];
                }
                else if (stat.inQuote && stat.inArg)
                {
                    stat.inArg   = 0;
                    stat.inQuote = 0;
                    inbuf[i]     = '\0';
                }
                else
                { /* Do Nothing */
                }
                break;

            case ' ':
                if (i > 0 && inbuf[i - 1] == '\\' && stat.inArg)
                {
                    (void)memcpy(&inbuf[i - 1], &inbuf[i], strlen(&inbuf[i]) + 1);
                    --i;
                    break;
                }
                if (!stat.inQuote && stat.inArg)
                {
                    stat.inArg = 0;
                    inbuf[i]   = '\0';
                }
                break;

            default:
                if (!stat.inArg)
                {
                    stat.inArg = 1;
                    argc++;
                    argv[argc - 1] = &inbuf[i];
                }
                break;
        }
    } while (!stat.done && ++i < MCU_CLI_STRING_SIZE);

    if (stat.inQuote != 0U)
        return WM_INVAILD_FAIL;

    if (argc < 1)
        return WM_INVAILD_STRING_FAIL;

    if (!ncp_host_cli.echo_disabled)
        (void)PRINTF("\r\n");

    /*
     * Some comamands can allow extensions like foo.a, foo.b and hence
     * compare commands before first dot.
     */
    i       = ((p = strchr(argv[0], '.')) == NULL) ? 0 : (p - argv[0]);
    command = lookup_command(argv[0], i);
    if (command == NULL)
        return WM_LOOKUP_FAIL;

    return command->function(argc, argv);
}

enum
{
    BASIC_KEY,
    EXT_KEY_FIRST_SYMBOL,
    EXT_KEY_SECOND_SYMBOL,
};

/* Get an input line.
 *
 * Returns: 1 if there is input, 0 if the line should be ignored. */
static int get_input(char *inbuf, unsigned int *bp)
{
    static int state = BASIC_KEY;
    static char second_char;
    int ret;
    size_t n;

    while (true)
    {
        /*Receive string command from input uart.*/
        ret = LPUART_RTOS_Receive(&ncp_host_input_uart_handle, recv_buffer, sizeof(recv_buffer), &n);
        if (ret == kStatus_LPUART_RxRingBufferOverrun)
        {
            /* Notify about hardware buffer overrun and un-received buffer content */
            memset(background_buffer, 0, NCP_HOST_INPUT_UART_BUF_SIZE);
            memset(inbuf, 0, MCU_CLI_STRING_SIZE);
            ncp_e("Ring buffer overrun, please enter string command again");
            continue;
        }
        inbuf[*bp] = recv_buffer[0];

        if (state == EXT_KEY_SECOND_SYMBOL)
        {
            if (second_char == 0x4F)
            {
                if (inbuf[*bp] == 0x4D)
                {
                    /* Num. keypad ENTER */
                    inbuf[*bp] = '\0';
                    *bp        = 0;
                    state      = BASIC_KEY;
                    return 1;
                }
            }
        }

        if (state == EXT_KEY_FIRST_SYMBOL)
        {
            second_char = inbuf[*bp];
            if (inbuf[*bp] == 0x4F)
            {
                state = EXT_KEY_SECOND_SYMBOL;
                continue;
            }
            if (inbuf[*bp] == 0x5B)
            {
                state = EXT_KEY_SECOND_SYMBOL;
                continue;
            }
        }
        if (inbuf[*bp] == 0x1B)
        {
            /* We may be seeing a first character from a
               extended key */
            state = EXT_KEY_FIRST_SYMBOL;
            continue;
        }
        state = BASIC_KEY;

        if (inbuf[*bp] == END_CHAR)
        { /* end of input line */
            inbuf[*bp] = '\0';
            *bp        = 0;
            return 1;
        }

        if ((inbuf[*bp] == 0x08) || /* backspace */
            (inbuf[*bp] == 0x7f))
        { /* DEL */
            if (*bp > 0)
            {
                (*bp)--;
                if (!ncp_host_cli.echo_disabled)
                    (void)PRINTF("%c %c", 0x08, 0x08);
            }
            continue;
        }

        if (inbuf[*bp] == '\t')
        {
            inbuf[*bp] = '\0';
            continue;
        }

        if (!ncp_host_cli.echo_disabled)
            (void)PRINTF("%c", inbuf[*bp]);

        (*bp)++;
        if (*bp >= MCU_CLI_STRING_SIZE)
        {
            (void)PRINTF("Error: input buffer overflow\r\n");
            (void)PRINTF(PROMPT);
            *bp = 0;
            return 0;
        }
    }
}

/* Print out a bad command string, including a hex
 * representation of non-printable characters.
 * Non-printable characters show as "\0xXX".
 */
static void print_bad_command(char *cmd_string)
{
    if (cmd_string != NULL)
    {
        unsigned char *c = (unsigned char *)cmd_string;
        (void)PRINTF("command '");
        while (*c != '\0')
        {
            if (isprint(*c) != 0)
            {
                (void)PRINTF("%c", *c);
            }
            else
            {
                (void)PRINTF("\\0x%x", *c);
            }
            ++c;
        }
        (void)PRINTF("' not found\r\n");
    }
}

/* Built-in "help" command: prints all registered commands and their help
 * text string, if any. */
int help_command(int argc, char **argv)
{
    int i, n;

    (void)PRINTF("\r\n");
    for (i = 0, n = 0; i < MAX_COMMANDS && n < ncp_host_cli.num_commands; i++)
    {
        if (ncp_host_cli.commands[i]->name != NULL)
        {
            (void)PRINTF("%s %s\r\n", ncp_host_cli.commands[i]->name,
                         ncp_host_cli.commands[i]->help ? ncp_host_cli.commands[i]->help : "");
            n++;
        }
    }

    return NCP_SUCCESS;
}

static struct ncp_host_cli_command built_ins[] = {
    {"help", NULL, help_command},
};

/*
 * Register ncp_host_cli command API
 */

int ncp_host_cli_register_command(const struct ncp_host_cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    if (ncp_host_cli.num_commands < MAX_COMMANDS)
    {
        /* Check if the command has already been registered.
         * Return 0, if it has been registered.
         */
        for (i = 0; i < ncp_host_cli.num_commands; i++)
        {
            if (ncp_host_cli.commands[i] == command)
                return 0;
        }
        ncp_host_cli.commands[ncp_host_cli.num_commands++] = command;
        return 0;
    }

    return 1;
}

int ncp_host_cli_unregister_command(const struct ncp_host_cli_command *command)
{
    int i;
    if (!command->name || !command->function)
        return 1;

    for (i = 0; i < ncp_host_cli.num_commands; i++)
    {
        if (ncp_host_cli.commands[i] == command)
        {
            ncp_host_cli.num_commands--;
            int remaining_cmds = ncp_host_cli.num_commands - i;
            if (remaining_cmds > 0)
            {
                (void)memmove(&ncp_host_cli.commands[i], &ncp_host_cli.commands[i + 1],
                              (remaining_cmds * sizeof(struct ncp_host_cli_command *)));
            }
            ncp_host_cli.commands[ncp_host_cli.num_commands] = NULL;
            return 0;
        }
    }

    return 1;
}

int ncp_host_cli_register_commands(const struct ncp_host_cli_command *commands, int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (ncp_host_cli_register_command(commands++) != 0)
            return 1;
    return 0;
}

int ncp_host_cli_unregister_commands(const struct ncp_host_cli_command *commands, int num_commands)
{
    int i;
    for (i = 0; i < num_commands; i++)
        if (ncp_host_cli_unregister_command(commands++) != 0)
            return 1;

    return 0;
}

int ncp_host_send_tlv_command()
{
    int ret                = NCP_STATUS_SUCCESS;
    uint16_t transfer_len  = 0;
    NCP_HOST_COMMAND *header = (NCP_HOST_COMMAND *)(mcu_tlv_command_buff);

    /* set cmd seqno */
    header->seqnum  = g_cmd_seqno;
    transfer_len = header->size;

    if (transfer_len == 0)
    {
        ret = -NCP_STATUS_ERROR;
        goto done;
    }

    mcu_last_cmd_sent = header->cmd;

    if (transfer_len >= sizeof(NCP_HOST_COMMAND))
    {
        /* Wakeup MCU device through GPIO if host configured GPIO wake mode */
        if ((global_power_config.wake_mode == WAKE_MODE_GPIO) && (mcu_device_status == MCU_DEVICE_STATUS_SLEEP))
        {
            GPIO_PinWrite(GPIO1, 27, 0);
            ncp_d("get gpio_wakelock after GPIO wakeup\r\n");
            /* Block here to wait for MCU device complete the PM3 exit process */
            OSA_SemaphoreWait((osa_semaphore_handle_t)gpio_wakelock, osaWaitForever_c);
            GPIO_PinWrite(GPIO1, 27, 1);
            /* Release semaphore here to make sure software can get it successfully when receiving sleep enter event for next sleep loop. */
            OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);
        }
        /* write response to host */
        ret = ncp_tlv_send(header, transfer_len);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_e("failed to write response");
            ret = -NCP_STATUS_ERROR;
            goto done;
        }

        g_cmd_seqno++;
    }
    else
    {
        ncp_e("length is less than cmd header len, cmd_len = %d", transfer_len);
        ret = -NCP_STATUS_ERROR;
        goto done;
    }

done:
    if (ret == -NCP_STATUS_ERROR)
        mcu_put_command_resp_sem();

    (void)memset((uint8_t *)header, 0, NCP_HOST_COMMAND_LEN);
    mcu_put_command_lock();

    return ret;
}
void ncp_host_input_task(void *pvParameters)
{
    ncp_host_input_uart_config.srcclk = NCP_HOST_INPUT_UART_CLK_FREQ;
    ncp_host_input_uart_config.base   = NCP_HOST_INPUT_UART;

    NVIC_SetPriority(NCP_HOST_INPUT_UART_IRQ, NCP_HOST_INPUT_UART_NVIC_PRIO);

    if (LPUART_RTOS_Init(&ncp_host_input_uart_handle, &t_ncp_host_input_uart_handle, &ncp_host_input_uart_config) !=
        NCP_SUCCESS)
    {
        vTaskSuspend(NULL);
    }

    /* Receive user input and send it back to terminal. */
    while (1)
    {
        int ret;

        if (ncp_host_cli.inbuf == NULL)
        {
            ncp_host_cli.inbuf = mcu_string_command_buff;
            ncp_host_cli.bp    = 0;
        }

        /*clear the last input strings, the whole command input is handled in get_input founction*/
        memset(mcu_string_command_buff, 0, MCU_CLI_STRING_SIZE);
        if (get_input(ncp_host_cli.inbuf, &ncp_host_cli.bp))
        {
            /*Wait for command response semaphore.*/
            mcu_get_command_resp_sem();

            if (strcmp(ncp_host_cli.inbuf, HALT_MSG) == 0)
                break;

            ret = handle_input(ncp_host_cli.inbuf);
            if (ret == WM_LOOKUP_FAIL)
            {
                print_bad_command(ncp_host_cli.inbuf);
                /*If string commands don't match with registered commands, release command response semaphore.*/
                mcu_put_command_resp_sem();
            }
            else if (ret == WM_INVAILD_FAIL)
            {
                ncp_e("syntax error");
                /*If the format of string command is error, release command response semaphore.*/
                mcu_put_command_resp_sem();
            }
            else if (ret == -NCP_FAIL)
            {
                ncp_e("Failed to process '%s' command", ncp_host_cli.inbuf);
                /*If failed to process string command, release command response semaphore.*/
                mcu_put_command_resp_sem();
            }
            else if (ret == WM_INVAILD_STRING_FAIL)
            {
                /*If the string command is empty, release command response semaphore.*/
                mcu_put_command_resp_sem();
            }
            else /*Send tlv command to ncp device */
                ncp_host_send_tlv_command();

            (void)PRINTF(PROMPT);
        }
    }
}

static void wakeup_int_callback(void *param)
{
    if(global_power_config.wakeup_host)
    {
        PRINTF("Wakeup host sucessfully\r\n");
        global_power_config.wakeup_host = 0;
    }
}

static void ncp_host_lpm_gpio_init()
{
    /* Define the init structure for the input/output switch pin */
    gpio_pin_config_t gpio_in_config = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_IntRisingEdge
    };
    gpio_pin_config_t gpio_out_config = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 1,
        .interruptMode = kGPIO_NoIntmode
    };
    /* Init input GPIO for wakeup MCU host */
    GPIO_PinInit(GPIO1, 26, &gpio_in_config);
    /* Init output GPIO for wakeup NCP device */
    GPIO_PinInit(GPIO1, 27, &gpio_out_config);
    hal_gpio_pin_config_t wakeup_config = {kHAL_GpioDirectionIn, 0, 1, 26};
    HAL_GpioInit(ncp_mcu_host_wakeup_handle, &wakeup_config);
    HAL_GpioSetTriggerMode(ncp_mcu_host_wakeup_handle, kHAL_GpioInterruptRisingEdge);
    HAL_GpioInstallCallback(ncp_mcu_host_wakeup_handle, wakeup_int_callback, NULL);
}

int ncp_host_cli_init(void)
{
    ncp_host_lpm_gpio_init();

    if(OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)gpio_wakelock) != NCP_SUCCESS)
    {
        ncp_e("Failed to create gpio_wakelock");
        return -NCP_STATUS_ERROR;
    }

    OSA_SemaphorePost((osa_semaphore_handle_t)gpio_wakelock);

    (void)memset((void *)&ncp_host_cli, 0, sizeof(ncp_host_cli));

    /* add our built-in commands */
    if (ncp_host_cli_register_commands(&built_ins[0], sizeof(built_ins) / sizeof(struct ncp_host_cli_command)) != 0)
        return -NCP_STATUS_ERROR;


    ncp_host_system_command_init();
#if CONFIG_NCP_WIFI
    ncp_host_wifi_command_init();
#endif
#if CONFIG_NCP_BLE
    ncp_host_ble_command_init();
#endif

    int n = 0;
    for (int i = 0; i < MAX_COMMANDS && n < ncp_host_cli.num_commands; i++)
    {
        if (ncp_host_cli.commands[i]->name != NULL)
        {
            (void)PRINTF("%s %s\r\n", ncp_host_cli.commands[i]->name,
                         ncp_host_cli.commands[i]->help ? ncp_host_cli.commands[i]->help : "");
            n++;
        }
    }

    (void)OSA_TaskCreate((osa_task_handle_t)ncp_host_input_thread, OSA_TASK(ncp_host_input_task), (osa_task_param_t)NULL);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief       This function initializes NCP host app. Create locks/queues/tasks.
 *
 * @return      Status returned
 */
int ncp_host_app_init()
{
    int ret;

    ret = OSA_SemaphoreCreateBinary(mcu_cmd_resp_sem);
    if (ret != NCP_SUCCESS)
    {
        ncp_e("Failed to create mcu command resposne semaphore: %d", ret);
        return -NCP_FAIL;
    }
    mcu_put_command_resp_sem();
    
    ret = OSA_MutexCreate(mcu_command_lock);
    if (ret != NCP_SUCCESS)
    {
        ncp_e("Failed to create mcu command lock: %d", ret);
        (void)OSA_SemaphoreDestroy(mcu_cmd_resp_sem);
        return -NCP_FAIL;
    }

    ret = ncp_host_cli_init();
    if (ret != NCP_SUCCESS)
    {
        ncp_e("Failed to init host cli: %d", ret);
        (void)OSA_SemaphoreDestroy(mcu_cmd_resp_sem);
        (void)OSA_MutexDestroy(mcu_command_lock);
        return -NCP_FAIL;
    }

    return NCP_SUCCESS;
}
