/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2021 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "connectivity_test_menus.h"
#include "connectivity_test.h"
#include "connectivity_test_platform.h"
#include "connectivity_test_timers.h"
#include "fsl_format.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_timer_manager.h"
#include "fsl_adapter_reset.h"
#include "fsl_os_abstraction.h"
#if !defined(RW610_SERIES) && !defined(RW612_SERIES)
#include "fwk_platform.h"
#endif
#include "board.h"
#include "app.h"

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
#if CT_Feature_RSSI_Has_Sign
typedef int8_t energy8_t;
typedef int32_t energy32_t;
#else
typedef uint8_t energy8_t;
typedef uint32_t energy32_t;
#endif
/************************************************************************************
*************************************************************************************
* Macros
*************************************************************************************
************************************************************************************/
#define gPrbs9BufferLength_c	 ( 65 )
#define gContTxModSelectPN9_c    ( 2 )
#define gContTxModSelectOnes_c   ( 1 )
#define gContTxModSelectZeros_c  ( 0 )
#define SelfNotificationEvent()  ((void)OSA_EventSet(gTaskEvent, gCTSelf_EVENT_c))

#define gUART_RX_EVENT_c         (1<<0)
#define gMcps_Cnf_EVENT_c        (1<<1)
#define gMcps_Ind_EVENT_c        (1<<2)
#define gMlme_EdCnf_EVENT_c      (1<<3)
#define gMlme_CcaCnf_EVENT_c     (1<<4)
#define gMlme_TimeoutInd_EVENT_c (1<<5)
#define gRangeTest_EVENT_c       (1<<6)
#define gCTSelf_EVENT_c          (1<<7)
#define gTimePassed_EVENT_c      (1<<8)

#define gEventsAll_c             (gUART_RX_EVENT_c | gMcps_Ind_EVENT_c | gMcps_Cnf_EVENT_c | \
gMlme_TimeoutInd_EVENT_c | gMlme_EdCnf_EVENT_c | gMlme_CcaCnf_EVENT_c | \
    gRangeTest_EVENT_c | gCTSelf_EVENT_c | gTimePassed_EVENT_c)

#define GetTimestampUS() ConnTestTimers_GetTime()

#ifdef gPHY_802_15_4g_d
#define GetTransmissionTime(payload, bitrate) ((((gPhyFSKPreambleLength_c + \
gPhyMRFSKPHRLength_c + gPhyMRFSKSFDLength_c + \
    sizeof(smacHeader_t) + payload +  gPhyFCSSize_c )*8000 )/ bitrate))
#else
#define GetTransmissionTime(payload, bitrate) (((6 + sizeof(smacHeader_t) + payload + 2)*32))
//bitrate is fixed for 2.4 GHz
#define crtBitrate      (0)
#endif

#if gMpmMaxPANs_c == 2
#define gNumPans_c   2
extern char * const cu8MpmMenuPs[];
#else
#define gNumPans_c   1
#endif

#define Serial_Print(a,b,c)  SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, (uint8_t *)b, strlen(b))
#define Serial_PrintDec(a,b) SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, FORMAT_Dec2Str(b), strlen((char const *)FORMAT_Dec2Str(b)))

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/*osa variables*/
OSA_EVENT_HANDLE_DEFINE(gTaskEvent);
osa_event_flags_t           gTaskEventFlags;

/*smac related variables*/
bool_t bTxDone;
bool_t bTxNoAck = FALSE;
bool_t bRxDone;
bool_t bScanDone;
bool_t gCCaGotResult;
bool_t gIsChannelIdle;
bool_t bEdDone;
bool_t failedPRBS9;
uint8_t u8LastRxRssiValue;
bool_t evTestParameters;
uint8_t au8ScanResults[129];
#if gMpmMaxPANs_c == 2
bool_t bDataInd[2];
uint8_t u8PanRSSI[2];
#endif

/*serial manager related variables*/
uint8_t gu8UartData;
bool_t evDataFromUART;
serial_handle_t mAppSer;
SERIAL_MANAGER_READ_HANDLE_DEFINE(g_connReadHandle);
SERIAL_MANAGER_WRITE_HANDLE_DEFINE(g_connWriteHandle);
uint32_t mRxBufferBytesAvailable = 0U;

/*connectivity test state machine variables*/
operationModes_t testOpMode;
operationModes_t prevOpMode;

channels_t       testChannel;
uint8_t          testPower;
uint8_t          testPayloadLen;
uint8_t          contTxModBitValue;
uint8_t          ccaThresh;
AckType_t        useAck;
uint8_t ChannelToScan;
bool_t shortCutsEnabled;
ConnectivityStates_t       connState;
ContinuousTxRxTestStates_t cTxRxState;
PerTxStates_t              perTxState;
PerRxStates_t              perRxState;
RangeTxStates_t            rangeTxState;
RangeRxStates_t            rangeRxState;
EditRegsStates_t    eRState;
oRStates_t          oRState;
rRStates_t          rRState;
dRStates_t          dRState;
CSenseTCtrlStates_t   cstcState;
smacTestMode_t contTestRunning;

#if CT_Feature_Xtal_Trim
uint8_t          xtalTrimValue;
#endif

/*asp related variables*/
AppToAspMessage_t aspTestRequestMsg;

extern uint8_t u8Prbs9Buffer[gPrbs9BufferLength_c];
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static uint8_t gau8RxDataBuffer[gMaxSmacSDULength_c  + sizeof(rxPacket_t)];
#if gMpmMaxPANs_c == 2
static uint8_t gau8RxDataBufferAlt[gMaxSmacSDULength_c + sizeof(rxPacket_t)];
#endif
uint8_t gau8TxDataBuffer[gMaxSmacSDULength_c  + sizeof(txPacket_t)];

txPacket_t * gAppTxPacket;
static rxPacket_t * gAppRxPacket;

static uint8_t timePassed;
CONN_TEST_TIMER_DEFINE(AppDelayTmr);
CONN_TEST_TIMER_DEFINE(RangeTestTmr);

static OSA_TASK_HANDLE_DEFINE(s_startTaskHandle);
/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
#if CT_Feature_Calibration
extern void StoreTrimValueToFlash (uint32_t trimValue, CalibrationOptionSelect_t option);
#endif

/*platform independent functions*/
static void SerialUIStateMachine(void);
static bool_t SerialContinuousTxRxTest(void);
static bool_t PacketErrorRateTx(void);
static bool_t PacketErrorRateRx(void);
static void SetRadioRxOnNoTimeOut(void);
static void HandleEvents(int32_t evSignals);

static void PrintPerRxFinalLine(uint16_t u16Received, uint16_t u16Total);
static bool_t stringComp(uint8_t * au8leftString, uint8_t * au8RightString, uint8_t bytesToCompare);

#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
static uint32_t HexString2Dec(uint8_t* hexString);
#endif
/********************************/

static void RangeTestDelayCallback(uint32_t param);
static bool_t RangeTx(void);
static bool_t RangeRx(void);

static bool_t EditRegisters(void);
bool_t OverrideRegisters(void);
bool_t ReadRegisters(void);
bool_t DumpRegisters(void);
bool_t bIsRegisterDirect = TRUE;

static bool_t CSenseAndTCtrl(void);
static void TransmissionControlHandler(void);
static void CarrierSenseHandler(void);
static smacErrors_t TestMode ( smacTestMode_t  mode);
static void PacketHandler_Prbs9(void);
static void IncrementChannelOnEdEvent();
#if gMpmMaxPANs_c == 2
static bool_t ConfigureAlternatePan(void);
#endif

extern void ReadRFRegs(registerAddressSize_t, registerAddressSize_t);
extern void PrintTestParameters(bool_t bEraseLine);

/* Timer related functions */
static void AppDelayCallback(uint32_t param);

/*************************************/
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
void InitProject(void);
void InitSmac(void);
void main_task(uint32_t param);
void UartRxCallBack(void* param, serial_manager_callback_message_t *message, serial_manager_status_t status );

/*osa start_task*/
void start_task(void *argument);

/************************************************************************************
*
* InitProject
*
************************************************************************************/
void InitProject(void)
{
    /*Global Data init*/
    testPayloadLen = gMaxSmacSDULength_c;

    testOpMode       = gDefaultOperationMode_c;
    testChannel      = gDefaultChannelNumber_c;
    testPower        = gDefaultOutputPower_c;
    useAck           = gAckTypeNone_c;
    testPayloadLen   = gDefaultPayload_c;
    contTestRunning  = gTestModeForceIdle_c;
    shortCutsEnabled = FALSE;
    connState        = gConnInitState_c;
    cTxRxState       = gCTxRxStateInit_c;
    perTxState       = gPerTxStateInit_c;
    perRxState       = gPerRxStateInit_c;
    rangeTxState     = gRangeTxStateInit_c;
    rangeRxState     = gRangeRxStateInit_c;
    prevOpMode       = gDefaultOperationMode_c;
    oRState          = gORStateInit_c;
    rRState          = gRRStateInit_c;
    dRState          = gDRStateInit_c;
    ccaThresh        = gDefaultCCAThreshold_c;
    bEdDone          = FALSE;
    evDataFromUART = FALSE;
#if gMpmMaxPANs_c == 2
    bDataInd[0]      = FALSE;
    bDataInd[1]      = FALSE;
#endif

    InitProject_custom();
}

/************************************************************************************
*************************************************************************************
* SAP functions
*************************************************************************************
************************************************************************************/

/*(Management) Sap handler for managing timeout indication and ED confirm */
smacErrors_t smacToAppMlmeSap(smacToAppMlmeMessage_t* pMsg, instanceId_t instance)
{
    switch(pMsg->msgType)
    {
    case gMlmeEdCnf_c:
        au8ScanResults[testChannel] = pMsg->msgData.edCnf.energyLeveldB;
        (void)OSA_EventSet(gTaskEvent, gMlme_EdCnf_EVENT_c);
        break;
    case gMlmeCcaCnf_c:
        (void)OSA_EventSet(gTaskEvent, gMlme_CcaCnf_EVENT_c);
        if(pMsg->msgData.ccaCnf.status == gErrorNoError_c)
            gIsChannelIdle = TRUE;
        else
            gIsChannelIdle = FALSE;
        break;
    case gMlmeTimeoutInd_c:
        (void)OSA_EventSet(gTaskEvent, gMlme_TimeoutInd_EVENT_c);
	break;
    default:
        break;
    }
    MEM_BufferFree(pMsg);
    return gErrorNoError_c;
}
/*(Data) Sap handler for managing data confirm and data indication */
smacErrors_t smacToAppMcpsSap(smacToAppDataMessage_t* pMsg, instanceId_t instance)
{
    switch(pMsg->msgType)
    {
    case gMcpsDataInd_c:
        if(pMsg->msgData.dataInd.pRxPacket->rxStatus == rxSuccessStatus_c)
        {
            u8LastRxRssiValue = pMsg->msgData.dataInd.u8LastRxRssi;
#if gMpmMaxPANs_c == 2
            bDataInd[instance] = TRUE;
            u8PanRSSI[instance] = pMsg->msgData.dataInd.u8LastRxRssi;
#endif
            (void)OSA_EventSet(gTaskEvent, gMcps_Ind_EVENT_c);
        }
        break;
    case gMcpsDataCnf_c:
		if(pMsg->msgData.dataCnf.status == gErrorNoAck_c) {
			bTxNoAck = TRUE;
		}

		(void)OSA_EventSet(gTaskEvent, gMcps_Cnf_EVENT_c);
        break;
    default:
        break;
    }

    MEM_BufferFree(pMsg);
    return gErrorNoError_c;
}

