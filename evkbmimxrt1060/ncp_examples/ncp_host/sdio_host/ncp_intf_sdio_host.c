/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if CONFIG_NCP_SDIO
#include <fsl_common.h>
#include <fsl_sdio.h>
#include "ncp_intf_sdio_host.h"
#include "fsl_adapter_gpio.h"
#include "fsl_gpio.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "sdmmc_config.h"
#include "ncp_adapter.h"
#include "ncp_debug.h"
#include "fsl_iomuxc.h"

/*******************************************************************************
 * Private macro
 ******************************************************************************/
#define SDHOST_CORE_INPUT_PRIO   3
#define SDHOST_CORE_STACK_SIZE   (1024)
#define SDHOST_CORE_START        0x01
#define SDHOST_RESCAN_PRIO       3
#define SDHOST_RESCAN_STACK_SIZE (1500)
#define SDHOST_RESCAN_START      0x01

#define NCP_HOST_SDIO_GPIO               GPIO1
#define NCP_HOST_SDIO_GPIO_NUM           1
#define NCP_HOST_SDIO_GPIO_PIN           21U
#define NCP_HOST_SDIO_GPIO_IRQ           GPIO1_Combined_16_31_IRQn
#define NCP_HOST_SDIO_GPIO_IRQ_PRIO      3

/*! @brief SD power reset */
#define BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE GPIO1
//#define BOARD_SDMMC_SD_POWER_RESET_GPIO_PORT 1
#define BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN 19U

/*!@ brief host interrupt priority*/
#define BOARD_SDMMC_SDIO_HOST_IRQ_PRIORITY (5U)

/** Card Control Registers : Card to host event */
#define CARD_TO_HOST_EVENT_REG 0x5C
/** Card Control Registers : Upload card ready */
#define UP_LD_CARD_RDY (0x1U << 1)
/** Card Control Registers : Download card ready */
#define DN_LD_CARD_RDY (0x1U << 0)

/** The number of times to try when polling for status bits */
#define MAX_POLL_TRIES 100U

/** Card Control Registers : Function 1 Block size 0 */
#define FN1_BLOCK_SIZE_0 0x110
/** Card Control Registers : Function 1 Block size 1 */
#define FN1_BLOCK_SIZE_1 0x111

#define SDIO_DATA_OUTBUF_LEN 2052U
#define SDIO_CMD_OUTBUF_LEN  4100U

/** Port for memory */
#define MEM_PORT 0x10000

/** Card Control Registers : sdio new mode register 1 */
#define CARD_CONFIG_2_1_REG 0xD9
/** Card Control Registers : cmd53 new mode */
#define CMD53_NEW_MODE (0x1U << 0)

/* Card Control Registers : Command port configuration 0 */
#define CMD_CONFIG_0       0xC4
#define CMD_PORT_RD_LEN_EN (0x1U << 2)
/* Card Control Registers : Command port configuration 1 */
#define CMD_CONFIG_1 0xC5
/* Card Control Registers : cmd port auto enable */
#define CMD_PORT_AUTO_EN (0x1U << 0)

/** Host Control Registers : Host interrupt RSR */
#define HOST_INT_RSR_REG  0x04
#define HOST_INT_RSR_MASK 0xFF

/** Card Control Registers : Miscellaneous Configuration Register */
#define CARD_MISC_CFG_REG 0xD8

/** BIT value */
#define MBIT(x) (((uint32_t)1) << (x))
/** Misc. Config Register : Auto Re-enable interrupts */
#define AUTO_RE_ENABLE_INT MBIT(4)

/** Firmware status 0 register (SCRATCH0_0) */
#define CARD_FW_STATUS0_REG 0xe8
/** Firmware status 1 register (SCRATCH0_1) */
#define CARD_FW_STATUS1_REG 0xe9

/** define SDIO block size for data Tx/Rx */
/* We support up to 480-byte block size due to FW buffer limitation. */
#define SDIO_BLOCK_SIZE 256U

/* Command port */
#define CMD_PORT_SLCT 0x8000U
/** Data port mask */
#define DATA_PORT_MASK 0xffffffffU

/** Host Control Registers : Host interrupt mask */
#define HOST_INT_MASK_REG 0x08
/** Host Control Registers : Upload host interrupt mask */
#define UP_LD_HOST_INT_MASK (0x1U)
/** Host Control Registers : Download host interrupt mask */
#define DN_LD_HOST_INT_MASK (0x2U)
/** Host Control Registers : Cmd port upload interrupt mask */
#define CMD_PORT_UPLD_INT_MASK (0x1U << 6)
/** Host Control Registers : Cmd port download interrupt mask */
#define CMD_PORT_DNLD_INT_MASK (0x1U << 7)
/** Enable Host interrupt mask */
#define HIM_ENABLE (UP_LD_HOST_INT_MASK | DN_LD_HOST_INT_MASK | CMD_PORT_UPLD_INT_MASK | CMD_PORT_DNLD_INT_MASK)

/* Card Control Registers : Command port read length 0 */
#define CMD_RD_LEN_0 0xC0
/* Card Control Registers : Command port read length 1 */
#define CMD_RD_LEN_1 0xC1

/** Firmware ready */
#define FIRMWARE_READY 0xfedcU

#define MNULL ((void *)0)

/** Port for registers */
#define REG_PORT 0U
/** SDIO Block/Byte mode mask */
#define SDIO_BYTE_MODE_MASK 0x80000000U
/** Maximum numbfer of registers to read for multiple port */
#define MAX_MP_REGS 196
/** Maximum port */
#define MAX_PORT 32U
/** Host Control Registers : Host interrupt status */
#define HOST_INT_STATUS_REG 0x0C
/** LSB of read bitmap */
#define RD_BITMAP_L 0x10
/** MSB of read bitmap */
#define RD_BITMAP_U 0x11
/** LSB of read bitmap second word */
#define RD_BITMAP_1L 0x12
/** MSB of read bitmap second word */
#define RD_BITMAP_1U 0x13
/** LSB of write bitmap */
#define WR_BITMAP_L 0x14
/** MSB of write bitmap */
#define WR_BITMAP_U 0x15
/** LSB of write bitmap second word */
#define WR_BITMAP_1L 0x16
/** MSB of write bitmap second word */
#define WR_BITMAP_1U 0x17

