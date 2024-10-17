//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Copyright 2021 - 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>

#ifndef RW610_FPGA
#include "board.h"
#include "fsl_clock.h"
#include "fsl_loader.h"
#undef BIT0
#endif
#include "fsl_os_abstraction.h"
#include "fsl_component_mem_manager.h"
#include "fsl_usart.h"
#include "fsl_flexcomm.h"
#include "fsl_adapter_imu.h"
#include "FunctionLib.h"
#include "fwk_platform_coex.h"

// #define NBU_APP_SUPPORT_LOWPOWER

#ifdef NBU_APP_SUPPORT_LOWPOWER
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "fsl_rtc.h"
#include "fsl_power.h"
#endif /* NBU_APP_SUPPORT_LOWPOWER */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifdef RW610_FPGA
#define Nbu_AppUartHandler      FLEXCOMM0_IRQHandler
#define NBU_APP_UART_BAUDRATE   115200
#define NBU_APP_UART_CLOCK_RATE 16000000U
#define NBU_APP_UART            USART0
#define NBU_APP_UART_IRQ        FLEXCOMM0_IRQn
#else
#define Nbu_AppUartHandler      BOARD_UART_IRQ_HANDLER
#define NBU_APP_UART_BAUDRATE   BOARD_DEBUG_UART_BAUDRATE
#define NBU_APP_UART_CLOCK_RATE BOARD_DEBUG_UART_CLK_FREQ
#define NBU_APP_UART            BOARD_DEBUG_UART
#define NBU_APP_UART_IRQ        BOARD_UART_IRQ
#endif
#define NBU_APP_UART_PACKET_BUFFER_SIZE 256 // Maxmium 255 Bytes for HCI CMD

//#define IMU_TASK_PRIORITY            (2U)

#define NBU_START_TASK_PRIORITY \
    (15U) // Change the priority of the START_TASK_PRIORITY to the minimum to ensure that other tasks can be scheduled
          // normally
#define NBU_START_TASK_STACK_SIZE 1000

#ifdef NBU_APP_SUPPORT_LOWPOWER

OSA_SEMAPHORE_HANDLE_DEFINE(k_enterLowPowerIdle);

/*!
 * 0 -> Full ram retention
 * 1 -> 512k ram retention
 * >1 -> No ram retention
 *
 */
#define APP_PM3_RAM_RET_SEL 0

#define APP_PM2_CONSTRAINTS                                                                           \
    6U, PM_RESC_SRAM_0K_384K_STANDBY, PM_RESC_SRAM_384K_448K_STANDBY, PM_RESC_SRAM_448K_512K_STANDBY, \
        PM_RESC_SRAM_512K_640K_STANDBY, PM_RESC_SRAM_640K_896K_STANDBY, PM_RESC_SRAM_896K_1216K_STANDBY

#if defined(APP_PM3_RAM_RET_SEL) && (APP_PM3_RAM_RET_SEL == 0)
/* full ram retention */
#define APP_PM3_CONSTRAINTS                                                                                    \
    7U, PM_RESC_SRAM_0K_384K_RETENTION, PM_RESC_SRAM_384K_448K_RETENTION, PM_RESC_SRAM_448K_512K_RETENTION,    \
        PM_RESC_SRAM_512K_640K_RETENTION, PM_RESC_SRAM_640K_896K_RETENTION, PM_RESC_SRAM_896K_1216K_RETENTION, \
        PM_RESC_CAU_SOC_SLP_REF_CLK_ON
#elif defined(APP_PM3_RAM_RET_SEL) && (APP_PM3_RAM_RET_SEL == 1)
/* 512k ram retention */
#define APP_PM3_CONSTRAINTS                                                                                 \
    4U, PM_RESC_SRAM_0K_384K_RETENTION, PM_RESC_SRAM_384K_448K_RETENTION, PM_RESC_SRAM_448K_512K_RETENTION, \
        PM_RESC_CAU_SOC_SLP_REF_CLK_ON
#else
/* no ram retention */
#define APP_PM3_CONSTRAINTS 1U, PM_RESC_CAU_SOC_SLP_REF_CLK_ON
#endif

#define APP_PM4_CONSTRAINTS 0U

#define HCI_VENDOR_HOST_ENTER_LOWPOWER 0xFCFFU
#endif /* NBU_APP_SUPPORT_LOWPOWER */

