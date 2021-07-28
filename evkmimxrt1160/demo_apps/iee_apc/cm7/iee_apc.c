/*
 * Copyright 2020, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_iee.h"
#include "fsl_iee_apc.h"
#include "app.h"
#include "fsl_cache.h"

#if USE_FLASH
#include "fsl_flexspi.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AES_KEY_LEN 32 /* 32 bytes (256bit) key lenght */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if USE_FLASH
/*
 *  Flash memory funtions
 */
extern void flexspi_nor_flash_init(FLEXSPI_Type *base);
extern status_t flexspi_nor_flash_erase_sector(FLEXSPI_Type *base, uint32_t address);
extern status_t flexspi_nor_erase_chip(FLEXSPI_Type *base);
extern status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base);
extern status_t flexspi_nor_flash_program(FLEXSPI_Type *base, uint32_t dstAddr, const uint32_t *src, uint32_t length);
#endif /* USE_FLASH */

/*
 *  Device read and write
 */
static bool Device_ReadWrite(void);

/*
 *  Setup funtions
 */
status_t IEE_APC_Setup(void);
#if USE_FLASH
status_t Flash_Setup(void);
#endif

/*
 *  Print function
 */
void Hex_Print(char *src, size_t size);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const uint8_t key1[32] = {0x15, 0xAE, 0x2B, 0xF0, 0x22, 0x93, 0x81, 0x98, 0xA4, 0xA4, 0x2A,
                                 0x07, 0xD1, 0x86, 0x50, 0x77, 0x4A, 0xAE, 0x17, 0x75, 0x05, 0x78,
                                 0x2E, 0x58, 0x78, 0xCA, 0x1C, 0xB4, 0xC4, 0x85, 0x5B, 0x07};

static const uint8_t key2[32] = {0x76, 0xA2, 0xE5, 0xB2, 0x11, 0xFA, 0x3A, 0x44, 0x21, 0xF5, 0x27,
                                 0xB6, 0x0A, 0xDE, 0x39, 0x1D, 0x2B, 0x18, 0x7A, 0x35, 0x75, 0x0C,
                                 0x29, 0xBF, 0xE6, 0x93, 0xC6, 0x11, 0x13, 0x10, 0x82, 0xE1};

/* Plain text: "HelloWolrdPlain!" */
static const uint8_t plain_text[16] = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x57, 0x6F, 0x72,
                                       0x6C, 0x64, 0x50, 0x6C, 0x61, 0x69, 0x6E, 0x21};

#if USE_FLASH

SDK_ALIGN(static const uint8_t aes_ctr[16], 8U) = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                   0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

SDK_ALIGN(static const uint8_t aes_ctr_cipher[16], 8U) = {0x78, 0x41, 0x24, 0x95, 0x2c, 0x3a, 0x1d, 0x3d,
                                                          0xfd, 0x99, 0x4d, 0x09, 0xa4, 0xee, 0xd3, 0xaa};

static const uint8_t aes_ctr_expect[16] = {0x0f, 0xa0, 0x47, 0xbc, 0x49, 0x7c, 0xd8, 0x4c,
                                           0x9c, 0x13, 0x6b, 0x6f, 0x2d, 0x53, 0x3d, 0x5c};

/* write buffer */
static uint8_t s_program_buffer[16] __attribute__((aligned(8)));

uint32_t StartAddrFlash     = EXAMPLE_FLEXSPI_AMBA_BASE + EXAMPLE_SECTOR * SECTOR_SIZE;
uint32_t EndAddrFlash       = EXAMPLE_FLEXSPI_AMBA_BASE + ((EXAMPLE_SECTOR + 1) * SECTOR_SIZE);
uint32_t *StartAddrPtrFlash = NULL;
#endif /* USE_FLASH */

AT_NONCACHEABLE_SECTION(static uint8_t s_read_buffer[16]); /* read buffer */

uint32_t *StartAddrPtr = (uint32_t *)StartAddr;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if USE_FLASH
flexspi_device_config_t deviceconfig = {
    .flexspiRootClk       = 12000000,
    .flashSize            = FLASH_SIZE,
    .CSIntervalUnit       = kFLEXSPI_CsIntervalUnit1SckCycle,
    .CSInterval           = 2,
    .CSHoldTime           = 3,
    .CSSetupTime          = 3,
    .dataValidTime        = 0,
    .columnspace          = 0,
    .enableWordAddress    = 0,
    .AWRSeqIndex          = 0,
    .AWRSeqNumber         = 0,
    .ARDSeqIndex          = NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD,
    .ARDSeqNumber         = 1,
    .AHBWriteWaitUnit     = kFLEXSPI_AhbWriteWaitUnit2AhbCycle,
    .AHBWriteWaitInterval = 0,
};

