/** @file ncp_intf_spi_slave.c
 *
 *  @brief main file
 *
 *  Copyright 2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#include "ncp_intf_spi_slave.h"
#include "fsl_spi.h"
#include "fsl_spi_dma.h"
#include "fsl_dma.h"
#include "fsl_gpio.h"
#include "fsl_os_abstraction.h"
#include "ncp_adapter.h"
#include "ncp_tlv_adapter.h"
#include "pin_mux.h"
#include "fsl_io_mux.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
dma_handle_t slaveTxHandle;
dma_handle_t slaveRxHandle;
spi_dma_handle_t slaveHandle;
/* Define the init structure for the output switch pin */
gpio_pin_config_t output_pin = {
    kGPIO_DigitalOutput,
    1
};

/* Define the init structure for the input switch pin */
gpio_pin_config_t in_pin = {
    kGPIO_DigitalInput,
    0,
};

OSA_SEMAPHORE_HANDLE_DEFINE(spi_slave_trans_comp);
OSA_SEMAPHORE_HANDLE_DEFINE(spi_hs_mutex);
OSA_EVENT_HANDLE_DEFINE(spi_slave_event);

#define BOARD_NORMAL_FLEXCOMM0_FRG_CLK \
    (&(const clock_frg_clk_config_t){0, kCLOCK_FrgMainClk, 255, 0})

#define NCP_SPI_TASK_PRIORITY    11
#define NCP_SPI_TASK_STACK_SIZE  1024
static void ncp_spi_intf_task(void *argv);
static OSA_TASK_HANDLE_DEFINE(ncp_spiTaskHandle);
static OSA_TASK_DEFINE(ncp_spi_intf_task, PRIORITY_RTOS_TO_OSA(3), 1, NCP_SPI_TASK_STACK_SIZE, 0);

#define NCP_SPI_HS_TASK_PRIORITY 11
#define NCP_SPI_HS_TASK_STACK_SIZE  1024
static void ncp_spi_hs_intf_task(void *argv);
static OSA_TASK_HANDLE_DEFINE(ncp_spihsTaskHandle);
static OSA_TASK_DEFINE(ncp_spi_hs_intf_task, PRIORITY_RTOS_TO_OSA(4), 1, NCP_SPI_HS_TASK_STACK_SIZE, 0);

static int ncp_spi_pm_flag   = 0;
static uint8_t hs_p[4] = {'\0'};

/*******************************************************************************
 * Code
 ******************************************************************************/
__attribute__((__noinline__))
static void spi_delay(uint32_t loop)
{
    if (loop > 0U)
    {
        __ASM volatile(
            "1:                             \n"
            "    SUBS   %0, %0, #1          \n"
            "    CMP    %0, #0              \n"
            "    BNE    1b                  \n"
            :
            : "r"(loop));
    }
}

static void spi_delay_us(uint32_t us)
{
    uint32_t instNum;

    instNum = ((SystemCoreClock + 999999UL) / 1000000UL) * us;
    spi_delay((instNum + 2U) / 3U);
}

static void ncp_spi_slave_cb(SPI_Type *base,
                                    spi_dma_handle_t *handle,
                                    status_t status,
                                    void *userData)
{
    if (status != kStatus_Success)
    {
        ncp_adap_e("Error occurred in SPI_SlaveTransferDMA, status = %d", status);
    }
    ncp_dev_spi("receive dma interrupt");
    OSA_SemaphorePost(spi_slave_trans_comp);
}

static void ncp_spi_slave_send_sd_signal(void)
{
    /* Toggle GPIO to inform SPI master about slave start to send data. */
    GPIO_PortToggle(GPIO, 0, NCP_SPI_SLAVE_GPIO_TX_MASK);
    spi_delay_us(1);
    /* Change GPIO signal level with twice toggle operations */
    GPIO_PortToggle(GPIO, 0, NCP_SPI_SLAVE_GPIO_TX_MASK);
}

static void ncp_spi_slave_send_ready_signal(void)
{
    /* Toggle GPIO to inform SPI master about slave TX ready. */
    GPIO_PortToggle(GPIO, 0, NCP_SPI_SLAVE_GPIO_RX_READY_MASK);
    spi_delay_us(1);
    /* Change GPIO signal level with twice toggle operations */
    GPIO_PortToggle(GPIO, 0, NCP_SPI_SLAVE_GPIO_RX_READY_MASK);
}