static uint8_t platformInitialized = 0;

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

typedef uint8_t hciEventCode_t;

typedef enum
{
    mDetectMarker_c = 0,
    mDetectHeader_c,
    mPacketInProgress_c,
    mDetectSpinelPacket
} detectState_t;

typedef enum
{
    gHciCommandPacket_c         = 0x01U, /*!<  HCI Command */
    gHciDataPacket_c            = 0x02U, /*!<  L2CAP Data Packet */
    gHciSynchronousDataPacket_c = 0x03U, /*!<  Not used in BLE */
    gHciEventPacket_c           = 0x04U, /*!<  HCI Event */
    gHciIsoDataPacket_c         = 0x05U, /*!<  HCI ISO data packet */
} hciPacketType_t;

typedef PACKED_STRUCT hciCommandPacketHeader_tag
{
    uint16_t opCode;
    uint8_t  parameterTotalLength;
}
hciCommandPacketHeader_t;

typedef PACKED_STRUCT hciAclDataPacketHeader_tag
{
    uint16_t handle : 12;
    uint16_t pbFlag : 2;
    uint16_t bcFlag : 2;
    uint16_t dataTotalLength;
}
hciAclDataPacketHeader_t;

typedef PACKED_STRUCT hciEventPacketHeader_tag
{
    hciEventCode_t eventCode;
    uint8_t        dataTotalLength;
}
hciEventPacketHeader_t;

typedef PACKED_STRUCT hciIsoDataPacketHeader_tag
{
    uint16_t handle : 12;
    uint16_t pbFlag : 2;
    uint16_t tsFlag : 1;
    uint16_t RFU_1 : 1;
    uint16_t dataLoadLength : 14;
    uint16_t RFU_2 : 2;
}
hciIsoDataPacketHeader_t;

typedef PACKED_STRUCT hcitPacketHdr_tag
{
    hciPacketType_t packetTypeMarker;
    PACKED_UNION
    {
        hciAclDataPacketHeader_t aclDataPacket;
        hciEventPacketHeader_t   eventPacket;
        hciCommandPacketHeader_t commandPacket;
        hciIsoDataPacketHeader_t isoDataPacket;
    };
}
hcitPacketHdr_t;

/* Hci packets header lengths */
#define gHciCommandPacketHeaderLength_c (3U)
#define gHciAclDataPacketHeaderLength_c (4U)
#define gHciEventPacketHeaderLength_c   (2U)
#define gHciIsoDataPacketHeaderLength_c (4U)

#define gHcLeAclDataPacketLengthDefault_c (500U - gHciAclDataPacketHeaderLength_c)
#define gHcitMaxPayloadLen_c              (gHcLeAclDataPacketLengthDefault_c + gHciAclDataPacketHeaderLength_c)

typedef PACKED_STRUCT hcitPacketStructured_tag
{
    hcitPacketHdr_t header;
    uint8_t         payload[gHcitMaxPayloadLen_c];
}
hcitPacketStructured_t;

typedef PACKED_UNION hcitPacket_tag
{
    /* The entire packet as unformatted data. */
    uint8_t raw[sizeof(hcitPacketStructured_t)];
}
hcitPacket_t;

typedef struct hcitComm_tag
{
    hcitPacket_t *  pPacket;
    hcitPacketHdr_t pktHeader;
    uint16_t        bytesReceived;
    uint16_t        expectedLength;
} hcitComm_t;

static hcitComm_t    mHcitData;
static detectState_t mPacketDetectStep;
static detectState_t mPacketDetectStep;
static hcitPacket_t  mHcitPacketRaw;

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
#ifdef NBU_APP_SUPPORT_LOWPOWER
static void     Nbu_LowPowerInit(void);
static void     Nbu_EnterLowPower(uint8_t mode, uint32_t timeoutUs);
static void     Nbu_SetConstraints(uint8_t powerMode);
static void     Nbu_ReleaseConstraints(uint8_t powerMode);
static void     Nbu_StartRtcTimer(uint64_t timeOutUs);
static void     Nbu_StopRtcTimer(void);
static status_t Nbu_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data);
#endif /* NBU_APP_SUPPORT_LOWPOWER */

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Define IMUMC handle*/
static IMUMC_HANDLE_DEFINE(NbuImumcHandle);
static hal_imumc_config_t rw610BleEndpoint_CPU3_tx;

