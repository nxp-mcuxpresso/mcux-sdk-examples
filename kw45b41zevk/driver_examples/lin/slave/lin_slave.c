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
#include "fsl_lin.h"
#include "lin_cfg.h"
#include "fsl_debug_console.h"
#include "fsl_tpm.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LIN_CLOCK_NAME kCLOCK_ScgSircClk
#define TJA_WAKEUP     1
#define TIMER_TPM      1
/* Whether to disable the PrintBuffer function */
#define DISABLE_PRINT_BUFFER 0

#define DEMO_TPM_BASEADDR   TPM0
#define DEMO_TPM_IRQn       TPM0_IRQn
#define DEMO_TPM_IRQHandler TPM0_IRQHandler
#define DEMO_TPM_CLOCK      kCLOCK_Tpm0
#define DEMO_TPM_CH_OUT_NUM 0
#define DEMO_TPM_CH_OUT     kTPM_Chnl_0
#define DEMO_TPM_CH_IN      kTPM_Chnl_3
#define DEMO_TPM_CH_OUT_FLG kTPM_Chnl0Flag
#define DEMO_TPM_CH_IN_FLG  kTPM_Chnl3Flag
#define DEMO_TPM_CH_OUT_IRQ kTPM_Chnl0InterruptEnable
#define DEMO_TPM_CH_IN_IRQ  kTPM_Chnl3InterruptEnable
/* timer frequency */
#define TIMER_FREQ CLOCK_GetFreq(kCLOCK_ScgSircClk)
/* (timer period (us) * (timer clock frequency)(Hz)) - 1 ) */
#define MODULO_VALUE ((500U * (CLOCK_GetFreq(kCLOCK_ScgSircClk) / 1000000U)) - 1U)
/* nanoseconds / timer clock frequency  */
#define TIMER_1TICK_DURATION_PS (1000000000000U / TIMER_FREQ)

#define DEMO_LIN_IRQn       LPUART0_IRQn
#define DEMO_LIN_IRQHandler LPUART0_IRQHandler

#define DEMO_SLPN_GPIO GPIOC
#define DEMO_SLPN_PIN  7
#define LI0_Slave      0x0

#define SLAVE_INSTANCE    LI0_Slave
#define HARDWARE_INSTANCE 0U

/* auto-baudrate feature requires use of very high interrupt priority for LPUART and timer */
#define DEMO_LIN_PRIO   1U
#define DEMO_TIMER_PRIO (DEMO_LIN_PRIO + 1U)
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
 * @param linState pointer to current LIN state
 */
void CallbackHandler(uint32_t instance, void *linState);
/*!
 * This function initializes master node and low level
 * @param linUserConfig pointer to user configuration structure
 * @param linCurrentState pointer to LIN low level state structure
 * @param clockSource LIN clock frequency
 */
lin_status_t SlaveInit(lin_user_config_t *linUserConfig, lin_state_t *linCurrentState, uint32_t clockSource);
/*!
 * This function calculates register values for supported baudrates
 * @param instance LIN instance
 * @param clocksSource LIN clock frequency
 * @param baudRatesVals pointer to baudrate values
 */
void CalculateBaudrates(uint32_t instance, uint32_t clocksSource, lin_baudrate_values_t *baudRatesVals);
/*!
 * This function process a frame id. If node is publisher, response is sent. If node is subscriber, response is awaited.
 * @param instance LIN instance
 * @param id frame ID
 */
void SlaveProcessId(uint8_t instance, uint8_t id);
/*!
 * This function set bus activity timeout
 * @param instance LIN instance
 */
void SlaveTimeoutService(uint8_t instance);
/*!
 * This function handle error raised by low-level
 * @param instance LIN instance
 * @param event_id ID of LIN bus event
 * @param id frame ID
 */
void SlaveHandleError(uint8_t instance, lin_event_id_t event_id, uint8_t id);
/*!
 * Updates signal, status, and flag after response is sent.
 * @param instance LIN instance
 * @param id frame ID
 */
void SlaveUpdateTx(uint8_t instance, uint8_t id);
/*!
 * This function update tx flag array after response is sent.
 * @param instance LIN instance
 * @param id frame ID
 */
void SlaveUpdateTxFlags(uint8_t instance, uint8_t id);
/*!
 * This function update signal, status, and flag after response is received.
 * @param instance LIN instance
 * @param id frame ID
 */
