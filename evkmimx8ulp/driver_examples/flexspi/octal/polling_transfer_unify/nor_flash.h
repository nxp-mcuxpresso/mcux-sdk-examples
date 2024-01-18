/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _NOR_FLASH_H_
#define _NOR_FLASH_H_

#define BIT(n)         (1U << (n))

#define SPI_NOR_MAX_ID_LEN (10)

#define CFI_MFR_ANY          0xFFFF
#define CFI_ID_ANY           0xFFFF
#define CFI_MFR_CONTINUATION 0x007F
#define CFI_CONTINUATION 0x7F

#define CFI_MFR_AMD      0x0001
#define CFI_MFR_AMIC     0x0037
#define CFI_MFR_ATMEL    0x001F
#define CFI_MFR_EON      0x001C
#define CFI_MFR_FUJITSU  0x0004
#define CFI_MFR_HYUNDAI  0x00AD
#define CFI_MFR_INTEL    0x0089
#define CFI_MFR_MACRONIX 0x00C2
#define CFI_MFR_NEC      0x0010
#define CFI_MFR_PMC      0x009D
#define CFI_MFR_SAMSUNG  0x00EC
#define CFI_MFR_SHARP    0x00B0
#define CFI_MFR_SST      0x00BF
#define CFI_MFR_ST       0x0020 /* STMicroelectronics */
#define CFI_MFR_MICRON   0x002C /* Micron */
#define CFI_MFR_TOSHIBA  0x0098
#define CFI_MFR_WINBOND  0x00DA

/*
 * Manufacturer IDs
 *
 * The first byte returned from the flash after sending opcode SPINOR_OP_RDID.
 * Sometimes these are the same as CFI IDs, but sometimes they aren't.
 */
#define SNOR_MFR_ATMEL      CFI_MFR_ATMEL
#define SNOR_MFR_GIGADEVICE 0xc8
#define SNOR_MFR_INTEL      CFI_MFR_INTEL
#define SNOR_MFR_ST         CFI_MFR_ST     /* ST Micro <--> Micron */
#define SNOR_MFR_MICRON     CFI_MFR_MICRON /* ST Micro <--> Micron */
#define SNOR_MFR_ISSI       CFI_MFR_PMC
#define SNOR_MFR_MACRONIX   CFI_MFR_MACRONIX
#define SNOR_MFR_SPANSION   CFI_MFR_AMD
#define SNOR_MFR_SST        CFI_MFR_SST
#define SNOR_MFR_WINBOND    0xef /* Also used by some Spansion */
#define SNOR_MFR_CYPRESS    0x34
#define SNOR_MFR_ADESTO     0x43

/*
 * Note on opcode nomenclature: some opcodes have a format like
 * SPINOR_OP_FUNCTION{4,}_x_y_z{_c}. The numbers x, y, z and c stand for the number
 * of I/O lines used for the opcode, address, data and command code (respectively). The
 * FUNCTION has an optional suffix of '4', to represent an opcode which
 * requires a 4-byte (32-bit) address.
 */

