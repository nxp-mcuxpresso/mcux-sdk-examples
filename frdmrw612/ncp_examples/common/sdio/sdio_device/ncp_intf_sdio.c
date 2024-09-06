/*
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"

#include "fsl_adapter_sdu.h"
#include "fsl_power.h"
#include "ncp_adapter.h"
#include "ncp_intf_sdio.h"
#ifdef CONFIG_HOST_SLEEP
#include "host_sleep.h"
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
/*NCP Message Type*/
#define NCP_MSG_TYPE_CMD   0x00010000
#define NCP_MSG_TYPE_EVT   0x00020000
#define NCP_MSG_TYPE_RSP   0x00030000

/*NCP command class*/
#define NCP_CMD_WLAN   0x00000000
#define NCP_CMD_BLE    0x10000000
#define NCP_CMD_15D4   0x20000000
#define NCP_CMD_MATTER 0x30000000
#define NCP_CMD_SYSTEM 0x40000000

#define NCP_CMD_WLAN_SOCKET      0x00900000

#define NCP_RSP_WLAN_SOCKET_SEND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RSP | 0x00000004)
#define NCP_RSP_WLAN_SOCKET_SENDTO    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RSP | 0x00000005)

#define SDIO_GET_MSG_TYPE(cmd)        ((cmd) & 0x000f0000)

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

/*NCP ommand header*/
/* 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8  6 5 4 3 2 1 0 */
/* |  class   |       subclass        |  msg type |               command id          | */
/* |              sequence number                 |                size               | */
/* |                 reserved                     |                  result           | */
typedef MLAN_PACK_START struct ncp_command_header
{
    /* class: bit 28 ~ 31 / subclass: bit 20 ~27 / msg type: bit 16 ~ 19 / command id: bit 0 ~ 15*/
    uint32_t cmd;
    uint16_t size;
    uint16_t seqnum;
    uint16_t result;
    uint16_t rsvd;
} MLAN_PACK_END NCP_COMMAND, NCP_RESPONSE;

//#define NCP_SDIO_TASK_PRIORITY    3
//#define NCP_SDIO_TASK_STACK_SIZE  1024

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
        return (int)NCP_STATUS_ERROR;
    }

    SDU_InstallCallback(SDU_TYPE_FOR_WRITE_CMD, ncp_tlv_dispatch);
    SDU_InstallCallback(SDU_TYPE_FOR_WRITE_DATA, ncp_tlv_dispatch);

    //(void)OSA_TaskCreate((osa_task_handle_t)ncp_sdioTaskHandle, OSA_TASK(ncp_sdio_intf_task), NULL);

    return (int)NCP_STATUS_SUCCESS;
}

int ncp_sdio_deinit(void *argv)
{
    ARG_UNUSED(argv);

    SDU_Deinit();

    return (int)NCP_STATUS_SUCCESS;
}

#if 0
int ncp_sdio_recv(uint8_t *tlv_buf, size_t *tlv_sz)
{
    int ret;

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(NULL != tlv_sz);


    NCP_SDIO_STATS_INC(rx);

    return (int)NCP_STATUS_SUCCESS;
}
#endif

int ncp_sdio_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    NCP_COMMAND *res = NULL;
    status_t ret = kStatus_Success;
    uint32_t msg_type;

    ARG_UNUSED(cb);

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(0 != tlv_sz);

    res = (NCP_COMMAND *)(tlv_buf + sizeof(sdio_header_t));

    msg_type = SDIO_GET_MSG_TYPE(res->cmd);
    switch (msg_type)
    {
        case NCP_MSG_TYPE_RSP:
            if ((res->cmd == NCP_RSP_WLAN_SOCKET_SEND)
              || (res->cmd == NCP_RSP_WLAN_SOCKET_SENDTO))
            {
                ret = SDU_Send(SDU_TYPE_FOR_READ_DATA, (uint8_t *)res, tlv_sz);
            }
            else
            {
                ret = SDU_Send(SDU_TYPE_FOR_READ_CMD, (uint8_t *)res, tlv_sz);
            }
            break;
        case NCP_MSG_TYPE_EVT:
            ret = SDU_Send(SDU_TYPE_FOR_READ_EVENT, (uint8_t *)res, tlv_sz);
            break;
        default:
            ncp_adap_e("%s: invalid msg_type %d", __FUNCTION__, msg_type);
            ret = kStatus_Fail;
            break;
    }

    if (ret != kStatus_Success)
    {
        ncp_adap_d("%s: fail 0x%x", __FUNCTION__, ret);
        NCP_SDIO_STATS_INC(drop);
        return (int)NCP_STATUS_ERROR;
    }

    NCP_SDIO_STATS_INC(tx);

    return (int)NCP_STATUS_SUCCESS;
}

static int ncp_sdio_pm_enter(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

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
        ret = SDU_EnterPowerDown();
        if(ret != kStatus_Success)
        {
            ncp_adap_e("Failed to deinit SDIO interface");
            return (int)NCP_PM_STATUS_ERROR;
        }
    }

    return (int)ret;
}

static int ncp_sdio_pm_exit(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

    if(pm_state == NCP_PM_STATE_PM2)
    {
        POWER_DisableWakeup(SDU_IRQn);
        SDU_FN0_CARD->CARD_CTRL5 &= (~SDU_FN0_CARD_CARD_CTRL5_CMD52_RES_VALID_MODE_MASK);
        SDU_FN_CARD->CARD_INTMASK0 &= (~SDU_FN_CARD_CARD_INTSTATUS0_HOST_PWR_UP_INT_MASK);
    }
    else if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = SDU_ExitPowerDown();
        if(ret != kStatus_Success)
        {
            ncp_adap_e("Failed to init SDIO interface");
            return (int)NCP_PM_STATUS_ERROR;
        }

        /* After wakeup from PM3, sdio device notify sdio host to re-enumerate by gpio */
#ifdef CONFIG_HOST_SLEEP
        ncp_notify_host_gpio_init();
        ncp_notify_host_gpio_output();
#endif
    }
    return ret;
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
