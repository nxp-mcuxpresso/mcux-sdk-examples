/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CORE1_SUPPORT_H_
#define CORE1_SUPPORT_H_

/*******************************************************************************
 * DEFINITION
 ******************************************************************************/
/* Core1 image linked to core0.*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_LK_IMAGE_START (&Image$$CORE1_REGION$$Base)
#define CORE1_LK_IMAGE_SIZE  ((uint32_t)&Image$$CORE1_REGION$$Length)
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
#define CORE1_LK_IMAGE_START (__section_begin("__core1_image"))
#define CORE1_LK_IMAGE_SIZE  ((uintptr_t)__section_end("__core1_image") - (uintptr_t)CORE1_IMAGE_START)
#elif defined(__GNUC__)
extern const char core1_image_start[];
extern const char core1_image_end[];
#define CORE1_LK_IMAGE_START ((void *)core1_image_start)
#define CORE1_LK_IMAGE_SIZE  ((uintptr_t)core1_image_end - (uintptr_t)core1_image_start)
#else
#error "Not supported compiler type"
#endif

#define CORE1_BOOT_ADDRESS       0x20600000 /* Should point to same SRAM as the core1 vector table base */

#if !defined(SDK_USE_FIXED_CORE1_ADDR)
#define CORE1_IMAGE_START CORE1_LK_IMAGE_START
#define CORE1_IMAGE_SIZE  CORE1_LK_IMAGE_SIZE
#else
/* If core1 image is separate, use fixed image address. */
#define CORE1_IMAGE_START  0x28200000 /* Should be same as the core1 image located in flash */
#define CORE1_IMAGE_SIZE   0x40000    /* Size for core1 image to copy, can be same or larger than core1
                                         binary size in bytes */
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

/*!
 * @brief Copy CPU1 image to the SRAM by specified address.
 * This API will copy the image from flash to CPU1 running SRAM.
 * NOTE, the memory address need to be writable by CPU0.
 *
 * @param addr The destination for image copy.
 */
void BOARD_CopyCore1Image(uint32_t addr);

/*!
 * @brief Power up all SRAM and Core1 in sense domain.
 */
void BOARD_ReleaseCore1Power(void);

/*!
 * @brief Boot CPU1.
 * This API will set CPU1 vector address, then enable the clock and release wait for CPU1.
 *
 * @param nsVector Non-Secure vector address.
 * @param sVector Secure vector address.
 */
void BOARD_BootCore1(uint32_t nsVector, uint32_t sVector);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* CORE1_SUPPORT_H_ */
