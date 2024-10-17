/** @file ncp_intf_usb_host_cdc.c
 *
 *  @brief main file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#include "ncp_intf_usb_host_cdc.h"
#include "ncp_debug.h"
#include "ncp_tlv_adapter.h"
#include "ncp_adapter.h"
#include "ncp_debug.h"
#include "usb_host_cdc_app.h"

#if CONFIG_NCP_USB
extern cdc_instance_struct_t g_cdc;
static uint8_t ncp_intf_usb_rxbuf[TLV_CMD_BUF_SIZE];

OSA_SEMAPHORE_HANDLE_DEFINE(usb_host_tx_sem);
OSA_SEMAPHORE_HANDLE_DEFINE(usb_host_rx_sem);

OSA_SEMAPHORE_HANDLE_DEFINE(usb_wakelock);
uint8_t usb_interface_pm_state = kStatus_DEV_Attached;

void ncp_usb_put_tx_sem(void)
{
    OSA_SemaphorePost(usb_host_tx_sem);
}

void ncp_usb_put_rx_sem(void)
{
    OSA_SemaphorePost(usb_host_rx_sem);
}

void ncp_usb_check_bus(void)
{
    if(kStatus_DEV_PM2 == usb_interface_pm_state)
    {
        usb_remote_wakeup_device();
        OSA_SemaphoreWait((osa_semaphore_handle_t)usb_wakelock, osaWaitForever_c);
        OSA_SemaphorePost((osa_semaphore_handle_t)usb_wakelock);
    }
}

/*Need called by usb_recv_task*/
void ncp_usb_get_rx_sem(void)
{
    OSA_SemaphoreWait(usb_host_rx_sem, osaWaitForever_c);
}

void ncp_usb_host_send_cb(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    ARG_UNUSED(param);   
    ARG_UNUSED(data);
    ARG_UNUSED(status);  
    
    ncp_adap_d("send dataLength :%d ", dataLength);
    ncp_usb_put_tx_sem();
}

int ncp_usb_host_send(uint8_t *data, size_t data_len, tlv_send_callback_t cb)
{
    uint16_t packet_size        = 0;
    uint16_t remaining_data_len = data_len;

    ncp_adap_d("transfer_size :%d!", data_len);
    ncp_usb_check_bus();
    
    while (remaining_data_len > 0)
    {
        packet_size = (remaining_data_len > TLV_CMD_BUF_SIZE) ? TLV_CMD_BUF_SIZE : remaining_data_len;
        packet_size = (packet_size > 0x4000) ? 0x4000 : packet_size;
		
        USB_HostCdcDataSend(g_cdc.classHandle, (uint8_t *)data + data_len - remaining_data_len, packet_size,
                            ncp_usb_host_send_cb, &g_cdc);

         OSA_SemaphoreWait(usb_host_tx_sem, osaWaitForever_c);

        remaining_data_len -= packet_size;
    }
	
    NCP_USB_STATS_INC(tx);
	
    return NCP_STATUS_SUCCESS;
}

void usb_host_save_recv_data(uint8_t *recv_data, uint32_t packet_len)
{
    static uint32_t usb_transfer_len = 0;
    static uint32_t usb_rx_len       = 0;

    if (usb_rx_len < TLV_CMD_HEADER_LEN)
    {
        memcpy((uint8_t *)&ncp_intf_usb_rxbuf[0] + usb_rx_len, recv_data, packet_len);
        usb_rx_len += packet_len;

        if (usb_rx_len >= TLV_CMD_HEADER_LEN)
        {
            usb_transfer_len = ((ncp_intf_usb_rxbuf[TLV_CMD_SIZE_HIGH_BYTES] << 8) |
                                ncp_intf_usb_rxbuf[TLV_CMD_SIZE_LOW_BYTES]) +
                               NCP_CHKSUM_LEN;
        }
    }
    else
    {
        if ((packet_len < (sizeof(ncp_intf_usb_rxbuf) - usb_rx_len)) && (usb_rx_len < usb_transfer_len))
        {
            memcpy((uint8_t *)&ncp_intf_usb_rxbuf[0] + usb_rx_len, recv_data, packet_len);
            usb_rx_len += packet_len;
        }
        else
        {
            ncp_adap_e("[%s] transfer warning. data_len : %d ", __func__, packet_len);
			NCP_USB_STATS_INC(err);
        }
    }

    if ((usb_rx_len >= usb_transfer_len) && (usb_transfer_len >= TLV_CMD_HEADER_LEN))
    {
        ncp_adap_d("recv data len: %d ", usb_transfer_len);
    
        ncp_tlv_dispatch(&ncp_intf_usb_rxbuf[0],usb_transfer_len - NCP_CHKSUM_LEN);

        usb_rx_len       = 0;
        usb_transfer_len = 0;
        NCP_USB_STATS_INC(rx);
        ncp_adap_d("data recv success ");
    }
}

