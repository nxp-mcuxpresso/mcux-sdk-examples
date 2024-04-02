/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"

#include "fsl_adapter_sdu.h"
#include "fsl_power.h"
#include "ncp_adapter.h"
#include "ncp_intf_sdio.h"
#include "fsl_gpio.h"
#include "fsl_io_mux.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/*NCP Bridge Message Type*/
#define NCP_BRIDGE_MSG_TYPE_CMD   0x0000
#define NCP_BRIDGE_MSG_TYPE_RESP  0x0001
#define NCP_BRIDGE_MSG_TYPE_EVENT 0x0002

/*NCP Bridge command class*/
#define NCP_BRIDGE_CMD_WLAN   0x00000000
#define NCP_BRIDGE_CMD_BLE    0x01000000
#define NCP_BRIDGE_CMD_15D4   0x02000000
#define NCP_BRIDGE_CMD_MATTER 0x03000000
#define NCP_BRIDGE_CMD_SYSTEM 0x04000000

#define NCP_BRIDGE_CMD_WLAN_SOCKET      0x00090000

#define NCP_BRIDGE_CMD_WLAN_SOCKET_SEND      (NCP_BRIDGE_CMD_WLAN | NCP_BRIDGE_CMD_WLAN_SOCKET | 0x00000004)
#define NCP_BRIDGE_CMD_WLAN_SOCKET_SENDTO    (NCP_BRIDGE_CMD_WLAN | NCP_BRIDGE_CMD_WLAN_SOCKET | 0x00000005)

#ifdef __GNUC__
/** Structure packing begins */
#define MLAN_PACK_START
/** Structure packeing end */
#define MLAN_PACK_END __attribute__((packed))
#else /* !__GNUC__ */
#ifdef PRAGMA_PACK
/** Structure packing begins */
#define MLAN_PACK_START
/** Structure packeing end */
#define MLAN_PACK_END
#else /* !PRAGMA_PACK */
/** Structure packing begins */
#define MLAN_PACK_START __packed
/** Structure packing end */
#define MLAN_PACK_END
#endif /* PRAGMA_PACK */
#endif /* __GNUC__ */

/*NCP Bridge command header*/
typedef MLAN_PACK_START struct bridge_command_header
{
    /*bit0 ~ bit15 cmd id  bit16 ~ bit23 cmd subclass bit24 ~ bit31 cmd class*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t msg_type;
} MLAN_PACK_END NCP_BRIDGE_COMMAND, NCP_BRIDGE_RESPONSE;

//#define NCP_SDIO_TASK_PRIORITY    3
//#define NCP_SDIO_TASK_STACK_SIZE  1024

#define NCP_SDIO_GPIO_NOTIFY        27
#define NCP_SDIO_GPIO_NOTIFY_MASK   0x8000000

/*******************************************************************************
 * Variables
 ******************************************************************************/

//static void ncp_sdio_intf_task(void *argv);

//static OSA_TASK_HANDLE_DEFINE(ncp_sdioTaskHandle);
//static OSA_TASK_DEFINE(ncp_sdio_intf_task, NCP_SDIO_TASK_PRIORITY, 1, NCP_SDIO_TASK_STACK_SIZE, 0);

/*******************************************************************************
 * API
 ******************************************************************************/

#if 0
static void ncp_sdio_intf_task(void *argv)
{
    int ret;
    uint8_t *tlv_buf = NULL;
    size_t tlv_size = 0;

    ARG_UNUSED(argv);

    while (1)
    {
        ret = SDU_GetRecvBuffer(&tlv_buf, &tlv_size);
        if (NCP_STATUS_SUCCESS == ret)
        {
            ncp_tlv_dispatch(tlv_buf, tlv_size);
            //Refill buffer here
        }
        else
        {
            ncp_adap_e("Failed to receive TLV command!");
        }
    }
}
#endif

int ncp_sdio_init(void *argv)
{
    status_t ret = 0;

    ARG_UNUSED(argv);

    ret = SDU_Init();
    if (ret != kStatus_Success)
    {
        ncp_adap_e("Failed to initialize SDIO");
        return NCP_STATUS_ERROR;
    }

    SDU_InstallCallback(SDU_TYPE_FOR_WRITE_CMD, ncp_tlv_dispatch);
    SDU_InstallCallback(SDU_TYPE_FOR_WRITE_DATA, ncp_tlv_dispatch);

    //(void)OSA_TaskCreate((osa_task_handle_t)ncp_sdioTaskHandle, OSA_TASK(ncp_sdio_intf_task), NULL);

    return NCP_STATUS_SUCCESS;
}

int ncp_sdio_deinit(void *argv)
{
    ARG_UNUSED(argv);

    SDU_Deinit();

    return NCP_STATUS_SUCCESS;
}

#if 0
int ncp_sdio_recv(uint8_t *tlv_buf, size_t *tlv_sz)
{
    int ret;

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(NULL != tlv_sz);


    NCP_SDIO_STATS_INC(rx);

    return NCP_STATUS_SUCCESS;
}
#endif

int ncp_sdio_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    NCP_BRIDGE_COMMAND *res = NULL;
    status_t ret = kStatus_Success;

    ARG_UNUSED(cb);

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(0 != tlv_sz);

    res = (NCP_BRIDGE_COMMAND *)(tlv_buf + sizeof(sdio_header_t));

    switch (res->msg_type)
    {
        case NCP_BRIDGE_MSG_TYPE_RESP:
            if ((res->cmd == NCP_BRIDGE_CMD_WLAN_SOCKET_SEND)
              || (res->cmd == NCP_BRIDGE_CMD_WLAN_SOCKET_SENDTO))
            {
                ret = SDU_Send(SDU_TYPE_FOR_READ_DATA, (uint8_t *)res, tlv_sz);
            }
            else
            {
                ret = SDU_Send(SDU_TYPE_FOR_READ_CMD, (uint8_t *)res, tlv_sz);
            }
            break;
        case NCP_BRIDGE_MSG_TYPE_EVENT:
            ret = SDU_Send(SDU_TYPE_FOR_READ_EVENT, (uint8_t *)res, tlv_sz);
            break;
        default:
            ncp_adap_e("%s: invalid msg_type %d", __FUNCTION__, res->msg_type);
            ret = kStatus_Fail;
            break;
    }

    if (ret != kStatus_Success)
    {
        ncp_adap_d("%s: fail 0x%x", __FUNCTION__, ret);
        NCP_SDIO_STATS_INC(drop);
        return NCP_STATUS_ERROR;
    }

    NCP_SDIO_STATS_INC(tx);

    return NCP_STATUS_SUCCESS;
}