const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    /* Normal read mode -SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x03, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_NORMAL + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Fast read mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x0B, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_1PAD, 0x08, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Fast read quad mode - SDR */
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xEB, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_4PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR, kFLEXSPI_4PAD, 0x06, kFLEXSPI_Command_READ_SDR, kFLEXSPI_4PAD, 0x04),

    /* Read extend parameters */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUS] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x81, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Write Enable */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x06, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Erase Sector  */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xD7, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),

    /* Page Program - single mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x02, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_SINGLE + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Page Program - quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x32, kFLEXSPI_Command_RADDR_SDR, kFLEXSPI_1PAD, 0x18),
    [4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD + 1] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_4PAD, 0x04, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read ID */
    [4 * NOR_CMD_LUT_SEQ_IDX_READID] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x9F, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enable Quad mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x01, kFLEXSPI_Command_WRITE_SDR, kFLEXSPI_1PAD, 0x04),

    /* Enter QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_ENTERQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x35, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Exit QPI mode */
    [4 * NOR_CMD_LUT_SEQ_IDX_EXITQPI] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_4PAD, 0xF5, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),

    /* Read status register */
    [4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0x05, kFLEXSPI_Command_READ_SDR, kFLEXSPI_1PAD, 0x04),

    /* Erase whole chip */
    [4 * NOR_CMD_LUT_SEQ_IDX_ERASECHIP] =
        FLEXSPI_LUT_SEQ(kFLEXSPI_Command_SDR, kFLEXSPI_1PAD, 0xC7, kFLEXSPI_Command_STOP, kFLEXSPI_1PAD, 0),
};
#endif /* USE_FLASH */


/*!
 * @brief Main function
 */

int main(void)
{
    status_t status = kStatus_Fail;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("IEE APC Demo started\r\n\r\n");

#if USE_FLASH
    /* Setup Flash memory */
    status = Flash_Setup();
    if (status != kStatus_Success)
    {
        PRINTF("Error while setup Flash memory \r\n");
        return status;
    }
#endif /* USE_FLASH */

    /* Setup IEE and IEE_APC for secure write/read*/
    status = IEE_APC_Setup();
    if (status != kStatus_Success)
    {
        PRINTF("Error while setup IEE & IEE_APC memory \r\n");
        return status;
    }

    /* Perform write and read of encrypted data */
    status = Device_ReadWrite();
    if (status != kStatus_Success)
    {
        PRINTF("Error while writting and reading encrypted data. \r\n");
        return status;
    }

#if USE_FLASH
    PRINTF("\r\nIEE & IEE_APC & Flash demo End.\r\n");
#else
    PRINTF("\r\nIEE & IEE_APC demo End.\r\n");
#endif

    while (1)
    {
    }
}
void Hex_Print(char *src, size_t size)
{
    PRINTF("0x");
    for (int i = 0; i < size; i++)
    {
        PRINTF("%x", *(src + i));
    }
    PRINTF(" -> \"");
    for (int i = 0; i < size; i++)
    {
        PRINTF("%c", *(src + i));
    }
    PRINTF("\"\r\n\r\n");
}

#if USE_FLASH
status_t Flash_Setup(void)
{
    status_t status = kStatus_Fail;

    flexspi_nor_flash_init(EXAMPLE_FLEXSPI);

#if FLASH_ERASE_CHIP
    /* Erase whole chip . */
    PRINTF("Erasing whole chip over FlexSPI...\r\n");

    status = flexspi_nor_erase_chip(EXAMPLE_FLEXSPI);
    if (status != kStatus_Success)
    {
        return status;
    }
    PRINTF("Erase finished !\r\n");
#endif /* FLASH_ERASE_CHIP */

    /* Enter quad mode. */
    status = flexspi_nor_enable_quad_mode(EXAMPLE_FLEXSPI);
    if (status != kStatus_Success)
    {
        return status;
    }

    /* Erase sector. */
    PRINTF("Erasing Serial NOR over FlexSPI...\r\n");
    status = flexspi_nor_flash_erase_sector(EXAMPLE_FLEXSPI, EXAMPLE_SECTOR * SECTOR_SIZE);
    if (status != kStatus_Success)
    {
        PRINTF("Erase sector failure !\r\n");
        return kStatus_Fail;
    }

    /* Test of erase. */
    memset(s_program_buffer, 0xFFU, sizeof(s_program_buffer));
    DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + EXAMPLE_SECTOR * SECTOR_SIZE, FLASH_PAGE_SIZE);
    memcpy(s_read_buffer, (void *)StartAddrFlash, sizeof(s_read_buffer));

    if (memcmp(s_program_buffer, s_read_buffer, sizeof(s_program_buffer)))
    {
        PRINTF("Erase data -  read out data value incorrect !\r\n ");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Erase data - successfully. \r\n");
    }

    /* Program cipher text to be decrypted */
    status = flexspi_nor_flash_program(EXAMPLE_FLEXSPI, EXAMPLE_SECTOR * SECTOR_SIZE, (void *)aes_ctr_cipher,
                                       sizeof(aes_ctr_cipher));
    if (status != kStatus_Success)
    {
        PRINTF("Page program failure !\r\n");
        return kStatus_Fail;
    }

    /* Test programed data */
    DCACHE_InvalidateByRange(EXAMPLE_FLEXSPI_AMBA_BASE + EXAMPLE_SECTOR * SECTOR_SIZE, FLASH_PAGE_SIZE);
    memcpy(s_read_buffer, (void *)StartAddrFlash, sizeof(aes_ctr_cipher));

    if (memcmp(s_read_buffer, aes_ctr_cipher, sizeof(aes_ctr_cipher)) != 0)
    {
        PRINTF("Program data -  read out data value incorrect !\r\n ");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("Program data - successfully. \r\n");
    }

    return status;
}
#endif /* USE_FLASH */

