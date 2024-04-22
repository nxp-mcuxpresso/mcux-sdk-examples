/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcmgr.h"

#include "fsl_ele_base_api.h"
#include "fsl_trdc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* When CM33 set TRDC, CM7 must NOT require TRDC ownership from ELE */
#define CM33_SET_TRDC 0U

/*
 * This workaround is used for kick-off CM7 when CM7 image vector table is not
 * at 0x0 and CM7 TCM ECC fuse is set(1).
 * When CM7 TCM ECC fuse is unset(0), this workaround is useless and no side
 * effect when applied.
 */
#define ENABLE_WORKAROUND_CM7_KICK_OFF 1U

#define CONTAINER_BASE_ADDR   0x38001000UL /* For FlexSPI, base addr(0x38000000) + 0x1000 offset */
#define CONTAINER_MAX_NUM     2U
#define CONTAINER_SIZE        0x400U /* in bytes */
#define CONTAINER_HEADER_SIZE 16U    /* in bytes */
#define IMAGE_ENTRY_SIZE      0x80U  /* in bytes */
#define IMAGE_TAG_NUM         0x87U
#define IMAGE_TYPE_EXECUTABLE 0x03U
#define CORE_ID_CM7           0x02U

#define CM7_ITCM_START_ADDR              0x0UL      /* from pespective of CM7 */
#define CM7_ITCM_END_ADDR                0x80000UL  /* from pespective of CM7 */
#define CM7_ITCM_ADDR_MAP_AT_CM33_DOMAIN 0x303C0000 /* from pespective of CM33 */
#define FLEXSPI_START_ADDR_NS            0x28000000UL
#define FLEXSPI_END_ADDR_NS              0x30000000UL
#define FLEXSPI_START_ADDR_S             0x38000000UL
#define FLEXSPI_END_ADDR_S               0x40000000UL

#define ELE_TRDC_AON_ID    0x74
#define ELE_TRDC_WAKEUP_ID 0x78
#define ELE_CORE_CM33_ID   0x1
#define ELE_CORE_CM7_ID    0x2

/*
 * Set ELE_STICK_FAILED_STS to 0 when ELE status check is not required,
 * which is useful when debug reset, where the core has already get the
 * TRDC ownership at first time and ELE is not able to release TRDC
 * ownership again for the following TRDC ownership request.
 */
#define ELE_STICK_FAILED_STS 1

#if ELE_STICK_FAILED_STS
#define ELE_IS_FAILED(x) (x != kStatus_Success)
#else
#define ELE_IS_FAILED(x) false
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_GetCore1ImageAddrSize(uint32_t *pImageSrcAddr,
                                     uint32_t *pImageDestAddr,
                                     uint32_t *pImageSize,
                                     uint32_t *pImageBootAddr);
