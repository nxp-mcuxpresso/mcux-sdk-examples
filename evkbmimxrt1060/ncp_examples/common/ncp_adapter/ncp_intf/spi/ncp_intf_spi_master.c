/** @file ncp_intf_spi_master.c
 *
 *  @brief main file
 *
 *  Copyright 2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#if CONFIG_NCP_SPI
#include "stdint.h"
#include "ncp_intf_spi_master.h"
#include "fsl_lpspi.h"
#include "fsl_clock.h"
#include "fsl_edma.h"
#include "fsl_iomuxc.h"
#include "fsl_dmamux.h"
#include "fsl_lpspi_edma.h"
#include "fsl_gpio.h"
#include "fsl_adapter_gpio.h"
#include "fsl_os_abstraction.h"
#include "ncp_adapter.h"
#include "ncp_debug.h"
#include "ncp_tlv_adapter.h"
#include "ncp_cmd_common.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_INIT(lpspi_master_edma_handle_t masterHandle) = {0};
edma_handle_t masterTxHandle;
edma_handle_t masterRxHandle;

#define NCP_SPI_TASK_PRIORITY     3
#define NCP_SPI_TASK_STACK_SIZE   1024
static void ncp_spi_intf_task(void *argv);
static OSA_TASK_HANDLE_DEFINE(ncp_spiTaskHandle);
static OSA_TASK_DEFINE(ncp_spi_intf_task, NCP_SPI_TASK_PRIORITY, 1, NCP_SPI_TASK_STACK_SIZE, 0);

#define NCP_HOST_CMD_SIZE_LOW_BYTE       4
#define NCP_HOST_CMD_SIZE_HIGH_BYTE      5
#define MCU_CHECKSUM_LEN                 4
#define NCP_HOST_RESPONSE_LEN            4096

edma_config_t userConfig = {0};

/* for slave inform master prepare dma ready */
OSA_SEMAPHORE_HANDLE_DEFINE(spi_slave_rx_ready);
/* for inform master transfer complete with slave */
OSA_SEMAPHORE_HANDLE_DEFINE(spi_slave_tx_complete);
/* for master tx and rx sync */
OSA_SEMAPHORE_HANDLE_DEFINE(spi_slave_rtx_sync);
/* for notify master rx task */
OSA_SEMAPHORE_HANDLE_DEFINE(spi_master_rx);

GPIO_HANDLE_DEFINE(NcpTlvSpiRxDetectGpioHandle);
GPIO_HANDLE_DEFINE(NcpTlvSpiRxReadyDetectGpioHandle);


uint32_t spi_master_buff[(OSA_EVENT_HANDLE_SIZE + 3) / 4];

#define NCP_SPI_QUEUE_NUM 256
static osa_msgq_handle_t ncp_spi_queue; /* ncp spi msgq */
OSA_MSGQ_HANDLE_DEFINE(ncp_spi_queue_buff, NCP_SPI_QUEUE_NUM,  sizeof(int));

/*******************************************************************************
 * Code
 ******************************************************************************/