status_t IEE_APC_Setup(void)
{
    iee_config_t iee_config;
    status_t status = kStatus_Fail;

    PRINTF("Set up IEE APC regions\r\n");
    /* IEE_APC set region 0 configuration */
    status = IEE_APC_SetRegionConfig(IEE_APC, kIEE_APC_Region0, (uint32_t)StartAddr, (uint32_t)EndAddr);
    if (status != kStatus_Success)
    {
        PRINTF("Error set IEE APC region");
        return status;
    }

#if USE_FLASH
    /* IEE_APC set region 1 configuration */
    status = IEE_APC_SetRegionConfig(IEE_APC, kIEE_APC_Region1, (uint32_t)StartAddrFlash, (uint32_t)EndAddrFlash);
    if (status != kStatus_Success)
    {
        PRINTF("Error set IEE APC region");
        return status;
    }
#endif /* USE_FLASH */

    PRINTF("Enable IEE APC regions\r\n");
    /* IEE_APC enable region 0 encryption */
    IEE_APC_RegionEnable(IEE_APC, kIEE_APC_Region0);
#if USE_FLASH
    IEE_APC_RegionEnable(IEE_APC, kIEE_APC_Region1);
#endif /* USE_FLASH */

    /* IEE_APC global enable */
    IEE_APC_GlobalEnable(IEE_APC);

    PRINTF("IEE init\r\n");
    /* Reset IEE to factory defaults */
    IEE_Init(IEE);

    /* Configure IEE region 0 */
    IEE_GetDefaultConfig(&iee_config);
    iee_config.bypass  = kIEE_AesUseMdField;
    iee_config.mode    = kIEE_ModeAesXTS;
    iee_config.keySize = kIEE_AesCTR256XTS512;

    PRINTF("Load keys into IEE\r\n");
    /* Set KEY1 for region 0 */
    status = IEE_SetRegionKey(IEE, kIEE_Region0, kIEE_AesKey1, key1, AES_KEY_LEN);
    if (status != kStatus_Success)
    {
        PRINTF("Error set key");
        return status;
    }

    /* Set KEY2 for region 0 */
    status = IEE_SetRegionKey(IEE, kIEE_Region0, kIEE_AesKey2, key2, AES_KEY_LEN);
    if (status != kStatus_Success)
    {
        PRINTF("Error set key");
        return status;
    }

    PRINTF("Set up IEE configuration\r\n\r\n");
    /* IEE Set region 0 configuration */
    IEE_SetRegionConfig(IEE, kIEE_Region0, &iee_config);

#if USE_FLASH
    iee_config_t iee_config_flash;
    /* Configure IEE region 1 */
    IEE_GetDefaultConfig(&iee_config_flash);
    iee_config_flash.bypass  = kIEE_AesUseMdField;
    iee_config_flash.mode    = kIEE_ModeAesCTRkeystream;
    iee_config_flash.keySize = kIEE_AesCTR256XTS512;

    PRINTF("Load keys into IEE\r\n");
    /* Set KEY1 for region 1 */
    status = IEE_SetRegionKey(IEE, kIEE_Region1, kIEE_AesKey1, key1, AES_KEY_LEN);
    if (status != kStatus_Success)
    {
        PRINTF("Error set key");
        return status;
    }

    /* Set KEY2 for region 1 */
    status = IEE_SetRegionKey(IEE, kIEE_Region1, kIEE_AesKey2, aes_ctr, sizeof(aes_ctr));
    if (status != kStatus_Success)
    {
        PRINTF("Error set key");
        return status;
    }

    PRINTF("Set up IEE configuration\r\n\r\n");
    /* IEE Set region 0 configuration */
    IEE_SetRegionConfig(IEE, kIEE_Region1, &iee_config_flash);
#endif /* USE_FLASH */

    /* Note: For security reasons it's possible to lock the configuration */
    /* Only system reset can clear Lock bit */
    /* Lock the IEE region configuration */
    // IEE_LockRegionConfig(IEE, kIEE_Region0);

    /* IEE_APC lock region configuration */
    // IEE_APC_LockRegionConfig(IEE_APC, kIEE_Region0, kIEE_APC_Domain0);

    return status;
}