void BOARD_PrepareCore1(uint32_t image_src_addr, uint32_t image_dest_addr, uint32_t image_size, uint32_t boot_addr);

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Get CM7 Image Address and size information from container header
 *
 * Container Header structure are shown as the following
 *
 *         31                    23                   15                   7                   0
 *         +--------------------+--------------------+--------------------+--------------------+
 *     0x0 |     Tag (0x87)     |                  Length                 |   Version (0x00)   |
 *         +--------------------+--------------------+--------------------+--------------------+
 *     0x4 |                                      SRK Flags                                    |
 *         +--------------------+--------------------+--------------------+--------------------+
 *     0x8 |  Num of Images     |   Fuse Version     |                 SW Version              |
 *         +--------------------+--------------------+--------------------+--------------------+
 *     0xC |         R          |                 Signature Block Offset                       |
 *         +--------------------+--------------------+--------------------+--------------------+
 *         |                                                                                   |
 *     +   |                                  Image Array Entry                                |
 *         |                                                                                   |
 *         +--------------------+--------------------+--------------------+--------------------+
 *         |                                                                                   |
 *     +   |                                   Signature Block                                 |
 *         |                                                                                   |
 *         +--------------------+--------------------+--------------------+--------------------+
 *
 * Its' image entry are further layout as the following
 *
 *                             +0x4                 +0x8                 +0xC
 *         +--------------------+--------------------+--------------------+--------------------+
 *     0x0 |   Image Offset     |    Image Size      |            Load Address                 |
 *         +--------------------+--------------------+--------------------+--------------------+
 *    0x10 |                Entry Point              |      Flags         |       Meta         |
 *         +--------------------+--------------------+--------------------+--------------------+
 *    0x20 |                                                                                   |
 *         |                                        HASH                                       |
 *         |                                                                                   |
 *         +--------------------+--------------------+--------------------+--------------------+
 *    0x60 |                                         R                                         |
 *         +--------------------+--------------------+--------------------+--------------------+
 *
 * Its' Flag contains the Image Type and Core
 *
 *         31                    23                   15         11        7         3         0
 *         +--------------------+--------------------+----------+---------+----------+---------+
 *   Flags |                             R                      |   H/E   |   Core   |   Type  |
 *         +--------------------+--------------------+--------------------+----------+---------+
 *
 * The function iterate through each container header and its image entries until it find
 * CM7 Exexutable Image. The information of the image is then returned to user
 *
 * More information can be found in RM "System Boot" Chapter
 *
 * @param pImageSrcAddr CM7 Image Srouce Address from CM33 perspective
 * @param pImageDestAddr CM7 Image Destination Address from CM33 perspective
 * @param pImageSize CM7 Image Size
 * @param pImageBootAddr CM7 Image Load Address from CM7 prespective
 */
status_t BOARD_GetCore1ImageAddrSize(uint32_t *pImageSrcAddr,
                                     uint32_t *pImageDestAddr,
                                     uint32_t *pImageSize,
                                     uint32_t *pImageBootAddr)
{
    /* parser the container to get the CM7 image location and size */

    uint32_t i, j;

    uint32_t image_src_addr  = 0U;
    uint32_t image_dest_addr = 0U;
    uint32_t image_size      = 0U;
    uint32_t image_boot_addr = 0U;

    status_t ret = kStatus_Fail;

    uint32_t *pContainer = (uint32_t *)CONTAINER_BASE_ADDR;

    for (i = 0U; i < CONTAINER_MAX_NUM; i++)
    {
        uint32_t image_num, image_tag = pContainer[0] >> 24U;

        if (image_tag != IMAGE_TAG_NUM)
        {
            break;
        }

        image_num = pContainer[2] >> 24U;
        if (image_num >= 2U)
        {
            uint32_t *pImage = &pContainer[CONTAINER_HEADER_SIZE / sizeof(uint32_t)];
            for (j = 0U; j < image_num; j++)
            {
                uint32_t image_type = pImage[6] & 0xFU;
                uint32_t image_core = (pImage[6] >> 4U) & 0xFU;

                if ((image_type == IMAGE_TYPE_EXECUTABLE) && (image_core == CORE_ID_CM7))
                {
                    /* pImage[0] is the image offset, which is based on current container header */
                    image_src_addr  = ((uint32_t)pContainer) + pImage[0];
                    image_size      = pImage[1];
                    image_dest_addr = pImage[2];
                    image_boot_addr = pImage[4];
                    ret             = kStatus_Success;
                    break;
                }

                pImage += IMAGE_ENTRY_SIZE / sizeof(uint32_t);
            }
        }

        pContainer += CONTAINER_SIZE / sizeof(uint32_t);
    }

    if (pImageSrcAddr != NULL)
    {
        *pImageSrcAddr = image_src_addr;
    }

    if (pImageDestAddr != NULL)
    {
        *pImageDestAddr = image_dest_addr;
    }

    if (pImageSize != NULL)
    {
        *pImageSize = image_size;
    }

    if (pImageBootAddr != NULL)
    {
        *pImageBootAddr = image_boot_addr;
    }

    return ret;
}