/*osa start_task*/
static void start_task(void *argument);
static OSA_TASK_HANDLE_DEFINE(s_startTaskHandle);
static OSA_TASK_DEFINE(start_task, NBU_START_TASK_PRIORITY, 2, NBU_START_TASK_STACK_SIZE, false);

static void hci_rxCallBack(const uint8_t recvChar);
static void spinel_rxCallBack(const uint8_t recvChar);

#ifdef NBU_APP_SUPPORT_LOWPOWER
AT_ALWAYS_ON_DATA(pm_handle_t g_pmHndle);
AT_ALWAYS_ON_DATA_INIT(pm_notify_element_t g_notify1) = {
    .notifyCallback = Nbu_UartControlCallback,
    .data           = NULL,
};
AT_ALWAYS_ON_DATA(pm_wakeup_source_t g_OstimerWakeupSource);
bool            timeOutOccured = true;
static uint8_t  powerMode      = 0U;
static uint32_t timeOutUs      = 0U;

static power_init_config_t initCfg = {
    /* VCORE AVDD18 supplied from iBuck on RD board. */
    .iBuck = true,
    /* CAU_SOC_SLP_REF_CLK not needed. */
    .gateCauRefClk = true,
};

#endif /* NBU_APP_SUPPORT_LOWPOWER */

/*******************************************************************************
 * Code
 ******************************************************************************/
void Nbu_ImumcSendMessage()
{
    uint8_t *pSerialPacket = NULL;
    uint32_t packet_Size   = (mHcitData.bytesReceived + 1U);

#ifdef NBU_APP_SUPPORT_LOWPOWER
    if ((mHcitData.pktHeader.packetTypeMarker == gHciCommandPacket_c) &&
        (mHcitData.pktHeader.commandPacket.opCode == HCI_VENDOR_HOST_ENTER_LOWPOWER))
    {
        /* power mode + timeout */
        assert(mHcitData.pktHeader.commandPacket.parameterTotalLength == 2U);

        powerMode = (uint8_t)mHcitData.pPacket->raw[3];
        timeOutUs = (uint32_t)mHcitData.pPacket->raw[4] * 1000000U;
        (void)OSA_SemaphorePost(k_enterLowPowerIdle);
    }
    else
#endif /* NBU_APP_SUPPORT_LOWPOWER */
    {
        pSerialPacket = MEM_BufferAlloc(packet_Size);
        if (pSerialPacket == NULL)
        {
            assert(0);
        }

        pSerialPacket[0] = mHcitData.pktHeader.packetTypeMarker;
        FLib_MemCpy(pSerialPacket + 1, (uint8_t *)mHcitData.pPacket, mHcitData.bytesReceived);

        if (kStatus_HAL_ImumcSuccess != HAL_ImumcSend((hal_imumc_handle_t)NbuImumcHandle, pSerialPacket, packet_Size))
        {
            assert(0);
        }

        MEM_BufferFree(pSerialPacket);
    }

    mPacketDetectStep = mDetectMarker_c;
}

// extern void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength);
void spinel_rxCallBack(const uint8_t recvChar)
{
    // otPlatUartReceived(aBuf, aBufLength);
}