/** Host Control Registers : Upload command port host interrupt status */
#define UP_LD_CMD_PORT_HOST_INT_STATUS (0x40U)
/** Host Control Registers : Download command port host interrupt status */
#define DN_LD_CMD_PORT_HOST_INT_STATUS (0x80U)
/** Host Control Registers : Upload host interrupt status */
#define UP_LD_HOST_INT_STATUS (0x1U)
/** Host Control Registers : Download host interrupt status */
#define DN_LD_HOST_INT_STATUS (0x2U)

/** Macros for Data Alignment : address */
#define ALIGN_ADDR(p, a) ((((uint32_t)(p)) + (((uint32_t)(a)) - 1U)) & ~(((uint32_t)(a)) - 1U))
/** DMA alignment */
#define DMA_ALIGNMENT 64U

/** LSB of read length for port 0 */
#define RD_LEN_P0_L 0x18
/** MSB of read length for port 0 */
#define RD_LEN_P0_U 0x19

/** Type command */
#define SDIO_TYPE_CMD 1U
/** Type data */
#define SDIO_TYPE_DATA 0U
/** Type event */
#define SDIO_TYPE_EVENT 3U

/** SDIO header length */
#define SDIO_HEADER_LEN 4U

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

#define NCP_CMD_WLAN_SOCKET_SEND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000004)
#define NCP_CMD_WLAN_SOCKET_SENDTO    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000005)

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

/*NCP command header*/
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

#define NCP_CMD_HEADER_LEN sizeof(NCP_COMMAND)

/*******************************************************************************
 * Definitations
 ******************************************************************************/