static void HandleEvents(int32_t evSignals)
{
    uint16_t u16SerBytesCount = 0;

    if(evSignals & gUART_RX_EVENT_c)
    {
        if(kStatus_SerialManager_Success == SerialManager_TryRead((serial_read_handle_t)g_connReadHandle, &gu8UartData, 1U, (uint32_t *)&u16SerBytesCount))
        {
            if(u16SerBytesCount)
            {
                if(shortCutsEnabled)
                {
                    ShortCutsParser(gu8UartData);
                }
                else
                {
                    evDataFromUART = TRUE;
                }
                mRxBufferBytesAvailable -= u16SerBytesCount;
                if(mRxBufferBytesAvailable)
                {
                    (void)OSA_EventSet(gTaskEvent, gUART_RX_EVENT_c);
                }
            }
        }
    }
    if(evSignals & gMcps_Cnf_EVENT_c)
    {
        bTxDone = TRUE;
    }
    if(evSignals & gMcps_Ind_EVENT_c)
    {
        bRxDone = TRUE;
    }
    if(evSignals & gMlme_TimeoutInd_EVENT_c)
    {
    }
    if(evSignals & gRangeTest_EVENT_c)
    {
        bRxDone=TRUE;
    }
    if(evSignals & gMlme_EdCnf_EVENT_c)
    {
        if (cTxRxState == gCTxRxStateRunnigScanTest_c)
        {
            IncrementChannelOnEdEvent();
        }
        if (cTxRxState == gCTxRxStateRunnigEdTest_c)
        {
            cTxRxState = gCTxRxStateRunningEdTestGotResult_c;
        }
        if (connState == gConnCSenseAndTCtrl_c)
        {
            bScanDone = TRUE;
        }
        bEdDone = TRUE;
    }
    if(evSignals & gMlme_CcaCnf_EVENT_c)
    {
        gCCaGotResult = TRUE;
        Serial_Print(mAppSer, "Channel ", gAllowToBlock_d);
        Serial_PrintDec(mAppSer, (uint32_t)testChannel);
        Serial_Print(mAppSer, " is ", gAllowToBlock_d);
        if(gIsChannelIdle)
            Serial_Print(mAppSer,"Idle\r\n", gAllowToBlock_d);
        else
            Serial_Print(mAppSer,"Busy\r\n", gAllowToBlock_d);
    }
    if(evSignals & gCTSelf_EVENT_c)
    {
    }
}


/*************************************************************************/
/*Main Task: Application entry point*/
/*************************************************************************/
void main_task(uint32_t param)
{
    static bool_t bIsInitialized = FALSE;
    static bool_t bUserInteraction = FALSE;
    //Initialize Memory Manager, Timer Manager and LEDs.
    if( !bIsInitialized )
    {
        //hardware_init();

        MEM_Init();
#if !defined(gConnTestUsePhyTimers_c) || (gConnTestUsePhyTimers_c == 0)
        PLATFORM_InitTimerManager();
#endif

        //initialize PHY
        Phy_Init();

#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
        for (uint8_t i = 0; i < gAppLedCnt_c; i++)
        {
            LED_Flash((led_handle_t)g_ledHandle[i], (led_flash_config_t *)&ledFlash);
        }
#endif /*gAppLedCnt_c > 0*/

        OSA_EventCreate((osa_event_handle_t)gTaskEvent, TRUE);

        InitApp();

        /*Prints the Welcome screens in the terminal*/
        PrintMenu(cu8Logo, (serial_write_handle_t)g_connWriteHandle);

        connState = gConnIdleState_c;
        bIsInitialized = TRUE;
    }
    if(!bUserInteraction)
    {
        while(1)
        {
            (void)OSA_EventWait(gTaskEvent, gEventsAll_c, FALSE, osaWaitForever_c ,&gTaskEventFlags);
            HandleEvents(gTaskEventFlags);
            if(evDataFromUART)
            {
                evDataFromUART = FALSE;
                if(gu8UartData == '\r')
                {
                    //LED_StopFlashingAllLeds();
                    SelfNotificationEvent();
                    bUserInteraction = TRUE;
                    break;
                }
                else
                {
                    PrintMenu(cu8Logo, (serial_write_handle_t)g_connWriteHandle);
                }
            }
            if(gUseRtos_c == 0)
            {
                break;
            }
        }
    }
    if(bUserInteraction)
    {
        while(1)
        {
            (void)OSA_EventWait(gTaskEvent, gEventsAll_c, FALSE, osaWaitForever_c ,&gTaskEventFlags);
            HandleEvents(gTaskEventFlags);
            SerialUIStateMachine();
            if (gUseRtos_c == 0)
            {
                break;
            }
        }
    }
}

void start_task(void *argument)
{
    main_task((uint32_t)argument);
}

static OSA_TASK_DEFINE(start_task, gMainThreadPriority_c, 1, gMainThreadStackSize_c, 0);

int main(void)
{
    /*init OSA: should be called before any other OSA API*/
    OSA_Init();

    BOARD_InitHardware();

#if defined(USE_NBU) && (USE_NBU == 1)
    PLATFORM_InitNbu();
    PLATFORM_InitMulticore();
#endif /* defined(USE_NBU) && (USE_NBU == 1) */

    APP_InitServices();

#ifndef FSL_RTOS_THREADX
    /* When using ThreadX, OS objects should be created in tx_application_define */
    (void)OSA_TaskCreate((osa_task_handle_t)s_startTaskHandle, OSA_TASK(start_task), NULL);
#endif

    /*start scheduler*/
    OSA_Start();

    /*won't run here*/
    assert(0);
    return 0;
}

#ifdef FSL_RTOS_THREADX
/* This should be implemented by the application.
 * The tx_application_define function executes after the basic ThreadX initialization is complete.
 * It is responsible for setting up all of the initial system resources, including threads, queues,
 * semaphores, mutexes, event flags, and memory pools. */
void tx_application_define(void* param)
{
    (void)OSA_TaskCreate((osa_task_handle_t)s_startTaskHandle, OSA_TASK(start_task), NULL);
}
#endif /* FSL_RTOS_THREADX */

/*************************************************************************/
/*InitApp: Initializes application mdoules and data*/
/*************************************************************************/
void InitApp()
{
    ConnTestTimers_InitTimer(AppDelayTmr, AppDelayCallback);
    ConnTestTimers_InitTimer(RangeTestTmr, RangeTestDelayCallback);

    gAppTxPacket = (txPacket_t*)gau8TxDataBuffer;   //Map TX packet to buffer
    gAppRxPacket = (rxPacket_t*)gau8RxDataBuffer;   //Map Rx packet to buffer
    gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;

    mAppSer = gSerMgrIf;
    SerialManager_OpenReadHandle((serial_handle_t)mAppSer, (serial_read_handle_t)g_connReadHandle);
    SerialManager_OpenWriteHandle((serial_handle_t)mAppSer, (serial_write_handle_t)g_connWriteHandle);
    (void)SerialManager_InstallRxCallback((serial_read_handle_t)g_connReadHandle, (serial_manager_callback_t)UartRxCallBack, NULL);

	ASP_Init(0);

    //Initialise SMAC
    InitSmac();
    //Tell SMAC who to call when it needs to pass a message to the application thread.
    Smac_RegisterSapHandlers((SMAC_APP_MCPS_SapHandler_t)smacToAppMcpsSap,(SMAC_APP_MLME_SapHandler_t)smacToAppMlmeSap,0);
#if gMpmMaxPANs_c == 2
    Smac_RegisterSapHandlers((SMAC_APP_MCPS_SapHandler_t)smacToAppMcpsSap,(SMAC_APP_MLME_SapHandler_t)smacToAppMlmeSap,1);
#endif

    InitProject();

    InitApp_custom();

    SMACFillHeader(&(gAppTxPacket->smacHeader), gBroadcastAddress_c);                   //@CMA, Conn Test. Start with broadcast address default
    (void)MLMEPAOutputAdjust(testPower);
    (void)MLMESetChannelRequest(testChannel);                                     //@CMA, Conn Test. Start Foperation at default channel
}