void SlaveUpdateRx(uint8_t instance, uint8_t id);
/*!
 * This function makes or updates unconditional frames.
 * @param instance LIN instance
 * @param id frame ID
 * @param type type of action
 */
void SlaveProcesUncdFrame(uint8_t instance, uint8_t id, uint8_t type);
/*!
 * This function returns index of array ID from RAM table.
 * @param instance LIN instance
 * @param id frame ID
 */
static inline uint8_t SlaveGetFrameIndex(uint8_t instance, uint8_t id);
/*!
 * This function returns index to baudrate values array by given baudrate
 * @param baudrate LIN baudrate
 */
static inline uint32_t CheckIndex(uint32_t baudrate);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* storage for timer counter */
uint16_t timerCounterValue[2] = {0u};
/* number of timer overflow */
volatile uint32_t timerOverflowInterruptCount = 0u;
/* buffer handling messages between lower and higher level communication */
static uint8_t g_linResponseBuffer[LIN_NUM_OF_IFCS][10];
/* supported baudrate for autobaudrate feature */
uint32_t baudRateArray[LIN_NUM_OF_SUPP_BAUDRATES] = {2400, 4800, 9600, 14400, 19200};
lin_baudrate_values_t g_baudRatesValues[LIN_NUM_OF_SUPP_BAUDRATES];
/* maximal header duration time */
static volatile uint16_t linMaxHeaderTimeoutVal[LIN_NUM_OF_IFCS];
/* maximal response duration time */
static uint16_t linMaxResTimeoutVal[LIN_NUM_OF_IFCS];
/*******************************************************************************
 * Code
 ******************************************************************************/
static inline uint32_t CheckIndex(uint32_t baudrate)
{
    uint32_t retval;
    switch (baudrate)
    {
        case 2400:
            retval = (uint32_t)kLIN_BAUD_2400;
            break;
        case 4800:
            retval = (uint32_t)kLIN_BAUD_4800;
            break;
        case 9600:
            retval = (uint32_t)kLIN_BAUD_9600;
            break;
        case 14400:
            retval = (uint32_t)kLIN_BAUD_14400;
            break;
        case 19200:
            retval = (uint32_t)kLIN_BAUD_19200;
            break;
        default:
            retval = 0xFF;
            break;
    }
    return retval;
}

static inline uint8_t SlaveGetFrameIndex(uint8_t instance, uint8_t id)
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
    frame_index = SlaveGetFrameIndex(instance, id);
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
    LIN_IRQHandler(SLAVE_INSTANCE);
    SDK_ISR_EXIT_BARRIER;
}

#if (defined(TIMER_TPM) && TIMER_TPM)
/*!
 * This interrupt checks for communication timeout and falling edge detection.
 * This interrupt is also invoked every 500us on timer overflow.
 */
void DEMO_TPM_IRQHandler(void)
{
    uint32_t counterVal;
    if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & DEMO_TPM_CH_IN_FLG)
    {
        TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, DEMO_TPM_CH_IN_FLG);
        if (LIN_AutoBaudCapture(SLAVE_INSTANCE) == LIN_SUCCESS)
        {
            TPM_DisableInterrupts(DEMO_TPM_BASEADDR, DEMO_TPM_CH_IN_FLG);
        }
    }

    if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & DEMO_TPM_CH_OUT_FLG)
    {
        TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, DEMO_TPM_CH_OUT_FLG);
        counterVal                                             = (uint16_t)(DEMO_TPM_BASEADDR->CNT);
        (DEMO_TPM_BASEADDR->CONTROLS[DEMO_TPM_CH_OUT_NUM].CnV) = counterVal + MODULO_VALUE;
        SlaveTimeoutService(SLAVE_INSTANCE);
    }

    if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & kTPM_TimeOverflowFlag)
    {
        /* Clear interrupt flag.*/
        TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, kTPM_TimeOverflowFlag);
        /* increase number of overflow count */
        timerOverflowInterruptCount++;
    }
    SDK_ISR_EXIT_BARRIER;
}
#endif