OSA_EVENT_HANDLE_DEFINE(sdhost_core_events);
OSA_EVENT_HANDLE_DEFINE(sdhost_rescan_events);
GPIO_HANDLE_DEFINE(ncp_host_sdio_notify_handle);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void sdhost_core_input(void *argv);
void sdhost_rescan_task(void *argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static sdio_card_t g_sdio_card;
static OSA_MUTEX_HANDLE_DEFINE(sdio_mutex);
static OSA_TASK_HANDLE_DEFINE(sdhost_core_thread);
static OSA_TASK_DEFINE(sdhost_core_input, SDHOST_CORE_INPUT_PRIO, 1, SDHOST_CORE_STACK_SIZE, 0);

static OSA_TASK_HANDLE_DEFINE(sdhost_rescan_thread);
static OSA_TASK_DEFINE(sdhost_rescan_task, SDHOST_RESCAN_PRIO, 1, SDHOST_RESCAN_STACK_SIZE, 0);

typedef struct _sdhost_ctrl
{
    /** IO port */
    uint32_t ioport;
    /** SDIO multiple port read bitmap */
    uint32_t mp_rd_bitmap;
    /** SDIO multiple port write bitmap */
    uint32_t mp_wr_bitmap;
    /** SDIO end port from txbufcfg */
    uint16_t mp_end_port;
    /** Current available port for read */
    uint8_t curr_rd_port;
    /** Current available port for write */
    uint8_t curr_wr_port;
    /** Array to store values of SDIO multiple port group registers */
    uint8_t *mp_regs;
} sdhost_ctrl_t;

static sdhost_ctrl_t sdhost_ctrl;

static uint32_t txportno;

static uint8_t mp_regs_buffer[MAX_MP_REGS + DMA_ALIGNMENT];
/*
 * Used to authorize the SDIO interrupt handler to accept the incoming
 * packet from the SDIO interface. If this flag is set a semaphore is
 * signalled.
 */
static bool g_txrx_flag;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */
/*! @brief Data written to the card */
SDK_ALIGN(uint8_t sdh_outbuf[SDIO_CMD_OUTBUF_LEN], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
SDK_ALIGN(uint8_t sdh_inbuf[SDIO_CMD_OUTBUF_LEN], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

typedef struct
{
    uint16_t size;
    uint16_t pkttype;
} SDIOHeader;

/** Interrupt status */
static uint8_t g_sdio_ireg;

static OSA_MUTEX_HANDLE_DEFINE(txrx_mutex);

/*******************************************************************************
 * Code
 ******************************************************************************/
int sdio_drv_creg_read(int addr, int fn, uint32_t *resp)
{
    if (KOSA_StatusSuccess != OSA_MutexLock(&sdio_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get sdio_mutex");
        return false;
    }

    if (SDIO_IO_Read_Direct(&g_sdio_card, (sdio_func_num_t)fn, (uint32_t)addr, (uint8_t *)resp) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }

    (void)OSA_MutexUnlock(&sdio_mutex);

    return true;
}

bool sdio_drv_creg_write(int addr, int fn, uint8_t data, uint32_t *resp)
{
    if (KOSA_StatusSuccess != OSA_MutexLock(&sdio_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get sdio_mutex");
        return false;
    }

    if (SDIO_IO_Write_Direct(&g_sdio_card, (sdio_func_num_t)fn, (uint32_t)addr, &data, true) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }

    *resp = data;

    (void)OSA_MutexUnlock(&sdio_mutex);

    return true;
}

int sdio_drv_read(uint32_t addr, uint32_t fn, uint32_t bcnt, uint32_t bsize, uint8_t *buf, uint32_t *resp)
{
    uint32_t flags = 0;
    uint32_t param;

    if (KOSA_StatusSuccess != OSA_MutexLock(&sdio_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get sdio_mutex");
        return false;
    }

    if (bcnt > 1U)
    {
        flags |= SDIO_EXTEND_CMD_BLOCK_MODE_MASK;
        param = bcnt;
    }
    else
    {
        param = bsize;
    }

    if (SDIO_IO_Read_Extended(&g_sdio_card, (sdio_func_num_t)fn, addr, buf, param, flags) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }

    (void)OSA_MutexUnlock(&sdio_mutex);

    return true;
}

extern uint8_t mcu_device_status;
bool sdio_drv_wakeup_card(sdio_func_num_t fn)
{
    uint8_t data = 0x02;

    if(SDIO_IO_Write_Direct(&g_sdio_card, fn, 0x0, &data, true) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }
    data = 0x0;
    if(SDIO_IO_Write_Direct(&g_sdio_card, fn, 0x0, &data, true) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }
    data = 0x2;
    if(SDIO_IO_Write_Direct(&g_sdio_card, fn, 0x0, &data, true) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }
    OSA_TimeDelay(100);
    data = 0x0;
    if(SDIO_IO_Write_Direct(&g_sdio_card, fn, 0x0, &data, true) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }
    return true;
}

bool sdio_drv_write(uint32_t addr, uint32_t fn, uint32_t bcnt, uint32_t bsize, uint8_t *buf, uint32_t *resp)
{
    uint32_t flags = 0;
    uint32_t param;

    if (KOSA_StatusSuccess != OSA_MutexLock(&sdio_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get sdio_mutex");
        return false;
    }

    if (bcnt > 1U)
    {
        flags |= SDIO_EXTEND_CMD_BLOCK_MODE_MASK;
        param = bcnt;
    }
    else
    {
        param = bsize;
    }

    /* MCU device is in sleep mode, wakeup card with CMD52 */
    if(mcu_device_status == 2)
    {
        if(!sdio_drv_wakeup_card((sdio_func_num_t)fn))
            return false;
    }

    if (SDIO_IO_Write_Extended(&g_sdio_card, (sdio_func_num_t)fn, addr, buf, param, flags) != kStatus_Success)
    {
        (void)OSA_MutexUnlock(&sdio_mutex);
        return false;
    }

    (void)OSA_MutexUnlock(&sdio_mutex);

    return true;
}

void sdhost_core_wait_event(osa_event_flags_t flagsToWait)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)sdhost_core_events, flagsToWait, 0, osaWaitForever_c, &Events);
}

void sdhost_core_set_event(osa_event_flags_t flagsToWait)
{
    (void)OSA_EventSet((osa_event_handle_t)sdhost_core_events, flagsToWait);
}

static void SDIO_CardInterruptCallBack(void *userData)
{
    SDMMCHOST_EnableCardInt(g_sdio_card.host, false);

    /* Wake up sdhost core thread. */
    if (g_txrx_flag)
    {
        g_txrx_flag = false;
        (void)sdhost_core_set_event(SDHOST_CORE_START);
    }
}

static uint32_t sdio_card_read_scratch_reg(void)
{
    uint32_t val    = 0;
    uint32_t rd_len = 0;

    (void)sdio_drv_creg_read(0x64, 1, &val);
    rd_len = (val & 0xffU);
    (void)sdio_drv_creg_read(0x65, 1, &val);
    rd_len |= ((val & 0xffU) << 8);
    (void)sdio_drv_creg_read(0x66, 1, &val);
    rd_len |= ((val & 0xffU) << 16);
    (void)sdio_drv_creg_read(0x67, 1, &val);
    rd_len |= ((val & 0xffU) << 24);

    return rd_len;
}

ncp_status_t sdio_ioport_init(void)
{
    /* this sets intmask on card and makes interrupts repeatable */
    uint32_t resp = 0;
    uint8_t data;

    sdhost_ctrl.ioport = MEM_PORT;

    ncp_adap_d("IOPORT : (0x%x)", sdhost_ctrl.ioport);

    /* Enable sdio cmd53 new mode */
    (void)sdio_drv_creg_read(CARD_CONFIG_2_1_REG, 1, &resp);
    data = (uint8_t)((resp & 0xff) | CMD53_NEW_MODE);
    (void)sdio_drv_creg_write(CARD_CONFIG_2_1_REG, 1, data, &resp);
    (void)sdio_drv_creg_read(CARD_CONFIG_2_1_REG, 1, &resp);

    /* configure cmd port  */
    /* enable reading rx length from the register  */
    (void)sdio_drv_creg_read(CMD_CONFIG_0, 1, &resp);
    data = (uint8_t)((resp & 0xff) | CMD_PORT_RD_LEN_EN);
    (void)sdio_drv_creg_write(CMD_CONFIG_0, 1, data, &resp);
    (void)sdio_drv_creg_read(CMD_CONFIG_0, 1, &resp);

    /* enable Dnld/Upld ready auto reset for cmd port
     * after cmd53 is completed */
    (void)sdio_drv_creg_read(CMD_CONFIG_1, 1, &resp);
    data = (uint8_t)((resp & 0xff) | CMD_PORT_AUTO_EN);
    (void)sdio_drv_creg_write(CMD_CONFIG_1, 1, data, &resp);
    (void)sdio_drv_creg_read(CMD_CONFIG_1, 1, &resp);

    /* Set Host interrupt reset to read to clear */
    (void)sdio_drv_creg_read(HOST_INT_RSR_REG, 1, &resp);
    data = (uint8_t)((resp & 0xff) | HOST_INT_RSR_MASK);
    (void)sdio_drv_creg_write(HOST_INT_RSR_REG, 1, data, &resp);

    /* Dnld/Upld ready set to auto reset */
    (void)sdio_drv_creg_read(CARD_MISC_CFG_REG, 1, &resp);
    data = (uint8_t)((resp & 0xff) | AUTO_RE_ENABLE_INT);
    (void)sdio_drv_creg_write(CARD_MISC_CFG_REG, 1, data, &resp);
    // txportno = sdhost_ctrl.ioport;
    return NCP_STATUS_SUCCESS;
}

static bool sdio_card_ready_wait(uint32_t card_poll)
{
    uint16_t dat  = 0U;
    uint32_t i    = 0U;
    uint32_t resp = 0;

    for (i = 0; i < card_poll; i++)
    {
        (void)sdio_drv_creg_read(CARD_FW_STATUS0_REG, 1, &resp);
        dat = (uint16_t)(resp & 0xffU);
        (void)sdio_drv_creg_read(CARD_FW_STATUS1_REG, 1, &resp);
        dat |= (uint16_t)((resp & 0xffU) << 8);
        if (dat == FIRMWARE_READY)
        {
            ncp_adap_d("Firmware Ready");
            return true;
        }
        vTaskDelay((5) / (portTICK_PERIOD_MS));
    }
    return false;
}

/*
 * This function gets interrupt status.
 */
void sdhost_interrupt(void)
{
    /* Read SDIO multiple port group registers */
    uint32_t resp = 0;
    int ret;

    /* Read the registers in DMA aligned buffer */
    ret = sdio_drv_read(REG_PORT | SDIO_BYTE_MODE_MASK, 1, 1, MAX_MP_REGS, sdhost_ctrl.mp_regs, &resp);

    if (!ret)
    {
        return;
    }

    uint8_t sdio_ireg = sdhost_ctrl.mp_regs[HOST_INT_STATUS_REG];

    if (sdio_ireg != 0U)
    {
        /*
         * DN_LD_HOST_INT_STATUS and/or UP_LD_HOST_INT_STATUS
         * Clear the interrupt status register
         */
        g_sdio_ireg |= sdio_ireg;
    }

#if CONFIG_SDIO_IO_DEBUG
    uint32_t rd_bitmap, wr_bitmap;
    rd_bitmap = (uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_L];
    rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_U]) << 8;
    rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_1L]) << 16;
    rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_1U]) << 24;

    ncp_adap_d("INT : rd_bitmap=0x%x", rd_bitmap);

    wr_bitmap = (uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_L];
    wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_U]) << 8;
    wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1L]) << 16;
    wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1U]) << 24;

    ncp_adap_d("INT : wr_bitmap=0x%x", wr_bitmap);

    ncp_adap_d("INT : sdio_ireg = (0x%x)", sdio_ireg);