static int ncp_spi_tx(uint8_t *buff, size_t data_size)
{
    int ret = 0;
    status_t spi_ret;
    spi_transfer_t slaveXfer;
    size_t left_len = 0;
    uint8_t *p = NULL;
    osa_event_flags_t events;
    OSA_EventWait((osa_event_handle_t)spi_slave_event, SLAVE_TX_ENABLE_EVENT, 0, osaWaitForever_c, &events);
    ncp_dev_spi("slave starts to send valid data");
    /* spi start valid data transfer */
    left_len = data_size;
    p = buff;
    slaveXfer.txData = p;
    slaveXfer.rxData = NULL;
    slaveXfer.dataSize = TLV_CMD_HEADER_LEN;
    slaveXfer.configFlags = kSPI_FrameAssert;
    spi_ret = (int)SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);
    if (spi_ret != kStatus_Success)
    {
        ncp_adap_e("Error occurred in SPI_SlaveTransferDMA");
        return ret;
    }
    /* notify master that the slave prepare DMA ready */
    ncp_spi_slave_send_ready_signal();
    /* wait for spi transfer complete */
    OSA_SemaphoreWait(spi_slave_trans_comp, osaWaitForever_c);
    ncp_dev_spi("spi transfer complete-tx-%d", __LINE__);

    /* Prepare DMA for remaining bytes */
    left_len -= TLV_CMD_HEADER_LEN;
    p += TLV_CMD_HEADER_LEN;
    while(left_len)
    {
        slaveXfer.txData = p;
        slaveXfer.rxData = NULL;
        if(left_len <= DMA_MAX_TRANSFER_COUNT)
            slaveXfer.dataSize = left_len;
        else
            slaveXfer.dataSize = DMA_MAX_TRANSFER_COUNT;
        slaveXfer.configFlags = kSPI_FrameAssert;
        ret = (int)SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);
        if(ret)
        {
            ncp_adap_e("Error occurred in SPI_SlaveTransferDMA");
            return ret;
        }
        /* notify master that the slave prepare DMA ready */
        ncp_spi_slave_send_ready_signal();
        OSA_SemaphoreWait(spi_slave_trans_comp, osaWaitForever_c);
        ncp_dev_spi("spi transfer complete-tx-%d", __LINE__);
        left_len -= slaveXfer.dataSize;
        p += slaveXfer.dataSize;
    }
    OSA_SemaphorePost(spi_hs_mutex);
    ncp_dev_spi("ncp slave tx finished");
    return ret;
}

static int ncp_spi_slave_tx(uint8_t *buff, size_t data_size)
{
    int ret = 0;
    ncp_dev_spi("spi slave sends spi tx signal");
    ncp_spi_slave_send_sd_signal();
    ret = ncp_spi_tx(buff, data_size);
    return ret;
}

static int ncp_spi_rx(uint8_t *buff, size_t *tlv_sz)
{
    int ret = 0;
    status_t spi_ret;
    spi_transfer_t slaveXfer;
    size_t total_len = 0, cmd_len = 0, left_len = 0;
    uint8_t *p = buff;
    osa_event_flags_t events;

    /* wait master send command */
    OSA_EventWait((osa_event_handle_t)spi_slave_event, SLAVE_RX_ENABLE_EVENT, 0, osaWaitForever_c, &events);

    ncp_dev_spi("slave starts to spi slave rx");
    /* start to spi transfer valid data */
    slaveXfer.txData = NULL;
    slaveXfer.rxData = p;
    slaveXfer.dataSize = TLV_CMD_HEADER_LEN;
    slaveXfer.configFlags = kSPI_FrameAssert;
    spi_ret = SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);
    if (spi_ret != kStatus_Success)
    {
        ncp_adap_e("Error occurred in SPI_SlaveTransferDMA");
        return ret;
    }
    /* notify master that the slave prepare DMA ready */
    ncp_spi_slave_send_ready_signal();
    OSA_SemaphoreWait(spi_slave_trans_comp, osaWaitForever_c);
    ncp_dev_spi("spi transfer complete-rx-%d", __LINE__);
    cmd_len = (p[TLV_CMD_SIZE_HIGH_BYTES] << 8) | p[TLV_CMD_SIZE_LOW_BYTES];
    if (cmd_len < TLV_CMD_HEADER_LEN || cmd_len > TLV_CMD_BUF_SIZE)
    {
        ncp_adap_e("the tlv header length is wrong");
        return -1;
    }
    /* p steps to next send pointer */
    p += TLV_CMD_HEADER_LEN;
    total_len = cmd_len + NCP_CHKSUM_LEN;
    left_len = total_len - TLV_CMD_HEADER_LEN;
    while(left_len)
    {
        slaveXfer.txData = NULL;
        /* same as above */
        slaveXfer.rxData = p;
        if(left_len <= DMA_MAX_TRANSFER_COUNT)
            slaveXfer.dataSize = left_len;
        else
            slaveXfer.dataSize = DMA_MAX_TRANSFER_COUNT;
        slaveXfer.configFlags = kSPI_FrameAssert;
        ret = (int)SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);
        if(ret)
        {
            ncp_adap_e("Error occurred in SPI_SlaveTransferDMA");
            return ret;
        }
        /* notify master that the slave prepare DMA ready */
        ncp_spi_slave_send_ready_signal();
        OSA_SemaphoreWait(spi_slave_trans_comp, osaWaitForever_c);
        ncp_dev_spi("spi transfer complete-rx-%d", __LINE__);
        /* update left length */
        left_len -= slaveXfer.dataSize;
        /* step to p */
        p += slaveXfer.dataSize;
    }
    *tlv_sz = total_len;
    OSA_SemaphorePost(spi_hs_mutex);
    ncp_dev_spi("ncp slave rx finished");
    return ret;
}


