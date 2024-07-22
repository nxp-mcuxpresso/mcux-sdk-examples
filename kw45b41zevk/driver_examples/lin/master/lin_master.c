/*
 * Copyright  2016-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_port.h"
#include "fsl_lin.h"
#include "lin_cfg.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Whether the SW buttons use the separate IRQ handler */
#define DEMO_SW_USE_SEPARATE_HANDLER 1
#define LIN_CLOCK_NAME               kCLOCK_ScgSircClk
#define TJA_WAKEUP                   1
#define TIMER_TPM                    1
/* Whether to disable the PrintBuffer function */
#define DISABLE_PRINT_BUFFER 0
#define DEMO_TPM_BASEADDR    TPM0
#define DEMO_TPM_IRQn        TPM0_IRQn
#define DEMO_TPM_IRQHandler  TPM0_IRQHandler
#define DEMO_TPM_CLOCK       kCLOCK_Tpm0
/* timer frequency */
#define TIMER_FREQ CLOCK_GetFreq(kCLOCK_ScgSircClk)
/* (timer period (us) * (timer clock frequency)(Hz)) - 1 ) */
#define MODULO_VALUE ((500U * (CLOCK_GetFreq(kCLOCK_ScgSircClk) / 1000000U)) - 1U)
/* nanoseconds / timer clock frequency  */
#define TIMER_1TICK_DURATION_PS (1000000000000U / TIMER_FREQ)

#define DEMO_LIN_IRQHandler LPUART0_IRQHandler

#define LI0_Master 0x0

#define DEMO_BUTTON1_GPIO BOARD_SW3_GPIO
#define DEMO_BUTTON1_PORT BOARD_SW3_PORT
#define DEMO_BUTTON1_PIN  BOARD_SW3_GPIO_PIN
#define DEMO_BUTTON2_GPIO BOARD_SW2_GPIO
#define DEMO_BUTTON2_PORT BOARD_SW2_PORT
#define DEMO_BUTTON2_PIN  BOARD_SW2_GPIO_PIN
#define DEMO_LED_GPIO     BOARD_LED1_GPIO
#define DEMO_LED_PIN      BOARD_LED1_GPIO_PIN
#define DEMO_SLPN_GPIO    GPIOC
#define DEMO_SLPN_PIN     7
#define DEMO_LED_ON()     LED1_ON()
#define DEMO_LED_OFF()    LED1_OFF()

#define DEMO_SW2_IRQn        BOARD_SW2_IRQ
#define DEMO_SW2_IRQ_HANDLER BOARD_SW2_IRQ_HANDLER
#define DEMO_SW3_IRQn        BOARD_SW3_IRQ
#define DEMO_SW3_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
#define MASTER_INSTANCE   LI0_Master
#define HARDWARE_INSTANCE 0U

/* make PID from ID */
#define MAKE_PID(id) LIN_ProcessParity(id, MAKE_PARITY)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if !DISABLE_PRINT_BUFFER
/*!
 * This function prints response content
 * @param instance LIN instance
 * @param id frame ID
 */
static void PrintBuffer(uint8_t instance, uint8_t id);
#endif

/*!
 * This timer returns the count of nanoseconds between two consequencing bits.
 * @param ns number of nanoseconds between two bits, return parameter
 */
void timerGetTimeIntervalCallback0(uint32_t *ns);
/*!
 * This function handles messages from low level
 * @param instance LIN instance
 * @param linState pointer to current lin state
 */
void CallbackHandler(uint32_t instance, void *linState);
/*!
 * This function initializes master node and low level
 * @param linUserConfig pointer to user configuration structure
 * @param linCurrentState pointer to LIN low level state structure
 * @param clockSource LIN clock frequency
 */
lin_status_t MasterInit(lin_user_config_t *linUserConfig, lin_state_t *linCurrentState, uint32_t clockSource);
/*!
 * This function switches the schedule table
 */
void MasterScheduleTick(void);
/*!
 * This function process a frame id. If node is publisher, response is sent. If node is subscriber, response is awaited.
 * @param instance LIN instance
 * @param id frame ID
 */
void MasterProcessId(uint8_t instance, uint8_t id);
/*!
 * This function set bus activity timeout
 * @param instance LIN instance
 */