#endif /* CONFIG_SDIO_IO_DEBUG */
}

void sdio_host_save_recv_data(uint8_t *recv_data, uint32_t packet_len)
{
    uint32_t sdio_transfer_len = 0;
    uint32_t sdio_rx_len       = 0;

    //memcpy((uint8_t *)&mcu_response_buff[0], recv_data, packet_len);
    sdio_rx_len += packet_len;

    if (sdio_rx_len >= NCP_CMD_HEADER_LEN)
    {
        sdio_transfer_len =
            ((recv_data[TLV_CMD_SIZE_HIGH_BYTES] << 8) | recv_data[TLV_CMD_SIZE_LOW_BYTES]) +
            NCP_CHKSUM_LEN;
    }
    else
    {
         ncp_adap_e("[%s] transfer warning. data_len : %d ", __func__, packet_len);
         NCP_SDIO_STATS_INC(err);
    }

    if ((sdio_rx_len >= sdio_transfer_len) && (sdio_transfer_len >= NCP_CMD_HEADER_LEN))
    {
        ncp_adap_d("recv data len: %d ", sdio_transfer_len);
        ncp_tlv_dispatch(recv_data, sdio_transfer_len - NCP_CHKSUM_LEN);
        sdio_rx_len       = 0;
        sdio_transfer_len = 0;
        NCP_SDIO_STATS_INC(rx);
        ncp_adap_d("data recv success ");
    }
}

/*
 * This function keeps on looping till all the packets are read
 */
static void handle_sdio_cmd_read(uint32_t rx_len, uint32_t rx_blocks)
{
    uint8_t ret;
    uint32_t blksize = SDIO_BLOCK_SIZE;
    uint32_t resp;

    /* addr = 0 fn = 1 */
    ret = sdio_drv_read(sdhost_ctrl.ioport | CMD_PORT_SLCT, 1, rx_blocks, blksize, sdh_inbuf, &resp);
    if (!ret)
    {
        ncp_adap_e("sdio_drv_read failed (%d)", ret);
        return;
    }

#if CONFIG_NCP_HOST_IO_DUMP
    SDIOHeader *sdioheader = (SDIOHeader *)(void *)sdh_inbuf;

    if (sdioheader->pkttype == SDIO_TYPE_CMD)
    {
        ncp_adap_d("handle_sdio_cmd_read: DUMP:");
        ncp_dump_hex((uint8_t *)sdh_inbuf, 1 * rx_len);
    }
#endif

    sdio_host_save_recv_data((uint8_t *)sdh_inbuf + SDIO_HEADER_LEN, rx_len - SDIO_HEADER_LEN);
}

/* returns port number from rd_bitmap. if ctrl port, then it clears
 * the bit and does nothing else
 * if data port then increments curr_port value also */
static ncp_status_t get_rd_port(uint32_t *pport)
{
    uint32_t rd_bitmap = sdhost_ctrl.mp_rd_bitmap;

    if (!(rd_bitmap & DATA_PORT_MASK))
        return NCP_STATUS_ERROR;

    /* Data */
    if ((sdhost_ctrl.mp_rd_bitmap & (1 << sdhost_ctrl.curr_rd_port)) != 0U)
    {
        sdhost_ctrl.mp_rd_bitmap &= (uint32_t)(~(1 << sdhost_ctrl.curr_rd_port));

        *pport = sdhost_ctrl.curr_rd_port;

        if (++sdhost_ctrl.curr_rd_port == sdhost_ctrl.mp_end_port)
            sdhost_ctrl.curr_rd_port = 0;
    }
    else
    {
        ncp_adap_e("wlan_get_rd_port : Returning FAILURE");
        return NCP_STATUS_ERROR;
    }

    //ncp_adap_d("port=%d mp_rd_bitmap=0x%x -> 0x%x", *pport, rd_bitmap, sdhost_ctrl.mp_rd_bitmap);

    return NCP_STATUS_SUCCESS;
}

/*
 * This function keeps on looping till all the packets are read
 */