#if (defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U))
static void TRDC_SetAllPermissions(void)
{
#define EDMA_DID 0x7U

    uint8_t i, j;

    /* Set the master domain access configuration for eDMA3 and eDMA4 */
    trdc_non_processor_domain_assignment_t edmaAssignment;

    (void)memset(&edmaAssignment, 0, sizeof(edmaAssignment));
    edmaAssignment.domainId       = EDMA_DID;
    edmaAssignment.privilegeAttr  = kTRDC_MasterPrivilege;
    edmaAssignment.secureAttr     = kTRDC_ForceSecure;
    edmaAssignment.bypassDomainId = true;
    edmaAssignment.lock           = false;

    TRDC_SetNonProcessorDomainAssignment(TRDC1, kTRDC1_MasterDMA3, &edmaAssignment);
    TRDC_SetNonProcessorDomainAssignment(TRDC2, kTRDC2_MasterDMA4, &edmaAssignment);

    /* Enable all access modes for MBC and MRC of TRDCA and TRDCW */
    trdc_hardware_config_t hwConfig;
    trdc_memory_access_control_config_t memAccessConfig;

    (void)memset(&memAccessConfig, 0, sizeof(memAccessConfig));
    memAccessConfig.nonsecureUsrX  = 1U;
    memAccessConfig.nonsecureUsrW  = 1U;
    memAccessConfig.nonsecureUsrR  = 1U;
    memAccessConfig.nonsecurePrivX = 1U;
    memAccessConfig.nonsecurePrivW = 1U;
    memAccessConfig.nonsecurePrivR = 1U;
    memAccessConfig.secureUsrX     = 1U;
    memAccessConfig.secureUsrW     = 1U;
    memAccessConfig.secureUsrR     = 1U;
    memAccessConfig.securePrivX    = 1U;
    memAccessConfig.securePrivW    = 1U;
    memAccessConfig.securePrivR    = 1U;

    TRDC_GetHardwareConfig(TRDC1, &hwConfig);
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }
    }

    for (j = 0U; j < hwConfig.mrcNumber; j++)
    {
        TRDC_MrcDomainNseClear(TRDC1, j, 1UL << EDMA_DID);
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC1, &memAccessConfig, i, j);
        }
    }

    for (j = 0U; j < hwConfig.mbcNumber; j++)
    {
        TRDC_MbcNseClearAll(TRDC1, j, 1UL << EDMA_DID, 0xFU);
    }

    TRDC_GetHardwareConfig(TRDC2, &hwConfig);
    for (i = 0U; i < hwConfig.mrcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MrcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }

    for (j = 0U; j < hwConfig.mrcNumber; j++)
    {
        TRDC_MrcDomainNseClear(TRDC2, j, 1UL << EDMA_DID);
    }

    for (i = 0U; i < hwConfig.mbcNumber; i++)
    {
        for (j = 0U; j < 8; j++)
        {
            TRDC_MbcSetMemoryAccessConfig(TRDC2, &memAccessConfig, i, j);
        }
    }

    for (j = 0U; j < hwConfig.mbcNumber; j++)
    {
        TRDC_MbcNseClearAll(TRDC2, j, 1UL << EDMA_DID, 0xFU);
    }
}
#endif /* (defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U)) */

/*
 * image_src_addr image_dest_addr is address from perspective of core0(CM33)
 * boot_addr is address from perspective of core1(CM7)
 */