static int spi_sig = 0;
static void rx_int_callback(void *param)
{
    int ret = 0;
    ret = OSA_MsgQPut(ncp_spi_queue, &spi_sig);
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("spi sig send queue failed");
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
static void rx_ready_int_callback(void *param)
{
    OSA_SemaphorePost(spi_slave_rx_ready);
}

static void ncp_host_spi_master_cb(LPSPI_Type *base,
                                   lpspi_master_edma_handle_t *handle,
                                   status_t status,
                                   void *userData)
{
    OSA_SemaphorePost(spi_slave_tx_complete);
}

int ncp_host_spi_master_tx(uint8_t *buff, uint16_t data_size)
{
    int ret = 0;
    lpspi_transfer_t masterXfer;
    uint16_t len = 0;
    uint8_t *p   = NULL;
    uint8_t hs_tx[4] = {'s', 'e', 'n', 'd'};

    /* spi slave and master handshake */
    OSA_SemaphoreWait(spi_slave_rtx_sync, osaWaitForever_c);
    mcu_host_spi_debug("master starts spi tx");
    masterXfer.txData   = hs_tx;
    masterXfer.rxData   = NULL;
    masterXfer.dataSize = 4;
    ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
    if (ret)
    {
        ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
        goto done;
    }
    OSA_SemaphoreWait(spi_slave_tx_complete, osaWaitForever_c);
    mcu_host_spi_debug("spi transfer complete-%d", __LINE__);

    OSA_SemaphoreWait(spi_slave_rx_ready, osaWaitForever_c);
    /* start to send valid data */
    len = data_size;
    p   = buff;
    masterXfer.txData   = p;
    masterXfer.rxData   = NULL;
    masterXfer.dataSize = NCP_CMD_HEADER_LEN;
    ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
    if (ret)
    {
        ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
        goto done;
    }
    OSA_SemaphoreWait(spi_slave_tx_complete, osaWaitForever_c);
    mcu_host_spi_debug("spi transfer complete-%d", __LINE__);

    len -= NCP_CMD_HEADER_LEN;
    p += NCP_CMD_HEADER_LEN;
    while (len)
    {
        OSA_SemaphoreWait(spi_slave_rx_ready, osaWaitForever_c);
        masterXfer.txData = p;
        masterXfer.rxData = NULL;
        if (len <= SPI_DMA_MAX_TRANSFER_COUNT)
            masterXfer.dataSize = len;
        else
            masterXfer.dataSize = SPI_DMA_MAX_TRANSFER_COUNT;
        ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
        if (ret)
        {
            ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
            goto done;
        }
        OSA_SemaphoreWait(spi_slave_tx_complete, osaWaitForever_c);
        mcu_host_spi_debug("spi transfer complete-%d", __LINE__);
        len -= masterXfer.dataSize;
        p += masterXfer.dataSize;
    }

done:
    /*wait slave prepare the handshake dma*/
    OSA_SemaphoreWait(spi_slave_rx_ready, osaWaitForever_c);
    OSA_SemaphorePost(spi_slave_rtx_sync);
    return ret;
}

int ncp_host_spi_master_rx(uint8_t *buff, size_t *tlv_sz)
{
    int ret = 0;
    lpspi_transfer_t masterXfer;
    uint16_t total_len = 0, resp_len = 0, len = 0;
    uint8_t *p   = NULL;
    uint8_t hs_rx[4] = {'r', 'e', 'c', 'v'};

    OSA_MsgQGet(ncp_spi_queue, &spi_sig, osaWaitForever_c);
    OSA_SemaphoreWait(spi_slave_rtx_sync, osaWaitForever_c);
    mcu_host_spi_debug("master starts spi rx");
    /* spi slave and master handshake */
    masterXfer.txData   = hs_rx;
    masterXfer.rxData   = NULL;
    masterXfer.dataSize = 4;
    ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
    if (ret)
    {
        ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
        goto done;
    }
    OSA_SemaphoreWait(spi_slave_tx_complete, osaWaitForever_c);
    mcu_host_spi_debug("spi transfer complete-%d", __LINE__);
    /* start to send valid data */
    OSA_SemaphoreWait(spi_slave_rx_ready, osaWaitForever_c);
    p  = buff;
    masterXfer.txData   = NULL;
    masterXfer.rxData   = p;
    masterXfer.dataSize = NCP_CMD_HEADER_LEN;
    ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
    if (ret)
    {
        ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
        goto done;
    }
    OSA_SemaphoreWait(spi_slave_tx_complete, osaWaitForever_c);
    mcu_host_spi_debug("spi transfer complete-%d", __LINE__);
    /* Length of the packet is indicated by byte[4] & byte[5] of
     * the packet excluding checksum [4 bytes]*/
    resp_len = (p[NCP_HOST_CMD_SIZE_HIGH_BYTE] << 8) | p[NCP_HOST_CMD_SIZE_LOW_BYTE];
    total_len = resp_len + MCU_CHECKSUM_LEN;

    if (resp_len < NCP_CMD_HEADER_LEN || total_len >= NCP_HOST_RESPONSE_LEN)
    {
        ncp_e("Invalid tlv reponse length from spi master");
        ret = -1;
        goto done;
    }
    len = total_len - NCP_CMD_HEADER_LEN;
    p += NCP_CMD_HEADER_LEN;
    while (len)
    {
        OSA_SemaphoreWait(&spi_slave_rx_ready, osaWaitForever_c);
        masterXfer.txData = NULL;
        masterXfer.rxData = p;
        if (len <= SPI_DMA_MAX_TRANSFER_COUNT)
            masterXfer.dataSize = len;
        else
            masterXfer.dataSize = SPI_DMA_MAX_TRANSFER_COUNT;
        ret = (int)LPSPI_MasterTransferEDMALite(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, &masterXfer);
        if (ret)
        {
            ncp_e("line = %d, read spi slave rx ready fail", __LINE__);
            goto done;
        }
        OSA_SemaphoreWait(&spi_slave_tx_complete, osaWaitForever_c);
        mcu_host_spi_debug("spi transfer complete-%d", __LINE__);
        len -= masterXfer.dataSize;
        p += masterXfer.dataSize;
    }

    /*wait slave prepare the handshake dma*/
    OSA_SemaphoreWait(spi_slave_rx_ready, osaWaitForever_c);
    *tlv_sz             = resp_len;
done:
    OSA_SemaphorePost(spi_slave_rtx_sync);
    mcu_host_spi_debug("finish master rx");
    return ret;
}

static void ncp_host_master_dma_setup(void)
{
/*DMA Mux setting and EDMA init*/
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
    /* DMA MUX init*/
    DMAMUX_Init(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE);

    DMAMUX_SetSource(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL,
                     EXAMPLE_LPSPI_MASTER_DMA_RX_REQUEST_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL);

    DMAMUX_SetSource(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL,
                     EXAMPLE_LPSPI_MASTER_DMA_TX_REQUEST_SOURCE);
    DMAMUX_EnableChannel(EXAMPLE_LPSPI_MASTER_DMA_MUX_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL);
#endif
    /* EDMA init*/
    EDMA_GetDefaultConfig(&userConfig);
#if defined(BOARD_GetEDMAConfig)
    BOARD_GetEDMAConfig(userConfig);
#endif
    EDMA_Init(EXAMPLE_LPSPI_MASTER_DMA_BASE, &userConfig);

    /*Set up lpspi master*/
    memset(&(masterRxHandle), 0, sizeof(masterRxHandle));
    memset(&(masterTxHandle), 0, sizeof(masterTxHandle));

    EDMA_CreateHandle(&(masterRxHandle), EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL);
    EDMA_CreateHandle(&(masterTxHandle), EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_TX_CHANNEL,
                       DEMO_LPSPI_TRANSMIT_EDMA_CHANNEL);
    EDMA_SetChannelMux(EXAMPLE_LPSPI_MASTER_DMA_BASE, EXAMPLE_LPSPI_MASTER_DMA_RX_CHANNEL,
                       DEMO_LPSPI_RECEIVE_EDMA_CHANNEL);
#endif
    NVIC_SetPriority(DMA0_DMA16_IRQn, NCP_HOST_DMA_IRQ_PRIO);
    NVIC_SetPriority(DMA1_DMA17_IRQn, NCP_HOST_DMA_IRQ_PRIO);
}

static int ncp_host_master_init(void)
{
    /* SPI init */
    int ret              = 0;
    uint32_t srcClock_Hz = 0U;
    lpspi_master_config_t masterConfig;
    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;

    LPSPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate                      = NCP_SPI_MASTER_CLOCK; // decrease this value for testing purpose.
    masterConfig.whichPcs                      = kLPSPI_Pcs0;
    masterConfig.pcsToSckDelayInNanoSec        = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.lastSckToPcsDelayInNanoSec    = 1000000000U / (masterConfig.baudRate * 2U);
    masterConfig.betweenTransferDelayInNanoSec = 1000000000U / (masterConfig.baudRate * 2U);

    srcClock_Hz = LPSPI_MASTER_CLK_FREQ;
    LPSPI_MasterInit(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterConfig, srcClock_Hz);

    return ret;
}

void ncp_host_gpio_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_01_GPIO1_IO17, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_00_GPIO1_IO16, 0U);
    /* rx input interrupt */
    hal_gpio_pin_config_t rx_config = {
        kHAL_GpioDirectionIn,
        0,
        NCP_HOST_GPIO_NUM,
        NCP_HOST_GPIO_PIN_RX,
    };
    HAL_GpioInit(NcpTlvSpiRxDetectGpioHandle, &rx_config);
    HAL_GpioSetTriggerMode(NcpTlvSpiRxDetectGpioHandle, kHAL_GpioInterruptFallingEdge);
    HAL_GpioInstallCallback(NcpTlvSpiRxDetectGpioHandle, rx_int_callback, NULL);

    /* rx_ready input interrupt */
    hal_gpio_pin_config_t rx_ready_config = {
        kHAL_GpioDirectionIn,
        0,
        NCP_HOST_GPIO_NUM,
        NCP_HOST_GPIO_PIN_RX_READY,
    };
    HAL_GpioInit(NcpTlvSpiRxReadyDetectGpioHandle, &rx_ready_config);
    HAL_GpioSetTriggerMode(NcpTlvSpiRxReadyDetectGpioHandle, kHAL_GpioInterruptFallingEdge);
    HAL_GpioInstallCallback(NcpTlvSpiRxReadyDetectGpioHandle, rx_ready_int_callback, NULL);

    NVIC_SetPriority(NCP_HOST_GPIO_IRQ, NCP_HOST_GPIO_IRQ_PRIO);
    EnableIRQ(NCP_HOST_GPIO_IRQ);

    /* Enable GPIO pin interrupt */
    GPIO_PortEnableInterrupts(NCP_HOST_GPIO, 1U << NCP_HOST_GPIO_PIN_RX);
    /* Enable GPIO pin rx_ready interrupt */
    GPIO_PortEnableInterrupts(NCP_HOST_GPIO, 1U << NCP_HOST_GPIO_PIN_RX_READY);

    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO, 0U);
    IOMUXC_SetPinMux(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI, 0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_00_LPSPI1_SCK, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_01_LPSPI1_PCS0, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_02_LPSPI1_SDO, 0x10B0U);
    IOMUXC_SetPinConfig(IOMUXC_GPIO_SD_B0_03_LPSPI1_SDI, 0x10B0U);
}