static void handle_sdio_packet_read(void)
{
    uint8_t ret;

    sdhost_ctrl.mp_rd_bitmap = (uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_L];
    sdhost_ctrl.mp_rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_U]) << 8;
    sdhost_ctrl.mp_rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_1L]) << 16;
    sdhost_ctrl.mp_rd_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[RD_BITMAP_1U]) << 24;

    uint32_t port = 0;
    // Just use one port firstly, would extended to multiple ports if needed
    sdhost_ctrl.curr_rd_port = 0;
    while (true)
    {
        ret = get_rd_port(&port);
        /* nothing to read */
        if (ret != NCP_STATUS_SUCCESS)
            break;
        uint32_t rx_len, rx_blocks;

        uint32_t len_reg_l = RD_LEN_P0_L + (port << 1);
        uint32_t len_reg_u = RD_LEN_P0_U + (port << 1);

        rx_len  = ((uint16_t)sdhost_ctrl.mp_regs[len_reg_u]) << 8;
        rx_len |= (uint16_t)sdhost_ctrl.mp_regs[len_reg_l];
        //ncp_adap_d("handle_sdio_packet_read: rx_len (%d)", rx_len);

        rx_blocks = (rx_len + SDIO_BLOCK_SIZE - 1) / SDIO_BLOCK_SIZE;
        rx_len    = (uint16_t)(rx_blocks * SDIO_BLOCK_SIZE);

        port = sdhost_ctrl.ioport + port;

        uint32_t resp;
        ret = sdio_drv_read(port, 1, rx_blocks, rx_len, sdh_inbuf, &resp);
        if (!ret)
        {
            ncp_adap_e("sdio_drv_read failed (%d)", ret);
            break;
        }

#if CONFIG_NCP_HOST_IO_DUMP
        SDIOHeader *sdioheader = (SDIOHeader *)(void *)sdh_inbuf;

        if (sdioheader->pkttype == SDIO_TYPE_DATA)
        {
            ncp_adap_d("handle_sdio_packet_read: DUMP:");
            ncp_dump_hex((uint8_t *)sdh_inbuf, 1 * rx_len);
        }
#endif
        sdio_host_save_recv_data((uint8_t *)sdh_inbuf + SDIO_HEADER_LEN, rx_len - SDIO_HEADER_LEN);
    }
}

/*
 * This is supposed to be called in thread context.
 */
ncp_status_t sdhost_process_int_status(void)
{
    ncp_status_t ret     = NCP_STATUS_SUCCESS;
    uint8_t cmd_rd_len_0 = CMD_RD_LEN_0;
    uint8_t cmd_rd_len_1 = CMD_RD_LEN_1;
    uint32_t rx_len;
    uint32_t rx_blocks;

    /* Get the interrupt status */
    sdhost_interrupt();

    uint8_t sdio_ireg = g_sdio_ireg;
    g_sdio_ireg       = 0;

    if (!sdio_ireg)
    {
        goto done;
    }

    /* check the command port */
    /*if ((sdio_ireg & DN_LD_CMD_PORT_HOST_INT_STATUS) != 0U)
    {
        ncp_adap_d("cmd sent");
    }*/

    if ((sdio_ireg & UP_LD_CMD_PORT_HOST_INT_STATUS) != 0U)
    {
        /* read the len of control packet */
        rx_len = ((uint32_t)sdhost_ctrl.mp_regs[cmd_rd_len_1]) << 8;
        rx_len |= (uint32_t)sdhost_ctrl.mp_regs[cmd_rd_len_0];
        //ncp_adap_d("RX: cmd port rx_len=%u", rx_len);

        rx_blocks = (rx_len + SDIO_BLOCK_SIZE - 1U) / SDIO_BLOCK_SIZE;

        //ncp_adap_d("CMD: cmd port rx_len=%u rx_blocks=%u", rx_len, rx_blocks);
        //rx_len = (uint32_t)(rx_blocks * SDIO_BLOCK_SIZE);

        handle_sdio_cmd_read(rx_len, rx_blocks);
    }

    sdhost_ctrl.mp_wr_bitmap = (uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_L];
    sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_U]) << 8;
    sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1L]) << 16;
    sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1U]) << 24;

    /*
     * DN_LD_HOST_INT_STATUS interrupt happens when the txmit sdio
     * ports are freed This is usually when we write to port most
     * significant port.
     */
    if ((sdio_ireg & DN_LD_HOST_INT_STATUS) != 0U)
    {
        sdhost_ctrl.mp_wr_bitmap = (uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_L];
        sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_U]) << 8;
        sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1L]) << 16;
        sdhost_ctrl.mp_wr_bitmap |= ((uint32_t)sdhost_ctrl.mp_regs[WR_BITMAP_1U]) << 24;
        //ncp_adap_d("data sent");
    }

    if ((sdio_ireg & UP_LD_HOST_INT_STATUS) != 0U)
    {
        /* This means there is data to be read */
        handle_sdio_packet_read();
    }

    ret = NCP_STATUS_SUCCESS;

done:
    return ret;
}

static ncp_status_t sdio_post_init(void)
{
    ncp_status_t status = NCP_STATUS_SUCCESS;
    uint32_t resp;

    (void)sdio_drv_creg_write(HOST_INT_MASK_REG, 1, HIM_ENABLE, &resp);

    return status;
}

/**
 *  @brief This function reads the CARD_TO_HOST_EVENT_REG and
 *  checks if input bits are set
 *  @param bits		bits to check status against
 *  @return		true if bits are set
 *                      SDIO_POLLING_STATUS_TIMEOUT if bits
 *                      aren't set
 */