static void hci_rxCallBack(const uint8_t recvChar)
{
    switch (mPacketDetectStep)
    {
        case mDetectMarker_c:
            if ((recvChar == (uint8_t)gHciDataPacket_c) || (recvChar == (uint8_t)gHciEventPacket_c) ||
                (recvChar == (uint8_t)gHciCommandPacket_c) || (recvChar == (uint8_t)gHciIsoDataPacket_c))
            {
                union
                {
                    hcitPacketHdr_t *pPacketHdr;
                    hcitPacket_t *   pPacket;
                } packetTemp; /* MISRA rule 11.3 */

                packetTemp.pPacketHdr = &mHcitData.pktHeader;
                mHcitData.pPacket     = packetTemp.pPacket;

                mHcitData.pktHeader.packetTypeMarker = (hciPacketType_t)recvChar;
                mHcitData.bytesReceived              = 1;

                mPacketDetectStep = mDetectHeader_c;
            }
            else if (recvChar == 0x7e) /* the first two bit of flag field of the header byte ("FLG") is always set to
                                          the value two (or "10" in binary) */
            {
                /* Is it a spinel packet ? */
                mPacketDetectStep = mDetectSpinelPacket;
            }
            break; // unknown pakcet marker;

        case mDetectHeader_c:
            mHcitData.pPacket->raw[mHcitData.bytesReceived++] = recvChar;

            switch (mHcitData.pktHeader.packetTypeMarker)
            {
                case gHciDataPacket_c:
                    /* ACL Data Packet */
                    if (mHcitData.bytesReceived == (gHciAclDataPacketHeaderLength_c + 1U))
                    {
                        /* Validate ACL Data packet length */
                        if (mHcitData.pktHeader.aclDataPacket.dataTotalLength > gHcLeAclDataPacketLengthDefault_c)
                        {
                            mHcitData.pPacket = NULL;
                            mPacketDetectStep = mDetectMarker_c;
                            break;
                        }
                        mHcitData.expectedLength =
                            gHciAclDataPacketHeaderLength_c + mHcitData.pktHeader.aclDataPacket.dataTotalLength;

                        mPacketDetectStep = mPacketInProgress_c;
                    }
                    break;

                case gHciEventPacket_c:
                    /* HCI Event Packet */
                    if (mHcitData.bytesReceived == (gHciEventPacketHeaderLength_c + 1U))
                    {
                        /* Validate HCI Event packet length
                        if( mHcitData.pktHeader.eventPacket.dataTotalLength > gHcEventPacketLengthDefault_c )
                        {
                            mHcitData.pPacket = NULL;
                            mPacketDetectStep = mDetectMarker_c;
                            break;
                        } */
                        mHcitData.expectedLength =
                            gHciEventPacketHeaderLength_c + (uint16_t)mHcitData.pktHeader.eventPacket.dataTotalLength;
                        mPacketDetectStep = mPacketInProgress_c;
                    }
                    break;

                case gHciCommandPacket_c:
                    /* HCI Command Packet */
                    if (mHcitData.bytesReceived == (gHciCommandPacketHeaderLength_c + 1U))
                    {
                        mHcitData.expectedLength = gHciCommandPacketHeaderLength_c +
                                                   (uint16_t)mHcitData.pktHeader.commandPacket.parameterTotalLength;
                        mPacketDetectStep = mPacketInProgress_c;
                    }
                    break;

                case gHciIsoDataPacket_c:
                    /*ISO Data Packet */
                    if (mHcitData.bytesReceived == (gHciIsoDataPacketHeaderLength_c + 1U))
                    {
                        mHcitData.expectedLength = gHciIsoDataPacketHeaderLength_c +
                                                   (uint16_t)mHcitData.pktHeader.isoDataPacket.dataLoadLength;
                        mPacketDetectStep = mPacketInProgress_c;
                    }
                    break;

                case gHciSynchronousDataPacket_c:
                default:; /* Not Supported */
                    break;
            }

            if (mPacketDetectStep == mPacketInProgress_c)
            {
                mHcitData.pPacket = &mHcitPacketRaw;
                FLib_MemCpy(mHcitData.pPacket, (uint8_t *)&mHcitData.pktHeader + 1, sizeof(hcitPacketHdr_t) - 1U);
                mHcitData.bytesReceived -= 1U;

                if (mHcitData.bytesReceived == mHcitData.expectedLength)
                {
                    Nbu_ImumcSendMessage();
                }
            }
            break;

        case mPacketInProgress_c:
            mHcitData.pPacket->raw[mHcitData.bytesReceived++] = recvChar;

            if (mHcitData.bytesReceived == mHcitData.expectedLength)
            {
                Nbu_ImumcSendMessage();
            }
            break;

        default:; /* No action required */
            break;
    }
}

void Nbu_AppUartHandler(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    uint8_t recvChar = 0;

    if ((kUSART_RxFifoNotEmptyFlag | kUSART_RxError) & USART_GetStatusFlags(NBU_APP_UART))
    {
        recvChar = USART_ReadByte(NBU_APP_UART);
    }

    if (mPacketDetectStep != mDetectSpinelPacket)
    {
        /* Try to decode an HCI packet */
        hci_rxCallBack(recvChar);
    }

    /* Now check the step again to know if it is a spinel packet */
    if (mPacketDetectStep == mDetectSpinelPacket)
    {
        spinel_rxCallBack(recvChar);
    }
}