int ncp_spi_init(void *arg)
{
    int ret = kStatus_Success;
    ncp_host_gpio_init();
    ret = OSA_SemaphoreCreateBinary(spi_master_rx);
    if (ret != kStatus_Success)
    {
        ncp_e("Error: Failed to create spi_master_rx semaphore: %d", ret);
        return -NCP_FAIL;
    }

    ret = OSA_SemaphoreCreateBinary(spi_slave_rtx_sync);
    if (ret != kStatus_Success)
    {
        ncp_e("Error: Failed to create spi_slave_rtx_sync_mutex semaphore: %d", ret);
        return -NCP_FAIL;
    }
    OSA_SemaphorePost(spi_slave_rtx_sync);

    ret = OSA_SemaphoreCreateBinary(spi_slave_tx_complete);
    if (ret != kStatus_Success)
    {
        ncp_e("Error: Failed to create spi_slave_tx_complete semaphore: %d", ret);
        return -NCP_FAIL;
    }

    ret = OSA_SemaphoreCreateBinary(&spi_slave_rx_ready);
    if (ret != kStatus_Success)
    {
        ncp_e("Error: Failed to create spi_slave_rx_ready semaphore: %d", ret);
        return -NCP_FAIL;
    }

    ncp_spi_queue = (osa_msgq_handle_t)ncp_spi_queue_buff;
    ret = OSA_MsgQCreate(ncp_spi_queue, NCP_SPI_QUEUE_NUM,  sizeof(int));
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_e("failed to create ncp spi queue: %d", ret);
        return -NCP_FAIL;
    }

    (void)OSA_TaskCreate((osa_task_handle_t)ncp_spiTaskHandle, OSA_TASK(ncp_spi_intf_task), NULL);
    /*Set clock source for LPSPI*/
    CLOCK_SetMux(kCLOCK_LpspiMux, EXAMPLE_LPSPI_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_LpspiDiv, EXAMPLE_LPSPI_CLOCK_SOURCE_DIVIDER);
    ret = ncp_host_master_init();
    if (ret != NCP_SUCCESS)
    {
        ncp_e("Failed to initialize SPI master(%d)", ret);
        return ret;
    }
    ncp_host_master_dma_setup();
    /* Set up handle for spi master */
    LPSPI_MasterTransferCreateHandleEDMA(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle, ncp_host_spi_master_cb, NULL,
                                         &masterRxHandle, &masterTxHandle);
    LPSPI_MasterTransferPrepareEDMALite(
        EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle,
        EXAMPLE_LPSPI_MASTER_PCS_FOR_TRANSFER | kLPSPI_MasterByteSwap | kLPSPI_MasterPcsContinuous);
    return ret;
}

