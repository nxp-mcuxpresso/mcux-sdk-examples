/*
 * qspi_flash.c
 *
 *  Created on: Dec 20, 2018
 *      Author: nxa27412
 */


/*
 * Copyright 2018-2019 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_device_registers.h"
#include "fsl_spifi.h"
#include "fsl_iocon.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SPIFI_DMA_CHANNEL    12
#define MHz                  (1000000UL)

#define SPI_BAUDRATE         (32*MHz)
// #define SPI_BAUDRATE         (16*MHz)

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#define QSPIFLASH_DEV_IS_MX25R8035F
//#define QSPIFLASH_DEV_IS_S25FL116


#if defined (QSPIFLASH_DEV_IS_MX25R8035F)

/* Macronix MX25R8035F QSPI Flash Part */
#define MX25R8035F_DEVICE_ID         0x1428C2             /* 0xC2 is Manufacturer Id for Macronix, 0x28 is the type, 0x14 is the density  */
#define XT25F08B_DEVICE_ID           0x14400B

#define MX25R8035F_PAGE_SIZE        256                  /* 256B sectors */
#define MX25R8035F_SECTOR_SIZE      4096                 /* 4KB sectors */
#define MX25R8035F_BLOCK_SIZE       QSPIFLASH_SECTOR_SIZE * 8     /* 32KB blocks */
#ifndef SPIFI_OPTIM_SIZE
#define QSPIFLASH_BLOCK2_SIZE        QSPIFLASH_BLOCK_SIZE * 2      /* 64KB blocks */
#endif
#define MX25R8035F_CHIP_SIZE        QSPIFLASH_SECTOR_SIZE * 256  /* 1MB capacity */

#define QSPIFLASH_PAGE_SIZE        MX25R8035F_PAGE_SIZE
#define QSPIFLASH_SECTOR_SIZE      MX25R8035F_SECTOR_SIZE
#define QSPIFLASH_BLOCK_SIZE       MX25R8035F_BLOCK_SIZE
#define QSPIFLASH_CHIP_SIZE        MX25R8035F_CHIP_SIZE
#define QSPIFLASH_PAGE_MASK       (QSPIFLASH_PAGE_SIZE - 1)

#define MX25_SR_WIP_POS 0     /* Write In Progress */
#define MX25_SR_WEL_POS 1     /* Write Enable Latch */
#define MX25_SR_BP_POS 2      /* Level of Protected block  */
#define MX25_SR_BP_WIDTH 4
#define MX25_SR_BP_MASK       (((1<<MX25_SR_BP_WIDTH)-1) << MX25_SR_BP_POS)
#define MX25_SR_QE_POS 6      /* Non Volatile  */


#define MX25_CR1_TB_POS 3     /* Top-Bottom selected */
#define MX25_CR1_DC_POS 6     /* Dummy Cycle */
#define MX25_CR2_LH_POS 1      /* LowPower / HighPerformance  */

#define MX25R8035_CFG_STATUS_QUAD_MODE   BIT(MX25_SR_QE_POS)
#define MX25R8035_CFG_REG1_DUMMY_CYCLE   (BIT(MX25_CR1_DC_POS) << 8)
#define MX25R8035_CFG_REG2_HI_PERF_MODE  (BIT(MX25_CR2_LH_POS) << 16)

#elif defined (QSPIFLASH_DEV_IS_S25FL116)
/* Spansion S25FL116 QSPI Flash Part */
#define S25FL116_DEVICE_ID        0x154001             /* 0x01 is Manufacturer Id for Spansion, 0x40 is the type, 0x15 is the density  */
#define S25FL116_PAGE_SIZE        256                  /* 256B sectors */
#define S25FL116_SECTOR_SIZE      4096                 /* 4KB sectors */
#define S25FL116_BLOCK_SIZE       S25FL116_SECTOR_SIZE * 16     /* 64KB blocks */
#define S25FL116_CHIP_SIZE        S25FL116_SECTOR_SIZE * 2000   /* 8MB capacity */

#define QSPIFLASH_PAGE_SIZE        S25FL116_PAGE_SIZE
#define QSPIFLASH_SECTOR_SIZE      S25FL116_SECTOR_SIZE
#define QSPIFLASH_BLOCK_SIZE       S25FL116_BLOCK_SIZE
#define QSPIFLASH_CHIP_SIZE        S25FL116_CHIP_SIZE