static hal_imumc_return_status_t Nbu_AppImumcRxCallback(void *param, uint8_t *data, uint32_t len)
{
    uint8_t *pSerialPacket = NULL;
    pSerialPacket          = MEM_BufferAlloc(len);
    if (pSerialPacket != NULL)
    {
        FLib_MemCpy(pSerialPacket, (uint8_t *)data, len);
        USART_WriteBlocking(NBU_APP_UART, pSerialPacket, len);
        MEM_BufferFree(pSerialPacket);
    }
    else
    {
        assert(0);
    }
    return kStatus_HAL_RL_RELEASE;
}

static void Nbu_SerialInit(void)
{
    status_t       status;
    usart_config_t usartConfig;

    USART_GetDefaultConfig(&usartConfig);
    usartConfig.baudRate_Bps              = NBU_APP_UART_BAUDRATE;
    usartConfig.parityMode                = kUSART_ParityDisabled;
    usartConfig.stopBitCount              = kUSART_OneStopBit;
    usartConfig.enableRx                  = 1;
    usartConfig.enableTx                  = 1;
    usartConfig.txWatermark               = kUSART_TxFifo0;
    usartConfig.rxWatermark               = kUSART_RxFifo1;
    usartConfig.enableHardwareFlowControl = 1;

#ifndef RW610_FPGA
    CLOCK_SetFRGClock(BOARD_DEBUG_UART_FRG_CLK);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
#endif
    status = USART_Init(NBU_APP_UART, &usartConfig, NBU_APP_UART_CLOCK_RATE);

    if (kStatus_Success != status)
    {
        assert(0);
    }
    /* Enable RX interrupt. */
    USART_EnableInterrupts(NBU_APP_UART, kUSART_RxLevelInterruptEnable | kUSART_RxErrorInterruptEnable);
    EnableIRQ(NBU_APP_UART_IRQ);
}

static void Nbu_ImumcInit()
{
    rw610BleEndpoint_CPU3_tx.local_addr  = 30;
    rw610BleEndpoint_CPU3_tx.remote_addr = 40;
    rw610BleEndpoint_CPU3_tx.imuLink     = kIMU_LinkCpu2Cpu3;

    if (kStatus_HAL_ImumcSuccess != HAL_ImumcInit(&NbuImumcHandle, &rw610BleEndpoint_CPU3_tx))
    {
        assert(0);
    }
    else
    {
        if (kStatus_HAL_ImumcSuccess !=
            HAL_ImumcInstallRxCallback((hal_imumc_handle_t)(&NbuImumcHandle), Nbu_AppImumcRxCallback, NULL))
        {
            assert(0);
        }
    }
}