static bool Device_ReadWrite(void)
{
    status_t status = kStatus_Fail;

    /* Write data to OCRAM */
    PRINTF("Write plain text data to be encrypted at addess 0x%x: \r\n", StartAddr);
    Hex_Print((char *)plain_text, sizeof(plain_text));
    memset((void *)StartAddrPtr, 0, 16U);
    memcpy((void *)StartAddrPtr, (void *)plain_text, sizeof(plain_text));

    /* Invalidate memory range with endcrypted data to be read */
    DCACHE_CleanByRange(StartAddr, 32U);

    PRINTF("Turn off ecryption/decryption\r\n\r\n");
    /* Disable IEE APC routing to IEE */
    IEE_APC_GlobalDisable(IEE_APC);

    /* Invalidate memory range with endcrypted data to be read */
    DCACHE_InvalidateByRange(StartAddr, 32U);
    /* Read encrypted data */
    memcpy(s_read_buffer, StartAddrPtr, sizeof(plain_text));

    /* Test if data are encrypted */
    if (memcmp(s_read_buffer, plain_text, sizeof(plain_text)) != 0)
    {
        PRINTF("Read encrypted data, From address 0x%x :\r\n", StartAddr);
        Hex_Print((char *)s_read_buffer, sizeof(plain_text));
        status = kStatus_Success;
    }
    else
    {
        PRINTF("Data was not encryped!\r\n ");
        return kStatus_Fail;
    }

    PRINTF("Turn on ecryption/decryption\r\n\r\n");
    /* Enable IEE APC routing to IEE */
    IEE_APC_GlobalEnable(IEE_APC);

    /* Invalidate memory range with endcrypted data to be read */
    DCACHE_InvalidateByRange(StartAddr, 32U);

    /* Read decrypted data */
    memcpy(s_read_buffer, StartAddrPtr, sizeof(plain_text));

    /* Test if data are decrypted */
    if (memcmp(s_read_buffer, plain_text, sizeof(plain_text)) == 0)
    {
        PRINTF("Read decrypted data, From address 0x%x: \r\n", StartAddr);
        Hex_Print((char *)s_read_buffer, sizeof(plain_text));
        status = kStatus_Success;
    }
    else
    {
        PRINTF("Data was not decryted!\r\n ");
        return kStatus_Fail;
    }

#if USE_FLASH
    PRINTF("Read decrypted data, From address 0x%x (FLASH): \r\n", StartAddrFlash);
    DCACHE_CleanInvalidateByRange(StartAddrFlash, FLASH_PAGE_SIZE);
    memcpy(s_read_buffer, (void *)StartAddrFlash, sizeof(aes_ctr_expect));

    /* Test if data are decrypted */
    if (memcmp(s_read_buffer, aes_ctr_expect, sizeof(aes_ctr_expect)) == 0)
    {
        PRINTF("Data are decrypted sucessuly \r\n\r\n");
    }
    else
    {
        PRINTF("Data was not decryted!\r\n ");
        return kStatus_Fail;
    }

    PRINTF("Turn off ecryption/decryption\r\n\r\n");
    /* Disable IEE APC routing to IEE */
    IEE_APC_GlobalDisable(IEE_APC);
    DCACHE_CleanInvalidateByRange(StartAddrFlash, FLASH_PAGE_SIZE);
    memcpy(s_read_buffer, (void *)StartAddrFlash, sizeof(aes_ctr_cipher));

    /* Test if data are encrypted */
    if (memcmp(s_read_buffer, aes_ctr_cipher, sizeof(aes_ctr_cipher)) == 0)
    {
        PRINTF("Data are encrypted sucessuly \r\n");
    }
    else
    {
        PRINTF("Data was not encrypted!\r\n ");
        return kStatus_Fail;
    }

#endif /* USE_FLASH */

    return status;
}