static void ncp_spi_output_gpio_init(void)
{
    IO_MUX_SetPinMux(IO_MUX_GPIO27);
    IO_MUX_SetPinMux(IO_MUX_GPIO11);
    GPIO_PortInit(GPIO, 0);
    GPIO_PortInit(GPIO, 1);
    /* Init output GPIO. Default level is high */
    /* GPIO 27 for TX and GPIO 11 for RX interrupt */
    GPIO_PinInit(GPIO, 0, NCP_SPI_GPIO_TX, &output_pin);
    GPIO_PinInit(GPIO, 0, NCP_SPI_GPIO_RX_READY, &output_pin);
    IO_MUX_SetPinOutLevelInSleep(27U, IO_MUX_SleepPinLevelUnchanged);
    IO_MUX_SetPinOutLevelInSleep(11U, IO_MUX_SleepPinLevelUnchanged);
}

static void ncp_spi_slave_dma_setup(void)
{
    /* DMA init */
    DMA_Init(NCP_SPI_SLAVE_DMA);
    /* Configure the DMA channel,priority and handle. */
    DMA_EnableChannel(NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_TX_CHANNEL);
    DMA_EnableChannel(NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_RX_CHANNEL);
    NVIC_SetPriority(DMA0_IRQn, SPI_DMA_ISR_PRIORITY);
    DMA_SetChannelPriority(NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_SetChannelPriority(NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&slaveTxHandle, NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_TX_CHANNEL);
    DMA_CreateHandle(&slaveRxHandle, NCP_SPI_SLAVE_DMA, NCP_SPI_SLAVE_DMA_RX_CHANNEL);
}

bool spi_hs_task_init = false;
static int ncp_spi_slave_init(void)
{
    int ret = 0;
    spi_slave_config_t slaveConfig;
    SPI_SlaveGetDefaultConfig(&slaveConfig);
    /* Initialize the SPI slave. */
    slaveConfig.sselPol = (spi_spol_t)NCP_SPI_SLAVE_SPOL;
    ret = (int)SPI_SlaveInit(NCP_SPI_SLAVE, &slaveConfig);

    return ret;
}
int ncp_spi_enter_power_down(void)
{
    DMA_AbortTransfer(&slaveTxHandle);
    DMA_AbortTransfer(&slaveRxHandle);
    SPI_MasterTransferAbortDMA(NCP_SPI_SLAVE, &slaveHandle);
    DMA_Deinit(NCP_SPI_SLAVE_DMA);

    return NCP_PM_STATUS_SUCCESS;
}

int ncp_spi_exit_power_down(void)
{
    spi_transfer_t slaveXfer;

    ncp_spi_output_gpio_init();
    CLOCK_SetFRGClock(BOARD_NORMAL_FLEXCOMM0_FRG_CLK);
    CLOCK_AttachClk(kFRG_to_FLEXCOMM0);
    (void)ncp_spi_slave_init();

    ncp_spi_slave_dma_setup();
    /* Set up handle for spi slave */
    (void)SPI_SlaveTransferCreateHandleDMA(NCP_SPI_SLAVE, &slaveHandle,
                                           ncp_spi_slave_cb, NULL,
                                           &slaveTxHandle, &slaveRxHandle);

    slaveXfer.txData = NULL;
    slaveXfer.rxData = hs_p;
    slaveXfer.dataSize = 4;
    slaveXfer.configFlags = kSPI_FrameAssert;
    (void)SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);

    return NCP_PM_STATUS_SUCCESS;
}
static int ncp_spi_init(void *argv)
{
    int ret = 0;
    ret = OSA_SemaphoreCreateBinary(spi_slave_trans_comp);
    if (ret != kStatus_Success)
    {
        ncp_adap_e("Create spi slave binary fail");
        return ret;
    }
    ret = OSA_SemaphoreCreateBinary(spi_hs_mutex);
    if (ret != kStatus_Success)
    {
        ncp_adap_e("Create spi slave binary fail");
        return ret;
    }
    OSA_SemaphorePost(spi_hs_mutex);
    ret = OSA_EventCreate(spi_slave_event, 1);
    if (ret != kStatus_Success)
    {
        ncp_adap_e("Create spi slave event fail");
        return ret;
    }
    ncp_spi_output_gpio_init();
    CLOCK_SetFRGClock(BOARD_NORMAL_FLEXCOMM0_FRG_CLK);
    CLOCK_AttachClk(kFRG_to_FLEXCOMM0);
    ret = ncp_spi_slave_init();
    if(ret != 0)
    {
        ncp_adap_e("Failed to initialize SPI slave(%d)", ret);
        return ret;
    }
    ncp_spi_slave_dma_setup();
    /* Set up handle for spi slave */
    ret = (int)SPI_SlaveTransferCreateHandleDMA(NCP_SPI_SLAVE, &slaveHandle,
                                                ncp_spi_slave_cb, NULL,
                                                &slaveTxHandle, &slaveRxHandle);
    ret = OSA_TaskCreate((osa_task_handle_t)ncp_spihsTaskHandle, OSA_TASK(ncp_spi_hs_intf_task), NULL);
    if (KOSA_StatusSuccess != ret)
    {
        ncp_adap_e("Failed to create task", ret);
    }
    ret = OSA_TaskCreate((osa_task_handle_t)ncp_spiTaskHandle, OSA_TASK(ncp_spi_intf_task), NULL);
    if (KOSA_StatusSuccess != ret)
    {
        ncp_adap_e("Failed to create task", ret);
    }

    return ret;
}