bool sdio_card_status(uint8_t bits)
{
    uint32_t resp = 0;
    uint32_t tries;

    for (tries = 0; tries < MAX_POLL_TRIES; tries++)
    {
        if (!(sdio_drv_creg_read(CARD_TO_HOST_EVENT_REG, 1, &resp)))
        {
            return false;
        }
        if ((resp & bits) == bits)
        {
            return true;
        }
        vTaskDelay((1) / (portTICK_PERIOD_MS));
    }
    return false;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface.
 */
void sdhost_core_input(void *argv)
{
    for (;;)
    {
        OSA_SR_ALLOC();
        OSA_ENTER_CRITICAL();
        /* Allow interrupt handler to deliver us a packet */
        g_txrx_flag = true;
        if (g_sdio_card.isHostReady)
        {
            SDMMCHOST_EnableCardInt(g_sdio_card.host, true);
        }

        OSA_EXIT_CRITICAL();

        (void)sdhost_core_wait_event(SDHOST_CORE_START);

        /* Protect the SDIO from other parallel activities */
        if (KOSA_StatusSuccess != OSA_MutexLock(&txrx_mutex, osaWaitForever_c))
        {
            ncp_adap_e("Failed to take txrx_mutex semaphore.");
            break;
        }

        (void)sdhost_process_int_status();

        (void)OSA_MutexUnlock(&txrx_mutex);
    } /* for ;; */
}

static void BOARD_SD_Enable(bool enable)
{
    if (enable)
    {
        /* Enable module */
        /* Enable power supply for SD */
        GPIO_PinWrite(BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE, BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN, 1);
        //OSA_TimeDelay(100);
    }
    else
    {
        /* Disable module */
        /* Disable power supply for SD */
        GPIO_PinWrite(BOARD_SDMMC_SD_POWER_RESET_GPIO_BASE, BOARD_SDMMC_SD_POWER_RESET_GPIO_PIN, 0);
        //OSA_TimeDelay(100);
    }
}

static void sdio_controller_init(void)
{
    (void)memset(&g_sdio_card, 0, sizeof(sdio_card_t));

    BOARD_SDIO_Config(&g_sdio_card, NULL, BOARD_SDMMC_SDIO_HOST_IRQ_PRIORITY, SDIO_CardInterruptCallBack);
    g_sdio_card.usrParam.pwr = NULL;

    BOARD_SD_Enable(false);

    g_sdio_card.currentTiming = kSD_TimingSDR104Mode;
}

static ncp_status_t sdio_card_init(void)
{
    int ret;
    (void)ret;

    if (SDIO_HostInit(&g_sdio_card) != KOSA_StatusSuccess)
    {
        ncp_adap_e("Failed to init sdio host");
        return NCP_STATUS_ERROR;
    }

    /* Switch to 1.8V */
    if ((g_sdio_card.usrParam.ioVoltage != NULL) && (g_sdio_card.usrParam.ioVoltage->type == kSD_IOVoltageCtrlByGpio))
    {
        if (g_sdio_card.usrParam.ioVoltage->func != NULL)
        {
            g_sdio_card.usrParam.ioVoltage->func(kSDMMC_OperationVoltage180V);
        }
    }
    else if ((g_sdio_card.usrParam.ioVoltage != NULL) &&
             (g_sdio_card.usrParam.ioVoltage->type == kSD_IOVoltageCtrlByHost))
    {
        SDMMCHOST_SwitchToVoltage(g_sdio_card.host, (uint32_t)kSDMMC_OperationVoltage180V);
    }
    else
    {
        /* Do Nothing */
    }
    g_sdio_card.operationVoltage = kSDMMC_OperationVoltage180V;

    BOARD_SD_Enable(true);

    ret = SDIO_CardInit(&g_sdio_card);
    if (ret != kStatus_Success)
    {
        return NCP_STATUS_ERROR;
    }

    uint32_t resp;

    (void)sdio_drv_creg_read(0x0, 0, &resp);

    ncp_adap_d("Card Version - (0x%x)", resp & 0xff);

    /* Mask interrupts in card */
    (void)sdio_drv_creg_write(0x4, 0, 0x3, &resp);
    /* Enable IO in card */
    (void)sdio_drv_creg_write(0x2, 0, 0x2, &resp);

    (void)SDIO_SetBlockSize(&g_sdio_card, (sdio_func_num_t)0, 256);
    (void)SDIO_SetBlockSize(&g_sdio_card, (sdio_func_num_t)1, 256);
    (void)SDIO_SetBlockSize(&g_sdio_card, (sdio_func_num_t)2, 256);

    return NCP_STATUS_SUCCESS;
}

static ncp_status_t sdio_drvInit(void)
{
    sdio_controller_init();

    if (sdio_card_init() != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("Card initialization failed");
        return NCP_STATUS_ERROR;
    }
    else
    {
        ncp_adap_d("Card initialization successful");
    }

    return NCP_STATUS_SUCCESS;
}

static ncp_status_t sdio_hostInit(void)
{
    int ret;
    (void)ret;

    ret = sdio_drvInit();
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("Failed to int sdio driver");
        return NCP_STATUS_ERROR;
    }

    uint32_t resp;
    bool sdio_card_stat;
    ret = sdio_drv_creg_read(CARD_TO_HOST_EVENT_REG, 1, &resp);
    if (ret && (resp & (DN_LD_CARD_RDY)) == 0U)
    {
        sdio_card_stat = sdio_card_status(UP_LD_CARD_RDY);
        if (sdio_card_stat != false)
        {
            uint32_t rd_len;
            rd_len = sdio_card_read_scratch_reg();
            if (rd_len > 0U)
            {
                (void)sdio_drv_creg_write(FN1_BLOCK_SIZE_0, 0, 0x8, &resp);
                (void)sdio_drv_creg_write(FN1_BLOCK_SIZE_1, 0, 0x0, &resp);

                uint8_t buf[256];
                ret = sdio_drv_read(0x10000, 1, rd_len, 8, buf, &resp);
                if (!ret)
                {
                    ncp_adap_e(
                        "SDIO read failed, "
                        "resp:%x",
                        resp);
                    return NCP_STATUS_ERROR;
                }
            }
        }
    }
    else if (!ret)
    {
        ncp_adap_e("failed to read EVENT_REG");
        return NCP_STATUS_ERROR;
    }

    return NCP_STATUS_SUCCESS;
}

static ncp_status_t ncp_sdhost_CardInit(void)
{
    int ret;
    (void)ret;

    // sdhost_ctrl.mp_wr_bitmap = 0;
    sdhost_ctrl.mp_wr_bitmap = 0xffffffff;
    sdhost_ctrl.mp_rd_bitmap = 0;
    sdhost_ctrl.curr_rd_port = 0;
    sdhost_ctrl.curr_wr_port = 0;
    sdhost_ctrl.mp_regs      = (uint8_t *)ALIGN_ADDR(mp_regs_buffer, DMA_ALIGNMENT);
    sdhost_ctrl.mp_end_port  = MAX_PORT;

    txportno = 0;

    ret = sdio_hostInit();
    if (ret != NCP_STATUS_SUCCESS)
    {
        ncp_adap_e("Failed to init sdio host driver");
        return NCP_STATUS_ERROR;
    }

    ret = sdio_ioport_init();
    if (ret == NCP_STATUS_SUCCESS)
    {
        if (sdio_card_ready_wait(1000) != true)
        {
            ncp_adap_e("SDIO slave not ready");
            return NCP_STATUS_ERROR;
        }
        else
        {
            ncp_adap_d("SDIO slave ready");
        }
    }

    ret = sdio_post_init();

    return NCP_STATUS_SUCCESS;
}