void BOARD_PrepareCore1(uint32_t image_src_addr, uint32_t image_dest_addr, uint32_t image_size, uint32_t boot_addr)
{
    if ((boot_addr >= CM7_ITCM_END_ADDR) || ((CM7_ITCM_START_ADDR > 0U) && (boot_addr < CM7_ITCM_START_ADDR)))
    {
        /* Out of CM7 ITCM address space, CM7 core and its TCM ECC probably not initialized yet, do it here */
        Prepare_CM7(boot_addr);

#if (defined(ENABLE_WORKAROUND_CM7_KICK_OFF) && (ENABLE_WORKAROUND_CM7_KICK_OFF > 0U))
        /*
         * When CM7 TCM ECC fuse is set(1)
         *   ROM will powerup CM7 domain, CM7 VTOR is settled(always 0) only when powerup,
         *   so CM33 image has no chance to update CM7 VTOR. Thus those CM7 image with
         *   vector table is not located at 0x0, boot fails.
         *   The workaround here, is to copy two header word of image vector(normally SP
         *   and Reset_Handler) to CM7 initial VTOR addr(0x0), as a fake vector table and stepping-stone,
         *   then CM7 boot successfully.
         *
         * Note:
         *   1. The fake vector table is one-time used, memory(0x0~0x07) is released when CM7
         *      starts to execute Reset_Handler.
         *   2. CM7 Reset_Handler will update CM7 VTOR to real vector addr of the CM7 image.
         *   3. The workaround doesn't take effect(also no side effect), when CM7 TCM ECC fuse is set(0)
         */
        uint32_t *pRamVect  = (uint32_t *)CM7_ITCM_ADDR_MAP_AT_CM33_DOMAIN;
        uint32_t *pBootVect = (uint32_t *)boot_addr;
        pRamVect[0]         = pBootVect[0];
        pRamVect[1]         = pBootVect[1];
#endif /* (defined(ENABLE_WORKAROUND_CM7_KICK_OFF) && (ENABLE_WORKAROUND_CM7_KICK_OFF > 0U)) */
    }

#if (defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U))
    status_t sts;

    /* Get ELE FW status */
    do
    {
        uint32_t ele_fw_sts;
        sts = ELE_BaseAPI_GetFwStatus(MU_RT_S3MUA, &ele_fw_sts);
    } while (sts != kStatus_Success);

    /* Release TRDC A to CM33 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_AON_ID, ELE_CORE_CM33_ID);
    } while (ELE_IS_FAILED(sts));

    /* Release TRDC W to CM33 core */
    do
    {
        sts = ELE_BaseAPI_ReleaseRDC(MU_RT_S3MUA, ELE_TRDC_WAKEUP_ID, ELE_CORE_CM33_ID);
    } while (ELE_IS_FAILED(sts));

    TRDC_SetAllPermissions();
#endif /* (defined(CM33_SET_TRDC) && (CM33_SET_TRDC > 0U)) */
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t core1_image_size, core1_image_source_addr, core1_image_dest_addr;
    uint32_t core1_boot_addr;

    /* Init board hardware. */
    MCMGR_EarlyInit();

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    (void)MCMGR_Init();

    (void)PRINTF("\r\nMulticore trigger demo!\r\n");

    if (kStatus_Success != BOARD_GetCore1ImageAddrSize(&core1_image_source_addr, &core1_image_dest_addr,
                                                       &core1_image_size, &core1_boot_addr))
    {
        (void)PRINTF("Core0 failed to get core1 image info!\r\n");
    }
    else
    {
        (void)PRINTF("Core0 get core1 image info sucessfully!\r\n");
        (void)PRINTF("  Core1 image reside addr = 0x%x\r\n", core1_image_source_addr);
        (void)PRINTF("  Core1 image dest addr   = 0x%x\r\n", core1_image_dest_addr);
        (void)PRINTF("  Core1 image size        = %u(~%uK) bytes\r\n", core1_image_size,
                     (core1_image_size + 1023UL) / 1024UL);
        (void)PRINTF("  Core1 image boot addr   = 0x%x\r\n", core1_boot_addr);

        BOARD_PrepareCore1(core1_image_source_addr, core1_image_dest_addr, core1_image_size, core1_boot_addr);

        /* Boot Secondary core application */
        (void)PRINTF("Core0 is starting core1...\r\n\r\n");
        (void)MCMGR_StartCore(kMCMGR_Core1, (void *)core1_boot_addr, 2, kMCMGR_Start_Synchronous);
        (void)PRINTF("Core1 application has been started.\r\n\r\n");
    }

    while (1)
    {
    }
}