/* Spansion SF25FL Status Registers */
#define S25FL116_SR1_BUSY_POS       0    /* Busy */
#define S25FL116_SR1_WEL_POS        1    /* Write Enable Latch */
#define S25FL116_SR1_BP_POS         2    /* Level of Protected block  */
#define S25FL116_SR1_BP_WIDTH       3    /* Write Enable Latch */
#define S25FL116_SR1_BP_MASK        (((1<<S25FL116_SR1_BP_WIDTH)-1) << S25FL116_SR1_BP_POS)
#define S25FL116_SR1_TB_POS         5    /* Top-Bottom Protect  */
#define S25FL116_SR1_SEC_POS        6    /* Sector-Block Protect */
#define S25FL116_SR1_SRP0_POS       7    /* Sector-Block Protect */

#define S25FL116_SR2_SRP1_POS       0    /* Busy */
#define S25FL116_SR2_QE_POS         1    /* Write Enable Latch */
#define S25FL116_SR2_LB_POS         2    /* Level of Protected block  */
#define S25FL116_SR2_LB_WIDTH       4    /* Write Enable Latch */
#define S25FL116_SR2_LB_MASK        (((1<<S25FL116_SR2_LB_WIDTH)-1) << S25FL116_SR2_LB_POS)
#define S25FL116_SR2_CMP_POS        6    /* Complement Protect */
#define S25FL116_SR2_SUS_POS        7    /* Suspend Status */

#define S25FL116_SR3_LC_POS         0    /* Latency Control */
#define S25FL116_SR3_LC_WIDTH       4
#define S25FL116_SR3_LC_MASK        (((1<<S25FL116_SR3_LC_WIDTH)-1) << S25FL116_SR3_LC_POS)
#define S25FL116_SR3_WRAP_EN_POS    4    /* Burst Wrap enable */
#define S25FL116_SR3_WRAP_LEN_POS   5    /* Burst Wrap length  */
#define S25FL116_SR3_WRAP_LEN_WIDTH 2
#define S25FL116_SR3_WRAP_LEN_MASK  (((1<<S25FL116_SR3_WRAP_LEN_WIDTH)-1) << S25FL116_SR3_WRAP_LEN_POS)

/* Spansion SF25FL Status Registers */
#define S25FL116_CR1_TB_POS         3     /* Top-Bottom selected */
#define S25FL116_CR1_DC_POS         6     /* Dummy Cycle */
#define S25FL116_CR2_LH_POS         1      /* LowPower / HighPerformance  */
#else
#endif
#define QSPI_CS_PIN 16
#define QSPI_WP_PIN 20

#ifndef SPIFI_DUAL_MODE_SUPPORT
#define SPIFI_DUAL_MODE_SUPPORT 0
#endif

#ifdef SPIFI_OPTIM_SIZE
#define IS_SPIFI_DUAL_MODE()         (SPIFI_DUAL_MODE_SUPPORT)
#else
#define IS_SPIFI_DUAL_MODE()         (CHIP_USING_SPIFI_DUAL_MODE() || SPIFI_DUAL_MODE_SUPPORT)
#endif
#define IS_ALIGNED(x, aligment)     (((uint32_t)(x) & (aligment-1)) == 0)

#define IOCON_GPIO_MODE_FUNC        (0U)
#define IOCON_QSPI_MODE_FUNC        (7U)

#define SPIFI_FAST_IO_MODE                                              \
    IOCON_PIO_FUNC(IOCON_QSPI_MODE_FUNC) |                              \
    /* No addition pin function */                                      \
    /* Fast mode, slew rate control is enabled */                       \
    IOCON_PIO_SLEW0(1) | IOCON_PIO_SLEW1(1) |                           \
    /* Input function is not inverted */                                \
    IOCON_PIO_INVERT(0) |                                               \
    /* Enables digital function */                                      \
    IOCON_PIO_DIGIMODE(1) |                                             \
    /* Input filter disabled */                                         \
    IOCON_PIO_FILTEROFF(0) |                                            \
    /* Open drain is disabled */                                        \
    IOCON_PIO_OD(0) |                                                   \
    /* SSEL is disabled */                                              \
    IOCON_PIO_SSEL(0)