static int ncp_spi_deinit(void *argv)
{
    DMA_AbortTransfer(&slaveTxHandle);
    DMA_AbortTransfer(&slaveRxHandle);
    SPI_MasterTransferAbortDMA(NCP_SPI_SLAVE, &slaveHandle);
    DMA_Deinit(NCP_SPI_SLAVE_DMA);
    OSA_SemaphoreDestroy(spi_slave_trans_comp);
    OSA_SemaphoreDestroy(spi_hs_mutex);
    (void)OSA_TaskDestroy((osa_task_handle_t)ncp_spiTaskHandle);
    (void)OSA_TaskDestroy((osa_task_handle_t)ncp_spihsTaskHandle);
    spi_hs_task_init = false;
    OSA_EventDestroy(spi_slave_event);
    ncp_adap_d("Deint SPI Slave");
    return 0;
}

static int ncp_spi_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    int ret = 0;
    ret = ncp_spi_slave_tx(tlv_buf, tlv_sz);
    if (ret < 0)
    {
        ncp_adap_e("SPI fail to send data %d", ret);
        NCP_SPI_STATS_INC(err);
    }
    else
        NCP_SPI_STATS_INC(tx);
    /*post ncp cmd resp send completed*/
    cb(NULL);

    return ret;
}

static int ncp_spi_recv(uint8_t *tlv_buf, size_t *tlv_sz)
{
    int ret = 0;
    ret = ncp_spi_rx(tlv_buf, tlv_sz);
    if (ret < 0)
    {
        ncp_adap_e("SPI fail to recv data %d", ret);
        NCP_SPI_STATS_INC(err);
    }
    else
        NCP_SPI_STATS_INC(rx);
    return ret;
}