/*callback in context of USB_HostTaskFn, so it can asynchronous*/
void ncp_usb_host_recv_cb(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    cdc_instance_struct_t *cdcInstance = (cdc_instance_struct_t *)param;
    //    ncp_adap_d("recv dataLength :%d status: %d ",dataLength,status);

    if ((dataLength > 0) && (0 == status))
    {
        usb_host_save_recv_data(data, dataLength);
    }

    ncp_usb_put_rx_sem();

    if (cdcInstance->bulkInMaxPacketSize == dataLength)
    {
        /* host will prime to receive zero length packet after recvive one maxpacketsize */
        ncp_usb_check_bus();
        USB_HostCdcDataRecv(g_cdc.classHandle, NULL, 0, ncp_usb_host_recv_cb, &g_cdc);
    }
}

int ncp_usb_host_init(void* argv)
{
    int ret = NCP_STATUS_SUCCESS;
    ARG_UNUSED(argv);
    
    ret = OSA_SemaphoreCreateBinary(usb_host_tx_sem);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to create usb_host_tx_sem: %d", ret);
        return NCP_STATUS_ERROR;
    }

    ret = OSA_SemaphoreCreateBinary(usb_host_rx_sem);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to create usb_host_rx_sem: %d", ret);
        return NCP_STATUS_ERROR;
    }
    
    ret = OSA_SemaphoreCreateBinary((osa_semaphore_handle_t)usb_wakelock);
    if (ret != NCP_STATUS_SUCCESS)
    {
        (void)ncp_adap_e("Error: Failed to create usb_wakelock\r\n");
        return -NCP_STATUS_ERROR;
    }
    OSA_SemaphorePost((osa_semaphore_handle_t)usb_wakelock);
    
    ret = usb_host_init();

    return ret;
}

int ncp_usb_host_deinit(void* argv)
{
    int ret = NCP_STATUS_SUCCESS;
    ARG_UNUSED(argv);
    
    ret = OSA_SemaphoreDestroy(usb_host_tx_sem);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to destroy usb_host_tx_sem: %d", ret);
        return NCP_STATUS_ERROR;
    }

    ret = OSA_SemaphoreDestroy(usb_host_rx_sem);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to destroy usb_host_rx_sem: %d", ret);
        return NCP_STATUS_ERROR;
    }

    ret = OSA_SemaphoreDestroy((osa_semaphore_handle_t)usb_wakelock);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to destroy usb_wakelock: %d", ret);
        return NCP_STATUS_ERROR;
    }
    
    usb_host_deinit();
    
    return ret;
}

static int ncp_usb_host_pm_enter(int32_t pm_state)
{
    /* TODO: NCP usb pm */
    return NCP_STATUS_SUCCESS;
}

static int ncp_usb_host_pm_exit(int32_t pm_state)
{
    /* TODO: NCP usb pm */
    return NCP_STATUS_SUCCESS;
}

static ncp_intf_pm_ops_t ncp_usb_host_pm_ops =
{
    .enter = ncp_usb_host_pm_enter,
    .exit  = ncp_usb_host_pm_exit,
};

ncp_intf_ops_t ncp_usb_ops =
{
    .init   = ncp_usb_host_init,
    .deinit = ncp_usb_host_deinit,
    .send   = ncp_usb_host_send,
    .recv   = NULL,
    .pm_ops = &ncp_usb_host_pm_ops,
};

#endif
