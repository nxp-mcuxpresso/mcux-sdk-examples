/** @file ncp_intf_usb_device_cdc.c
 *
 *  @brief main file
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#include "usb_misc.h"
#include "ncp_intf_usb_device_cdc.h"
#include "ncp_debug.h"
#include "ncp_tlv_adapter.h"
#include "ncp_adapter.h"

#if CONFIG_NCP_USB
extern usb_cdc_vcom_struct_t s_cdcVcom;
OSA_SEMAPHORE_HANDLE_DEFINE(usb_device_tx_sem);
static uint8_t ncp_intf_usb_rxbuf[TLV_CMD_BUF_SIZE];

void ncp_usb_put_tx_sem(void)
{
    OSA_SemaphorePost(usb_device_tx_sem);
}

void ncp_usb_device_recv(uint8_t *recv_data, uint32_t packet_len)
{
    static uint32_t usb_transfer_len = 0;
    static uint32_t usb_rx_len       = 0;

    /*reset ncp_intf_usb_rxbuf on first usb bulk received*/
    if (!usb_rx_len)
    {
        memset(ncp_intf_usb_rxbuf, 0, sizeof(ncp_intf_usb_rxbuf));
    }

    if (usb_rx_len < TLV_CMD_HEADER_LEN)
    {
        memcpy((uint8_t *)&ncp_intf_usb_rxbuf[0] + usb_rx_len, recv_data, packet_len);
        usb_rx_len += packet_len;

        if (usb_rx_len >= TLV_CMD_HEADER_LEN)
        {
            usb_transfer_len =
                ((ncp_intf_usb_rxbuf[TLV_CMD_SIZE_HIGH_BYTES] << 8) | ncp_intf_usb_rxbuf[TLV_CMD_SIZE_LOW_BYTES]) +
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
            ncp_adap_e("[%s] transfer warning. data_len : %d  ", __func__, packet_len);
			NCP_USB_STATS_INC(err);
        }
    }

    if ((usb_rx_len >= usb_transfer_len) && (usb_transfer_len >= TLV_CMD_HEADER_LEN))
    {
        ncp_adap_d("recv data len: %d", usb_transfer_len);

        ncp_tlv_dispatch(&ncp_intf_usb_rxbuf[0],usb_transfer_len - NCP_CHKSUM_LEN);

        usb_rx_len       = 0;
        usb_transfer_len = 0;
        NCP_USB_STATS_INC(rx);
        ncp_adap_d("usb data recv success");
    }
}

int ncp_usb_device_send(uint8_t *data, size_t data_len, tlv_send_callback_t cb)
{
    uint16_t packet_size        = 0;
    uint16_t remaining_data_len = data_len;
    ARG_UNUSED(cb);

    ncp_adap_d("usb transfer_size :%d!\r\n", data_len);

    while (remaining_data_len > 0)
    {
        packet_size = (remaining_data_len > TLV_CMD_BUF_SIZE) ? TLV_CMD_BUF_SIZE : remaining_data_len;
		/*Max packet size is 16k*/
        packet_size = (packet_size > 0x4000) ? 0x4000 : packet_size;

        USB_DeviceCdcAcmSend(s_cdcVcom.cdcAcmHandle, USB_CDC_VCOM_BULK_IN_ENDPOINT,
                             (uint8_t *)data + data_len - remaining_data_len, packet_size);

        OSA_SemaphoreWait(usb_device_tx_sem, osaWaitForever_c);

        remaining_data_len -= packet_size;
    }

    NCP_USB_STATS_INC(tx);

    return 0;
}

int ncp_usb_device_init(void* argv)
{
    int ret = NCP_STATUS_SUCCESS;
    ARG_UNUSED(argv);

    ret = OSA_SemaphoreCreateBinary(usb_device_tx_sem);
    if (ret != kStatus_Success)
    {
        ncp_adap_e("Create usb device tx sem failed");
        return ret;
    }

    ret = usb_device_init();

    return ret;
}

int ncp_usb_device_deinit(void* argv)
{
    ARG_UNUSED(argv);
    int ret = NCP_STATUS_SUCCESS;

    ret = OSA_SemaphoreDestroy(usb_device_tx_sem);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("failed to destroy usb_device_tx_sem: %d", ret);
        return NCP_STATUS_ERROR;
    }

    return usb_device_deinit();
}

static int ncp_usb_device_pm_enter(int32_t pm_state)
{
    int ret = 0;

    if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = USB_DeviceEnterPowerDown();
        if(ret != 0)
        {
            ncp_adap_e("Failed to deinit USB interface");
            return NCP_STATUS_ERROR;
        }
    }
    return NCP_STATUS_SUCCESS;
}

static int ncp_usb_device_pm_exit(int32_t pm_state)
{
    int ret = 0;

    if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = USB_DeviceExitPowerDown();
        if(ret != 0)
        {
            ncp_adap_e("Failed to init USB interface");
            return NCP_STATUS_ERROR;
        }
    }
    return NCP_STATUS_SUCCESS;
}

static ncp_intf_pm_ops_t ncp_usb_device_pm_ops =
{
    .enter = ncp_usb_device_pm_enter,
    .exit  = ncp_usb_device_pm_exit,
};


ncp_intf_ops_t ncp_usb_ops =
{
    .init   = ncp_usb_device_init,
    .deinit = ncp_usb_device_deinit,
    .send   = ncp_usb_device_send,
    .recv   = NULL,
    .pm_ops = &ncp_usb_device_pm_ops,
};

#endif