#define GPIO_PULLUP_MODE                                                \
    /* Pin is configured as GPIO */                                     \
    IOCON_PIO_FUNC(IOCON_GPIO_MODE_FUNC) |                              \
    /* Selects pull-down function */                                    \
    IOCON_PIO_MODE(0) |                                                 \
    /* Standard mode, output slew rate control is disabled */           \
    IOCON_PIO_SLEW0(0) | IOCON_PIO_SLEW1(0) |                           \
    /* Input function is not inverted */                                \
    IOCON_PIO_INVERT(0) |                                               \
    /* Enables digital function */                                      \
    IOCON_PIO_DIGIMODE(1) |                                             \
    /* Input filter disabled */                                         \
    IOCON_PIO_FILTEROFF(0) |                                            \
    /* Open drain is disabled */                                        \
    IOCON_PIO_OD(0) |                                                   \
    /* SSEL is disabled */                                              \
    IOCON_PIO_SSEL(0)

#define SPIFI_FAST_IO_MODE_INACT   (SPIFI_FAST_IO_MODE | IOCON_PIO_MODE(2))
#define SPIFI_FAST_IO_MODE_PULLUP  (SPIFI_FAST_IO_MODE | IOCON_PIO_MODE(0))

/*******************************************************************************
 * Types
 ******************************************************************************/
typedef enum _command_t
{
      RDID,      /* Write Status Register */
      RDSR,      /* Read Status Register */

//    RDCR,      /* Read Configuration Register */
      WREN,      /* Write Enable */
//    WRDI,      /* Write Disable */
      WRSR,      /* Write Status Register */
//    PP4,       /* Quad Page Program */
#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
      PP,        /* Page Program */
      DREAD,     /* 1I-2O read */
#else
      QPP,       /* Quad Input Page Program */
      QREAD,     /* 1I-4O read */
#endif
#else
      QPP,       /* Quad Input Page Program */
      PP,        /* Page Program */
      QREAD,     /* 1I-4O read */
      DREAD,     /* 1I-2O read */
#endif /* SPIFI_OPTIM_SIZE */
      SE,        /* Sector Erase */
//    READ,      /* Read */
//    READ2,     /* 2I-2O read */
//    READ4,     /* 4I-4O read */
#ifndef SPIFI_OPTIM_SIZE
      BE64K,     /* Block Erase */
      CE,        /* Chip Erase */
#endif
      BE32K,     /* Block Erase */
      DP,        /* Deep Power Down */
      MAX_CMD
} command_t;


typedef struct QSpiFlashConfig_tag {
    uint32_t device_id;
    uint16_t page_size;
    uint16_t sector_size;
    uint32_t block_size;
    uint32_t capacity;
} QSpiFlashConfig_t;


/*******************************************************************************
 * Data
 ******************************************************************************/
#if 0

const QSpiFlashConfig_t Macronix_MX25R8035F_device_def = {
        .device_id   = MX25R8035F_DEVICE_ID,
        .page_size   = MX25R8035F_PAGE_SIZE,
        .sector_size = MX25R8035F_SECTOR_SIZE,
        .block_size  = MX25R8035F_BLOCK_SIZE,
        .capacity    = MX25R8035F_CHIP_SIZE
};
const QSpiFlashConfig_t Spansion_S25FL116_device_def = {
        .device_id   = S25FL116_DEVICE_ID,
        .page_size   =  S25FL116_PAGE_SIZE,
        .sector_size = S25FL116_SECTOR_SIZE,
        .block_size  = S25FL116_BLOCK_SIZE,
        .capacity    = S25FL116_CHIP_SIZE
};
const QSpiFlashConfig_t * const supported_flash_devices[] = {
        &Macronix_MX25R8035F_device_def,
#if 0
        &Spansion_S25FL116_device_def
#endif
};
#endif



spifi_config_t config = {
        .timeout = 0xFFFFU,
        .csHighTime = 0xFU,
        .disablePrefetch = false,
        .disableCachePrefech = false,
        .isFeedbackClock = true,
        .spiMode = kSPIFI_SPISckLow,
        .isReadFullClockCycle = false,
        .dualMode = kSPIFI_QuadMode
};

