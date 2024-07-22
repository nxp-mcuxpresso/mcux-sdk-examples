/*
 * Copyright 2022-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "crc.h"
#include "ncp_tlv_adapter.h"
#include "ncp_adapter.h"
#include "fsl_os_abstraction.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if CONFIG_NCP_UART
extern ncp_intf_ops_t ncp_uart_ops;
#elif (CONFIG_NCP_SPI)
extern ncp_intf_ops_t ncp_spi_ops;
#elif (CONFIG_NCP_USB)
extern ncp_intf_ops_t ncp_usb_ops;
#elif (CONFIG_NCP_SDIO)
extern ncp_intf_ops_t ncp_sdio_ops;
#endif

/* Global variable containing NCP internal statistics */
#if CONFIG_NCP_DEBUG
ncp_stats_t ncp_stats;
#endif

ncp_tlv_adapter_t ncp_tlv_adapter;

/* NCP adapter TX variable*/
/* NCP adapter tx queue */
static osa_msgq_handle_t ncp_tlv_msgq_handle; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(ncp_tlv_msgq_buff, NCP_TLV_QUEUE_LENGTH,  sizeof(void *));

/* NCP adapter tx mutex for queue counter*/
OSA_MUTEX_HANDLE_DEFINE(ncp_tlv_queue_mutex);
/* NCP adapter tx task */
static OSA_TASK_HANDLE_DEFINE(ncp_tlv_thread);

void ncp_tlv_process(osa_task_param_t arg);
static OSA_TASK_DEFINE(ncp_tlv_process, NCP_TLV_TX_TASK_PRIORITY, 1, NCP_TLV_TX_TASK_STACK_SIZE, 0);
/* NCP adapter tx queue counter */
static int ncp_tlv_queue_len = 0;

/*******************************************************************************
 * API
 ******************************************************************************/

static void ncp_tlv_cb(void *arg)
{
    /* todo */
}

static void ncp_tlv_free_elmt(ncp_tlv_qelem_t **qbuf)
{
    OSA_MemoryFree(*qbuf);
    *qbuf = NULL;
    OSA_MutexLock((osa_mutex_handle_t)ncp_tlv_queue_mutex, osaWaitForever_c);
    if (ncp_tlv_queue_len <= 0)
        ncp_adap_e("invalid ncp tlv queue length: %d", NCP_TLV_QUEUE_LENGTH);
    else
        ncp_tlv_queue_len--;
    OSA_MutexUnlock(ncp_tlv_queue_mutex);
}

void ncp_tlv_process(osa_task_param_t arg)
{
    ncp_tlv_qelem_t *qbuf = NULL;
    while (1)
    {
        if( OSA_MsgQGet(ncp_tlv_msgq_handle, &qbuf, osaWaitForever_c) == KOSA_StatusSuccess)
        {
            NCP_TLV_STATS_INC(rx);
			/* sync send data */
            ncp_tlv_adapter.intf_ops->send(qbuf->tlv_buf, qbuf->tlv_sz, ncp_tlv_cb);
			/* free element */
            ncp_tlv_free_elmt(&qbuf);
        }
        else
            NCP_TLV_STATS_INC(err);
    }
}

/*
    enqueue the qelement to ncp tx queue
*/
static ncp_status_t ncp_tlv_tx_enque(ncp_tlv_qelem_t *qelem)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;
    OSA_MutexLock(ncp_tlv_queue_mutex, osaWaitForever_c);
    if (ncp_tlv_queue_len == NCP_TLV_QUEUE_LENGTH)
    {
        ncp_adap_e("ncp tlv queue is full max queue length: %d", NCP_TLV_QUEUE_LENGTH);
        status = NCP_STATUS_QUEUE_FULL;
        goto Fail;
    }
    if (OSA_MsgQPut(ncp_tlv_msgq_handle, &qelem) != KOSA_StatusSuccess)
    {
        ncp_adap_e("ncp tlv enqueue failure");
        NCP_TLV_STATS_INC(err);
        status = NCP_STATUS_ERROR;
        goto Fail;
    }
    else
    {
        ncp_tlv_queue_len++;
		NCP_TLV_STATS_INC(tx);
        ncp_adap_d("enque tlv_buf success");
    }