static ncp_status_t ncp_sdhost_CardDeinit(void)
{
    //SDIO_Deinit(&g_sdio_card);

    return NCP_STATUS_SUCCESS;
}

void sdhost_rescan_wait_event(osa_event_flags_t flagsToWait)
{
    uint32_t Events;
    (void)OSA_EventWait((osa_event_handle_t)sdhost_rescan_events, flagsToWait, 0, osaWaitForever_c, &Events);
}

void sdhost_rescan_set_event(osa_event_flags_t flagsToWait)
{
    (void)OSA_EventSet((osa_event_handle_t)sdhost_rescan_events, flagsToWait);
}

void sdhost_rescan_task(void *argv)
{
    for (;;)
    {
        (void)sdhost_rescan_wait_event(SDHOST_RESCAN_START);

        if (NCP_STATUS_SUCCESS != ncp_sdhost_CardDeinit())
        {
            ncp_adap_e("Failed to deinit sdio host");
        }

        if (NCP_STATUS_SUCCESS != ncp_sdhost_CardInit())
        {
            ncp_adap_e("Failed to re-enumerate sdio card");
        }
    } /* for ;; */
}

static void sdio_notify_int_callback(void *param)
{
    (void)sdhost_rescan_set_event(SDHOST_RESCAN_START);
}

void ncp_host_sdio_notify_gpio_init(void)
{
    IOMUXC_SetPinMux(IOMUXC_GPIO_AD_B1_05_GPIO1_IO21, 0U);

    hal_gpio_pin_config_t notify_config = {kHAL_GpioDirectionIn, 0, NCP_HOST_SDIO_GPIO_NUM, NCP_HOST_SDIO_GPIO_PIN};
    HAL_GpioInit(ncp_host_sdio_notify_handle, &notify_config);
    HAL_GpioSetTriggerMode(ncp_host_sdio_notify_handle, kHAL_GpioInterruptRisingEdge);
    HAL_GpioInstallCallback(ncp_host_sdio_notify_handle, sdio_notify_int_callback, NULL);

    NVIC_SetPriority(NCP_HOST_SDIO_GPIO_IRQ, NCP_HOST_SDIO_GPIO_IRQ_PRIO);
    EnableIRQ(NCP_HOST_SDIO_GPIO_IRQ);

    /* Enable GPIO pin interrupt */
    GPIO_PortEnableInterrupts(NCP_HOST_SDIO_GPIO, 1U << NCP_HOST_SDIO_GPIO_PIN);
}

int ncp_sdhost_init(void *argv)
{
    if (KOSA_StatusSuccess != OSA_MutexCreate(&txrx_mutex))
    {
        ncp_adap_e("Failed to create txrx_mutex");
        return NCP_STATUS_ERROR;
    }

    if (KOSA_StatusSuccess != OSA_MutexCreate(&sdio_mutex))
    {
        ncp_adap_e("Failed to create sdio_mutex");
        return NCP_STATUS_ERROR;
    }

    (void)OSA_EventCreate(sdhost_core_events, 1U);
    (void)OSA_TaskCreate((osa_task_handle_t)sdhost_core_thread, OSA_TASK(sdhost_core_input), (osa_task_param_t)NULL);
    (void)OSA_EventCreate(sdhost_rescan_events, 1U);
    (void)OSA_TaskCreate((osa_task_handle_t)sdhost_rescan_thread, OSA_TASK(sdhost_rescan_task), (osa_task_param_t)NULL);

    if (NCP_STATUS_SUCCESS != ncp_sdhost_CardInit())
    {
        ncp_adap_e("Failed to enumerate sdio card");
        return NCP_STATUS_ERROR;
    }

    ncp_host_sdio_notify_gpio_init();

    return NCP_STATUS_SUCCESS;
}

int ncp_sdhost_deinit(void *argv)
{
    if (NCP_STATUS_SUCCESS != ncp_sdhost_CardDeinit())
    {
        ncp_adap_e("Failed to deinit sdio host");
        return NCP_STATUS_ERROR;
    }

    (void)OSA_TaskDestroy(sdhost_rescan_thread);
    (void)OSA_EventDestroy(sdhost_rescan_events);

    if (KOSA_StatusSuccess != OSA_MutexDestroy(&sdio_mutex))
    {
        ncp_adap_e("Failed to delete sdio mutex");
        return NCP_STATUS_ERROR;
    }

    if (KOSA_StatusSuccess != OSA_MutexDestroy(&txrx_mutex))
    {
        ncp_adap_e("Failed to delete txrx mutex");
        return NCP_STATUS_ERROR;
    }

    (void)OSA_TaskDestroy(sdhost_core_thread);
    (void)OSA_EventDestroy(sdhost_core_events);

    return NCP_STATUS_SUCCESS;
}

void calculate_sdio_write_params(uint32_t txlen, uint32_t *tx_blocks, uint32_t *buflen)
{
    *tx_blocks = (txlen + SDIO_BLOCK_SIZE - 1) / SDIO_BLOCK_SIZE;

    *buflen = SDIO_BLOCK_SIZE;
}

#if 0
static int get_free_port(void)
{
    /* Check if the port is available */
    if (!((1 << txportno) & sdhost_ctrl.mp_wr_bitmap))
    {
        ncp_adap_e("txportno out of sync txportno = (%d) mp_wr_bitmap = (0x%x)", txportno,
                     sdhost_ctrl.mp_wr_bitmap);

        return STATUS_FAILURE;
    }
    else
    {
        /* Mark the port number we will use */
        sdhost_ctrl.mp_wr_bitmap &= ~(1 << txportno);
    }
    return NCP_STATUS_SUCCESS;
}
#endif