#if defined(TIMER_TPM) && TIMER_TPM
void timerGetTimeIntervalCallback0(uint32_t *ns)
{
    uint16_t counterMaxValue      = 0xFFFFU;
    uint32_t currentOverflowCount = timerOverflowInterruptCount;

    /* check current CNT value */
    timerCounterValue[1] = (uint16_t)(DEMO_TPM_BASEADDR->CNT);
    /* calculate number of ns from current and previous count value */
    if (timerCounterValue[1] >= timerCounterValue[0])
    {
        /* Correction: Timer overflow interrupt should be delayed by other processes
         * if TOF is set, timer overflow occurred so increase the number of interrupt and clear a flag
         * */
        if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & kTPM_TimeOverflowFlag)
        {
            TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, kTPM_TimeOverflowFlag);
            currentOverflowCount++;
        }
        *ns = (uint32_t)(((uint32_t)((timerCounterValue[1] - timerCounterValue[0]) * TIMER_1TICK_DURATION_PS) / 1000U) +
                         (currentOverflowCount * counterMaxValue));
    }
    else /* (timerCounterValue[1]<timerCounterValue[0]) */
    {
        *ns = ((uint32_t)(((counterMaxValue - timerCounterValue[0] + timerCounterValue[1])) * TIMER_1TICK_DURATION_PS) /
               1000U);
        if (TPM_GetStatusFlags(DEMO_TPM_BASEADDR) & kTPM_TimeOverflowFlag)
        {
            TPM_ClearStatusFlags(DEMO_TPM_BASEADDR, kTPM_TimeOverflowFlag);
            currentOverflowCount++;
        }
        if (currentOverflowCount > 0U)
        {
            *ns += (uint32_t)((currentOverflowCount - 1U) * counterMaxValue);
        }
    }

    /* set current count value to previous count value */
    timerCounterValue[0] = timerCounterValue[1];
    /* clear timerOverflowInterruptCount mark */
    timerOverflowInterruptCount = 0u;
}
#endif

void CallbackHandler(uint32_t instance, void *linState)
{
    lin_state_t *linCurrentState     = (lin_state_t *)linState;
    lin_user_config_t *linUserConfig = g_linUserconfigPtr[instance];
    uint8_t bytesRemaining;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];
    uint32_t index;

    /* check timeout flag */
    if (linCurrentState->timeoutCounterFlag == (bool)1U)
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
            SlaveProcessId(instance, linCurrentState->currentId);
            break;
        case LIN_RECV_BREAK_FIELD_OK:
            /* reload frame timeout */
            prot_state_ptr->frame_timeout_cnt =
                linMaxResTimeoutVal[HARDWARE_INSTANCE] + linMaxHeaderTimeoutVal[HARDWARE_INSTANCE];
            break;
        case LIN_TX_COMPLETED:
            /* update protocol state */
            SlaveUpdateTx(instance, linCurrentState->currentId);
            break;
        case LIN_RX_COMPLETED:
            /* update protocol state */
            SlaveUpdateRx(instance, linCurrentState->currentId);
            break;
        case LIN_BUS_ACTIVITY_TIMEOUT:
        case LIN_TIMEOUT_ERROR:
            /* check for remaining bytes */
            (void)LIN_GetReceiveStatus(instance, &bytesRemaining);
            if (bytesRemaining > 0U)
            {
                /* report timeout to higher level */
                SlaveHandleError(instance, LIN_NO_DATA_TIMEOUT, linCurrentState->currentId);
            }
            break;
        case LIN_PID_ERROR:
        case LIN_FRAME_ERROR:
        case LIN_CHECKSUM_ERROR:
        case LIN_READBACK_ERROR:
        case LIN_SYNC_ERROR:
        case LIN_LAST_RESPONSE_SHORT_ERROR:
            /* report error to higher level */
            SlaveHandleError(instance, linCurrentState->currentEventId, linCurrentState->currentId);
            break;
        case LIN_BAUDRATE_ADJUSTED:
            index = CheckIndex(linUserConfig->baudRate);
            if (index != 0xFF)
            {
                LIN_SetBaudrate(SLAVE_INSTANCE, g_baudRatesValues[index].osrValue, g_baudRatesValues[index].sbrValue);
            }
            /* change protocol baudrate */
            prot_state_ptr->baud_rate = linUserConfig->baudRate;
            /* update protocol timeout values */
            prot_state_ptr->frame_timeout_cnt = LIN_CalcMaxResTimeoutCnt(prot_state_ptr->baud_rate, 8U);
            break;
        case LIN_WAKEUP_SIGNAL:
        case LIN_NO_EVENT:
        case LIN_SYNC_OK:
        case LIN_NO_DATA_TIMEOUT:
        default:
            /* do nothing */
            break;
    }
    prot_state_ptr->idle_timeout_cnt = prot_user_config_ptr->max_idle_timeout_cnt;
}