static spifi_command_t command[MAX_CMD] =
{
    [RDID]  = {4                  , false, kSPIFI_DataInput,  0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x9F},
    [RDSR]  = {1                  , false, kSPIFI_DataInput,  0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x05},
//  [RDCR]  = {4                  , false, kSPIFI_DataInput,  0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x15},
    [WREN]  = {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x06},
//  [WRDI]  = {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x04},
    [WRSR]  = {3                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,   0x01},
//  [PP4]   = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0x38},
#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
    [PP]    =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0x02},
    [DREAD] =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  1, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x3B},
#else
    [QPP]   = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x32},
    [QREAD] = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  1, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
#endif
#else
    [QPP]   = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x32},
    [PP]    =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0x02},
    [QREAD] = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  1, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x6B},
    [DREAD] =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  1, kSPIFI_CommandDataQuad,     kSPIFI_CommandOpcodeAddrThreeBytes, 0x3B},
#endif

    [SE]    = {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0x20},
//  [READ]  = {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0x03},
//  [READ2] =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  1, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0xBB},
//  [READ4] =  {QSPIFLASH_PAGE_SIZE, false, kSPIFI_DataInput,  3, kSPIFI_CommandOpcodeSerial, kSPIFI_CommandOpcodeAddrThreeBytes, 0xEB},
#ifndef SPIFI_OPTIM_SIZE
    [BE64K] =  {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0xD8},
    [CE]    =  {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,           0x60},
#endif
    [BE32K] =  {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeAddrThreeBytes, 0x52},
    [DP]    =  {0                  , false, kSPIFI_DataOutput, 0, kSPIFI_CommandAllSerial,    kSPIFI_CommandOpcodeOnly,           0xB9},
};

const iocon_group_t spifi_io_cfg[] = {
    [0] = {
        .port = 0,
        .pin =  16,               /* SPIFI Chip Select */
        .modefunc = SPIFI_FAST_IO_MODE_PULLUP,
    },
    [1] = {
        .port = 0,
        .pin =  17,                 /*SPIFI DIO3 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [2] = {
        .port = 0,
        .pin =  18,                 /*SPIFI CLK */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [3] = {
        .port = 0,
        .pin =  19,                 /*SPIFI DIO0 */
        .modefunc =SPIFI_FAST_IO_MODE_INACT,
    },
    [4] = {
        .port = 0,
        .pin =  20,                 /*SPIFI DIO2 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
    [5] = {
        .port = 0,
        .pin =  21,                 /*SPIFI DIO1 */
        .modefunc = SPIFI_FAST_IO_MODE_INACT,
    },
};

static uint8_t initialized = 0;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void qspi_wait_for_completion(void);

static uint32_t SPIFI_readCommand (command_t cmd);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

static void ConfigureSpiFi(void)
{
    /* Set up SPIFI: it comprises 6 pins : PIO[16:21]
     * PIO[16] : CSN
     * PIO[17] : IO3 (in Quad Access Mode only)
     * PIO[18] : CLK
     * PIO[19] : IO0
     * PIO[20] : IO2 (in Quad Access Mode only)
     * PIO[21] : IO1
     * */
    CLOCK_EnableClock(kCLOCK_Iocon);
    for (int i = 0; i < 6; i++)
    {
        if ((i==1 || i==4) && IS_SPIFI_DUAL_MODE())
        {
#ifdef DK6
            /* In DK6 board, IO2 & IO3 needs to be set with Pull-Ups instead of
             * the default Pull-Downs config which is placing the QSPI Flash in Reset
             */
            IOCON_PinMuxSet(IOCON,
                            spifi_io_cfg[i].port,
                            spifi_io_cfg[i].pin,
                            (uint32_t)GPIO_PULLUP_MODE);
#else
            /* We assume that customers will have connected IO2 & IO3 to Vdd
             * hence no need to configure them
             */
            continue;
#endif
        }
        else
        {
            IOCON_PinMuxSet(IOCON,
                            spifi_io_cfg[i].port,
                            spifi_io_cfg[i].pin,
                            spifi_io_cfg[i].modefunc);
        }
    }
    CLOCK_DisableClock(kCLOCK_Iocon);
}

/*! *********************************************************************************
* \brief   Set the SPIFI Flash into read mode
*
* \param[in] Addr      Start memory address
*
********************************************************************************** */
static void SPIFI_SetRead(uint32_t Addr)
{

    /* Set start address */
    SPIFI_SetCommandAddress(SPIFI, FSL_FEATURE_SPIFI_START_ADDR + Addr );

    /* Enable read */
#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
    SPIFI_SetMemoryCommand(SPIFI, &command[DREAD]);
#else
    SPIFI_SetMemoryCommand(SPIFI, &command[QREAD]);
#endif
#else
    if (IS_SPIFI_DUAL_MODE())
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[DREAD]);
    }
    else
    {
        SPIFI_SetMemoryCommand(SPIFI, &command[QREAD]);
    }