/* Flash opcodes. */
#define SPINOR_OP_WREN         0x06 /* Write enable */
#define SPINOR_OP_WREN_0XF9    0xf9 /* 2nd byte of command set(Write enable) of OPI Command Set from macronix MX25UM51345G */
#define SPINOR_OP_RDSR         0x05 /* Read status register */
#define SPINOR_OP_RDSR_0XFA    0xfa /* 2nd byte of command set Read status register of OPI Command Set of macronix MX25UM51345G */
#define SPINOR_OP_RDSR1        0x05 /* Read status register byte 1 */
#define SPINOR_OP_WRSR         0x01 /* Write status register */
#define SPINOR_OP_WRSR1        0x01 /* Write status register byte 1 */
#define SPINOR_OP_RDSR2        0x3f /* Read status register byte 2 */
#define SPINOR_OP_WRSR2        0x3e /* Write status register byte 2 */
#define SPINOR_OP_WRCR2        0x72 /* Write configuration register2 of macronix MX25UM51345G */
#define SPINOR_OP_READ         0x03 /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST    0x0b /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_FAST_4B 0x0c /* Read data bytes (high frequency), from macronix MX25UM51345G */
#define SPINOR_OP_READ_1_1_2   0x3b /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2   0xbb /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4   0x6b /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4   0xeb /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8   0x8b /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8   0xcb /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP           0x02 /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4     0x32 /* Quad page program */
#define SPINOR_OP_PP_1_4_4     0x38 /* Quad page program */
#define SPINOR_OP_PP_1_1_8     0x82 /* Octal page program */
#define SPINOR_OP_PP_1_8_8     0xc2 /* Octal page program */
#define SPINOR_OP_BE_4K        0x20 /* Erase 4KiB block */
#define SPINOR_OP_BE_4K_0XD7    0xd7 /* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K       0x52 /* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE   0xc7 /* Erase whole flash chip, 0xc7 or 0x60 */
#define SPINOR_OP_CHIP_ERASE_0X9F   0x9f /* 2nd byte of command set Erase whole flash chip of OPI Command Set for macronix MX25UM51345G */
#define SPINOR_OP_CHIP_ERASE_0X38   0x38 /* 2nd byte of command set Erase whole flash chip of OPI Command Set for macronix MX25UM51345G */
#define SPINOR_OP_SE           0xd8 /* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID         0x9f /* Read JEDEC ID */
#define SPINOR_OP_RDID2        0x60 /* 2nd byte of command set(Read JEDEC ID) of OPI Command Set for macronix MX25UM51345G */
#define SPINOR_OP_RDSFDP       0x5a /* Read SFDP */
#define SPINOR_OP_RDCR         0x35 /* Read configuration register */
#define SPINOR_OP_RDFSR        0x70 /* Read flag status register */
#define SPINOR_OP_CLFSR        0x50 /* Clear flag status register */
#define SPINOR_OP_RDEAR        0xc8 /* Read Extended Address Register */
#define SPINOR_OP_WREAR        0xc5 /* Write Extended Address Register */
#define SPINOR_OP_SRSTEN       0x66 /* Software Reset Enable */
#define SPINOR_OP_SRST         0x99 /* Software Reset */

/* ADESTO flash opcode */
#define SPINOR_OP_WRSR2_0X31 0x31 /* Write status register byte 2 for ADES octal nor flash(such as: ATXP032) */
#define SPINOR_OP_ENTER_OCTAL_MODE_0XE8 0xe8 /* Enter Octal Mode */
#define SPINOR_OP_RETURN_TO_SPI_FROM_OCTAL_0XFF \
    0xff /* Return to Standard SPI Mode from Octal Mode(ADESTO support the command) */