ncp_status_t ncp_sdhost_send_data(uint8_t *buf, uint32_t length)
{
    int ret;
    (void)ret;
    uint32_t tx_blocks = 0, buflen = 0;
    SDIOHeader *sdioheader = (SDIOHeader *)(void *)sdh_outbuf;
    uint32_t resp;

    if ((buf == MNULL) || !length)
    {
        return NCP_STATUS_ERROR;
    }

    if (length > SDIO_DATA_OUTBUF_LEN)
    {
        ncp_adap_e("Insufficient buffer");
        return NCP_STATUS_ERROR;
    }

    if (KOSA_StatusSuccess != OSA_MutexLock(&txrx_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get txrx_mutex");
        return NCP_STATUS_ERROR;
    }
    (void)memset(sdh_outbuf, 0, SDIO_CMD_OUTBUF_LEN);
    sdioheader->pkttype = SDIO_TYPE_DATA;
    sdioheader->size    = length + SDIO_HEADER_LEN;
    calculate_sdio_write_params(sdioheader->size, &tx_blocks, &buflen);
    (void)memcpy((void *)(sdh_outbuf + SDIO_HEADER_LEN), (const void *)(buf + SDIO_HEADER_LEN), length);

    /*ret = get_free_port();
    if (ret == NCP_STATUS_ERROR)
    {
        ncp_adap_e("Get free port failed");
        return NCP_STATUS_ERROR;
    }
    else
    {
         ncp_adap_d("Get free port %d", txportno);
    }*/
    (void)sdio_drv_write(sdhost_ctrl.ioport + txportno, 1, tx_blocks, buflen, (uint8_t *)sdh_outbuf, &resp);
    /*txportno++;
    if (txportno == sdhost_ctrl.mp_end_port)
    {
        txportno = 0;
    }*/
    (void)OSA_MutexUnlock(&txrx_mutex);

    return NCP_STATUS_SUCCESS;
}

ncp_status_t ncp_sdhost_send_cmd(uint8_t *buf, uint32_t length)
{
    int ret;
    (void)ret;
    SDIOHeader *sdioheader = (SDIOHeader *)(void *)sdh_outbuf;
    uint32_t resp;

    if ((buf == MNULL) || !length)
    {
        return NCP_STATUS_ERROR;
    }

    if (length > SDIO_CMD_OUTBUF_LEN)
    {
        ncp_adap_e("Insufficient buffer");
        return NCP_STATUS_ERROR;
    }

    if (KOSA_StatusSuccess != OSA_MutexLock(&txrx_mutex, osaWaitForever_c))
    {
        ncp_adap_e("failed to get txrx_mutex");
        return NCP_STATUS_ERROR;
    }
    (void)memset(sdh_outbuf, 0, SDIO_CMD_OUTBUF_LEN);
    sdioheader->pkttype = SDIO_TYPE_CMD;
    sdioheader->size    = length + SDIO_HEADER_LEN;
    uint32_t tx_blocks = 0, buflen = 0;
    calculate_sdio_write_params(sdioheader->size, &tx_blocks, &buflen);

    (void)memcpy((void *)(sdh_outbuf + SDIO_HEADER_LEN), (const void *)(buf + SDIO_HEADER_LEN), length);
    (void)sdio_drv_write(sdhost_ctrl.ioport | CMD_PORT_SLCT, 1, tx_blocks, buflen, (uint8_t *)sdh_outbuf, &resp);
    (void)OSA_MutexUnlock(&txrx_mutex);

    return NCP_STATUS_SUCCESS;
}

int ncp_sdhost_send(uint8_t *tlv_buf, size_t tlv_sz, tlv_send_callback_t cb)
{
    NCP_COMMAND *res = NULL;
    status_t ret = kStatus_Success;
    uint32_t msg_type;

    ARG_UNUSED(cb);

    NCP_ASSERT(NULL != tlv_buf);
    NCP_ASSERT(0 != tlv_sz);

    res = (NCP_COMMAND *)(tlv_buf + SDIO_HEADER_LEN);

    msg_type = SDIO_GET_MSG_TYPE(res->cmd);
    switch (msg_type)
    {
        case NCP_MSG_TYPE_CMD:
            if ((res->cmd == NCP_CMD_WLAN_SOCKET_SEND)
              || (res->cmd == NCP_CMD_WLAN_SOCKET_SENDTO))
            {
                ret = ncp_sdhost_send_data((uint8_t *)tlv_buf, tlv_sz);
            }
            else
            {
                ret = ncp_sdhost_send_cmd((uint8_t *)tlv_buf, tlv_sz);
            }
            break;
        default:
            ncp_adap_e("%s: invalid msg_type %d", __FUNCTION__, msg_type);
            ret = kStatus_Fail;
            break;
    }

    if (ret != kStatus_Success)
    {
        ncp_adap_e("%s: fail 0x%x", __FUNCTION__, ret);
        NCP_SDIO_STATS_INC(drop);
        return NCP_STATUS_ERROR;
    }

    NCP_SDIO_STATS_INC(tx);

    return NCP_STATUS_SUCCESS;
}

static int ncp_sdhost_pm_enter(int32_t pm_state)
{
    /* TODO: NCP sdhost pm */
    return NCP_STATUS_SUCCESS;
}

static int ncp_sdhost_pm_exit(int32_t pm_state)
{
    /* TODO: NCP sdhost pm */
    return NCP_STATUS_SUCCESS;
}

static ncp_intf_pm_ops_t ncp_sdhost_pm_ops =
{
    .enter = ncp_sdhost_pm_enter,
    .exit  = ncp_sdhost_pm_exit,
};

ncp_intf_ops_t ncp_sdio_ops =
{
    .init   = ncp_sdhost_init,
    .deinit = ncp_sdhost_deinit,
    .send   = ncp_sdhost_send,
    .recv   = NULL,
    .pm_ops = &ncp_sdhost_pm_ops,
};
#endif /* CONFIG_NCP_SDIO */