#endif
}

/*! *********************************************************************************
* \brief   Write a page in the external memory, at a given address
*
* \param[in] NoOfBytes Number of bytes to read
* \param[in] Addr      Start memory address
* \param[in] Outbuf    Pointer to the data
*
********************************************************************************** */
static void SPIFI_WritePage(uint32_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf)
{
    if(NoOfBytes > 0)
    {
        SPIFI_SetCommand(SPIFI, &command[WREN]);
        SPIFI_SetCommandAddress(SPIFI, Addr + FSL_FEATURE_SPIFI_START_ADDR);

#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
        command[PP].dataLen = NoOfBytes;
        SPIFI_SetCommand(SPIFI, &command[PP]);
#else
        command[QPP].dataLen = NoOfBytes;
        SPIFI_SetCommand(SPIFI, &command[QPP]);
#endif
#else
        if (IS_SPIFI_DUAL_MODE())
        {
            command[PP].dataLen = NoOfBytes;
            SPIFI_SetCommand(SPIFI, &command[PP]);
        }
        else
        {
            command[QPP].dataLen = NoOfBytes;
            SPIFI_SetCommand(SPIFI, &command[QPP]);
        }
#endif
        SPIFI_WriteBuffer(SPIFI, Outbuf, NoOfBytes);
        qspi_wait_for_completion();
        SPIFI_SetRead(0);
    }
}


int SPIFI_Flash_Init(void)
{
    if(!initialized)
    {
        int status = -1;

        spifi_config_t config = {0};
    #define SPIFI_CLK  kMAIN_CLK_to_SPIFI
    //#define SPIFI_CLK kFRO48M_to_SPIFI

        do {
            // Init SPIFI clk
//#define NO_OPT_SZ
    #ifdef NO_OPT_SZ
            RESET_SetPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
            CLOCK_AttachClk(SPIFI_CLK);
            uint32_t divisor = CLOCK_GetSpifiClkFreq() / SPI_BAUDRATE;
            CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, divisor ? divisor : 1, false);
            CLOCK_EnableClock(kCLOCK_Spifi);
            /* Set the clock divider */
            SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_SPIFI_CLK_SET_MASK;
            ConfigureSpiFi();
            RESET_ClearPeripheralReset(kSPIFI_RST_SHIFT_RSTn);
    #else
            SYSCON->PRESETCTRLSET[0] = SYSCON_PRESETCTRL0_SPIFI_RST_SHIFT; /* Hold SPIFI under reset */
            SYSCON->SPIFICLKDIV |= SYSCON_SPIFICLKDIV_HALT_MASK;
            SYSCON->SPIFICLKSEL = kCLOCK_SpifiMainClk;  /* Main CLK to SPIFI CLK : supposed to be 32MHz */

            ConfigureSpiFi();
            /* Set the clock divider */
            uint32_t divisor = 32 / SPI_BAUDRATE;
            // CLOCK_SetClkDiv(kCLOCK_DivSpifiClk, divisor ? divisor : 1, false);
            SYSCON->SPIFICLKDIV &= ~SYSCON_SPIFICLKDIV_DIV_MASK;
            if ((divisor-1) > 0)
            {
                SYSCON->SPIFICLKDIV |= SYSCON_SPIFICLKDIV_DIV(divisor-1);
            }
            SYSCON->SPIFICLKDIV &= ~SYSCON_SPIFICLKDIV_HALT_MASK;
            SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_SPIFI_CLK_SET_MASK;
            SYSCON->PRESETCTRLCLR[0] = SYSCON_PRESETCTRL0_SPIFI_RST_SHIFT; /* Release SPIFI reset */
    #endif
            /* Initialize SPIFI */
            SPIFI_GetDefaultConfig(&config);
            bool dual_mode = false;
#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
            config.dualMode = kSPIFI_DualMode;
            dual_mode = true;
#endif
#else
            if (IS_SPIFI_DUAL_MODE())
            {
                config.dualMode = kSPIFI_DualMode;
                dual_mode = true;
            }
#endif
            SPIFI_Init(SPIFI, &config);

            uint32_t val = SPIFI_readCommand(RDID);
            val &= 0x00FFFFFF;

            /* Write enable */
            SPIFI_SetCommand(SPIFI, &command[WREN]);

            switch (val)
            {
                case MX25R8035F_DEVICE_ID:
                {
                    /* Set write register command */
                    uint32_t cfg_word = 0x000000; /* 24 bit register */
    #ifdef gSpiFiHiPerfMode_d
                    cfg_word |= MX25R8035_CFG_REG2_HI_PERF_MODE;
    #endif
                    if (! dual_mode )
                    {
                        /* insert dummy cycles for Quad mode operation */
                        cfg_word |= MX25R8035_CFG_STATUS_QUAD_MODE;
                    }
                    SPIFI_SetCommand(SPIFI, &command[WRSR]);
                    SPIFI_WritePartialWord(SPIFI, cfg_word, command[WRSR].dataLen);
                    status = 0;
                }
                break;

                case XT25F08B_DEVICE_ID:
                    command[WRSR].dataLen=2;
                    SPIFI_SetCommand(SPIFI, &command[WRSR]);
                    SPIFI_WriteDataHalfword(SPIFI, 0x0200);
                    status = 0;
                    break;
                default:
                    break;
            }

            qspi_wait_for_completion();

            // Set address
            uint32_t addr = 0;
            SPIFI_SetCommandAddress(SPIFI, FSL_FEATURE_SPIFI_START_ADDR + addr);

            /* Setup memory command */
            SPIFI_SetCommand(SPIFI, &command[WREN]);

#ifdef SPIFI_OPTIM_SIZE
#if IS_SPIFI_DUAL_MODE()
            SPIFI_SetMemoryCommand(SPIFI, &command[DREAD]);
#else
            SPIFI_SetMemoryCommand(SPIFI, &command[QREAD]);
#endif
#else
            command_t read_op =  dual_mode ? DREAD : QREAD;
            SPIFI_SetMemoryCommand(SPIFI, &command[read_op]);
#endif
            initialized = 1;

        } while (0);
        return status;
    }
    return (0);
}