static void start_task(void *argument)
{
    if (!platformInitialized)
    {
        platformInitialized = 1;

        /* Initialize BLE controller */
        PLATFORM_InitControllers(connBle_c);

        Nbu_SerialInit();

        Nbu_ImumcInit();
    }

    while (true)
    {
#ifdef NBU_APP_SUPPORT_LOWPOWER
        if (KOSA_StatusSuccess == OSA_SemaphoreWait(k_enterLowPowerIdle, osaWaitForever_c))
        {
            Nbu_EnterLowPower(powerMode, timeOutUs);
        }
#endif /* NBU_APP_SUPPORT_LOWPOWER */
        if (gUseRtos_c == 0)
        {
            break;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /*init OSA: should be called before any other OSA API*/
    OSA_Init();

    MEM_Init();

#ifndef RW610_FPGA
    /* Initialize MCU clock */
    extern void BOARD_InitHardware(void);
    BOARD_InitHardware();
#endif

#ifdef NBU_APP_SUPPORT_LOWPOWER
    Nbu_LowPowerInit();
    (void)OSA_SemaphoreCreate(k_enterLowPowerIdle, 1U);
#endif
    (void)OSA_TaskCreate((osa_task_handle_t)s_startTaskHandle, OSA_TASK(start_task), NULL);

    /*start scheduler*/
    OSA_Start();

    /*won't run here*/
    assert(0);
    return 0;
}

#ifdef NBU_APP_SUPPORT_LOWPOWER

static void Nbu_LowPowerInit(void)
{
    uint32_t resetSrc;

    POWER_InitPowerConfig(&initCfg);
    resetSrc = POWER_GetResetCause();
    POWER_ClearResetCause(resetSrc);
    DisableIRQ(RTC_IRQn);
    POWER_ClearWakeupStatus(RTC_IRQn);
    POWER_DisableWakeup(RTC_IRQn);
    RTC_Init(RTC);
    RTC_EnableAlarmTimerInterruptFromDPD(RTC, true);
    RTC_EnableWakeUpTimerInterruptFromDPD(RTC, true);
    RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
    RTC_StartTimer(RTC);

    PM_CreateHandle(&g_pmHndle);
    PM_RegisterNotify(kPM_NotifyGroup0, &g_notify1);
    PM_InitWakeupSource(&g_OstimerWakeupSource, (uint32_t)RTC_IRQn, NULL, true);
    PM_RegisterTimerController(&g_pmHndle, Nbu_StartRtcTimer, Nbu_StopRtcTimer, NULL, NULL);
    PM_EnablePowerManager(true);
}

static void Nbu_EnterLowPower(uint8_t mode, uint32_t timeoutUs)
{
    uint32_t irqMask = DisableGlobalIRQ();

    Nbu_SetConstraints(mode);
    PM_EnterLowPower(timeoutUs);
    Nbu_ReleaseConstraints(mode);

    EnableGlobalIRQ(irqMask);
}

static void Nbu_SetConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_PM2:
        {
            PM_SetConstraints(powerMode, APP_PM2_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_PM3:
        {
            PM_SetConstraints(powerMode, APP_PM3_CONSTRAINTS);
            break;
        }

        case PM_LP_STATE_PM4:
        {
            PM_SetConstraints(powerMode, APP_PM4_CONSTRAINTS);
            break;
        }

        default:
        {
            /* PM0/PM1 has no reousrce constraints. */
            PM_SetConstraints(powerMode, 0U);
            break;
        }
    }
}

static void Nbu_ReleaseConstraints(uint8_t powerMode)
{
    switch (powerMode)
    {
        case PM_LP_STATE_PM2:
        {
            PM_ReleaseConstraints(powerMode, APP_PM2_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_PM3:
        {
            PM_ReleaseConstraints(powerMode, APP_PM3_CONSTRAINTS);
            break;
        }
        case PM_LP_STATE_PM4:
        {
            PM_ReleaseConstraints(powerMode, APP_PM4_CONSTRAINTS);
            break;
        }
        default:
        {
            /* PM0/PM1 has no reousrce constraints. */
            PM_ReleaseConstraints(powerMode, 0U);
            break;
        }
    }
}

static void Nbu_StartRtcTimer(uint64_t timeOutUs)
{
    uint32_t currSeconds;

    if (timeOutOccured == true)
    {
        /* Read the RTC seconds register to get current time in seconds */
        currSeconds = RTC_GetSecondsTimerCount(RTC);
        /* Add alarm seconds to current time */
        currSeconds += (uint32_t)((timeOutUs + 999999U) / 1000000U);
        /* Set alarm time in seconds */
        RTC_SetSecondsTimerMatch(RTC, currSeconds);

        timeOutOccured = false;
    }
}

static void Nbu_StopRtcTimer(void)
{
}

static status_t Nbu_UartControlCallback(pm_event_type_t eventType, uint8_t powerState, void *data)
{
    if (powerState >= PM_LP_STATE_PM2 && powerState <= PM_LP_STATE_PM3)
    {
        if (eventType == kPM_EventEnteringSleep)
        {
            /* Wait for debug console output finished. */
            while (((uint32_t)kUSART_TxFifoEmptyFlag & USART_GetStatusFlags(NBU_APP_UART)) == 0U)
            {
            }
        }
        else
        {
            Nbu_SerialInit();
        }
    }

    return kStatus_Success;
}

extern void RTC_IRQHandler(void);
void        RTC_IRQHandler()
{
    if (RTC_GetStatusFlags(RTC) & kRTC_AlarmFlag)
    {
        /* Clear alarm flag */
        RTC_ClearStatusFlags(RTC, kRTC_AlarmFlag);
        POWER_ClearWakeupStatus(RTC_IRQn);
        timeOutOccured = true;
    }
}

#endif /* NBU_APP_SUPPORT_LOWPOWER */