/* GigaDevice flash opcode */
#define SPINOR_OP_RD_NONVOLATILE_CFG_0XB5 0xb5 /* Read Nonvolatile Configuration Register (from GigaDevice GD25LX256E) */
#define SPINOR_OP_RD_VOLATILE_CFG_0X85 0x85 /* Read volatile Configuration Register (from GigaDevice GD25LX256E) */
#define SPINOR_OP_WR_NONVOLATILE_CFG_0XB1 0xb1 /* Write Nonvolatile Configuration Register (from GigaDevice GD25LX256E) */
#define SPINOR_OP_WR_VOLATILE_CFG_0X81 0x81 /* Write volatile Configuration Register (from GigaDevice GD25LX256E) */
#define SPINOR_OP_ENABLE_4_BYTE_ADDR_MODE_0XB7 0xb7 /* Enable 4-Byte Address Mode (from GigaDevice GD25LX256E) */
#undef SPINOR_OP_DISABLE_4_BYTE_ADDR_MODE_0XB7
#define SPINOR_OP_DISABLE_4_BYTE_ADDR_MODE_0xE9 0xe9 /* Disable 4-Byte Address Mode (from GigaDevice GD25LX256E) */
#define SPINOR_OP_SE_0X20 0x20 /* SECTOR ERASE (from GigaDevice GD25LX256E) */
#define SPINOR_OP_SE_0X21 0x21 /* SECTOR ERASE (from GigaDevice GD25LX256E) */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ_4B       0x13 /* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B  0x0c /* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2_4B 0x3c /* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2_4B 0xbc /* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4_4B 0x6c /* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4_4B 0xec /* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8_4B 0x7c /* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8_4B 0xcc /* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_READ1_8_8_8_4B 0xec /* Read data bytes (8 I/O read); For macronix MX25UM51345G, 1st byte of command set(8READ) of SPI Command Set */
#define SPINOR_OP_READ2_8_8_8_4B 0x13 /* Read data bytes (8 I/O read); For macronix MX25UM51345G, 2nd byte of command set(8READ) of SPI Command Set */
#define SPINOR_OP_READ_DTR_0XEE_8_8_8_4B 0xee /* Read data bytes (8 I/O DT read); For macronix MX25UM51345G, 1st byte of command set(8DTRD) of SPI Command Set */
#define SPINOR_OP_READ_DTR_0X11_8_8_8_4B 0x11 /* Read data bytes (8 I/O DT read); For macronix MX25UM51345G, 2nd byte of command set(8DTRD) of SPI Command Set */
#define SPINOR_OP_PP_4B         0x12 /* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_4B_0XED    0xed /* 2nd byte of command set Page program (up to 256 bytes) of OPI Command Set of macronix MX25UM51345G */
#define SPINOR_OP_PP_1_1_4_4B   0x34 /* Quad page program */
#define SPINOR_OP_PP_1_4_4_4B   0x3e /* Quad page program */
#define SPINOR_OP_PP_1_1_8_4B   0x84 /* Octal page program */
#define SPINOR_OP_PP_1_8_8_4B   0x8e /* Octal page program */
#define SPINOR_OP_BE_4K_4B      0x21 /* Erase 4KiB block */
#define SPINOR_OP_BE_32K_4B     0x5c /* Erase 32KiB block */
#define SPINOR_OP_SE_4B         0xdc /* Sector erase (usually 64KiB) */
#define SPINOR_OP_SE_4B_0X21    0x21 /* Sector erase for macronix MX25UM51345G */
#define SPINOR_OP_SE_4B_0XDE    0xde /* 2nd byte of command set Sector erase of OPI Command Set for macronix MX25UM51345G */

#define SPINOR_ADDRESS_32_BITS (0x20) /* 4 byte address */
#define SPINOR_ADDRESS_24_BITS (0x18) /* 3 byte address */

#define SPINOR_DUMMY_CYCLE_NUMBER_0X1 (0x1) /* 1 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X2 (0x2) /* 2 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X4 (0x4) /* 4 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X8 (0x8) /* 8 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X10 (0x10) /* 16 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X16 (0x16) /* 22 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X20 (0x20) /* 32 dummy clock cycle */
#define SPINOR_DUMMY_CYCLE_NUMBER_0X29 (0x29) /* 41 dummy clock cycle */

#define SPINOR_DATA_SIZE_1_BYTES (0x1) /* data size is 1 bytes */
#define SPINOR_DATA_SIZE_4_BYTES (0x4) /* data size is 4 bytes */

/* Status Register bits. */
#define SR_WIP BIT(0) /* Write in progress */
#define SR_WEL BIT(1) /* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define SR_BP0  BIT(2) /* Block protect 0 */
#define SR_BP1  BIT(3) /* Block protect 1 */
#define SR_BP2  BIT(4) /* Block protect 2 */
#define SR_TB   BIT(5) /* Top/Bottom protect */
#define SR_SRWD BIT(7) /* SR write protect */
/* Spansion/Cypress specific status bits */
#define SR_E_ERR BIT(5)
#define SR_P_ERR BIT(6)

/*
 * Adesto specific Status/Control Register Byte 1 bits
 * 0: Sector Protection Registers are unlocked(default)
 * 1: Sector Protection Registers are locked.
 */