lin_status_t SlaveInit(lin_user_config_t *linUserConfig, lin_state_t *linCurrentState, uint32_t clockSource)
{
    lin_status_t status                                    = LIN_SUCCESS;
    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];
    /* initialize protocol variables */

    if (clockSource == 0)
    {
        /* with no clock report error */
        status = LIN_ERROR;
    }
    else
    {
        /* initialize LIN low lovel */
        status = LIN_Init(SLAVE_INSTANCE, linUserConfig, linCurrentState, clockSource);

        /* make hardware instance visible to lower level */
        linUserConfig->hardware_instance = HARDWARE_INSTANCE;
        /* register time interval callback */
        linUserConfig->timerGetTimeIntervalCallback = &timerGetTimeIntervalCallback0;
        /* install callback service */
        LIN_InstallCallback(SLAVE_INSTANCE, CallbackHandler);

        prot_state_ptr->baud_rate           = linUserConfig->baudRate;
        prot_state_ptr->response_buffer_ptr = g_linResponseBuffer[HARDWARE_INSTANCE];
        prot_state_ptr->idle_timeout_cnt    = prot_user_config_ptr->max_idle_timeout_cnt;

        if (linUserConfig->autobaudEnable == true)
        {
            /* in case of auto-baudrate feature */
            linMaxResTimeoutVal[HARDWARE_INSTANCE]    = LIN_CalcMaxResTimeoutCnt(2400U, 8U);
            prot_state_ptr->frame_timeout_cnt         = linMaxResTimeoutVal[HARDWARE_INSTANCE];
            linMaxHeaderTimeoutVal[HARDWARE_INSTANCE] = LIN_CalcMaxHeaderTimeoutCnt(2400U);
            /* Set the highest priority */
            NVIC_SetPriority(DEMO_LIN_IRQn, DEMO_LIN_PRIO);
        }
        else
        {
            /* if auto-baudrate feature is used, enhance timeout for sync field */
            linMaxResTimeoutVal[HARDWARE_INSTANCE]    = LIN_CalcMaxResTimeoutCnt(prot_state_ptr->baud_rate, 8U);
            prot_state_ptr->frame_timeout_cnt         = linMaxResTimeoutVal[HARDWARE_INSTANCE];
            linMaxHeaderTimeoutVal[HARDWARE_INSTANCE] = LIN_CalcMaxHeaderTimeoutCnt(prot_state_ptr->baud_rate);
        }
    }
    return status;
}

void CalculateBaudrates(uint32_t instance, uint32_t clocksSource, lin_baudrate_values_t *baudRatesVals)
{
    uint8_t sbrt;
    uint32_t tempBaudrate;
    uint32_t *osr;
    uint16_t *sbr;

    /* calculate OSR and SBR register valus for all supported baudrates for autobaudrate feature */
    for (sbrt = 0; sbrt < LIN_NUM_OF_SUPP_BAUDRATES; sbrt++)
    {
        tempBaudrate = baudRatesVals[sbrt].baudRate = baudRateArray[sbrt];
        sbr                                         = &baudRatesVals[sbrt].sbrValue;
        osr                                         = &baudRatesVals[sbrt].osrValue;
        LIN_CalculateBaudrate(instance, tempBaudrate, clocksSource, osr, sbr);
    }
}