int ncp_spi_txrx_is_finish(void)
{
    return ncp_spi_pm_flag;
}

static int ncp_spi_pm_enter(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

    if(pm_state == NCP_PM_STATE_PM3)
    {
        if(!ncp_spi_pm_flag)
	    {
            /* Tx or Rx is not finished */
            ret = NCP_PM_STATUS_NOT_READY;
            return ret;
        }
        ret = ncp_spi_enter_power_down();
        if(ret != 0)
        {
            ncp_adap_e("Failed to enter SPI power down");
            ret = (int)NCP_PM_STATUS_ERROR;
	}
    }
    return ret;
}

static int ncp_spi_pm_exit(int32_t pm_state)
{
    int ret = (int)NCP_PM_STATUS_SUCCESS;

    if(pm_state == NCP_PM_STATE_PM3)
    {
        ret = ncp_spi_exit_power_down();
        if(ret != 0)
        {
            ncp_adap_e("Failed to exit SPI power down");
            ret = (int)NCP_PM_STATUS_ERROR;
	    }
    }
    return ret;
}

static ncp_intf_pm_ops_t ncp_spi_pm_ops =
{
    .enter = ncp_spi_pm_enter,
    .exit  = ncp_spi_pm_exit,
};

ncp_intf_ops_t ncp_spi_ops =
{
    .init   = ncp_spi_init,
    .deinit = ncp_spi_deinit,
    .send   = ncp_spi_send,
    .recv   = ncp_spi_recv,
    .pm_ops = &ncp_spi_pm_ops,
};

static uint8_t ncp_spi_tlvbuf[TLV_CMD_BUF_SIZE];
static void ncp_spi_intf_task(void *argv)
{
    int ret;
    size_t tlv_size = 0;;
    uint8_t *recv_buf = ncp_spi_tlvbuf;
    while (1)
    {
        /*wait ncp cmd resp send completed*/
        ret = ncp_spi_recv(ncp_spi_tlvbuf, &tlv_size);
        if (ret != NCP_STATUS_SUCCESS)
        {
            ncp_adap_e("Failed to receive command header(%d)", ret);
            continue;
        }
        else
        {
            ncp_adap_d("ncp spi receive data size = (%d)", tlv_size);
        }
        ncp_tlv_dispatch(recv_buf, tlv_size - NCP_CHKSUM_LEN);
    }
}

static void ncp_spi_hs_intf_task(void *argv)
{
    int ret = 0;
    spi_transfer_t slaveXfer;
    PRINTF("Start the spi handshake task\r\n");
    while (1)
    {
        OSA_SemaphoreWait(spi_hs_mutex, osaWaitForever_c);
        ncp_dev_spi("enter spi hs task");
        /* spi master and slave handshake */
        slaveXfer.txData = NULL;
        slaveXfer.rxData = hs_p;
        slaveXfer.dataSize = 4;
        slaveXfer.configFlags = kSPI_FrameAssert;
        ret = (int)SPI_SlaveTransferDMA(NCP_SPI_SLAVE, &slaveHandle, &slaveXfer);
        if (ret)
        {
            ncp_adap_e("Error occurred in SPI_SlaveTransferDMA");
            continue;
        }
        if(spi_hs_task_init)
        {
            /* notify master that the slave prepare DMA ready */
            ncp_spi_slave_send_ready_signal();
        }
        spi_hs_task_init = true;
        ncp_spi_pm_flag = 1;
        ncp_dev_spi("spi can enter pm mode");
        OSA_SemaphoreWait(spi_slave_trans_comp, osaWaitForever_c);
        ncp_spi_pm_flag = 0;
        ncp_dev_spi("spi can't enter pm mode");
        if (memcmp(hs_p, "send", 4) == 0)
        {
            ncp_dev_spi("spi hs receive send command");
            OSA_EventClear(spi_slave_event, SLAVE_TX_ENABLE_EVENT);
            OSA_EventSet(spi_slave_event, SLAVE_RX_ENABLE_EVENT);
        }
        else if (memcmp(hs_p, "recv", 4) == 0)
        {
            ncp_dev_spi("spi hs receive recv command");
            OSA_EventClear(spi_slave_event, SLAVE_RX_ENABLE_EVENT);
            OSA_EventSet(spi_slave_event, SLAVE_TX_ENABLE_EVENT);
        }
        else
        {
            ncp_adap_e("Unkonw spi handshake status");
        }
    }
}