void MasterTimeoutService(uint8_t instance);
/*!
 * This function handle error raised by low-level
 * @param instance LIN instance
 * @param event_id ID of LIN bus event
 * @param id frame ID
 */
void MasterHandleError(uint8_t instance, lin_event_id_t event_id, uint8_t id);
/*!
 * Updates signal, status, and flag after response is sent.
 * @param instance LIN instance
 * @param id frame ID
 */
void MasterUpdateTx(uint8_t instance, uint8_t id);
/*!
 * This function update signal, status, and flag after response is received.
 * @param instance LIN instance
 * @param id frame ID
 */
void MasterUpdateRx(uint8_t instance, uint8_t id);
/*!
 * This function update tx flag array after response is sent.
 * @param instance LIN instance
 * @param id frame ID
 */
void MasterUpdateTxFlags(uint8_t instance, uint8_t id);
/*!
 * This function makes or updates unconditional frames.
 * @param instance LIN instance
 * @param id frame ID
 * @param type type of action
 */
void MasterProcesUncdFrame(uint8_t instance, uint8_t id, uint8_t type);
/*!
 * This function returns index of array ID from RAM table.
 * @param instance LIN instance
 * @param id frame ID
 */
static inline uint8_t MasterGetFrameIndex(uint8_t instance, uint8_t id);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* storage for timer counter */
uint16_t timerCounterValue[2] = {0u};
/* number of timer overflow */
uint16_t timerOverflowInterruptCount = 0u;
/* buffer handling messages between lower and higher level communication */
static uint8_t g_linResponseBuffer[LIN_NUM_OF_IFCS][10];
/* maximal header duration time */
static volatile uint16_t linMaxHeaderTimeoutVal[LIN_NUM_OF_IFCS];
/* maximal response duration time */
static uint16_t linMaxResTimeoutVal[LIN_NUM_OF_IFCS];
/*******************************************************************************
 * Code
 ******************************************************************************/
static inline uint8_t MasterGetFrameIndex(uint8_t instance, uint8_t id)
{
    uint8_t retVal = 0xFF;
    uint8_t i;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];

    for (i = prot_user_config_ptr->number_of_configurable_frames; i > 0U; i--)
    {
        if (prot_user_config_ptr->list_identifiers_RAM_ptr[i] == id)
        {
            retVal = i + prot_user_config_ptr->frame_start - 1U;
            break;
        }
    }
    return retVal;
}

#if !DISABLE_PRINT_BUFFER
static void PrintBuffer(uint8_t instance, uint8_t id)
{
    uint8_t i = 0;
    uint8_t frame_index;
    uint8_t frame_len;
    char c;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* get index of frame in RAM table */
    frame_index = MasterGetFrameIndex(instance, id);
    /* get length of frame in RAM table */
    frame_len = prot_user_config_ptr->frame_tbl_ptr[frame_index].frm_len;
    /* print whole response from buffer */
    while (i < frame_len)
    {
        c = prot_state_ptr->response_buffer_ptr[i++];
        PRINTF("%c", c);
    }
    PRINTF("\r\n");
}
#endif

/*!
 * This interrupt routine handles LIN bus low level communication
 */
void DEMO_LIN_IRQHandler(void)
{
    LIN_IRQHandler(MASTER_INSTANCE);
    SDK_ISR_EXIT_BARRIER;
}

#if (defined(DEMO_SW_USE_SEPARATE_HANDLER) && DEMO_SW_USE_SEPARATE_HANDLER)
/*!
 * This interrupt routine puts a node sends wakeup signal on button press
 */