void SlaveProcessId(uint8_t instance, uint8_t id)
{
    uint8_t frame_index;
    uint32_t response_length;
    uint32_t lin_max_frame_res_timeout_val;
    const lin_frame_struct *lin_frame_ptr;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* get index of current frame from RAM table */
    frame_index = SlaveGetFrameIndex(instance, id);

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
                SlaveProcesUncdFrame(instance, id, LIN_MAKE_UNCONDITIONAL_FRAME);

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

void SlaveProcesUncdFrame(uint8_t instance, uint8_t id, uint8_t type)
{
    uint8_t frame_index;
    uint8_t *response_buffer_ptr;
    uint8_t i;
    uint8_t frame_byte_offset;
    const lin_node_attribute *node_attr_ptr;

    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* get index of current frame from RAM table */
    frame_index = SlaveGetFrameIndex(instance, id);

    /* get protocol response buffer */
    if (frame_index != 0xFF)
    {
        response_buffer_ptr = prot_state_ptr->response_buffer_ptr;
        /* get protocol response buffer length */
        prot_state_ptr->response_length = prot_user_config_ptr->frame_tbl_ptr[frame_index].frm_len;
        /* get protocol response buffer offset */
        frame_byte_offset = prot_user_config_ptr->frame_tbl_ptr[frame_index].frm_offset;

        /* check for frame type */
        /* Make frames */
        if (LIN_MAKE_UNCONDITIONAL_FRAME == type)
        {
            node_attr_ptr = &g_lin_node_attribute_array[HARDWARE_INSTANCE];

            for (i = 0U; i < node_attr_ptr->num_frame_have_esignal; i++)
            {
                /* Check if this frame carries response error signal */
                if (id == node_attr_ptr->resp_err_frm_id_ptr[i])
                {
                    /* Set the flag that is used to indicate the reponse error signal is going to be sent. */
                    prot_state_ptr->transmit_error_resp_sig_flg = (bool)1U;
                    break;
                }
            }
            /* Get data from configuration buffer */
            for (i = 0U; i < prot_state_ptr->response_length; i++)
            {
                response_buffer_ptr[i] = g_lin_frame_data_buffer[frame_byte_offset + i];
            }
            /* If this frame carries the response error signal */
            if (prot_state_ptr->transmit_error_resp_sig_flg)
            {
                if (prot_state_ptr->error_in_response)
                {
                    /* If error occured during response, set the bit that is used to carry response error to
                     * indicate error happened */
                    response_buffer_ptr[node_attr_ptr->response_error_byte_offset] |=
                        (1U << node_attr_ptr->response_error_bit_offset);
                }
                else
                {
                    /* Otherwise clear the bit that is used to carry response error */
                    response_buffer_ptr[node_attr_ptr->response_error_byte_offset] &=
                        ~(1U << node_attr_ptr->response_error_bit_offset);
                }
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
void SlaveUpdateTx(uint8_t instance, uint8_t id)
{
    lin_protocol_state_t *prot_state_ptr = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Set successful transfer */
    prot_state_ptr->successful_transfer = 1U;

    if (prot_state_ptr->num_of_processed_frame < 0xFFU)
    {
        /* increase a number of processed frames */
        prot_state_ptr->num_of_processed_frame++;
    }
    /* check whether error signal was transmitted */
    if ((bool)1U == prot_state_ptr->transmit_error_resp_sig_flg)
    {
        /* Clear error in response status once the response error signal is sent */
        prot_state_ptr->error_in_response = 0U;
        /* clear error flag */
        prot_state_ptr->transmit_error_resp_sig_flg = (bool)0U;
    }
    /* check for valid id of unconditional frame */
    if (id <= 0x3BU)
    {
        /* increase a number of succesfull frames */
        prot_state_ptr->num_of_successfull_frame++;
#if !DISABLE_PRINT_BUFFER
        PrintBuffer(instance, id);
#endif
    }
}

void SlaveUpdateRx(uint8_t instance, uint8_t id)
{
    uint8_t frame_index, flag_offset, flag_size, i;
    const lin_protocol_user_config_t *prot_user_config_ptr = &g_lin_protocol_user_cfg_array[HARDWARE_INSTANCE];
    lin_protocol_state_t *prot_state_ptr                   = &g_lin_protocol_state_array[HARDWARE_INSTANCE];

    /* Set successful transfer */
    prot_state_ptr->successful_transfer = 1U;

    if (prot_state_ptr->num_of_processed_frame < 0xFFU)
    {
        /* increase a number of processed frames */
        prot_state_ptr->num_of_processed_frame++;
    }

    frame_index = SlaveGetFrameIndex(instance, id);
    if (id <= 0x3BU)
    {
        /* increase a number of succesfull frames */
        prot_state_ptr->num_of_successfull_frame++;
        /* make unconditional frame */
        SlaveProcesUncdFrame(instance, id, LIN_UPDATE_UNCONDITIONAL_FRAME);

        /* Update rx frame flags */
        flag_offset = prot_user_config_ptr->frame_tbl_ptr[frame_index].flag_offset;
        flag_size   = prot_user_config_ptr->frame_tbl_ptr[frame_index].flag_size;
        for (i = 0U; i < flag_size; i++)
        {
            g_lin_flag_handle_tbl[flag_offset] = 0xFFU;
            flag_offset++;
        }
#if !DISABLE_PRINT_BUFFER
        /* print received response */
        PrintBuffer(instance, id);
#endif
    }
    else
    {
        ; /* misra compliance */
    }
}

void SlaveTimeoutService(uint8_t instance)
{
    lin_state_t *linState                                  = g_linStatePtr[SLAVE_INSTANCE];
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

void SlaveHandleError(uint8_t instance, lin_event_id_t event_id, uint8_t id)
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
        case LIN_LAST_RESPONSE_SHORT_ERROR:
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
    tpm_config_t tpmInfo;

    lin_user_config_t linUserConfigSlave;
    lin_state_t linCurrentState;
    lin_status_t status;
    uint32_t linfreq;
    gpio_pin_config_t slpn_config = {
        kGPIO_DigitalOutput,
        1,
    };
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_DebugConsolePins();
    BOARD_InitLinLpuart();
    BOARD_BootClockRUN();
    /* Set LIN LPUART clock */
    CLOCK_SetIpSrc(kCLOCK_Lpuart0, kCLOCK_IpSrcFro6M);
#if defined(TIMER_TPM) && TIMER_TPM
    /* Set Timer LPUART clock */
    CLOCK_SetIpSrc(DEMO_TPM_CLOCK, kCLOCK_IpSrcFro6M);
#endif
    BOARD_InitDebugConsole();

#if defined(TJA_WAKEUP) && TJA_WAKEUP
    /* Wakeup TJA transceiver */
    GPIO_PinInit(DEMO_SLPN_GPIO, DEMO_SLPN_PIN, &slpn_config);
#endif

    /*
     * .linUserConfigSlave.autobaudEnable = true;
     * .linUserConfigSlave.baudRate = 19200;
     * .linUserConfigSlave.nodeFunction = SLAVE;
     * .linUserConfigSlave.timerGetTimeIntervalCallback = NULL;
     */
    LIN_GetSlaveDefaultConfig(&linUserConfigSlave);
    /* get LIN clock frequency */
    linfreq = CLOCK_GetFreq(LIN_CLOCK_NAME);

#if defined(TIMER_TPM) && TIMER_TPM
    TPM_GetDefaultConfig(&tpmInfo);
    tpmInfo.prescale = kTPM_Prescale_Divide_1;
    /* Initialize TPM module */
    TPM_Init(DEMO_TPM_BASEADDR, &tpmInfo);
    /* Set module value */
    DEMO_TPM_BASEADDR->MOD = 0xFFFF;
    /* Setup TPM output compare mode */
    TPM_SetupOutputCompare(DEMO_TPM_BASEADDR, DEMO_TPM_CH_OUT, kTPM_NoOutputSignal, MODULO_VALUE);
    /* Setup TPM input capture mode - capture LPUART-RX falling edge */
    TPM_SetupInputCapture(DEMO_TPM_BASEADDR, DEMO_TPM_CH_IN, kTPM_RiseAndFallEdge);
    /* Enable interrupt on overflow */
    TPM_EnableInterrupts(DEMO_TPM_BASEADDR,
                         kTPM_TimeOverflowInterruptEnable | DEMO_TPM_CH_OUT_IRQ | DEMO_TPM_CH_IN_IRQ);
    /* Set the second highest priority */
    NVIC_SetPriority(DEMO_TPM_IRQn, DEMO_TIMER_PRIO);
    /* Enable at the NVIC */
    EnableIRQ(DEMO_TPM_IRQn);
#endif

    if (TIMER_FREQ == 0U)
    {
        PRINTF("\r\n Timer initialization failed!");
        return -1;
    }
    /* Initialize slave node */
    status = SlaveInit(&linUserConfigSlave, &linCurrentState, linfreq);

    if (status != LIN_SUCCESS)
    {
        PRINTF("\r\n LIN initialization failed!");
        return -1;
    }
    PRINTF("\r\n LIN slave initialized");

    if (linUserConfigSlave.autobaudEnable == true)
    {
        /* Prepare baudrate array for autobadrate mode */
        CalculateBaudrates(SLAVE_INSTANCE, linfreq, g_baudRatesValues);
    }

    /* Go to sleep until master starts communication */
    LIN_GoToSleepMode(SLAVE_INSTANCE);
    if (linCurrentState.currentNodeState != LIN_NODE_STATE_SLEEP_MODE)
    {
        PRINTF("\r\n LIN sleep command failed!");
    }
    PRINTF("\r\n Awaiting data from Master");
#if defined(TIMER_TPM) && TIMER_TPM
    TPM_StartTimer(DEMO_TPM_BASEADDR, kTPM_SystemClock);
#endif

    for (;;)
    {                 /* Infinite loop to avoid leaving the main function */
        __asm("NOP"); /* something to use as a breakpoint stop while looping */
    }
}