static int ncp_spi_deinit(void *argv)
{
    EDMA_AbortTransfer(&masterTxHandle);
    EDMA_AbortTransfer(&masterRxHandle);
    LPSPI_MasterTransferAbortEDMA(EXAMPLE_LPSPI_MASTER_BASEADDR, &masterHandle);
    EDMA_Deinit(EXAMPLE_LPSPI_MASTER_DMA_BASE);
    ncp_adap_d("Deint SPI Slave");
    return 0;
}

static int ncp_spi_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    int ret = 0;
    ret = ncp_host_spi_master_tx(tlv_buf, tlv_sz);
    if (ret < 0)
    {
        ncp_adap_e("SPI fail to send data %d", ret);
    }

    /*post ncp cmd resp send completed*/
    if (cb)
        cb(NULL);
    return ret;
}

static int ncp_spi_recv(uint8_t *tlv_buf, size_t *tlv_sz)
{
    int ret = 0;
    ret = ncp_host_spi_master_rx(tlv_buf, tlv_sz);
    if (ret < 0)
    {
        ncp_adap_e("SPI fail to recv data %d", ret);
    }
    return ret;
}

static int ncp_spi_pm_enter(int32_t pm_state)
{
    /* TODO: NCP uart pm */
    return 0;
}

static int ncp_spi_pm_exit(int32_t pm_state)
{
    /* TODO: NCP uart pm */
    return 0;
}

static ncp_intf_pm_ops_t ncp_spi_pm_ops =
{
    .enter = ncp_spi_pm_enter,
    .exit  = ncp_spi_pm_exit,
};

static uint8_t ncp_spi_tlvbuf[TLV_CMD_BUF_SIZE];
static void ncp_spi_intf_task(void *argv)
{
    int ret;
    size_t tlv_size = 0;

    ARG_UNUSED(argv);

    while (1)
    {
        ret = ncp_spi_recv(ncp_spi_tlvbuf, &tlv_size);
        if (NCP_STATUS_SUCCESS == ret)
        {
            ncp_tlv_dispatch(ncp_spi_tlvbuf, tlv_size);
        }
        else
        {
            ncp_adap_e("Failed to receive TLV command!");
        }
    }
}


ncp_intf_ops_t ncp_spi_ops =
{
    .init   = ncp_spi_init,
    .deinit = ncp_spi_deinit,
    .send   = ncp_spi_send,
    .recv   = ncp_spi_recv,
    .pm_ops = &ncp_spi_pm_ops,
};
#endif