Fail:
    OSA_MutexUnlock((osa_mutex_handle_t)ncp_tlv_queue_mutex);
    return status;
}


/*
    qbuf_len = sizeof(ncp_tlv_qelem_t) + sdio_intf_head + tlv_sz + chksum_len
    qbuf_tlv = qbuf + sizeof(ncp_tlv_qelem_t)
    memcpy_buf = qbuf + sizeof(ncp_tlv_qelem_t) + sdio_intf_head
    chksum_buf = qbuf + sizeof(ncp_tlv_qelem_t) + sdio_intf_head + tlv_sz
    sdio_intf_head: reserved length for sdio interface header
*/
ncp_status_t ncp_tlv_send(void *tlv_buf, size_t tlv_sz)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;
    ncp_tlv_qelem_t *qbuf = NULL;
    uint8_t *qbuf_tlv = NULL, *chksum_buf = NULL;
    uint16_t qlen = 0, sdio_intf_head = 0, chksum_len = 4;
    uint32_t chksum = 0;
#if CONFIG_NCP_SDIO
    sdio_intf_head = 4;
#endif
    qlen = sizeof(ncp_tlv_qelem_t) + sdio_intf_head + tlv_sz + chksum_len;
    qbuf = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(qlen);
    if (!qbuf)
    {
        NCP_TLV_STATS_INC(drop);
        ncp_adap_d("%s: failed to allocate memory for tlv queue element", __FUNCTION__);
        return NCP_STATUS_NOMEM;
    }
    qbuf->tlv_sz = tlv_sz + chksum_len;
    qbuf->priv = NULL;
    qbuf_tlv = (uint8_t *)qbuf + sizeof(ncp_tlv_qelem_t);
    memcpy(qbuf_tlv + sdio_intf_head, tlv_buf, tlv_sz);
    qbuf->tlv_buf = qbuf_tlv;
    chksum = ncp_tlv_chksum(qbuf_tlv + sdio_intf_head, (uint16_t)tlv_sz);
    chksum_buf = qbuf_tlv + sdio_intf_head + tlv_sz;
    chksum_buf[0] = chksum & 0xff;
    chksum_buf[1] = (chksum & 0xff00) >> 8;
    chksum_buf[2] = (chksum & 0xff0000) >> 16;
    chksum_buf[3] = (chksum & 0xff000000) >> 24;

    status = ncp_tlv_tx_enque(qbuf);
    if(status != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("ncp tlv enque element fail");
        if (qbuf)
        {
            OSA_MemoryFree(qbuf);
            qbuf = NULL;
        }
    }

    return status;
}

static ncp_status_t ncp_tlv_tx_init(void)
{
    if (OSA_MutexCreate(ncp_tlv_queue_mutex) != KOSA_StatusSuccess)
    {
        ncp_adap_e("%s, ncp tx mutex create fail");
        return NCP_STATUS_ERROR;
    }
    ncp_tlv_msgq_handle = (osa_msgq_handle_t)ncp_tlv_msgq_buff;
    if (OSA_MsgQCreate(ncp_tlv_msgq_handle, NCP_TLV_QUEUE_LENGTH,  sizeof(void *)) != KOSA_StatusSuccess)
    {
        ncp_adap_e("ncp tx msg queue create fail");
        return NCP_STATUS_ERROR;
    }
    (void)OSA_TaskCreate((osa_task_handle_t)ncp_tlv_thread, OSA_TASK(ncp_tlv_process), NULL);
    ncp_adap_d("ncp tx init success");
    return NCP_STATUS_SUCCESS;
}