void DEMO_SW2_IRQ_HANDLER(void)
{
    /* Get external interrupt flags */
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    if (GPIO_GpioGetInterruptFlags(DEMO_BUTTON2_GPIO) & (1U << DEMO_BUTTON2_PIN))
#else
    if (GPIO_PortGetInterruptFlags(DEMO_BUTTON2_GPIO) & (1U << DEMO_BUTTON2_PIN))
#endif
    {
        /* Clear external interrupt flag. */
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
        GPIO_GpioClearInterruptFlags(DEMO_BUTTON2_GPIO, 1U << DEMO_BUTTON2_PIN);
#else
        GPIO_PortClearInterruptFlags(DEMO_BUTTON2_GPIO, 1U << DEMO_BUTTON2_PIN);
#endif
        /* Send a wakeup signal */
        LIN_SendWakeupSignal(MASTER_INSTANCE);
        /* Turn on LED. */
        DEMO_LED_ON();
    }
    else
    {
        ;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * This interrupt routine puts a node into sleep mode on button press
 */
void DEMO_SW3_IRQ_HANDLER(void)
{
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Get external interrupt flags */
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    if (GPIO_GpioGetInterruptFlags(DEMO_BUTTON1_GPIO) & (1U << DEMO_BUTTON1_PIN))
#else
    if (GPIO_PortGetInterruptFlags(DEMO_BUTTON1_GPIO) & (1U << DEMO_BUTTON1_PIN))
#endif
    {
        /* Clear external interrupt flag. */
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
        GPIO_GpioClearInterruptFlags(DEMO_BUTTON1_GPIO, 1U << DEMO_BUTTON1_PIN);
#else
        GPIO_PortClearInterruptFlags(DEMO_BUTTON1_GPIO, 1U << DEMO_BUTTON1_PIN);
#endif
        /* Turn off LED. */
        DEMO_LED_OFF();
        /* Update goto_sleep flag */
        prot_state_ptr->go_to_sleep_flg = (bool)1U;
    }
    else
    {
        ;
    }
    SDK_ISR_EXIT_BARRIER;
}
#else
/*!
 * This interrupt routine puts a node into sleep mode or sends wakeup signal on button press
 */
void DEMO_SW_IRQ_HANDLER(void)
{
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Get external interrupt flags */
    if (GPIO_PortGetInterruptFlags(DEMO_BUTTON1_GPIO) & (1U << DEMO_BUTTON1_PIN))
    {
        /* Clear external interrupt flag. */
        GPIO_PortClearInterruptFlags(DEMO_BUTTON1_GPIO, 1U << DEMO_BUTTON1_PIN);
        /* Turn off LED. */
        DEMO_LED_OFF();
        /* Update goto_sleep flag */
        prot_state_ptr->go_to_sleep_flg = (bool)1U;
    }
    else if (GPIO_PortGetInterruptFlags(DEMO_BUTTON2_GPIO) & (1U << DEMO_BUTTON2_PIN))
    {
        /* Clear external interrupt flag. */
        GPIO_PortClearInterruptFlags(DEMO_BUTTON2_GPIO, 1U << DEMO_BUTTON2_PIN);
        /* Send a wakeup signal */
        LIN_SendWakeupSignal(MASTER_INSTANCE);
        /* Turn on LED. */
        DEMO_LED_ON();
    }
    else
    {
        ;
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

#if defined(TIMER_TPM) && TIMER_TPM
/*!
 * This interrupt routine checks for bus timeout and switches schedule table
 */
void DEMO_TPM_IRQHandler(void)
{
    static volatile uint32_t tick_count = 0U;

    /* if timer overflow flag */
    if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & kTPM_TimeOverflowFlag)
    {
        /* Clear interrupt flag.*/
        TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, kTPM_TimeOverflowFlag);
        /* increase number of overflow count */
        timerOverflowInterruptCount++;
        /* check timeout */
        MasterTimeoutService(MASTER_INSTANCE);

        /* reload schedule every 5ms */
        if (++tick_count >= 10U)
        {
            tick_count = 0U;
            MasterScheduleTick();
        }
        else
        {
            ;
        }
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

#if defined(TIMER_TPM) && TIMER_TPM
void timerGetTimeIntervalCallback0(uint32_t *ns)
{
    uint16_t moduloValue = MODULO_VALUE;

    /* check current CNT value */
    timerCounterValue[1] = (uint16_t)(DEMO_TPM_BASEADDR->CNT);
    /* calculate number of ns from current and previous count value */
    if (timerCounterValue[1] >= timerCounterValue[0])
    {
        *ns = (uint32_t)((uint32_t)(((timerCounterValue[1] - timerCounterValue[0]) * TIMER_1TICK_DURATION_PS) / 1000U) +
                         (timerOverflowInterruptCount * 500000U));
    }
    else /* (timerCounterValue[1]<timerCounterValue[0]) */
    {
        *ns =
            (uint32_t)(((moduloValue - timerCounterValue[0] + timerCounterValue[1]) * TIMER_1TICK_DURATION_PS) / 1000U);
        if (timerOverflowInterruptCount > 0)
        {
            *ns += (uint32_t)((timerOverflowInterruptCount - 1) * 500000U);
        }
    }

    /* set current count value to previous count value */
    timerCounterValue[0] = timerCounterValue[1];
    /* clear timerOverflowInterruptCount mark */
    timerOverflowInterruptCount = 0U;
}
#endif

void CallbackHandler(uint32_t instance, void *linState)
{
    lin_state_t *linCurrentState = (lin_state_t *)linState;
    uint8_t bytesRemaining;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* check timeout flag */
    if (linCurrentState->timeoutCounterFlag == true)
    {
        /* set timeout error event id */
        linCurrentState->currentEventId     = LIN_TIMEOUT_ERROR;
        linCurrentState->timeoutCounterFlag = false;
    }
    /* check event id */
    switch (linCurrentState->currentEventId)
    {
        case LIN_PID_OK:
            /* process PID of frame */
            MasterProcessId(instance, linCurrentState->currentId);
            break;
        case LIN_RECV_BREAK_FIELD_OK:
            /* reload frame timeout */
            prot_state_ptr->frame_timeout_cnt =
                linMaxResTimeoutVal[HARDWARE_INSTANCE] + linMaxHeaderTimeoutVal[HARDWARE_INSTANCE];
            break;
        case LIN_TX_COMPLETED:
            /* update protocol state */
            MasterUpdateTx(instance, linCurrentState->currentId);
            break;
        case LIN_RX_COMPLETED:
            /* update protocol state */
            MasterUpdateRx(instance, linCurrentState->currentId);
            break;
        case LIN_BUS_ACTIVITY_TIMEOUT:
        case LIN_TIMEOUT_ERROR:
            /* check for remaining bytes */
            (void)LIN_GetReceiveStatus(instance, &bytesRemaining);
            if (bytesRemaining > 0U)
            {
                /* report timeout to higher level */
                MasterHandleError(instance, LIN_NO_DATA_TIMEOUT, linCurrentState->currentId);
            }
            break;
        case LIN_PID_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_SYNC_ERROR:
            /* report error to higher level */
            MasterHandleError(instance, linCurrentState->currentEventId, linCurrentState->currentId);
            break;
        case LIN_WAKEUP_SIGNAL:
        case LIN_NO_EVENT:
        case LIN_SYNC_OK:
        default:
            /* do nothing */
            break;
    }
    /* update idle timeout */
    prot_state_ptr->idle_timeout_cnt = prot_user_config_ptr->max_idle_timeout_cnt;
}

lin_status_t MasterInit(lin_user_config_t *linUserConfig, lin_state_t *linCurrentState, uint32_t clockSource)
{
    lin_status_t status = LIN_SUCCESS;
    /* initialize protocol variables */
    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    if (clockSource == 0)
    {
        /* with no clock report error */
        status = LIN_ERROR;
    }
    else
    {
        /* initialize LIN low lovel */
        status = LIN_Init(MASTER_INSTANCE, linUserConfig, linCurrentState, clockSource);
        /* make hardware instance visible to lower level */
        g_linUserconfigPtr[MASTER_INSTANCE]->hardware_instance = HARDWARE_INSTANCE;
        /* register time interval callback */
        linUserConfig->timerGetTimeIntervalCallback = &timerGetTimeIntervalCallback0;
        /* install callback service */
        LIN_InstallCallback(MASTER_INSTANCE, CallbackHandler);

        prot_state_ptr->baud_rate           = linUserConfig->baudRate;
        prot_state_ptr->response_buffer_ptr = g_linResponseBuffer[HARDWARE_INSTANCE];
        prot_state_ptr->idle_timeout_cnt    = prot_user_config_ptr->max_idle_timeout_cnt;

        linMaxResTimeoutVal[HARDWARE_INSTANCE]    = LIN_CalcMaxResTimeoutCnt(prot_state_ptr->baud_rate, 8U);
        prot_state_ptr->frame_timeout_cnt         = linMaxResTimeoutVal[HARDWARE_INSTANCE];
        linMaxHeaderTimeoutVal[HARDWARE_INSTANCE] = LIN_CalcMaxHeaderTimeoutCnt(prot_state_ptr->baud_rate);
    }
    return status;
}

void MasterScheduleTick(void)
{
    lin_status_t status = LIN_SUCCESS;
    uint8_t current_id;
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];
    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_state_t *linCurrentState                           = g_linStatePtr[MASTER_INSTANCE];
    static uint32_t prev_error_in_transfer                 = 0U;
    static int curr_id_index                               = 1;
    static int32_t frame_delay_count                       = 0;

    /* check if frame delay runs out */
    if (frame_delay_count <= 0)
    {
        /* check for go to sleep flag */
        if (prot_state_ptr->go_to_sleep_flg == (bool)1U)
        {
            /* go to sleep mode */
            LIN_GoToSleepMode(MASTER_INSTANCE);
            /* clear sleep mode flag */
            prot_state_ptr->go_to_sleep_flg = (bool)0U;
        }
        else if (linCurrentState->currentNodeState != LIN_NODE_STATE_SLEEP_MODE)
        {
            /* check for error occurred during last transfer */
            if ((prot_state_ptr->error_in_response != prev_error_in_transfer))
            {
                /* if previous transfer was not succesfull, try again */
                if (curr_id_index > 1U)
                {
                    curr_id_index--;
                }
            }
            /* get frame id */
            current_id = prot_user_config_ptr->list_identifiers_RAM_ptr[curr_id_index];

            if (0xFFU != current_id)
            {
                /* get delay of frame to be sent */
                frame_delay_count = prot_user_config_ptr->frame_tbl_ptr[curr_id_index - 1].delay;
                prot_state_ptr->frame_timeout_cnt =
                    linMaxResTimeoutVal[HARDWARE_INSTANCE] + linMaxHeaderTimeoutVal[HARDWARE_INSTANCE];
                /* send a frame header */
                status = LIN_MasterSendHeader(MASTER_INSTANCE, current_id);
                /* check if was succesfull */
                if (status != LIN_SUCCESS)
                {
                    /* go to sleep mode */
                    LIN_GoToSleepMode(MASTER_INSTANCE);
                }
                prev_error_in_transfer = prot_state_ptr->error_in_response;
            }
            curr_id_index++;
            /* first and last index of RAM_configuration list are not valid */
            if (curr_id_index >= prot_user_config_ptr->number_of_configurable_frames)
            {
                curr_id_index = 1;
            }
        }
        else
        {
            ; /* do nothing */
        }
    }
    else
    {
        /* decrease frame delay count */
        frame_delay_count--;
    }
}

void MasterProcessId(uint8_t instance, uint8_t id)
{
    uint8_t frame_index;
    uint32_t response_length;
    uint32_t lin_max_frame_res_timeout_val;
    const lin_frame_struct *lin_frame_ptr;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* get index of current frame from RAM table */
    frame_index = MasterGetFrameIndex(instance, id);

    if (frame_index != 0xFF)
    {
        prot_state_ptr->go_to_sleep_flg = (bool)0U;
        /* get frame buffer pointer */
        lin_frame_ptr = &(prot_user_config_ptr->frame_tbl_ptr[frame_index]);
        /* check if id represents a supported frame */
        if (id <= 0x3BU)
        {
            response_length               = lin_frame_ptr->frm_len;
            lin_max_frame_res_timeout_val = LIN_CalcMaxResTimeoutCnt(prot_state_ptr->baud_rate, response_length);

            /* check response type */
            if (LIN_RES_PUB == lin_frame_ptr->frm_response)
            {
                /* make unconditional frame */
                MasterProcesUncdFrame(instance, id, LIN_MAKE_UNCONDITIONAL_FRAME);

                if ((response_length <= 8U) && (response_length > 0U))
                {
                    /* Set response */
                    LIN_SetResponse(instance, &(prot_state_ptr->response_buffer_ptr[0]), response_length,
                                    lin_max_frame_res_timeout_val);
                }
            }
            else
            {
                if ((response_length <= 8U) && (response_length != 0U))
                {
                    /* wait for response */
                    LIN_RxResponse(instance, &(prot_state_ptr->response_buffer_ptr[0]), response_length,
                                   lin_max_frame_res_timeout_val);
                }
            }
        }
        /* frame is not supported */
        else
        {
            /* ignore frame */
            LIN_IgnoreResponse(instance);
        }
    }
    /* unknown id */
    else
    {
        /* ignore frame */
        LIN_IgnoreResponse(instance);
    }
}

void MasterProcesUncdFrame(uint8_t instance, uint8_t id, uint8_t type)
{
    uint8_t frame_index;
    uint8_t *response_buffer_ptr;
    uint8_t i;
    uint8_t frame_byte_offset;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* get index of current frame from RAM table */
    frame_index = MasterGetFrameIndex(instance, id);
    /* get protocol reponse buffer */
    if (frame_index != 0xFF)
    {
        response_buffer_ptr = prot_state_ptr->response_buffer_ptr;
        /* get protocolo reponse buffer lenght */
        prot_state_ptr->response_length = prot_user_config_ptr->frame_tbl_ptr[frame_index].frm_len;
        /* get protocolo reponse buffer offset */
        frame_byte_offset = prot_user_config_ptr->frame_tbl_ptr[frame_index].frm_offset;

        /* check for frame type */
        if (LIN_MAKE_UNCONDITIONAL_FRAME == type)
        {
            /* Get data from configuration buffer */
            for (i = 0U; i < prot_state_ptr->response_length; i++)
            {
                response_buffer_ptr[i] = g_lin_frame_data_buffer[frame_byte_offset + i];
            }
        }
        /* update unconditional frame */
        else
        {
            for (i = 0U; i < prot_state_ptr->response_length; i++)
            {
                /* Set data of configuration buffer */
                g_lin_frame_data_buffer[frame_byte_offset + i] = response_buffer_ptr[i];
            }
        }
    }
}

void MasterUpdateTx(uint8_t instance, uint8_t id)
{
    uint8_t frame_index;
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Set successful transfer */
    prot_state_ptr->successful_transfer = 1U;

    if (prot_state_ptr->num_of_processed_frame < 0xFFU)
    {
        /* increase a number of processed frames */
        prot_state_ptr->num_of_processed_frame++;
    }
    /* Check if frame contains error signal */
    if ((bool)1U == prot_state_ptr->transmit_error_resp_sig_flg)
    {
        /* Set error in response */
        prot_state_ptr->error_in_response = 0U;
        /* clear error flag */
        prot_state_ptr->transmit_error_resp_sig_flg = (bool)0U;
    }
    else
    {
        /* increase a number of succesfull frames */
        prot_state_ptr->num_of_successfull_frame++;
    }
#if !DISABLE_PRINT_BUFFER
    /* print received response */
    PrintBuffer(instance, id);
#endif
    /* get index of current frame from RAM table */
    frame_index = MasterGetFrameIndex(instance, id);
    /* update Tx flags */
    MasterUpdateTxFlags(instance, frame_index);
}

void MasterUpdateRx(uint8_t instance, uint8_t id)
{
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Set successful transfer */
    prot_state_ptr->successful_transfer = 1U;

    if (prot_state_ptr->num_of_processed_frame < 0xFFU)
    {
        /* increase a number of processed frames */
        prot_state_ptr->num_of_processed_frame++;
    }
    /* check if id represents a supported frame */
    if (id <= 0x3BU)
    {
        /* make unconditional frame */
        MasterProcesUncdFrame(instance, id, LIN_UPDATE_UNCONDITIONAL_FRAME);
        /* increase a number of succesfull frames */
        prot_state_ptr->num_of_successfull_frame++;
#if !DISABLE_PRINT_BUFFER
        /* print received response */
        PrintBuffer(instance, id);
#endif
    }
    else
    {
        ;
    }
}

void MasterUpdateTxFlags(uint8_t instance, uint8_t id)
{
    uint8_t flag_offset;
    uint8_t flag_size;
    uint8_t i;
    const lin_frame_struct *lin_frame_ptr;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_frame_ptr                                          = &(prot_user_config_ptr->frame_tbl_ptr[id]);

    if (LIN_FRM_UNCD == lin_frame_ptr->frm_type)
    {
        flag_offset = lin_frame_ptr->flag_offset;
        flag_size   = lin_frame_ptr->flag_size;
        /* Update transmit flags */
        for (i = 0U; i < flag_size; i++)
        {
            g_lin_flag_handle_tbl[flag_offset] = 0x00U;
            flag_offset++;
        }
    }
}

void MasterTimeoutService(uint8_t instance)
{
    lin_state_t *linState                                  = g_linStatePtr[MASTER_INSTANCE];
    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* check if timeout occurs during communication */
    LIN_TimeoutService(instance);

    /* check for current node state */
    switch (linState->currentNodeState)
    {
        case LIN_NODE_STATE_IDLE:
            /* check if idle timeout runs out */
            if (!(prot_state_ptr->idle_timeout_cnt-- > 0U))
            {
                /* Set go to sleep flag */
                prot_state_ptr->go_to_sleep_flg = (bool)1U;

                /* Put current node to Idle state, reset idle timeout count */
                prot_state_ptr->idle_timeout_cnt = prot_user_config_ptr->max_idle_timeout_cnt;

                /* Put current node to sleep mode */
                (void)LIN_GoToSleepMode(instance);
            }
            break;
        case LIN_NODE_STATE_RECV_PID:
        case LIN_NODE_STATE_SEND_PID:
        case LIN_NODE_STATE_RECV_SYNC:
            /* timeout send has occurred - change state of the node and inform core */
            if (!(prot_state_ptr->frame_timeout_cnt-- > 0U))
            {
                /* Go to idle state */
                (void)LIN_GotoIdleState(instance);

                /* Reset frame count timeout */
                prot_state_ptr->frame_timeout_cnt = linMaxResTimeoutVal[HARDWARE_INSTANCE];
            }
            break;
        case LIN_NODE_STATE_SEND_DATA:
        case LIN_NODE_STATE_SEND_DATA_COMPLETED:
            /* timeout send has occurred - change state of the node and inform core */
            if (!(prot_state_ptr->frame_timeout_cnt-- > 0U))
            {
                /* Abort frame data transferring */
                (void)LIN_AbortTransferData(instance);

                /* Reset frame count timeout */
                prot_state_ptr->frame_timeout_cnt = linMaxResTimeoutVal[HARDWARE_INSTANCE];
            }
            break;
        case LIN_NODE_STATE_UNINIT:
        case LIN_NODE_STATE_SLEEP_MODE:
        case LIN_NODE_STATE_SEND_BREAK_FIELD:
        case LIN_NODE_STATE_RECV_DATA:
        case LIN_NODE_STATE_RECV_DATA_COMPLETED:
        default:
            /* do nothing */
            break;
    }
}

void MasterHandleError(uint8_t instance, lin_event_id_t event_id, uint8_t id)
{
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* increase a number of processed frames */
    if (prot_state_ptr->num_of_processed_frame < 0xFFU)
    {
        prot_state_ptr->num_of_processed_frame++;
    }

    switch (event_id)
    {
        /* PID error */
        case LIN_PID_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_SYNC_ERROR:
            /* Set response error */
            prot_state_ptr->error_in_response += 1U;
            break;
        case LIN_NO_DATA_TIMEOUT:
            /* Set timeout error */
            prot_state_ptr->timeout_in_response += 1U;
            break;
        default:
            /* do nothing */
            break;
    }
}

/*!
 * @brief Application entry point.
 */
int main(void)
{
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
    };
    gpio_pin_config_t slpn_config = {
        kGPIO_DigitalOutput,
        1,
    };

    tpm_config_t tpmInfo;
    lin_user_config_t linUserConfigMaster;
    lin_state_t linCurrentState;
    uint32_t linfreq;
    lin_status_t status;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_DebugConsolePins();
    BOARD_InitLeds();
    BOARD_InitButtons();
    BOARD_InitLinLpuart();
    /* Set LIN LPUART clock */
    CLOCK_SetIpSrc(kCLOCK_Lpuart0, kCLOCK_IpSrcFro6M);
#if defined(TIMER_TPM) && TIMER_TPM
    /* Set Timer LPUART clock */
    CLOCK_SetIpSrc(DEMO_TPM_CLOCK, kCLOCK_IpSrcFro6M);
#endif
    BOARD_InitDebugConsole();

    /*
     * .linUserConfigMaste.autobaudEnable = false;
     * .linUserConfigMaster.baudRate = 19200;
     * .linUserConfigMaster.nodeFunction = MASTER;
     * .linUserConfigMaster.timerGetTimeIntervalCallback = NULL;
     */

    LIN_GetMasterDefaultConfig(&linUserConfigMaster);
    /* get LIN clock frequency */
    linfreq = CLOCK_GetFreq(LIN_CLOCK_NAME);

#if defined(TIMER_TPM) && TIMER_TPM
    TPM_GetDefaultConfig(&tpmInfo);
    /* Initialize TPM module */
    TPM_Init(DEMO_TPM_BASEADDR, &tpmInfo);
    /* Set module value */
    DEMO_TPM_BASEADDR->MOD = MODULO_VALUE;
    /* Enable interrupt on overflow */
    TPM_EnableInterrupts(DEMO_TPM_BASEADDR, kTPM_TimeOverflowInterruptEnable);
    /* Enable at the NVIC */
    EnableIRQ(DEMO_TPM_IRQn);
#endif

    if (TIMER_FREQ == 0U)
    {
        PRINTF("\r\n Timer initialization failed!");
        return -1;
    }
#if defined(TJA_WAKEUP) && TJA_WAKEUP
    /* Wakeup TJA transceiver */
    GPIO_PinInit(DEMO_SLPN_GPIO, DEMO_SLPN_PIN, &slpn_config);
#endif
    /* Initialize LEDs */
    GPIO_PinInit(DEMO_LED_GPIO, DEMO_LED_PIN, &led_config);
    /* Initialize buttons */

#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    GPIO_SetPinInterruptConfig(DEMO_BUTTON1_GPIO, DEMO_BUTTON1_PIN, kGPIO_InterruptFallingEdge);
    GPIO_SetPinInterruptConfig(DEMO_BUTTON2_GPIO, DEMO_BUTTON2_PIN, kGPIO_InterruptFallingEdge);
#else
    PORT_SetPinInterruptConfig(DEMO_BUTTON1_PORT, DEMO_BUTTON1_PIN, kPORT_InterruptFallingEdge);
    PORT_SetPinInterruptConfig(DEMO_BUTTON2_PORT, DEMO_BUTTON2_PIN, kPORT_InterruptFallingEdge);
#endif

    GPIO_PinInit(DEMO_BUTTON1_GPIO, DEMO_BUTTON1_PIN, &sw_config);
    GPIO_PinInit(DEMO_BUTTON2_GPIO, DEMO_BUTTON2_PIN, &sw_config);

#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    GPIO_GpioClearInterruptFlags(DEMO_BUTTON1_GPIO, 1U << DEMO_BUTTON1_PIN);
    GPIO_GpioClearInterruptFlags(DEMO_BUTTON2_GPIO, 1U << DEMO_BUTTON2_PIN);
#else
    GPIO_PortClearInterruptFlags(DEMO_BUTTON1_GPIO, 1U << DEMO_BUTTON1_PIN);
    GPIO_PortClearInterruptFlags(DEMO_BUTTON2_GPIO, 1U << DEMO_BUTTON2_PIN);
#endif

#if (defined(DEMO_SW_USE_SEPARATE_HANDLER) && DEMO_SW_USE_SEPARATE_HANDLER)
    /* Enable at the NVIC */
    EnableIRQ(DEMO_SW2_IRQn);
    EnableIRQ(DEMO_SW3_IRQn);
#else
    EnableIRQ(DEMO_SW_IRQn);
#endif
    /* Initialize master node */
    status = MasterInit(&linUserConfigMaster, &linCurrentState, linfreq);

    if (status != LIN_SUCCESS)
    {
        PRINTF("\r\n LIN initialization failed!");
        return -1;
    }
    PRINTF("\r\n LIN master is initialized");
    /* Sleep until SW2 is pressed */
    LIN_GoToSleepMode(MASTER_INSTANCE);
    if (linCurrentState.currentNodeState != LIN_NODE_STATE_SLEEP_MODE)
    {
        PRINTF("\r\n LIN sleep command failed!");
        return -1;
    }
    PRINTF("\r\n Awaiting SW2 press!");
#if defined(TIMER_TPM) && TIMER_TPM
    /* Start scheduler */
    TPM_StartTimer(DEMO_TPM_BASEADDR, kTPM_SystemClock);
#endif

    for (;;)
    {                 /* Infinite loop to avoid leaving the main function */
        __asm("NOP"); /* something to use as a breakpoint stop while looping */
    }
}