static void ncp_sdio_notify_gpio_output(void)
{
    /* Toggle GPIO to notify sdio host that sdio device is ready for re-enumeration */
    GPIO_PortToggle(GPIO, 0, NCP_SDIO_GPIO_NOTIFY_MASK);
}

static void ncp_sdio_notify_gpio_init(void)
{
    /* Define the init structure for the output switch pin */
    gpio_pin_config_t sdio_output_pin = {
        kGPIO_DigitalOutput,
        1
    };

    IO_MUX_SetPinMux(IO_MUX_GPIO27);
    GPIO_PortInit(GPIO, 0);
    /* Init output GPIO. Default level is high */
    /* After wakeup from PM3, sdio device use GPIO 27 to notify sdio host */
    GPIO_PinInit(GPIO, 0, NCP_SDIO_GPIO_NOTIFY, &sdio_output_pin);
}

static int ncp_sdio_pm_enter(int32_t pm_state)
{
    ncp_pm_status_t ret = NCP_PM_STATUS_SUCCESS;

    if(pm_state == NCP_PM_STATE_PM2)
    {
        //SDU_FN_CARD->FN_CARD_INTMASK = 0x1;
        /* Enable host power up interrupt */
        SDU_FN_CARD->CARD_INTMASK0 |= SDU_FN_CARD_CARD_INTSTATUS0_HOST_PWR_UP_INT_MASK;
        /* Enable this bit so that hardware can send R5 immediately in sleep mode */
        SDU_FN0_CARD->CARD_CTRL5 |= SDU_FN0_CARD_CARD_CTRL5_CMD52_RES_VALID_MODE_MASK;
        POWER_ClearWakeupStatus(SDU_IRQn);
        NVIC_ClearPendingIRQ(SDU_IRQn);
        POWER_EnableWakeup(SDU_IRQn);
    }
    else if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = ncp_sdio_deinit(NULL);
        if(ret != 0)
        {
            ncp_adap_e("Failed to deinit SDIO interface");
            return NCP_PM_STATUS_ERROR;
        }
    }
    
    return (int)ret;
}

static int ncp_sdio_pm_exit(int32_t pm_state)
{
    ncp_pm_status_t ret = NCP_PM_STATUS_SUCCESS;

    if(pm_state == NCP_PM_STATE_PM2)
    {
        POWER_DisableWakeup(SDU_IRQn);
        SDU_FN0_CARD->CARD_CTRL5 &= (~SDU_FN0_CARD_CARD_CTRL5_CMD52_RES_VALID_MODE_MASK);
        SDU_FN_CARD->CARD_INTMASK0 &= (~SDU_FN_CARD_CARD_INTSTATUS0_HOST_PWR_UP_INT_MASK);
    }
    else if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = ncp_sdio_init(NULL);
        if(ret != 0)
        {
            ncp_adap_e("Failed to init SDIO interface");
            return NCP_PM_STATUS_ERROR;
        }

        /* After wakeup from PM3, sdio device notify sdio host to re-enumerate by gpio */
        ncp_sdio_notify_gpio_init();
        ncp_sdio_notify_gpio_output();
    }
    return (int)ret;
}

static ncp_intf_pm_ops_t ncp_sdio_pm_ops =
{
    .enter = ncp_sdio_pm_enter,
    .exit  = ncp_sdio_pm_exit,
};

ncp_intf_ops_t ncp_sdio_ops =
{
    .init   = ncp_sdio_init,
    .deinit = ncp_sdio_deinit,
    .send   = ncp_sdio_send,
    .recv   = NULL,
    .pm_ops = &ncp_sdio_pm_ops,
};