void SPIFI_Flash_Deinit(void)
{
    SPIFI_SetCommand(SPIFI, &command[DP]);

    SPIFI_Deinit(SPIFI);
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRLSET0_SPIFI_CLK_SET_MASK;
#ifdef NO_OPT_SZ
    CLOCK_DisableClock(kCLOCK_Spifi);
#endif
    initialized = 0;
}


static void qspi_wait_for_completion(void)
{
    uint32_t val = 0;

    /* Check WIP bit */
    do
    {
        SPIFI_SetCommand(SPIFI, &command[RDSR]);
        {
            while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U);
        }
        val = SPIFI_ReadPartialWord(SPIFI, command[RDSR].dataLen);
    } while (val & 0x1); /* BIT(0) is WRITE IN PROGRESS/BUSY flag */
}


uint32_t SPIFI_readCommand (command_t cmd)
{
    /* Reset the SPIFI to switch to command mode */
    SPIFI_ResetCommand(SPIFI);
    /* Setup command */
    SPIFI_SetCommand(SPIFI, &command[cmd]);
    while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U);

    // Return data
    return(SPIFI_ReadData(SPIFI));
}

#ifndef SPIFI_OPTIM_SIZE
void SPIFI_ChipErase(void)
{
    /* Reset the SPIFI to switch to command mode */
    SPIFI_ResetCommand(SPIFI);

    qspi_wait_for_completion();

    SPIFI_SetCommand(SPIFI, &command[WREN]);

    /* Chip Erase command */
    SPIFI_SetCommand(SPIFI, &command[CE]);
    while ((SPIFI->STAT & SPIFI_STAT_INTRQ_MASK) == 0U);

}
#endif