#define SR_SPRL_ADESTO BIT(7)
/*
 * DPDS: Deep Power-Down Status
 * 0: Device is in Active or Standby Mode
 * 1: Device is in Deep Power-Down Mode
 */
#define SR_DPDS_ADESTO BIT(6)
/*
 * EPE: Program/Erase Error
 * 0: Erase or Program operation was successful
 * 1: Erase or Program error detected
 */
#define SR_EPE_ADESTO BIT(5)
/*
 * UDPDS: Ultra-Deep Power-Down Status
 * 0: Device is in Active, Standby or DPD Mode
 * 1: Device is in Ultra-Deep Power-Down Mode
 */
#define SR_UDPDS_ADESTO BIT(4)
/*
 * SWP: Software Protection Status
 * 00: All sectors are software unprotected(all Sector Protection Registers are 0)
 * 01: Some sectors are software protected. Read individual Sector Protection Registers to determine which sectors are protected.
 * 10: Reserved are protected
 * 11: All sectors are software protected(all Sector Protection Regaters are 1 - default)
 */
#define SR_SWP1_ADESTO BIT(3)
#define SR_SWP0_ADESTO BIT(2)
#define SR_SWP_ALL_SECTORS_UNPROTECTED (0)
#define SR_SWP_SOME_SECTORS_PROTECTED (SR_SWP0_ADESTO)
#define SR_SWP_ALL_SECTORS_PROTECTED (SR_SWP1_ADESTO | SR_SWP0_ADESTO)

#define SR_QUAD_EN_MX BIT(6) /* Macronix Quad I/O */

/* Enhanced Volatile Configuration Register bits */
#define EVCR_QUAD_EN_MICRON BIT(7) /* Micron Quad I/O */

/* Flag Status Register bits */
#define FSR_READY  BIT(7) /* Device status, 0 = Busy, 1 = Ready */
#define FSR_E_ERR  BIT(5) /* Erase operation status */
#define FSR_P_ERR  BIT(4) /* Program operation status */
#define FSR_PT_ERR BIT(1) /* Protection error bit */

/* Flag Status Register bits of GigaDevice */
#define FSR_ADS_GIGADEVICE BIT(0) /* Flash is in 3-Byte address mode when ADS=0(default), and in 4-Byte address mode when ADS=1 */

/* Configuration Register bits. */
#define CR_QUAD_EN_SPAN BIT(1) /* Spansion Quad I/O */

/*
 * Configuration Register 2 of MACRONIX
 * DOPI: DTR(Double Transfer Rate) OPI Enable
 * SOPI: STR(Single Transfer Rate) OPI Enable
 * 00 - SPI
 * 01 - STR OPI enable
 * 10 - DTR OPI enable
 * 11 - inhibit
 */
#define CR2_DOPI_MACRONIX BIT(1)
#define CR2_SOPI_MACRONIX BIT(0)
#define CR2_DTR_OPI_ENABLE_MACRONIX (CR2_DOPI_MACRONIX)
#define CR2_STR_OPI_ENABLE_MACRONIX (CR2_SOPI_MACRONIX)


/* Status Register 2 bits. */
#define SR2_QUAD_EN_BIT7 BIT(7)

/*
 * Adesto Status/Control Register Byte 2 bits
 * SDR/DDR: Select SDR or DDR mode
 * 0: SDR - Single data rate mode(Default)
 * 1: DDR - Dual data rate mode
 */
#define SR2_SDR_DDR_ADESTO BIT(7)
/*
 * AUDPD: Audo Ultra-Deep Power-Down enable
 * ADPD: Auto Deep Power-Down enable
 * 00: Normal mode - Go to Standby after Program/Erase
 * 01: ADPD Set: Go to DPD after Program/Erase
 * 10: AUDPD Set: Go to UDPD after Program/Erase
 * 11: Illegal combination - Reserved for future use
 */
#define SR2_AUDPD_ADESTO BIT(6)
#define SR2_ADPD_ADESTO BIT(5)
/*
 * OME: Octal mode enable
 * 0: SPI mode enabled
 * 1: Octal mode enabled
 */
