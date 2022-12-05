/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdlib.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_puf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE_CLK_FREQ CLOCK_GetFreq(kCLOCK_CoreSysClk)
/* Worst-case time in ms to fully discharge PUF SRAM */
#define PUF_DISCHARGE_TIME 400
#define PUF                KEY_MANAGER__PUF
#define PUF_INTRINSIC_KEY_SIZE 16

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* User key in little-endian format. */
/* 32 bytes key for ECB method: "Thispasswordisveryuncommonforher". */
static const uint8_t s_userKey256[] __attribute__((aligned)) = {
    0x72, 0x65, 0x68, 0x72, 0x6f, 0x66, 0x6e, 0x6f, 0x6d, 0x6d, 0x6f, 0x63, 0x6e, 0x75, 0x79, 0x72,
    0x65, 0x76, 0x73, 0x69, 0x64, 0x72, 0x6f, 0x77, 0x73, 0x73, 0x61, 0x70, 0x73, 0x69, 0x68, 0x54};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_PUF_Enable(void)
{
    /* Enable PUF in shadow registers */
    uint32_t fuse    = 0;
    uint32_t enabled = 0;

    /* Load walues from OCOTP */
    fuse = OCOTP->FUSEN[6].FUSE;

    enabled = (fuse & 0x00000040) >> 6U;

    /* Check whether PUF is disabled and FUSE word locked */
    if (enabled == 0)
    {
        PRINTF("Error: PUF is not enabled in FUSE (0x860[6])!/r/n");
        while (1)
            ;
    }

    /* Enable PUF in FUSE 6 shadow register (no FUSE itself) */
}


/*!
 * @brief Main function.
 */
int main(void)
{
    status_t result;
    uint8_t activationCode[PUF_ACTIVATION_CODE_SIZE];
    uint8_t keyCode0[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(32)];
    uint8_t keyCode1[PUF_GET_KEY_CODE_SIZE_FOR_KEY_SIZE(PUF_INTRINSIC_KEY_SIZE)];
    uint8_t intrinsicKey[PUF_INTRINSIC_KEY_SIZE] = {0};

    /* Init hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_PUF_Enable();

    /* Initialize random number generator used to generate key mask for HW key */
    /* In real application, the seed shall be obtained from a hardware random number generator. */
    srand(0xbabadeda);
    PRINTF("PUF Peripheral Driver Example\r\n\r\n");

    /* PUF SRAM Configuration*/
    puf_config_t conf;
    PUF_GetDefaultConfig(&conf);

    /* Initialize PUF peripheral */
    result = PUF_Init(PUF, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error Initializing PUF!\r\n");
        goto cleanup;
    }

    /* Perform enroll to get device specific PUF activation code */
    /* Note: Enroll operation is usually performed only once for each device. */
    /* Activation code is stored and used in Start operation */
    result = PUF_Enroll(PUF, activationCode, sizeof(activationCode));
    if (result == kStatus_EnrollNotAllowed)
    {
        PRINTF("Enroll is not allowed!\r\n");
        PRINTF("Will make powercycle and enroll again!\r\n");
        (void)PUF_PowerCycle(PUF, &conf);
        result = PUF_Enroll(PUF, activationCode, sizeof(activationCode));
    }

    if (result != kStatus_Success)
    {
        PRINTF("Error during Enroll!\r\n");
        goto cleanup;
    }

    PUF_Deinit(PUF, &conf);

    /* Reinitialize PUF after enroll */
    result = PUF_Init(PUF, &conf);
    if (result != kStatus_Success)
    {
        PRINTF("Error Initializing PUF!\r\n");
        goto cleanup;
    }

    /* Start PUF by loading generated activation code */
    result = PUF_Start(PUF, activationCode, sizeof(activationCode));
    if (result == kStatus_StartNotAllowed)
    {
        PRINTF("Start is not allowed!\r\n");
        PRINTF("Will make powercycle and start again!\r\n");
        (void)PUF_PowerCycle(PUF, &conf);
        result = PUF_Start(PUF, activationCode, sizeof(activationCode));
    }

    if (result != kStatus_Success)
    {
        PRINTF("Error during Start !\r\n");
        goto cleanup;
    }

    /* Create keycode for user key with index 0 */
    /* Index 0 selects that the key shall be ouptut (by PUF_GetHwKey()) to a SoC specific private hardware bus. */
    result = PUF_SetUserKey(PUF, kPUF_KeyIndex_00, s_userKey256, 32, keyCode0, sizeof(keyCode0));
    if (result != kStatus_Success)
    {
        PRINTF("Error setting user key!\r\n");
        goto cleanup;
    }

    /* Generate new intrinsic key with index 1 */
    result = PUF_SetIntrinsicKey(PUF, kPUF_KeyIndex_01, PUF_INTRINSIC_KEY_SIZE, keyCode1, sizeof(keyCode1));
    if (result != kStatus_Success)
    {
        PRINTF("Error setting intrinsic key!\r\n");
        goto cleanup;
    }

    /* Reconstruct key from keyCode0 to HW bus for crypto module */
    result = PUF_GetHwKey(PUF, keyCode0, sizeof(keyCode0), kPUF_KeySlot0, rand());
    if (result != kStatus_Success)
    {
        PRINTF("Error reconstructing key to HW bus!\r\n");
        goto cleanup;
    }

    /* Reconstruct intrinsic key from keyCode1 generated by PUF_SetIntrinsicKey() */
    result = PUF_GetKey(PUF, keyCode1, sizeof(keyCode1), intrinsicKey, sizeof(intrinsicKey));
    if (result != kStatus_Success)
    {
        PRINTF("Error reconstructing intrinsic key!\r\n");
        goto cleanup;
    }

    PRINTF("Reconstructed intrinsic key = ");
    for (int i = 0; i < PUF_INTRINSIC_KEY_SIZE; i++)
    {
        PRINTF("%x ", intrinsicKey[i]);
    }

cleanup:
    PUF_Deinit(PUF, &conf);
    PRINTF("\r\n\r\nExample end.\r\n");

    while (1)
    {
    }
}