/************************************************************************************
*
* Connectivity Test State Machine
*
************************************************************************************/
void SerialUIStateMachine(void)
{
    if((gConnSelectTest_c == connState) && evTestParameters)
    {
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }
    switch(connState)
    {
    case gConnIdleState_c:
        PrintMenu(cu8MainMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        connState = gConnSelectTest_c;
        break;
    case gConnSelectTest_c:
        if(evDataFromUART){
            if('1' == gu8UartData)
            {
                cTxRxState = gCTxRxStateInit_c;
                connState = gConnContinuousTxRxState_c;
            }
            else if('2' == gu8UartData)
            {
                perTxState = gPerTxStateInit_c;
                perRxState = gPerRxStateInit_c;
                connState = gConnPerState_c;
            }
            else if('3' == gu8UartData)
            {
                rangeTxState = gRangeTxStateInit_c;
                rangeRxState = gRangeRxStateInit_c;
                connState = gConnRangeState_c;
            }
            else if('4' == gu8UartData)
            {
                cstcState = gCsTcStateInit_c;
                connState = gConnCSenseAndTCtrl_c;
            }
#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
            else if('5' == gu8UartData)
            {
                eRState = gERStateInit_c;
                connState = gConnRegEditState_c;
            }
#endif
#if CT_Feature_Bitrate_Select
            else if('6' == gu8UartData)
            {
                bsState = gBSStateInit_c;
                connState = gConnBitrateSelectState_c;
            }
#endif
#if CT_Feature_Calibration
            else if('7' == gu8UartData)
            {
                connState = gConnEDMeasCalib_c;
                edCalState= gEdCalStateInit_c;
            }
#endif
            else if('!' == gu8UartData)
            {
                HAL_ResetMCU();
            }
            evDataFromUART = FALSE;
            SelfNotificationEvent();
        }
        break;
    case gConnContinuousTxRxState_c:
        if(SerialContinuousTxRxTest())
        {
            connState = gConnIdleState_c;
            SelfNotificationEvent();
        }
        break;
    case gConnPerState_c:
        if(mTxOperation_c == testOpMode)
        {
            if(PacketErrorRateTx())
            {
                connState = gConnIdleState_c;
                SelfNotificationEvent();
            }
        }
        else
        {
            if(PacketErrorRateRx())
            {
                connState = gConnIdleState_c;
                SelfNotificationEvent();
            }
        }
        break;
    case gConnRangeState_c:
        if(mTxOperation_c == testOpMode)
        {
            if(RangeTx())
            {
                connState = gConnIdleState_c;
                SelfNotificationEvent();
            }
        }
        else
        {
            if(RangeRx())
            {
                connState = gConnIdleState_c;
                SelfNotificationEvent();
            }
        }
        break;
    case gConnRegEditState_c:
        if(EditRegisters())
        {
            connState = gConnIdleState_c;
            SelfNotificationEvent();
        }
        break;
#if CT_Feature_Bitrate_Select
    case gConnBitrateSelectState_c:
        if(Bitrate_Select())
        {
            connState = gConnIdleState_c;
        }
        break;
#endif
    case gConnCSenseAndTCtrl_c:
        if(CSenseAndTCtrl())
        {
            connState = gConnIdleState_c;
            SelfNotificationEvent();
        }
        break;
#if CT_Feature_Calibration
    case gConnEDMeasCalib_c:
        if(EDCalibrationMeasurement())
        {
            connState = gConnIdleState_c;
            SelfNotificationEvent();
        }
        break;
#endif
    default:
        break;

    }
    if(prevOpMode != testOpMode)
    {
        perTxState = gPerTxStateInit_c;
        perRxState = gPerRxStateInit_c;
        rangeTxState = gRangeTxStateInit_c;
        rangeRxState = gRangeRxStateInit_c;
        prevOpMode = testOpMode;
        SelfNotificationEvent();
    }
}

/************************************************************************************
*
* Continuous Tests State Machine
*
************************************************************************************/
bool_t SerialContinuousTxRxTest(void)
{
    bool_t bBackFlag = FALSE;
    uint8_t u8Index;
    energy8_t e8TempEnergyValue;
    if(evTestParameters)
    {
        (void)TestMode(gTestModeForceIdle_c);
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        if(gTestModePRBS9_c == contTestRunning)
        {
            cTxRxState = gCTxRxStateRunningPRBS9Test_c;
        }
        (void)TestMode(contTestRunning);

        if(gCTxRxStateSelectTest_c == cTxRxState)
        {
            PrintTestParameters(TRUE);
        }
        else
        {
            PrintTestParameters(FALSE);
            Serial_Print(mAppSer, "\r\n", gAllowToBlock_d);
        }

        if(gCTxRxStateRunnigRxTest_c == cTxRxState)
        {
            bRxDone = FALSE;
            gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
            (void)MLMERXEnableRequest(gAppRxPacket, 0);
        }
        evTestParameters = FALSE;
    }

    switch(cTxRxState)
    {
    case gCTxRxStateIdle_c:
        if((evDataFromUART) && ('\r' == gu8UartData))
        {
            cTxRxState = gCTxRxStateInit_c;
            evDataFromUART = FALSE;
            SelfNotificationEvent();
        }
        break;
    case gCTxRxStateInit_c:
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8ContinuousTestMenu, (serial_write_handle_t)g_connWriteHandle);
        //Phy in StandBy, smacstate in Idle.
        (void)TestMode(gTestModeForceIdle_c);
        while(MLMESetChannelRequest(testChannel));
        Serial_Print(mAppSer, cu8ContinuousTestTags[contTestRunning], gAllowToBlock_d);
        if(contTestRunning == gTestModeContinuousTxModulated_c)
        {
            Serial_Print(mAppSer, cu8TxModTestTags[contTxModBitValue],gAllowToBlock_d);
        }
        (void)TestMode(contTestRunning);
        Serial_Print(mAppSer, "\r\n\r\n", gAllowToBlock_d);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        cTxRxState = gCTxRxStateSelectTest_c;
        break;
    case gCTxRxStateSelectTest_c:
        if(evDataFromUART)
        {
            if('1' == gu8UartData)
            {
                contTestRunning = gTestModeForceIdle_c;
                cTxRxState = gCTxRxStateInit_c;
                SelfNotificationEvent();
            }
            else if('2' == gu8UartData)
            {
                shortCutsEnabled = FALSE;
                (void)TestMode(gTestModeForceIdle_c);
                contTestRunning = gTestModePRBS9_c;
                MLMESetChannelRequest(testChannel);
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop the Continuous PRBS9 test\r\n", gAllowToBlock_d);
                (void)TestMode(contTestRunning);
                cTxRxState = gCTxRxStateRunningPRBS9Test_c;
            }
            else if('3' == gu8UartData)
            {
                contTestRunning = gTestModeContinuousTxModulated_c;
                cTxRxState = gCTxRxStateRunningTXModSelectOpt;
                //        Serial_Print(mAppSer, "\f\r\n To use this mode shunt pins 3-4 on J18", gAllowToBlock_d);
                Serial_Print(mAppSer, "\f\r\nPress 2 for PN9, 1 to modulate values of 1 and 0 to modulate values of 0", gAllowToBlock_d);

            }
            else if('4' == gu8UartData)
            {
                if(gTestModeContinuousTxUnmodulated_c != contTestRunning)
                {
                    contTestRunning = gTestModeContinuousTxUnmodulated_c;
                    cTxRxState = gCTxRxStateInit_c;
                    SelfNotificationEvent();
                }
            }
            else if('5' == gu8UartData)
            {
                shortCutsEnabled = FALSE;
                (void)TestMode(gTestModeForceIdle_c);
                MLMESetChannelRequest(testChannel);
                contTestRunning = gTestModeForceIdle_c;
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop receiving broadcast packets \r\n", gAllowToBlock_d);
                bRxDone = FALSE;
                gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
                (void)MLMERXEnableRequest(gAppRxPacket, 0);
                cTxRxState = gCTxRxStateRunnigRxTest_c;
            }
            else if('6' == gu8UartData)
            {
                (void)TestMode(gTestModeForceIdle_c);
                contTestRunning = gTestModeForceIdle_c;
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop the Continuous ED test\r\n", gAllowToBlock_d);
                cTxRxState = gCTxRxStateRunnigEdTest_c;
                ConnTestTimers_StartDelay(AppDelayTmr, 200);
            }
            else if('7' == gu8UartData)
            {
                (void)TestMode(gTestModeForceIdle_c);
                contTestRunning = gTestModeForceIdle_c;
                ChannelToScan= gDefaultChannelNumber_c;
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop the Continuous SCAN test\r\n", gAllowToBlock_d);
                bScanDone = FALSE;
                cTxRxState = gCTxRxStateRunnigScanTest_c;
                SelfNotificationEvent();
            }
            else if('8' == gu8UartData)
            {
                (void)TestMode(gTestModeForceIdle_c);
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop the Continuous CCA test\r\n", gAllowToBlock_d);
                contTestRunning = gTestModeForceIdle_c;
                cTxRxState = gCTxRxStateRunnigCcaTest_c;
                ConnTestTimers_StartDelay(AppDelayTmr, 100);
                MLMECcaRequest();
            }
#if CT_Feature_BER_Test
            else if ('9' == gu8UartData)
            {
                Serial_Print(mAppSer, "\f\r\nPress [p] to stop the Continuous BER test\r\n", gAllowToBlock_d);
                contTestRunning = gTestModeContinuousRxBER_c;
                cTxRxState = gCTxRxStateInit_c;
                SelfNotificationEvent();
            }
#endif
            else if('p' == gu8UartData)
            {
                (void)TestMode(gTestModeForceIdle_c);
                (void)MLMESetChannelRequest(testChannel);
                ConnTestTimers_StopDelay(AppDelayTmr);
                timePassed = FALSE;
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
        }
        break;
    case gCTxRxStateRunningTXModSelectOpt:
        if(evDataFromUART)
        {
            if (gu8UartData == '2')
            {
                contTxModBitValue = gContTxModSelectPN9_c;
            }
            else if (gu8UartData == '1')
            {
                contTxModBitValue = gContTxModSelectOnes_c;
            }
            else if (gu8UartData == '0')
            {
                contTxModBitValue = gContTxModSelectZeros_c;
            }

            evDataFromUART = FALSE;
            cTxRxState = gCTxRxStateInit_c;
            SelfNotificationEvent();
        }
        break;
    case gCTxRxStateRunningPRBS9Test_c:
        if(bTxDone || failedPRBS9)
        {
            failedPRBS9 = FALSE;
            bTxDone     = FALSE;
            PacketHandler_Prbs9();
        }
        if(evDataFromUART && 'p' == gu8UartData)
        {
            contTestRunning = gTestModeForceIdle_c;
            (void)TestMode(gTestModeForceIdle_c);
            (void)MLMESetChannelRequest(testChannel);
            ConnTestTimers_StopDelay(AppDelayTmr);
            timePassed = FALSE;
            Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Continuous test menu ", gAllowToBlock_d);
            cTxRxState = gCTxRxStateIdle_c;
            evDataFromUART = FALSE;
            shortCutsEnabled = TRUE;
        }
        break;
    case gCTxRxStateRunnigRxTest_c:
        if(bRxDone)
        {
            if (gAppRxPacket->rxStatus == rxSuccessStatus_c)
            {
                Serial_Print(mAppSer, "New Packet: ", gAllowToBlock_d);
                for(u8Index = 0; u8Index < (gAppRxPacket->u8DataLength); u8Index++){
                    (void)SerialManager_WriteBlocking((serial_write_handle_t)g_connWriteHandle, FORMAT_Hex2Ascii(gAppRxPacket->smacPdu.smacPdu[u8Index]), 2U);
                }
                Serial_Print(mAppSer, " \r\n", gAllowToBlock_d);
            }
            bRxDone = FALSE;
            gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
            (void)MLMERXEnableRequest(gAppRxPacket, 0);
        }
        if((evDataFromUART) && ('p' == gu8UartData))
        {
            (void)MLMERXDisableRequest();
            (void)TestMode(gTestModeForceIdle_c);
            Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Continuous test menu ", gAllowToBlock_d);
            cTxRxState = gCTxRxStateIdle_c;
            evDataFromUART = FALSE;
        }
        break;
    case gCTxRxStateRunnigEdTest_c:
        if(timePassed)
        {
            timePassed = FALSE;
            ConnTestTimers_StartDelay(AppDelayTmr, 100);
            MLMEScanRequest(testChannel);
        }
        if((evDataFromUART) && ('p' == gu8UartData))
        {
            Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Continuous test menu ", gAllowToBlock_d);
            cTxRxState = gCTxRxStateIdle_c;
            evDataFromUART = FALSE;
            timePassed = FALSE;
            ConnTestTimers_StopDelay(AppDelayTmr);
        }

        break;
    case gCTxRxStateRunningEdTestGotResult_c:
        Serial_Print(mAppSer, "Energy on the Channel ", gAllowToBlock_d);
        Serial_PrintDec(mAppSer, (uint32_t)testChannel);
        Serial_Print(mAppSer, " : ", gAllowToBlock_d);
        e8TempEnergyValue = (energy8_t)au8ScanResults[testChannel];
#if CT_Feature_RSSI_Has_Sign
        if(e8TempEnergyValue < 0)
        {
            e8TempEnergyValue *= -1;
#else
            if(e8TempEnergyValue !=0)
            {
#endif
                Serial_Print(mAppSer, "-", gAllowToBlock_d);
            }
            Serial_PrintDec(mAppSer, (uint32_t)e8TempEnergyValue);
            Serial_Print(mAppSer, "dBm\r\n", gAllowToBlock_d);
            cTxRxState = gCTxRxStateRunnigEdTest_c;
            break;
        case gCTxRxStateRunnigCcaTest_c:
            if(timePassed && gCCaGotResult)
            {
                gCCaGotResult = FALSE;
                timePassed = FALSE;
                MLMECcaRequest();
                ConnTestTimers_StartDelay(AppDelayTmr, 100);
            }
            if((evDataFromUART) && ('p' == gu8UartData))
            {
                Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Continuous test menu ", gAllowToBlock_d);
                cTxRxState = gCTxRxStateIdle_c;
                evDataFromUART = FALSE;
                timePassed = FALSE;
                ConnTestTimers_StopDelay(AppDelayTmr);
            }
            break;
        case gCTxRxStateRunnigScanTest_c:
            if(bScanDone && timePassed)
            {
                //Enters here until all channels have been scanned. Then starts to print.
                Serial_Print(mAppSer, "Results : ", gAllowToBlock_d);
                for(u8Index = gMinChannel_c; u8Index <= gMaxChannel_c ; u8Index++)
                {
                    e8TempEnergyValue = (energy8_t)au8ScanResults[u8Index];
#if CT_Feature_RSSI_Has_Sign
                    if(e8TempEnergyValue < 0)
                    {
                        e8TempEnergyValue *= -1;
#else
                    if(e8TempEnergyValue != 0)
                    {
#endif
                        Serial_Print(mAppSer, "-", gAllowToBlock_d);
                    }
                    Serial_PrintDec(mAppSer, (uint32_t) e8TempEnergyValue);
                    Serial_Print(mAppSer, ",", gAllowToBlock_d);
                }
                Serial_Print(mAppSer, "\b \r\n", gAllowToBlock_d);
                bScanDone = FALSE;
                ChannelToScan = gDefaultChannelNumber_c;                             // Restart channel count
                timePassed = FALSE;
            }

            if((evDataFromUART) && ('p' == gu8UartData))
            {
                Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Continuous test menu ", gAllowToBlock_d);
                cTxRxState = gCTxRxStateIdle_c;
                evDataFromUART = FALSE;
            }
            else
            {
                if(ChannelToScan == gDefaultChannelNumber_c)
                {
                    smacErrors_t err = MLMEScanRequest((channels_t)ChannelToScan);
                    if(err == gErrorNoError_c)
                        ChannelToScan++;
                }
                //Each of the other channels is scanned after SMAC notifies us that
                //it has obtained the energy value on the currently scanned channel
                //(channel scanning is performed asynchronously). See IncrementChannelOnEdEvent().
            }
            break;
        default:
            break;
    }
    return bBackFlag;
}

/************************************************************************************
*
* PER Handler for board that is performing TX
*
************************************************************************************/
bool_t PacketErrorRateTx(void)
{
    const uint16_t u16TotalPacketsOptions[] = {1,25,100,500,1000,2000,5000,10000,65535};
    static uint16_t u16TotalPackets;
    static uint16_t u16SentPackets;
    static uint32_t miliSecDelay;
    static uint32_t u32MinDelay = 4;
    uint8_t u8Index;
    bool_t bBackFlag = FALSE;

    if(evTestParameters)
    {
        (void)MLMERXDisableRequest();
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }

    switch(perTxState)
    {
    case gPerTxStateInit_c:
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8PerTxTestMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        perTxState = gPerTxStateSelectPacketNum_c;
        miliSecDelay = 0;
        u32MinDelay = 4;
        (void)MLMERXDisableRequest();
        break;
    case gPerTxStateSelectPacketNum_c:
        if(evDataFromUART)
        {
            if((gu8UartData >= '0') && (gu8UartData <= '8'))
            {
                u16TotalPackets = u16TotalPacketsOptions[gu8UartData - '0'];
                shortCutsEnabled = FALSE;
                u32MinDelay += (GetTransmissionTime(testPayloadLen, crtBitrate) / 1000);
                Serial_Print(mAppSer,"\r\n\r\n Please type TX interval in miliseconds ( > ",gAllowToBlock_d);
                Serial_PrintDec(mAppSer, u32MinDelay);
                Serial_Print(mAppSer,"ms ) and press [ENTER]\r\n", gAllowToBlock_d);
                perTxState = gPerTxStateInputPacketDelay_c;
            }
            else if('p' == gu8UartData)
            {
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
        }
        break;
    case gPerTxStateInputPacketDelay_c:
        if(evDataFromUART)
        {
            if(gu8UartData == '\r')
            {
                if(miliSecDelay < u32MinDelay)
                {
                    Serial_Print(mAppSer,"\r\n\tError: TX Interval too small\r\n",gAllowToBlock_d);
                    perTxState = gPerTxStateInit_c;
                    SelfNotificationEvent();
                }
                else
                {
                    perTxState = gPerTxStateStartTest_c;
                    SelfNotificationEvent();
                }
            }
            else if((gu8UartData >= '0') && (gu8UartData <='9'))
            {
                miliSecDelay = miliSecDelay*10 + (gu8UartData - '0');
                Serial_PrintDec(mAppSer, (uint32_t)(gu8UartData - '0'));
            }
            else if('p' == gu8UartData)
            {
                perTxState = gPerTxStateInit_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    case gPerTxStateStartTest_c:
        gAppTxPacket->u8DataLength = testPayloadLen;
        u16SentPackets = 0;

        gAppTxPacket->smacPdu.smacPdu[0] = (u16TotalPackets >> 8);
        gAppTxPacket->smacPdu.smacPdu[1] = (uint8_t)u16TotalPackets;
        gAppTxPacket->smacPdu.smacPdu[2] = ((u16SentPackets+1) >> 8);
        gAppTxPacket->smacPdu.smacPdu[3] = (uint8_t)(u16SentPackets+1);
        FLib_MemCpy(&(gAppTxPacket->smacPdu.smacPdu[4]), "SMAC PER Demo",13);
        if(17 < testPayloadLen)
        {
            for(u8Index=17;u8Index<testPayloadLen;u8Index++)
            {
                gAppTxPacket->smacPdu.smacPdu[u8Index] = (u8Index%10)+'0';
            }
        }
        bTxDone = FALSE;
        (void)MCPSDataRequest(gAppTxPacket);
        u16SentPackets++;
        Serial_Print(mAppSer, "\f\r\n Running PER Tx, Sending ", gAllowToBlock_d);
        Serial_PrintDec(mAppSer, (uint32_t)u16TotalPackets);
        Serial_Print(mAppSer, " Packets", gAllowToBlock_d);

        perTxState = gPerTxStateRunningTest_c;
        ConnTestTimers_StartDelay(AppDelayTmr, miliSecDelay);
        break;
    case gPerTxStateRunningTest_c:
        if(bTxDone && timePassed)
        {
            bTxDone = FALSE;
            timePassed = FALSE;

            Serial_Print(mAppSer,"\r\n Packet ",gAllowToBlock_d);
            Serial_PrintDec(mAppSer,(uint32_t)u16SentPackets);

			if (bTxNoAck == TRUE) {
				Serial_Print(mAppSer, " (no-ack)", gAllowToBlock_d);
				bTxNoAck = FALSE;
			}

            if(u16SentPackets == u16TotalPackets)
            {
                Serial_Print(mAppSer, "\r\n PER Tx DONE \r\n", gAllowToBlock_d);
                Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the PER Tx test menu ", gAllowToBlock_d);
                perTxState = gPerTxStateIdle_c;
            }
            else
            {
                gAppTxPacket->smacPdu.smacPdu[2] = ((u16SentPackets+1) >> 8);
                gAppTxPacket->smacPdu.smacPdu[3] = (uint8_t)(u16SentPackets+1);
                gAppTxPacket->u8DataLength = testPayloadLen;
                (void)MCPSDataRequest(gAppTxPacket);
                u16SentPackets++;
                ConnTestTimers_StartDelay(AppDelayTmr, miliSecDelay);
            }
        }
        if(evDataFromUART && gu8UartData == ' ')
        {
            Serial_Print(mAppSer,"\r\n\r\n-Test interrupted by user. Press [ENTER] to continue\r\n\r\n",gAllowToBlock_d);

            ConnTestTimers_StopDelay(AppDelayTmr);
            timePassed = FALSE;

            MLMETXDisableRequest();
            bTxDone = FALSE;

            perTxState = gPerTxStateIdle_c;
        }
        break;
    case gPerTxStateIdle_c:
        if((evDataFromUART) && ('\r' == gu8UartData))
        {
            perTxState = gPerTxStateInit_c;
            evDataFromUART = FALSE;
            SelfNotificationEvent();
        }
        break;
    default:
        break;
    }

    return bBackFlag;
}

/************************************************************************************
*
* PER Handler for board that is performing RX
*
************************************************************************************/
bool_t PacketErrorRateRx(void)
{
    static energy32_t e32RssiSum[gNumPans_c];
    static uint16_t u16ReceivedPackets[gNumPans_c];
    static uint16_t u16PacketsIndex[gNumPans_c];
    static uint16_t u16TotalPackets[gNumPans_c];
    static energy8_t  e8AverageRssi[gNumPans_c];
    static bool_t bPrintStatistics = FALSE;
    energy8_t e8TempRssivalue;
    uint8_t u8PanCount = 0;

    bool_t bBackFlag = FALSE;
    if(evTestParameters)
    {
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }
    switch(perRxState)
    {
    case gPerRxStateInit_c:
        shortCutsEnabled = TRUE;
        bPrintStatistics = FALSE;
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8PerRxTestMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        u16TotalPackets[0] = 0;
        u16ReceivedPackets[0] = 0;
        u16PacketsIndex[0] = 0;
        e32RssiSum[0] = 0;
#if gMpmMaxPANs_c == 2
        u16TotalPackets[1] = 0;
        u16ReceivedPackets[1] = 0;
        u16PacketsIndex[1] = 0;
        e32RssiSum[1] = 0;
        bDataInd[0] = FALSE;
        bDataInd[1] = FALSE;
        perRxState = gPerRxConfigureAlternatePan_c;
        SelfNotificationEvent();
#else
        perRxState = gPerRxWaitStartTest_c;
#endif
        break;
#if gMpmMaxPANs_c == 2
    case gPerRxConfigureAlternatePan_c:
        if(evDataFromUART && gu8UartData == 'p')
        {
            evDataFromUART = FALSE;
            perRxState = gPerRxStateInit_c;
            bBackFlag = TRUE;
            break;
        }
        if(ConfigureAlternatePan() == TRUE)
        {
            perRxState = gPerRxWaitStartTest_c;
        }
        break;
#endif
    case gPerRxWaitStartTest_c:
        if(evDataFromUART)
        {
            if(' ' == gu8UartData)
            {
                Serial_Print(mAppSer, "\f\n\rPER Test Rx Running\r\n\r\n", gAllowToBlock_d);
                bRxDone = FALSE;
                gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
                MLMESetActivePan(gSmacPan0_c);
                (void)MLMERXEnableRequest(gAppRxPacket, 0);
#if gMpmMaxPANs_c == 2
                MLMESetActivePan(gSmacPan1_c);
                gAppRxPacket = (rxPacket_t*)gau8RxDataBufferAlt;
                gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
                (void)MLMERXEnableRequest(gAppRxPacket, 0);
#endif
                shortCutsEnabled = FALSE;
                perRxState = gPerRxStateStartTest_c;
            }
            else if('p' == gu8UartData)
            {
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
        }
        break;
    case gPerRxStateStartTest_c:
        if(bRxDone)
        {
#if gMpmMaxPANs_c == 2
            if(bDataInd[gSmacPan0_c] == TRUE)
            {
                gAppRxPacket = (rxPacket_t*)(gau8RxDataBuffer);
                bDataInd[gSmacPan0_c] = FALSE;
                if(bDataInd[gSmacPan1_c] == TRUE)
                {
                    OSA_EventSet(gTaskEvent, gMcps_Ind_EVENT_c);
                }
            }
            else if(bDataInd[gSmacPan1_c] == TRUE)
            {
                gAppRxPacket = (rxPacket_t*)(gau8RxDataBufferAlt);
                bDataInd[gSmacPan1_c] = FALSE;
                if(bDataInd[gSmacPan0_c] == TRUE)
                {
                    OSA_EventSet(gTaskEvent, gMcps_Ind_EVENT_c);
                }
            }
#endif
            if (gAppRxPacket->rxStatus == rxSuccessStatus_c)
            {
                if(stringComp((uint8_t*)"SMAC PER Demo",&gAppRxPacket->smacPdu.smacPdu[4],13))
                {
                    u16TotalPackets[gAppRxPacket->instanceId] =
                        ((uint16_t)gAppRxPacket->smacPdu.smacPdu[0] <<8) + gAppRxPacket->smacPdu.smacPdu[1];
                    u16PacketsIndex[gAppRxPacket->instanceId] =
                        ((uint16_t)gAppRxPacket->smacPdu.smacPdu[2] <<8) + gAppRxPacket->smacPdu.smacPdu[3];
                    u16ReceivedPackets[gAppRxPacket->instanceId]++;
#if gMpmMaxPANs_c == 2
                    e32RssiSum[gAppRxPacket->instanceId] +=
                        (energy8_t)u8PanRSSI[gAppRxPacket->instanceId];
#else
                    e32RssiSum[gAppRxPacket->instanceId] += (energy8_t)u8LastRxRssiValue;
#endif
                    e8AverageRssi[gAppRxPacket->instanceId] =
                        (energy8_t)(e32RssiSum[gAppRxPacket->instanceId]/u16ReceivedPackets[gAppRxPacket->instanceId]);
#if gMpmMaxPANs_c == 2
                    Serial_Print(mAppSer, "Pan: ", gAllowToBlock_d);
                    Serial_PrintDec(mAppSer, (uint32_t)gAppRxPacket->instanceId);
                    Serial_Print(mAppSer,". ", gAllowToBlock_d);
#endif
                    Serial_Print(mAppSer, "Packet ", gAllowToBlock_d);
                    Serial_PrintDec(mAppSer,(uint32_t)u16ReceivedPackets[gAppRxPacket->instanceId]);
                    Serial_Print(mAppSer, ". Packet index: ",gAllowToBlock_d);
                    Serial_PrintDec(mAppSer, (uint32_t)u16PacketsIndex[gAppRxPacket->instanceId]);
                    Serial_Print(mAppSer, ". Rssi during RX: ", gAllowToBlock_d);
#if gMpmMaxPANs_c == 2
                    e8TempRssivalue = (energy8_t)u8PanRSSI[gAppRxPacket->instanceId];
#else
                    e8TempRssivalue = (energy8_t)u8LastRxRssiValue;
#endif
#if CT_Feature_RSSI_Has_Sign
                    if(e8TempRssivalue < 0)
                    {
                        e8TempRssivalue *= -1;
#else
                    if(e8TempRssivalue != 0)
                    {
#endif
                        Serial_Print(mAppSer, "-", gAllowToBlock_d);
                    }
                    Serial_PrintDec(mAppSer, (uint32_t)e8TempRssivalue);
                    Serial_Print(mAppSer, "\r\n", gAllowToBlock_d);
                    if(u16PacketsIndex[gAppRxPacket->instanceId] ==
                       u16TotalPackets[gAppRxPacket->instanceId])
                    {
#if gMpmMaxPANs_c != 2
                        bPrintStatistics = TRUE;
                        SelfNotificationEvent();
                        perRxState = gPerRxStateIdle_c;
#else
                        if(u16PacketsIndex[1-gAppRxPacket->instanceId] ==
                           u16TotalPackets[1-gAppRxPacket->instanceId] &&
                               u16TotalPackets[1-gAppRxPacket->instanceId] != 0)
                        {
                            bPrintStatistics = TRUE;
                            SelfNotificationEvent();
                            perRxState = gPerRxStateIdle_c;
                        }
#endif
                    }
                }
           }
           bRxDone = FALSE;
           if(u16PacketsIndex[gAppRxPacket->instanceId] < u16TotalPackets[gAppRxPacket->instanceId])
           {
               /*set active pan and enter rx after receiving packet*/
               MLMESetActivePan(gAppRxPacket->instanceId);
               gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
               MLMERXEnableRequest(gAppRxPacket, 0);
           }
       }
       if(evDataFromUART)
       {
           if(' ' == gu8UartData)
           {
               u8PanCount = 0;
               do
               {
                   Serial_Print(mAppSer,"\r\n Statistics on PAN", gAllowToBlock_d);
                   Serial_PrintDec(mAppSer, (uint32_t)u8PanCount);
                   Serial_Print(mAppSer, "\r\n", gAllowToBlock_d);
                   MLMESetActivePan((smacMultiPanInstances_t)u8PanCount);
                   (void)MLMERXDisableRequest();
                   Serial_Print(mAppSer,"\r\nAverage Rssi during PER: ",gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
                   if(e8AverageRssi[u8PanCount] < 0)
                   {
                       e8AverageRssi[u8PanCount] *= -1;
#else
                   if(e8AverageRssi[u8PanCount] != 0)
                   {
#endif
                       Serial_Print(mAppSer, "-",gAllowToBlock_d);
                   }
                   Serial_PrintDec(mAppSer, (uint32_t)e8AverageRssi[u8PanCount]);
                   Serial_Print(mAppSer," dBm\r\n",gAllowToBlock_d);
                   Serial_Print(mAppSer, "\n\rPER Test Rx Stopped\r\n\r\n", gAllowToBlock_d);
                   PrintPerRxFinalLine(u16ReceivedPackets[u8PanCount],u16TotalPackets[u8PanCount]);
                }
                while(++u8PanCount < gNumPans_c);
                perRxState = gPerRxStateIdle_c;
           }
           evDataFromUART = FALSE;
       }
            break;
       case gPerRxStateIdle_c:
           if(bPrintStatistics == TRUE)
           {
               bPrintStatistics = FALSE;
               u8PanCount = 0;
               do
               {
                   Serial_Print(mAppSer,"\r\nAverage Rssi during PER: ", gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
                   if(e8AverageRssi[u8PanCount] < 0)
                   {
                       e8AverageRssi[u8PanCount] *= -1;
#else
                   if(e8AverageRssi[u8PanCount] != 0)
                   {
#endif
                       Serial_Print(mAppSer, "-", gAllowToBlock_d);
                   }
                   Serial_PrintDec(mAppSer, (uint32_t)e8AverageRssi[u8PanCount]);
                   Serial_Print(mAppSer," dBm\r\n",gAllowToBlock_d);
#if gMpmMaxPANs_c == 2
                   Serial_Print(mAppSer, "\n\rPER Test Finished on Pan ", gAllowToBlock_d);
                   Serial_PrintDec(mAppSer, u8PanCount);
                   Serial_Print(mAppSer, "\r\n\r\n", gAllowToBlock_d);
#else
                   Serial_Print(mAppSer, "\n\rPER Test Finished\r\n\r\n", gAllowToBlock_d);
#endif
                   PrintPerRxFinalLine(u16ReceivedPackets[u8PanCount],u16TotalPackets[u8PanCount]);
                }
                while(++u8PanCount < gNumPans_c);
           }
           if((evDataFromUART) && ('\r' == gu8UartData))
           {
               MLMESetActivePan(gSmacPan0_c);
               gAppRxPacket = (rxPacket_t*)gau8RxDataBuffer;
               perRxState = gPerRxStateInit_c;
               SelfNotificationEvent();
           }
           evDataFromUART = FALSE;
           break;
       default:
           break;
     }
     return bBackFlag;
}

/************************************************************************************
*
* Range Test Handler for board that is performing TX
*
************************************************************************************/
bool_t RangeTx(void)
{
    bool_t bBackFlag = FALSE;
    static energy32_t e32RSSISum;
    static uint16_t u16ReceivedPackets;
    static uint16_t u16PacketsDropped;
    energy8_t  e8AverageRSSI;
    energy8_t  e8CurrentRSSI;

    if(evTestParameters)
    {
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }

    switch(rangeTxState)
    {
    case gRangeTxStateInit_c:
        e32RSSISum = 0;
        u16ReceivedPackets = 0;
        u16PacketsDropped = 0;
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8RangeTxTestMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        rangeTxState = gRangeTxWaitStartTest_c;
        break;
    case gRangeTxWaitStartTest_c:
        if(evDataFromUART)
        {
            if(' ' == gu8UartData)
            {
                shortCutsEnabled = FALSE;
                Serial_Print(mAppSer, "\f\r\nRange Test Tx Running\r\n", gAllowToBlock_d);
                rangeTxState = gRangeTxStateStartTest_c;
                ConnTestTimers_StartDelay(AppDelayTmr, 200);
            }
            else if('p' == gu8UartData)
            {
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
        }
        break;
    case gRangeTxStateStartTest_c:
        if(!timePassed) //waiting 200 ms
            break;
        timePassed = FALSE;
        bTxDone = FALSE;
        gAppTxPacket->u8DataLength = 16;
        gAppTxPacket->smacPdu.smacPdu[0]  = 0;
        FLib_MemCpy(&(gAppTxPacket->smacPdu.smacPdu[1]), "SMAC Range Demo",15);
        MLMERXDisableRequest();
        ConnTestTimers_StopDelay(RangeTestTmr);
        (void)MCPSDataRequest(gAppTxPacket);
        rangeTxState = gRangeTxStateRunningTest_c;
        break;
    case gRangeTxStateRunningTest_c:
        if(bTxDone)
        {
            ConnTestTimers_StartDelay(RangeTestTmr, 80);
            SetRadioRxOnNoTimeOut();
            rangeTxState = gRangeTxStatePrintTestResults_c;
        }
        break;
    case gRangeTxStatePrintTestResults_c:
        if(bRxDone)
        {
            if(gAppRxPacket->rxStatus == rxSuccessStatus_c)
            {
                if(stringComp((uint8_t*)"SMAC Range Demo",&gAppRxPacket->smacPdu.smacPdu[1],15))
                {
                    e8CurrentRSSI = (energy8_t)(gAppRxPacket->smacPdu.smacPdu[0]);
                    e32RSSISum += e8CurrentRSSI;

                    u16ReceivedPackets++;
                    e8AverageRSSI = (energy8_t)(e32RSSISum/u16ReceivedPackets);
                    Serial_Print(mAppSer, "\r\n RSSI = ", gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
                    if(e8CurrentRSSI < 0)
                    {
                        e8CurrentRSSI *= -1;
#else
                    if(e8CurrentRSSI !=0)
                    {
#endif
                        Serial_Print(mAppSer, "-", gAllowToBlock_d);
                    }
                    Serial_PrintDec(mAppSer, (uint32_t)e8CurrentRSSI);
                    Serial_Print(mAppSer," dBm", gAllowToBlock_d);
                }
                else
                {
                    ConnTestTimers_StartDelay(RangeTestTmr, 80);
                    SetRadioRxOnNoTimeOut();
                }
            }
            else
            {
                u16PacketsDropped++;
                Serial_Print(mAppSer, "\r\nPacket Dropped", gAllowToBlock_d);
                bRxDone= FALSE;
            }
            if(evDataFromUART && (' ' == gu8UartData))
            {
                Serial_Print(mAppSer, "\n\r\n\rRange Test Tx Stopped\r\n\r\n", gAllowToBlock_d);
                e8AverageRSSI = (energy8_t)(e32RSSISum/u16ReceivedPackets);
                Serial_Print(mAppSer, "Average RSSI     ", gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
                if(e8AverageRSSI < 0)
                {
                    e8AverageRSSI *= -1;
#else
                if(e8AverageRSSI != 0)
                {
#endif
                    Serial_Print(mAppSer, "-", gAllowToBlock_d);

                }
                Serial_PrintDec(mAppSer, (uint32_t) e8AverageRSSI);

                Serial_Print(mAppSer," dBm", gAllowToBlock_d);
                Serial_Print(mAppSer, "\r\nPackets dropped ", gAllowToBlock_d);
                Serial_PrintDec(mAppSer, (uint32_t)u16PacketsDropped);
                Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Range Tx test menu", gAllowToBlock_d);
                rangeTxState = gRangeTxStateIdle_c;
                (void)MLMERXDisableRequest();
                ConnTestTimers_StopDelay(AppDelayTmr);
                timePassed = FALSE;
            }
            else
            {
                rangeTxState = gRangeTxStateStartTest_c;
                ConnTestTimers_StartDelay(AppDelayTmr, 200);
            }
            evDataFromUART = FALSE;
       }
       break;
     case gRangeTxStateIdle_c:
         if((evDataFromUART) && ('\r' == gu8UartData))
         {
             rangeTxState = gRangeTxStateInit_c;
             SelfNotificationEvent();
         }
         evDataFromUART = FALSE;
         break;
     default:
         break;
    }
    return bBackFlag;
}

/************************************************************************************
*
* Range Test Handler for board that is performing RX
*
************************************************************************************/
bool_t RangeRx(void)
{
    bool_t bBackFlag = FALSE;
    static energy32_t e32RSSISum;
    static uint16_t u16ReceivedPackets;
    energy8_t  e8AverageRSSI;
    energy8_t  e8CurrentRSSI;

    if(evTestParameters)
    {
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }
    switch(rangeRxState)
    {
    case gRangeRxStateInit_c:
        e32RSSISum = 0;
        u16ReceivedPackets = 0;
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8RangeRxTestMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        rangeRxState = gRangeRxWaitStartTest_c;
        break;
    case gRangeRxWaitStartTest_c:
        if(evDataFromUART)
        {
            if(' ' == gu8UartData)
            {
                shortCutsEnabled = FALSE;
                Serial_Print(mAppSer, "\f\r\nRange Test Rx Running\r\n", gAllowToBlock_d);
                rangeRxState = gRangeRxStateStartTest_c;
            }
            else if('p' == gu8UartData)
            {
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
            SelfNotificationEvent();
        }
        break;
    case gRangeRxStateStartTest_c:
        SetRadioRxOnNoTimeOut();
        rangeRxState = gRangeRxStateRunningTest_c;
        break;
    case gRangeRxStateRunningTest_c:
        if(evDataFromUART && (' ' == gu8UartData))
        {
            (void)MLMERXDisableRequest();
            Serial_Print(mAppSer, "\n\r\n\rRange Test Rx Stopped\r\n\r\n", gAllowToBlock_d);
            e8AverageRSSI = (energy8_t)(e32RSSISum/u16ReceivedPackets);
            Serial_Print(mAppSer, "Average RSSI     ", gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
            if(e8AverageRSSI < 0)
            {
                e8AverageRSSI *= -1;
#else
            if(e8AverageRSSI != 0)
            {
#endif
                Serial_Print(mAppSer, "-", gAllowToBlock_d);
            }
            Serial_PrintDec(mAppSer, (uint32_t) e8AverageRSSI);
            Serial_Print(mAppSer," dBm", gAllowToBlock_d);
            Serial_Print(mAppSer, "\r\n\r\n Press [enter] to go back to the Range Rx test menu", gAllowToBlock_d);
            rangeRxState = gRangeRxStateIdle_c;
        }
        evDataFromUART = FALSE;
        if(bRxDone)
        {
            if(gAppRxPacket->rxStatus == rxSuccessStatus_c)
            {
                if(stringComp((uint8_t*)"SMAC Range Demo",&gAppRxPacket->smacPdu.smacPdu[1],15))
                {
                    bRxDone = FALSE;
                    ConnTestTimers_StartDelay(AppDelayTmr, 4);
                }
                else
                {
                    SetRadioRxOnNoTimeOut();
                }
            }
            else
            {
                SetRadioRxOnNoTimeOut();
            }
        }
        if(timePassed)
        {
            timePassed = FALSE;
            bTxDone = FALSE;
            gAppTxPacket->smacPdu.smacPdu[0] = u8LastRxRssiValue;
            FLib_MemCpy(&(gAppTxPacket->smacPdu.smacPdu[1]), "SMAC Range Demo",15);
            gAppTxPacket->u8DataLength = 16;
            (void)MCPSDataRequest(gAppTxPacket);
            rangeRxState = gRangeRxStatePrintTestResults_c;
        }
        break;
    case gRangeRxStatePrintTestResults_c:
        if(bTxDone)
        {
            e8CurrentRSSI= (energy8_t)u8LastRxRssiValue;
            e32RSSISum += e8CurrentRSSI;
            u16ReceivedPackets++;
            e8AverageRSSI = (uint8_t)(e32RSSISum/u16ReceivedPackets);
            Serial_Print(mAppSer, "\r\n RSSI = ", gAllowToBlock_d);
#if CT_Feature_RSSI_Has_Sign
            if(e8CurrentRSSI < 0)
            {
                e8CurrentRSSI *= -1;
#else
            if(e8CurrentRSSI != 0)
            {
#endif
                Serial_Print(mAppSer, "-" , gAllowToBlock_d);
            }
            Serial_PrintDec(mAppSer, (uint32_t) e8CurrentRSSI);
            Serial_Print(mAppSer," dBm", gAllowToBlock_d);
            rangeRxState = gRangeRxStateStartTest_c;
            SelfNotificationEvent();
        }
        break;
    case gRangeRxStateIdle_c:
        if((evDataFromUART) && ('\r' == gu8UartData))
        {
            rangeRxState = gRangeRxStateInit_c;
            SelfNotificationEvent();
        }
        evDataFromUART = FALSE;
        break;
    default:
        break;
    }
    return bBackFlag;
}

/************************************************************************************
*
* Handler for viewing/modifying XCVR registers
*
************************************************************************************/
bool_t EditRegisters(void)
{
    bool_t bBackFlag = FALSE;
    if(evTestParameters)
    {
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMESetChannelRequest(testChannel);
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }

    switch(eRState)
    {
    case gERStateInit_c:
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8RadioRegistersEditMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        eRState = gERWaitSelection_c;
        break;
    case gERWaitSelection_c:
        if(evDataFromUART)
        {
#if CT_Feature_Direct_Registers
            if('1' == gu8UartData)
            {
                bIsRegisterDirect = TRUE;
                oRState = gORStateInit_c;
                eRState = gERStateOverrideReg_c;
                SelfNotificationEvent();
            }
            else if('2' == gu8UartData)
            {
                bIsRegisterDirect = TRUE;
                rRState = gRRStateInit_c;
                eRState = gERStateReadReg_c;
                SelfNotificationEvent();
            }
#if CT_Feature_Indirect_Registers
            else if('3' == gu8UartData)
            {
                bIsRegisterDirect = FALSE;
                oRState = gORStateInit_c;
                eRState = gERStateOverrideReg_c;
                SelfNotificationEvent();
            }
            else if('4' == gu8UartData)
            {
                bIsRegisterDirect = FALSE;
                rRState = gRRStateInit_c;
                eRState = gERStateReadReg_c;
                SelfNotificationEvent();
            }
            else if('5' == gu8UartData)
            {
                dRState = gDRStateInit_c;
                eRState  = gERStateDumpAllRegs_c;
                SelfNotificationEvent();
            }
#else
            else if('3' == gu8UartData)
            {
                dRState = gDRStateInit_c;
                eRState  = gERStateDumpAllRegs_c;
                SelfNotificationEvent();
            }
#endif
            else
#endif
                if('p' == gu8UartData)
                {
                    bBackFlag = TRUE;
                }
            evDataFromUART = FALSE;
        }
        break;
    case gERStateOverrideReg_c:
        if(OverrideRegisters())
        {
            eRState = gERStateInit_c;
            SelfNotificationEvent();
        }
        break;
    case gERStateReadReg_c:
        if(ReadRegisters())
        {
            eRState = gERStateInit_c;
            SelfNotificationEvent();
        }
        break;
    case gERStateDumpAllRegs_c:
        if(DumpRegisters()) {
            eRState = gERStateInit_c;
            SelfNotificationEvent();
        }
        break;
    default:
        break;
    }
    return bBackFlag;
}

/************************************************************************************
*
* Dump registers
*
************************************************************************************/
bool_t DumpRegisters(void)
{
#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
    bool_t bBackFlag = FALSE;

    switch(dRState)
    {
    case gDRStateInit_c:
        Serial_Print(mAppSer, "\f\r\rDump Registers\r\n", gAllowToBlock_d);
        Serial_Print(mAppSer, "\r\n-Press [space] to dump registers\r\n", gAllowToBlock_d);
        Serial_Print(mAppSer, "\r\n-Press [p] Previous Menu\r\n", gAllowToBlock_d);
        shortCutsEnabled = FALSE;
        dRState = gDRStateDumpRegs_c;
        SelfNotificationEvent();
        break;
    case gDRStateDumpRegs_c:
        if(evDataFromUART){
            if(gu8UartData == 'p')
            {
                bBackFlag = TRUE;
            }
            else if (gu8UartData == ' ')
            {
                Serial_Print(mAppSer, "\r\n -Dumping registers... \r\n", gAllowToBlock_d);
                const registerLimits_t* interval = registerIntervals;

                while(!((*interval).regStart == 0 && (*interval).regEnd == 0))
                {
                    Serial_Print(mAppSer, "\r\n -Access type: ", gAllowToBlock_d);
                    if( (*interval).bIsRegisterDirect )
                    {
                        Serial_Print(mAppSer,"direct\r\n", gAllowToBlock_d);
                    }
                    else
                    {
                        Serial_Print(mAppSer,"indirect\r\n", gAllowToBlock_d);
                    }
                    bIsRegisterDirect = (*interval).bIsRegisterDirect;
                    ReadRFRegs((*interval).regStart, (*interval).regEnd);
                    interval++;
                }
                dRState = gDRStateInit_c;
                SelfNotificationEvent();
            }
        }
        evDataFromUART = FALSE;
        break;
    default:
        break;
    }
    return bBackFlag;
#else
    return TRUE;
#endif
}

/************************************************************************************
*
* Read and print register values with addresses from u8RegStartAddress
* to u8RegStopAddress
*
************************************************************************************/
void ReadRFRegs(registerAddressSize_t rasRegStartAddress, registerAddressSize_t rasRegStopAddress)
{
#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
    static uint16_t rasRegAddress;
    registerSize_t rsRegValue;
    Serial_Print(mAppSer, " ---------------------------------------  ", gAllowToBlock_d);
    for(rasRegAddress = rasRegStartAddress;
        rasRegAddress <= rasRegStopAddress;
        rasRegAddress += (gRegisterSize_c) )
    {
        Serial_Print(mAppSer, "\r\n|    Address : 0x", gAllowToBlock_d);
        Serial_PrintHex(mAppSer, (uint8_t*)&rasRegAddress,gRegisterSize_c,0);

        aspTestRequestMsg.msgType = aspMsgTypeXcvrReadReq_c;
        aspTestRequestMsg.msgData.aspXcvrData.addr = (uint16_t)rasRegAddress;
        aspTestRequestMsg.msgData.aspXcvrData.len  = gRegisterSize_c;
        aspTestRequestMsg.msgData.aspXcvrData.mode = !bIsRegisterDirect;

        APP_ASP_SapHandler(&aspTestRequestMsg, 0);
        rsRegValue = *((registerSize_t*)aspTestRequestMsg.msgData.aspXcvrData.data);

        Serial_Print(mAppSer, " Data value : 0x", gAllowToBlock_d);
        Serial_PrintHex(mAppSer, (uint8_t*)&rsRegValue, gRegisterSize_c, 0);
        Serial_Print(mAppSer, "   |", gAllowToBlock_d);
    }
    Serial_Print(mAppSer, "\r\n ---------------------------------------  \r\n", gAllowToBlock_d);
#endif
}

/************************************************************************************
*
* Read register
*
************************************************************************************/
bool_t ReadRegisters(void)
{
#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
    bool_t bBackFlag = FALSE;
    static uint8_t au8RxString[5];
    static uint8_t u8Index;
    static registerAddressSize_t rasRegAddress;
    static registerSize_t rsRegValue;
    static char    auxToPrint[2];

    switch(rRState)
    {
    case gRRStateInit_c:
        Serial_Print(mAppSer, "\f\r\rRead Registers\r\n", gAllowToBlock_d);
        Serial_Print(mAppSer, "\r\n-Press [p] Previous Menu\r\n", gAllowToBlock_d);
        shortCutsEnabled = FALSE;
        rRState = gRRStateStart_c;
        SelfNotificationEvent();
        break;
    case gRRStateStart_c:
        Serial_Print(mAppSer, "\r\n -write the Register address in Hex and [enter]: 0x", gAllowToBlock_d);
        u8Index = 0;
        rRState = gRRWaitForTheAddress_c;
        break;
    case gRRWaitForTheAddress_c:
        if(evDataFromUART)
        {
            if((!isAsciiHex(gu8UartData)) && ('\r' != gu8UartData))
            {
                if('p' == gu8UartData)
                {
                    bBackFlag = TRUE;
                }
                else
                {
                    Serial_Print(mAppSer, "\r\n -Invalid Character!! ", gAllowToBlock_d);
                    rRState = gRRStateStart_c;
                    SelfNotificationEvent();
                }
            }
            else if((gRegisterAddressASCII_c == u8Index) && ('\r' != gu8UartData))
            {
                Serial_Print(mAppSer, "\r\n -Value out of Range!! ", gAllowToBlock_d);
                rRState = gRRStateStart_c;
                SelfNotificationEvent();
            }
            else if(isAsciiHex(gu8UartData))
            {
                au8RxString[u8Index++] = gu8UartData;
                auxToPrint[0] = gu8UartData;
                auxToPrint[1] = '\0';
                Serial_Print(mAppSer, auxToPrint, gAllowToBlock_d);
            }
            else
            {
                au8RxString[u8Index] = 0;
                rasRegAddress = (registerAddressSize_t)HexString2Dec(au8RxString);
                aspTestRequestMsg.msgType = aspMsgTypeXcvrReadReq_c;
                aspTestRequestMsg.msgData.aspXcvrData.addr = (uint16_t)rasRegAddress;
                aspTestRequestMsg.msgData.aspXcvrData.len  = gRegisterSize_c;
                aspTestRequestMsg.msgData.aspXcvrData.mode = !bIsRegisterDirect;
                APP_ASP_SapHandler(&aspTestRequestMsg, 0);
                rsRegValue = *((registerSize_t*)aspTestRequestMsg.msgData.aspXcvrData.data);

                Serial_Print(mAppSer, "\r\n -Register value : 0x", gAllowToBlock_d);
                Serial_PrintHex(mAppSer, (uint8_t*)&rsRegValue,gRegisterSize_c,0);
                Serial_Print(mAppSer, "\r\n", gAllowToBlock_d);

                rRState = gRRStateStart_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    default:
        break;
    }
    return bBackFlag;
#else
    return TRUE;
#endif
}

/************************************************************************************
*
* Override Register
*
************************************************************************************/
bool_t OverrideRegisters(void)
{
#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
    bool_t bBackFlag = FALSE;
    static uint8_t au8RxString[5];
    static uint8_t u8Index;
    static registerAddressSize_t rasRegAddress;
    static registerSize_t rsRegValue;
    static char auxToPrint[2];

    switch(oRState)
    {
    case gORStateInit_c:
        Serial_Print(mAppSer, "\f\r\nWrite Registers\r\n", gAllowToBlock_d);
        Serial_Print(mAppSer, "\r\n-Press [p] Previous Menu\r\n", gAllowToBlock_d);
        shortCutsEnabled = FALSE;
        oRState = gORStateStart_c;
        SelfNotificationEvent();
        break;
    case gORStateStart_c:
        Serial_Print(mAppSer, "\r\n -write the Register address in Hex and [enter]: 0x", gAllowToBlock_d);
        u8Index = 0;
        oRState = gORWaitForTheAddress_c;
        break;
    case gORWaitForTheAddress_c:
        if(evDataFromUART){
            if((!isAsciiHex(gu8UartData)) && ('\r' != gu8UartData))
            {
                if('p' == gu8UartData)
                {
                    bBackFlag = TRUE;
                }
                else
                {
                    Serial_Print(mAppSer, "\r\n -Invalid Character!! ", gAllowToBlock_d);
                    oRState = gORStateStart_c;
                    SelfNotificationEvent();
                }
            }
            else if((gRegisterAddressASCII_c == u8Index) && ('\r' != gu8UartData))
            {
                Serial_Print(mAppSer, "\r\n -Value out of Range!! ", gAllowToBlock_d);
                oRState = gORStateStart_c;
                SelfNotificationEvent();
            }
            else if(isAsciiHex(gu8UartData))
            {
                au8RxString[u8Index++] = gu8UartData;
                auxToPrint[0] = gu8UartData;
                auxToPrint[1] = '\0';
                Serial_Print(mAppSer, auxToPrint, gAllowToBlock_d);
            }
            else
            {
                au8RxString[u8Index] = 0;
                rasRegAddress = (registerAddressSize_t)HexString2Dec(au8RxString);
                Serial_Print(mAppSer, "\r\n -write the Register value to override in Hex and [enter]: 0x", gAllowToBlock_d);
                u8Index = 0;
                oRState = gORWaitForTheValue_c;
            }
            evDataFromUART = FALSE;
        }
        break;
    case gORWaitForTheValue_c:
        if(evDataFromUART)
        {
            if((!isAsciiHex(gu8UartData)) && ('\r' != gu8UartData))
            {
                if('p' == gu8UartData)
                {
                    bBackFlag = TRUE;
                }
                else
                {
                    Serial_Print(mAppSer, "\r\n -Invalid Character!! ", gAllowToBlock_d);
                    oRState = gORStateStart_c;
                    SelfNotificationEvent();
                }
            }
            else if((2 == u8Index) && ('\r' != gu8UartData))
            {
                Serial_Print(mAppSer, "\r\n -Value out of Range!! ", gAllowToBlock_d);
                oRState = gORStateStart_c;
                SelfNotificationEvent();
            }
            else if(isAsciiHex(gu8UartData))
            {
                au8RxString[u8Index++] = gu8UartData;
                auxToPrint[0] = gu8UartData;
                auxToPrint[1] = '\0';
                Serial_Print(mAppSer, auxToPrint, gAllowToBlock_d);
            }
            else
            {
                au8RxString[u8Index] = 0;
                rsRegValue = (registerSize_t)HexString2Dec(au8RxString);
                aspTestRequestMsg.msgType = aspMsgTypeXcvrWriteReq_c;
                aspTestRequestMsg.msgData.aspXcvrData.addr = (uint16_t)rasRegAddress;
                aspTestRequestMsg.msgData.aspXcvrData.len  = gRegisterAddress_c;
                aspTestRequestMsg.msgData.aspXcvrData.mode = !bIsRegisterDirect;
                FLib_MemCpy(aspTestRequestMsg.msgData.aspXcvrData.data, &rsRegValue, gRegisterSize_c);
                APP_ASP_SapHandler(&aspTestRequestMsg, 0);

                Serial_Print(mAppSer, "\r\n Register overridden \r\n", gAllowToBlock_d);
                u8Index = 0;
                oRState = gORStateStart_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    default:
        break;
    }
    return bBackFlag;
#else
    return TRUE;
#endif
}

/************************************************************************************
*
* Handler for Carrier Sense Test and Transmission Control Test
*
************************************************************************************/
bool_t CSenseAndTCtrl(void)
{
    bool_t bBackFlag = FALSE;
    static uint8_t testSelector = 0;

    if(evTestParameters)
    {
        (void)MLMESetChannelRequest(testChannel);
#if CT_Feature_Calibration
        (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
        (void)MLMEPAOutputAdjust(testPower);
#if CT_Feature_Xtal_Trim
        aspTestRequestMsg.msgType = aspMsgTypeSetXtalTrimReq_c;
        aspTestRequestMsg.msgData.aspXtalTrim.trim = xtalTrimValue;
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);
#endif
        PrintTestParameters(TRUE);
        evTestParameters = FALSE;
    }

    switch(cstcState)
    {
    case gCsTcStateInit_c:
        TestMode(gTestModeForceIdle_c);
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8RadioCSTCSelectMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        shortCutsEnabled = TRUE;
        bTxDone = FALSE;
        bScanDone = FALSE;
        timePassed = FALSE;

        cstcState = gCsTcStateSelectTest_c;
        break;
    case gCsTcStateSelectTest_c:
        if(evDataFromUART)
        {
            if('1' == gu8UartData)
            {
                cstcState = gCsTcStateCarrierSenseStart_c;
                testSelector = 1;
                SelfNotificationEvent();
            }
            else if ('2' == gu8UartData)
            {
                cstcState = gCsTcStateTransmissionControlStart_c;
                testSelector = 2;
                SelfNotificationEvent();
            }
            else if( 'p' == gu8UartData)
            {
                cstcState = gCsTcStateInit_c;
                testSelector = 0;
                bBackFlag = TRUE;
            }
            evDataFromUART = FALSE;
        }
        break;
    default:
        if(testSelector == 1)
            CarrierSenseHandler();
        else if(testSelector == 2)
            TransmissionControlHandler();
        break;
    }
    return bBackFlag;
}

/************************************************************************************
*
* Handler for Transmission Control Test called by above function
*
************************************************************************************/
void TransmissionControlHandler(void)
{
    uint32_t totalTimeMs;
    const uint16_t u16TotalPacketsOptions[] = {1,25,100,500,1000,2000,5000,10000,65535};
    static uint16_t u16TotalPackets;
    static uint16_t u16PacketCounter = 0;
    static uint16_t miliSecDelay = 0;
    static phyTime_t startTime;
    int8_t fillIndex = 0;
    uint8_t* smacPduPtr;
    energy8_t e8TempRssivalue;

    switch(cstcState)
    {
    case gCsTcStateTransmissionControlStart_c:
        PrintMenu(cu8ShortCutsBar, (serial_write_handle_t)g_connWriteHandle);
        PrintMenu(cu8CsTcTestMenu, (serial_write_handle_t)g_connWriteHandle);
        PrintTestParameters(FALSE);
        miliSecDelay = 0;
        u16TotalPackets = 0;
        u16PacketCounter = 0;
        fillIndex = testPayloadLen / gPrbs9BufferLength_c;

        while(fillIndex > 0)
        {
            fillIndex--;
            smacPduPtr = gAppTxPacket->smacPdu.smacPdu + fillIndex * gPrbs9BufferLength_c;
            FLib_MemCpy(smacPduPtr, u8Prbs9Buffer, gPrbs9BufferLength_c);
        }
        smacPduPtr = gAppTxPacket->smacPdu.smacPdu + ((testPayloadLen / gPrbs9BufferLength_c)*gPrbs9BufferLength_c);
        FLib_MemCpy(smacPduPtr, u8Prbs9Buffer, (testPayloadLen % gPrbs9BufferLength_c));

        gAppTxPacket->u8DataLength = testPayloadLen;

        cstcState = gCsTcStateTransmissionControlSelectNumOfPackets_c;
        break;
    case gCsTcStateTransmissionControlSelectNumOfPackets_c:
        if(evDataFromUART)
        {
            if((gu8UartData >= '0') && (gu8UartData <= '8'))
            {
                u16TotalPackets = u16TotalPacketsOptions[gu8UartData - '0'];
                cstcState = gCsTcStateTransmissionControlSelectInterpacketDelay_c;
                Serial_Print(mAppSer,"\r\n\r\n Please type InterPacket delay in miliseconds and press [ENTER]",gAllowToBlock_d);
                Serial_Print(mAppSer,"\r\n(During test, exit by pressing [SPACE])\r\n\r\n",gAllowToBlock_d);
                SelfNotificationEvent();
            }
            else if('p' == gu8UartData)
            {
                cstcState = gCsTcStateInit_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    case gCsTcStateTransmissionControlSelectInterpacketDelay_c:
        if(evDataFromUART)
        {
            if(gu8UartData == '\r' && miliSecDelay != 0)
            {
                cstcState = gCsTcStateTransmissionControlPerformingTest_c;
                startTime = GetTimestampUS();
                (void)MLMEScanRequest(testChannel);
            }
            else if((gu8UartData >= '0') && (gu8UartData <='9'))
            {
                miliSecDelay = miliSecDelay*10 + (gu8UartData - '0');
                Serial_PrintDec(mAppSer, (uint32_t)(gu8UartData - '0'));
            }
            else if('p' == gu8UartData)
            {
                cstcState = gCsTcStateInit_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    case gCsTcStateTransmissionControlPerformingTest_c:
        if(bScanDone)
        {
            bScanDone = FALSE;
            (void)MCPSDataRequest(gAppTxPacket);
        }
        if(bTxDone)
        {
            bTxDone = FALSE;
            u16PacketCounter++;
            Serial_Print(mAppSer,"\r\n\tPacket number: ",gAllowToBlock_d);
            Serial_PrintDec(mAppSer, (uint32_t)(u16PacketCounter));
            Serial_Print(mAppSer, "; RSSI value: ", gAllowToBlock_d);
            e8TempRssivalue = (energy8_t) au8ScanResults[testChannel];
#if CT_Feature_RSSI_Has_Sign
            if(e8TempRssivalue < 0)
            {
                e8TempRssivalue *= -1;
#else
            if(e8TempRssivalue != 0)
            {
#endif
                Serial_Print(mAppSer, "-", gAllowToBlock_d);
            }
            Serial_PrintDec(mAppSer, (uint32_t)(uint8_t)e8TempRssivalue);
            Serial_Print(mAppSer," dBm\r\n",gAllowToBlock_d);
            if(u16PacketCounter < u16TotalPackets)
            {
                totalTimeMs  = (uint32_t)(GetTimestampUS() - startTime);
                totalTimeMs -= GetTransmissionTime(testPayloadLen, crtBitrate);
                totalTimeMs = (totalTimeMs % 1000 < 500) ? totalTimeMs/1000 : (totalTimeMs/1000)+1;
                if(totalTimeMs > miliSecDelay)
                {
                    Serial_Print(mAppSer, " Overhead + Transmission + ED = ~",gAllowToBlock_d);
                    Serial_PrintDec(mAppSer, totalTimeMs);
                    Serial_Print(mAppSer,"ms\r\n Interpacket delay too small (Press [ENTER] to continue)\r\n",gAllowToBlock_d);
                    cstcState = gCsTcStateTransmissionControlEndTest_c;
                    SelfNotificationEvent();
                    break;
                }
                ConnTestTimers_StartDelay(AppDelayTmr, miliSecDelay - totalTimeMs);
            }
            else
            {
                Serial_Print(mAppSer,"\r\n\r\nFinished transmitting ",gAllowToBlock_d);
                Serial_PrintDec(mAppSer, (uint32_t)u16TotalPackets);
                Serial_Print(mAppSer," packets!\r\n\r\n",gAllowToBlock_d);
                Serial_Print(mAppSer,"\r\n -Press [ENTER] to end Transmission Control Test", gAllowToBlock_d);
                cstcState = gCsTcStateTransmissionControlEndTest_c;
            }
        }
        if(timePassed)
        {
            timePassed = FALSE;
            startTime = GetTimestampUS();
            (void)MLMEScanRequest(testChannel);
        }
        if(evDataFromUART && gu8UartData == ' ')
        {
            Serial_Print(mAppSer,"\r\n\r\n-Test interrupted by user. Press [ENTER] to continue\r\n\r\n",gAllowToBlock_d);
            cstcState = gCsTcStateTransmissionControlEndTest_c;
        }
        break;
    case gCsTcStateTransmissionControlEndTest_c:
        if(evDataFromUART && gu8UartData == '\r')
        {
            cstcState = gCsTcStateInit_c;
            SelfNotificationEvent();
        }
        evDataFromUART = FALSE;
        break;
    default:
        break;
    }
}
/************************************************************************************
*
* Handler for Carrier Sense Test
*
************************************************************************************/
void CarrierSenseHandler(void)
{
    int8_t fillIndex = 0;
    uint8_t* smacPduPtr;
    energy8_t e8TempRssivalue;

    switch(cstcState)
    {
    case gCsTcStateCarrierSenseStart_c:
#if CT_Feature_Calibration
        if( gMode1Bitrate_c == crtBitrate )
        {
            (void)MLMESetAdditionalRFOffset(gOffsetIncrement + 30);
        }
        else
        {
            (void)MLMESetAdditionalRFOffset(gOffsetIncrement + 60);
        }
#endif
        (void)MLMESetChannelRequest(testChannel);

        Serial_Print(mAppSer, "\r\n\r\n Press [SPACE] to begin/interrupt test",gAllowToBlock_d);
        Serial_Print(mAppSer,  "\r\n Press [p] to return to previous menu", gAllowToBlock_d);
        shortCutsEnabled = FALSE;
        Serial_Print(mAppSer,"\r\n",gAllowToBlock_d);

        fillIndex = testPayloadLen / gPrbs9BufferLength_c;
        while(fillIndex > 0)
        {
            fillIndex--;
            smacPduPtr = gAppTxPacket->smacPdu.smacPdu + fillIndex * gPrbs9BufferLength_c;
            FLib_MemCpy(smacPduPtr, u8Prbs9Buffer, gPrbs9BufferLength_c);
        }
        smacPduPtr = gAppTxPacket->smacPdu.smacPdu + ((testPayloadLen / gPrbs9BufferLength_c)*gPrbs9BufferLength_c);
        FLib_MemCpy(smacPduPtr, u8Prbs9Buffer, (testPayloadLen % gPrbs9BufferLength_c));

        gAppTxPacket->u8DataLength = testPayloadLen;

        cstcState = gCsTcStateCarrierSenseSelectType_c;
        break;
    case gCsTcStateCarrierSenseSelectType_c:
        if(evDataFromUART)
        {
            if(' ' == gu8UartData)
            {
                cstcState = gCsTcStateCarrierSensePerformingTest_c;
                (void)MLMEScanRequest(testChannel);
            }
            else if ('p' == gu8UartData)
            {
#if CT_Feature_Calibration
                (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
                (void)MLMESetChannelRequest(testChannel);
                cstcState = gCsTcStateInit_c;
                SelfNotificationEvent();
            }
            evDataFromUART = FALSE;
        }
        break;
    case gCsTcStateCarrierSensePerformingTest_c:
        if(bScanDone)
        {
            bScanDone = FALSE;
            Serial_Print(mAppSer, "\r\n\tSampling done. RSSI value: ", gAllowToBlock_d);
            e8TempRssivalue = (energy8_t)au8ScanResults[testChannel];
#if CT_Feature_RSSI_Has_Sign
            if(e8TempRssivalue < 0)
            {
                e8TempRssivalue *= -1;
#else
            if(e8TempRssivalue != 0)
            {
#endif
                Serial_Print(mAppSer, "-", gAllowToBlock_d);
            }
            Serial_PrintDec(mAppSer, (uint32_t)(uint8_t)e8TempRssivalue);
            Serial_Print(mAppSer, "dBm", gAllowToBlock_d);
            if(e8TempRssivalue > ccaThresh)
            {
                (void)MCPSDataRequest(gAppTxPacket);
            }
            else
            {
                (void)MLMEScanRequest(testChannel);
            }
        }
        if(bTxDone)
        {
            bTxDone = FALSE;

            Serial_Print(mAppSer,"\r\n Transmission Performed\r\n", gAllowToBlock_d);
            Serial_Print(mAppSer,"\r\n -Press [ENTER] to end Carrier Sense Test", gAllowToBlock_d);
            cstcState = gCsTcStateCarrierSenseEndTest_c;
        }
        if(evDataFromUART && gu8UartData == ' ')
        {
            Serial_Print(mAppSer,"\r\n\r\n-Test interrupted by user. Press [ENTER] to continue\r\n\r\n",gAllowToBlock_d);
            cstcState = gCsTcStateCarrierSenseEndTest_c;
        }
        break;
    case gCsTcStateCarrierSenseEndTest_c:
        if(evDataFromUART && gu8UartData == '\r')
        {
#if CT_Feature_Calibration
            (void)MLMESetAdditionalRFOffset(gOffsetIncrement);
#endif
            (void)MLMESetChannelRequest(testChannel);
            cstcState = gCsTcStateInit_c;
            SelfNotificationEvent();
        }
        evDataFromUART = FALSE;
        break;
    default:
        break;
    }
}

/************************************************************************************
*
* Auxiliary Functions
*
************************************************************************************/

/**************************************************************************************/
void SetRadioRxOnNoTimeOut(void)
{
    bRxDone = FALSE;
    gAppRxPacket->u8MaxDataLength = gMaxSmacSDULength_c;
    (void)MLMERXEnableRequest(gAppRxPacket, 0);
}

/**************************************************************************************/
void PrintPerRxFinalLine(uint16_t u16Received, uint16_t u16Total)
{
    Serial_Print(mAppSer,"Received ", gAllowToBlock_d);
    Serial_PrintDec(mAppSer,(uint32_t)u16Received);
    Serial_Print(mAppSer," of ", gAllowToBlock_d);
    Serial_PrintDec(mAppSer,(uint32_t)u16Total);
    Serial_Print(mAppSer," packets transmitted \r\n", gAllowToBlock_d);
    Serial_Print(mAppSer,"\r\n Press [enter] to go back to the Per Rx test menu", gAllowToBlock_d);
}

/************************************************************************************
*
*
* By employing this function, users can execute a test of the radio. Test mode
* implements the following:
*   -PRBS9 Mode,
*   -Force_idle,
*   -Continuos TX without modulation,
*   -Continuos TX with modulation.(0's,1's and PN patterns)
*
************************************************************************************/
smacErrors_t TestMode
(
smacTestMode_t  mode  /*IN: The test mode to start.*/
)
{
    aspTestRequestMsg.msgType = aspMsgTypeTelecTest_c;

#if(TRUE == smacParametersValidation_d)
    if(gMaxTestMode_c <= mode)
    {
        return gErrorOutOfRange_c;
    }
#endif

    if(gTestModeForceIdle_c == mode)
    {
        MLMEPhySoftReset();
        aspTestRequestMsg.msgData.aspTelecTest.mode = gTestForceIdle_c;
    }
    else if(gTestModeContinuousTxModulated_c == mode)
    {
        if(contTxModBitValue==gContTxModSelectOnes_c)
        {
            aspTestRequestMsg.msgData.aspTelecTest.mode = gTestContinuousTxModOne_c;
        }
        else if(contTxModBitValue == gContTxModSelectZeros_c)
        {
            aspTestRequestMsg.msgData.aspTelecTest.mode = gTestContinuousTxModZero_c;
        }
        else if(contTxModBitValue == gContTxModSelectPN9_c)
        {
#ifdef gPHY_802_15_4g_d
            aspTestRequestMsg.msgData.aspTelecTest.mode = gTestContinuousTxContPN9_c;
#else
            aspTestRequestMsg.msgData.aspTelecTest.mode = gTestPulseTxPrbs9_c;
#endif
        }
    }
    else if(gTestModeContinuousTxUnmodulated_c == mode)
    {
        aspTestRequestMsg.msgData.aspTelecTest.mode = gTestContinuousTxNoMod_c;
    }
    else if(gTestModeContinuousRxBER_c == mode)
    {
        aspTestRequestMsg.msgData.aspTelecTest.mode = gTestContinuousRx_c;
    }
    else if(gTestModePRBS9_c == mode)
    {
        /*Set Data Mode*/
        gAppTxPacket->u8DataLength = gPrbs9BufferLength_c;
        FLib_MemCpy(gAppTxPacket->smacPdu.smacPdu, u8Prbs9Buffer, gPrbs9BufferLength_c);
        PacketHandler_Prbs9();
    }
    if(gTestModePRBS9_c != mode)
        (void)APP_ASP_SapHandler(&aspTestRequestMsg, 0);

    return gErrorNoError_c;
}

/************************************************************************************
* PacketHandler_Prbs9
*
* This function sends OTA the content of a PRBS9 polynomial of 65 bytes of payload.
*
*
************************************************************************************/
void PacketHandler_Prbs9(void)
{
    smacErrors_t err;
    /*@CMA, Need to set Smac to Idle in order to get PRBS9 to work after a second try on the Conn Test menu*/
    (void)MLMERXDisableRequest();
    (void)MLMETXDisableRequest();
    err = MCPSDataRequest(gAppTxPacket);
    if(err != gErrorNoError_c)
    {
        failedPRBS9 = TRUE;
        SelfNotificationEvent(); //in case data isn't sent, no confirm event will fire.
        //this way we need to make sure the application will not freeze.
    }
}

/*****************************************************************************
* UartRxCallBack function
*
* Interface assumptions:
* This callback is triggered when a new byte is received over the UART
*
* Return Value:
* None
*****************************************************************************/
void UartRxCallBack(void* param, serial_manager_callback_message_t *message, serial_manager_status_t status )
{
    mRxBufferBytesAvailable += message->length;
    (void)OSA_EventSet((osa_event_handle_t)gTaskEvent, gUART_RX_EVENT_c);
}

/************************************************************************************
*
* Increments channel on the ED Confirm event and fires a new ED measurement request
*
************************************************************************************/
static void IncrementChannelOnEdEvent()
{
    bScanDone = FALSE;
    smacErrors_t err;
    if (ChannelToScan <= gMaxChannel_c)
    {
        err = MLMEScanRequest((channels_t)ChannelToScan);
        if(err == gErrorNoError_c)
            ChannelToScan++;                                                //Increment channel to scan
    }
    else
    {
        bScanDone = TRUE;                                               //PRINT ALL CHANNEL RESULTS
        ConnTestTimers_StartDelay(AppDelayTmr, 300);                   //Add delay between channel scanning series.
    }
}

/************************************************************************************
*
* Configures channel for second pan and sets dwell time
*
************************************************************************************/
#if gMpmMaxPANs_c == 2
static bool_t ConfigureAlternatePan(void)
{
    bool_t bBackFlag = FALSE;
    static uint8_t u8Channel = 0;
    static uint8_t u8PS = 0;
    static uint8_t u8Range = 0;
    static MpmPerConfigStates_t mMpmPerState = gMpmStateInit_c;

    switch(mMpmPerState)
    {
    case gMpmStateInit_c:
        if(evDataFromUART)
        {
            evDataFromUART = FALSE;
            if(gu8UartData == ' ')
            {
                shortCutsEnabled = FALSE;
                u8Channel    = 0;
                u8PS         = 0;
                u8Range      = 0;
                Serial_Print(mAppSer, "\r\n\r\nDual Pan RX\r\nType channel number between ", gAllowToBlock_d);
                Serial_PrintDec(mAppSer, (uint32_t)gMinChannel_c);
                Serial_Print(mAppSer, " and ", gAllowToBlock_d);
                Serial_PrintDec(mAppSer, (uint32_t)gMaxChannel_c);
                Serial_Print(mAppSer, " and press [ENTER]. \r\nMake sure input channel differs from channel"
                             " selected using shortcut keys\r\n", gAllowToBlock_d);
                mMpmPerState = gMpmStateConfigureChannel_c;
            }
        }
        break;
    case gMpmStateConfigureChannel_c:
        if(evDataFromUART)
        {
            evDataFromUART = FALSE;
            if(gu8UartData == '\r')
            {
                mMpmPerState = gMpmStateConfirmChannel_c;
                SelfNotificationEvent();
            }
            else if (gu8UartData >= '0' && gu8UartData <= '9')
            {
                if( (uint16_t)(u8Channel*10 + (gu8UartData-'0')) <= 0xFF)
                {
                    u8Channel = u8Channel*10 + (gu8UartData - '0');
                    Serial_PrintDec(mAppSer, (uint32_t)(gu8UartData-'0'));
                }
            }
        }
        break;
    case gMpmStateConfirmChannel_c:
        if(u8Channel < gMinChannel_c || u8Channel > gMaxChannel_c)
        {
            Serial_Print(mAppSer, "\r\n\t Error: Invalid channel. Input valid channel\r\n",gAllowToBlock_d);
            u8Channel = 0;
            mMpmPerState = gMpmStateConfigureChannel_c;
        }else if(u8Channel == testChannel)
        {
            Serial_Print(mAppSer, "\r\n\t Error: Same channel for both PANs. Input valid channel\r\n", gAllowToBlock_d);
            u8Channel = 0;
            mMpmPerState = gMpmStateConfigureChannel_c;
        }else
        {
            (void)MLMESetActivePan(gSmacPan1_c);
            (void)MLMESetChannelRequest((channels_t)u8Channel);
            (void)MLMESetActivePan(gSmacPan0_c);
            Serial_Print(mAppSer,"\r\nConfigure Dwell Time (PS*(RANGE + 1) ms): \r\n", gAllowToBlock_d);
            PrintMenu(cu8MpmMenuPs, mAppSer);
            mMpmPerState = gMpmStateInputDwellPS_c;
        }
        break;
    case gMpmStateInputDwellPS_c:
        if(evDataFromUART)
        {
            evDataFromUART = FALSE;
            if(gu8UartData >='0' && gu8UartData <='3')
            {
                u8PS = gu8UartData - '0';
                Serial_Print(mAppSer, "\r\nInput RANGE parameter between 0 and 63"
                             " and press [ENTER]\r\n",gAllowToBlock_d);
                mMpmPerState = gMpmStateInputDwellRange_c;
            }
        }
        break;
    case gMpmStateInputDwellRange_c:
        if(evDataFromUART)
        {
            evDataFromUART = FALSE;
            if(gu8UartData >='0' && gu8UartData <='9')
            {
                u8Range = u8Range*10 + (gu8UartData - '0');
                if(u8Range >= ((mDualPanDwellTimeMask_c >> mDualPanDwellTimeShift_c) + 1))
                {
                    Serial_Print(mAppSer,"\r\n\tError: Invalid RANGE. Input new value.\r\n",gAllowToBlock_d);
                    u8Range = 0;
                }
                else
                {
                    Serial_PrintDec(mAppSer, (uint32_t)(gu8UartData - '0'));
                }
            }
            else if(gu8UartData == '\r')
            {
                (void)MLMEConfigureDualPanSettings(TRUE, TRUE, u8PS, u8Range);
                Serial_Print(mAppSer,"\r\n\t Done! Press [SPACE] to start test\r\n", gAllowToBlock_d);
                mMpmPerState = gMpmStateExit_c;
                SelfNotificationEvent();
            }
        }
        break;
    case gMpmStateExit_c:
        bBackFlag = TRUE;
        mMpmPerState = gMpmStateInit_c;
        break;
    }

    return bBackFlag;
}
#endif
/***********************************************************************
*********************Utilities Software********************************
************************************************************************/

static bool_t stringComp(uint8_t * au8leftString, uint8_t * au8RightString, uint8_t bytesToCompare)
{
    do
    {
    }while((*au8leftString++ == *au8RightString++) && --bytesToCompare);
    return(0 == bytesToCompare);
}

static void AppDelayCallback(uint32_t param)
{
    (void)param;
    timePassed = TRUE;
    (void)OSA_EventSet(gTaskEvent, gTimePassed_EVENT_c);
}

void RangeTestDelayCallback(uint32_t param)
{
    (void)OSA_EventSet(gTaskEvent, gRangeTest_EVENT_c);
}

#if CT_Feature_Direct_Registers || CT_Feature_Indirect_Registers
static uint32_t HexString2Dec(uint8_t* hexString)
{
    uint32_t decNumber = 0;
    uint8_t  idx = 0;
    while(hexString[idx] && idx < 8)
    {
        if(hexString[idx] >= 'a' && hexString[idx] <= 'f')
        {
            decNumber = (decNumber << 4) + hexString[idx] - 'a' + 10;
        }
        else if(hexString[idx] >= 'A' && hexString[idx] <= 'F')
        {
            decNumber = (decNumber << 4) + hexString[idx] - 'A' + 10;
        }
        else if(hexString[idx] >= '0' && hexString[idx] <= '9')
        {
            decNumber = (decNumber << 4) + hexString[idx] - '0';
        }
        else
        {
            break;
        }
        ++idx;
    }
    return decNumber;
}
#endif

/***********************************************************************
************************************************************************/