/*! *********************************************************************************
* \brief   Write a data buffer into the external memory, at a given address
*
* \param[in] NoOfBytes Number of bytes to write
* \param[in] Addr      Start memory address
* \param[in] inbuf     Pointer to the data
*
********************************************************************************** */
void SPIFI_writeData(uint32_t NoOfBytes, uint32_t Addr, uint8_t *Outbuf)
{
    uint32_t bytes;
    if (Addr >= QSPIFLASH_CHIP_SIZE)
    {
        return;
    }

    if (NoOfBytes > 0)
    {
        while ((Addr & QSPIFLASH_PAGE_MASK) + NoOfBytes > QSPIFLASH_PAGE_MASK)
        {
            bytes = QSPIFLASH_PAGE_SIZE - (Addr & QSPIFLASH_PAGE_MASK);
            SPIFI_WritePage(bytes, Addr, Outbuf);
            NoOfBytes -= bytes;
            Addr += bytes;
            Outbuf += bytes;

        }
        SPIFI_WritePage(NoOfBytes, Addr, Outbuf);
        qspi_wait_for_completion();
        SPIFI_SetRead(0);
    }
}

/*! *********************************************************************************
* \brief   Erase data block the external memory, at a given address
*
* \param[in] Addr              Start memory address
* \param[in] block_size        size of the block to erase
*
********************************************************************************** */
static void SPIFI_eraseBlock(uint32_t Addr, uint32_t block_size)
{
    uint8_t cmd;
    if (Addr >= QSPIFLASH_CHIP_SIZE)
    {
        return;
    }

    SPIFI_SetCommand(SPIFI, &command[WREN]);

    switch (block_size)
    {
    case QSPIFLASH_SECTOR_SIZE:
        cmd = SE;
        break;
    case QSPIFLASH_BLOCK_SIZE:
        cmd = BE32K;
        break;
#ifndef SPIFI_OPTIM_SIZE
    case QSPIFLASH_BLOCK2_SIZE:
        cmd = BE64K;
        break;
#endif
    default:
        return;
    }
    /* Set block address: any address within the block is acceptable to the MX25R8035F */
    SPIFI_SetCommandAddress(SPIFI, Addr);
    /* Erase sector or block */
    SPIFI_SetCommand(SPIFI, &command[cmd]);

}

/*! *********************************************************************************
* \brief   This function erases as many block/sectors as necessary to erase
*          the expected number of bytes
*
* \param[in] Addr              Start memory address
* \param[in] size            size of the area to erase
*
********************************************************************************** */
uint8_t SPIFI_eraseArea(uint32_t Addr, int32_t size)
{
    uint8_t status = 1;
    uint32_t sz;
    int32_t remain_sz = (int32_t)size;
    uint32_t erase_addr = Addr;

    do {
        if ((uint32_t)Addr >= QSPIFLASH_CHIP_SIZE)
        {
            break;
        }

        if (!IS_ALIGNED(Addr, QSPIFLASH_SECTOR_SIZE))
        {
            break;
        }
        if (remain_sz == 0)
        {
            break;
        }

        for (erase_addr = Addr; remain_sz > 0; )
        {
            sz = QSPIFLASH_SECTOR_SIZE;
#ifndef SPIFI_OPTIM_SIZE
            if ((IS_ALIGNED(erase_addr, QSPIFLASH_BLOCK2_SIZE) && (remain_sz >= QSPIFLASH_BLOCK2_SIZE)))  /* QSPIFLASH_BLOCK_SIZE*2 */
            {
                sz = QSPIFLASH_BLOCK2_SIZE;
            }
            else
#endif
            {
                if ((IS_ALIGNED(erase_addr, QSPIFLASH_BLOCK_SIZE) && (remain_sz >= QSPIFLASH_BLOCK_SIZE))) /* QSPIFLASH_BLOCK_SIZE */
                {
                    sz = QSPIFLASH_BLOCK_SIZE;
                }
            }
            SPIFI_eraseBlock(erase_addr, sz);
            erase_addr += sz;
            remain_sz -= sz;
        }
        status = 0;
    } while (0);
    qspi_wait_for_completion();
    SPIFI_SetRead(0);
    return status;
}

/*! *********************************************************************************
* \brief   This function returns the size of the SPIFI Memory
*
********************************************************************************** */
uint32_t SPIFI_getSize(void)
{
    return QSPIFLASH_CHIP_SIZE;
}

/*! *********************************************************************************
* \brief   This function returns the size of a SPIFI Memory Sector
*
********************************************************************************** */
uint32_t SPIFI_getSectorSize(void)
{
    return QSPIFLASH_SECTOR_SIZE;
}
/*******************************************************************/