static void ncp_tlv_tx_deinit(void)
{
    ncp_tlv_qelem_t *qbuf = NULL;
    if (OSA_TaskDestroy(ncp_tlv_thread) != KOSA_StatusSuccess)
    {
        ncp_adap_e("ncp adapter tx deint Task fail");
    }
    while (1)
    {
        if( OSA_MsgQGet(ncp_tlv_msgq_handle, &qbuf, 0) == KOSA_StatusSuccess)
        {
            /* free element */
            ncp_tlv_free_elmt(&qbuf);
        }
        else
        {
            ncp_adap_d("ncp adapter queue flush completed");
            break;
        }
    }
    OSA_MutexLock(ncp_tlv_queue_mutex, osaWaitForever_c);
    ncp_tlv_queue_len = 0;
    OSA_MutexUnlock(ncp_tlv_queue_mutex);
    if (OSA_MutexDestroy(ncp_tlv_queue_mutex) != KOSA_StatusSuccess)
    {
        ncp_adap_e("ncp adapter tx deint queue mutex fail");
    }
    if (OSA_MsgQDestroy(ncp_tlv_msgq_handle) != KOSA_StatusSuccess)
    {
        ncp_adap_e("ncp adapter tx deint MsgQ fail");
    }
}

ncp_status_t ncp_adapter_init(void)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;

#if CONFIG_NCP_UART
    ncp_tlv_adapter.intf_ops = &ncp_uart_ops;
#elif (CONFIG_NCP_SPI)
    ncp_tlv_adapter.intf_ops = &ncp_spi_ops;
#elif (CONFIG_NCP_USB)
    ncp_tlv_adapter.intf_ops = &ncp_usb_ops;
#elif (CONFIG_NCP_SDIO)
    ncp_tlv_adapter.intf_ops = &ncp_sdio_ops;
#endif
    /* Init CRC32 */
    ncp_tlv_chksum_init();
    status = ncp_tlv_tx_init();
    if (status != NCP_STATUS_SUCCESS)
    {
        return status;
    }
    /* Init interface */
    status = (ncp_status_t)ncp_tlv_adapter.intf_ops->init(NULL);
    if (status != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("ncp adapater init fail");
        return status;
    }

    return status;
}

ncp_status_t ncp_adapter_deinit(void)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;

    /* Deinit interface */
    status = (ncp_status_t)ncp_tlv_adapter.intf_ops->deinit(NULL);

    ncp_tlv_adapter.intf_ops = NULL;
    ncp_tlv_tx_deinit();
    return status;
}

void ncp_tlv_install_handler(uint8_t class, void *func_cb)
{
    NCP_ASSERT((uint8_t)NCP_MAX_CLASS > class);
    NCP_ASSERT(NULL != func_cb);

    ncp_tlv_adapter.tlv_handler[class] = (tlv_callback_t)func_cb;
}

void ncp_tlv_uninstall_handler(uint8_t class)
{
    NCP_ASSERT((uint8_t)NCP_MAX_CLASS > class);

    ncp_tlv_adapter.tlv_handler[class] = NULL;
}

void ncp_tlv_dispatch(void *tlv, size_t tlv_sz)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;
    uint32_t local_checksum = 0, remote_checksum = 0;
    uint8_t class = 0;

    ncp_adap_d("Receive TLV command, dispatch it!");

    /* check CRC */
    remote_checksum = NCP_GET_PEER_CHKSUM((uint8_t *)tlv, tlv_sz);
    local_checksum  = ncp_tlv_chksum(tlv, tlv_sz);
    if (remote_checksum != local_checksum)
    {
        status = NCP_STATUS_CHKSUMERR;
        ncp_adap_e("Checksum validation failed!");
        return;
    }

    /* TLV command class */
    class = NCP_GET_CLASS(*((uint32_t *)tlv));
    if (ncp_tlv_adapter.tlv_handler[class])
        ncp_tlv_adapter.tlv_handler[class](tlv, tlv_sz, status);
}