#define SR2_OME_ADESTO BIT(3)

/* Address of Nonvolatile/Volatile Configuration Register of GigaDevice(GD25LX256E) */
#define SPINOR_VOLATILE_CFG_REG_ADDR_IO_MODE_0X0 0x0
#define SPINOR_VOLATILE_CFG_REG_ADDR_DUMMY_CYCLE_CFG_0X1 0x1

#define SPINOR_IO_MODE_SPI_WITH_DQS_0XFF (0xff) /* SPI with DQS(Default) */
#define SPINOR_IO_MODE_SPI_WITHOUT_DQS_0XDF (0xdf) /* SPI without DQS */
#define SPINOR_IO_MODE_OCTAL_DTR_WITH_DQS_0XE7 (0xe7) /* OCTAL DTR with DQS */
#define SPINOR_IO_MODE_OCTAL_DTR_WITHOUT_DQS_0XC7 (0xc7) /* OCTAL DTR without DQS */
#define SPINOR_IO_MODE_OCTAL_STR_WITH_DQS_0XB7 (0xb7) /* OCTAL STR with DQS */
#define SPINOR_IO_MODE_OCTAL_STR_WITHOUT_DQS_0X97 (0x97) /* OCTAL STR without DQS */

/* look up table macros */
#ifndef CUSTOM_LUT_LENGTH
#define CUSTOM_LUT_LENGTH 64
#endif

#ifndef NOR_CMD_LUT_SEQ_IDX_READ
#define NOR_CMD_LUT_SEQ_IDX_READ 0
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READSTATUS_SPI
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_SPI 1
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_SPI
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_SPI 2
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READID_SPI
#define NOR_CMD_LUT_SEQ_IDX_READID_SPI 3
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_OPI 4
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_QPI 4
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_DPI
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE_DTR_DPI 4
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_OPI 5
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_QPI 5
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_DPI
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR_DTR_DPI 5
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_CONFIG
#define NOR_CMD_LUT_SEQ_IDX_CONFIG 6
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_OPI 7
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_DTR_QPI 7
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_ENTEROPI
#define NOR_CMD_LUT_SEQ_IDX_ENTEROPI 8
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_WRITE
#define NOR_CMD_LUT_SEQ_IDX_WRITE 9
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READSTATUS_STR_OPI
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_STR_OPI 10
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_OPI 11
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_QPI 11
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_DPI
#define NOR_CMD_LUT_SEQ_IDX_CHIPERASE_DTR_DPI 11
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READID_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_READID_DTR_OPI 12
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READID_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_READID_DTR_QPI 12
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READID_DTR_DPI
#define NOR_CMD_LUT_SEQ_IDX_READID_DTR_DPI 12
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_OPI
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_OPI 13
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_QPI
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_QPI 13
#endif
#ifndef NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_DPI
#define NOR_CMD_LUT_SEQ_IDX_READSTATUS_DTR_DPI 13
#endif

/*
 * Device Operation Mode:
 * STR: Single Transfer Rate - SDR: Single Data Rate
 * DTR: Double Transfer Rate - DDR: Dual Data Rate
 */
typedef enum
{
    SPINOR_STR_MODE = 0,
    SPINOR_DTR_MODE = 1,
} operation_mode;

/*
 * IO Mode:
 * - Single I/O
 * - Octal I/O
 */
typedef enum
{
    SPINOR_SPI_MODE = 0, /* STR operation */
    SPINOR_OPI_MODE = 1, /* STR/DTR operation in Octal Peripheral Interface(Octal I/O) */
} io_mode;

typedef struct
{
    uint16_t manufacturerId; /* Manufacturer ID */
    io_mode ioMode; /* IO Mode(Single I/O, Octal I/O) */
    operation_mode opMode; /* STR, DTR */
    bool addrMode; /* Flash is in 3-Byte Address Mode when addrMode=0(FALSE), Flash is in 4-Byte Address Mode when addrMode=1(TRUE) */
} spi_nor_flash_state;
#endif
