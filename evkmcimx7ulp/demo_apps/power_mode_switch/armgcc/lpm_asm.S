/*
 * Copyright 2017-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

        .syntax unified
        .arch armv7-m
#if FSL_SDK_DRIVER_QUICK_ACCESS_ENABLE
        .section CodeQuickAccess, "ax"
#else
        .text
#endif
        .extern  LPM_NvicStateSave
        .extern  LPM_NvicStateRestore
        .extern  LPM_SystemSuspend
        .extern  LPM_SystemResume
        .extern  SystemInit
        .extern  __isr_vector

        .thumb

        .thumb_func
        .align 2
        .globl LPM_Suspend
        .type LPM_Suspend, %function
LPM_Suspend:
        /* Primask has been set before entering this function */
        PUSH    {R4-R11, LR}                 /* Save task return context */
        MRS     R0, CONTROL
        PUSH    {R0}                         /* Save CONTROL register */

        /* Save NVIC */
        LDR     R1, =LPM_NvicStateSave
        BLX     R1

        LDR     R1, =0x1FFD1C00
        /* Save IOMUXC0 MUX register */
        LDR     R2, =0x4103D000
        LDR     R3, =0x4103D0D0
LPM_S0:
        CMP     R2, R3
        ITTT    LT
        LDRLT   R0, [R2], #4
        STRLT   R0, [R1], #4
        BLT     LPM_S0
        /* Save IOMUXC0 SELECT INPUT register */
        LDR     R2, =0x4103D100
        LDR     R3, =0x4103D22C
LPM_S1:
        CMP     R2, R3
        ITTT    LT
        LDRLT   R0, [R2], #4
        STRLT   R0, [R1], #4
        BLT     LPM_S1
        /* Save GPIOA register */
        LDR     R2, =0x4100F000
        LDR     R0, [R2, #0x0]  /* PDOR */
        STR     R0, [R1], #4
        LDR     R0, [R2, #0x14] /* PDDR */
        STR     R0, [R1], #4
        /* Save GPIOB register */
        LDR     R2, =0x4100F040
        LDR     R0, [R2, #0x0]  /* PDOR */
        STR     R0, [R1], #4
        LDR     R0, [R2, #0x14] /* PDDR */
        STR     R0, [R1], #4

        MOV     R0, SP                       /* Set 1st param to SP */
        LDR     R1, =LPM_SystemSuspend
        BLX     R1                           /* Won't return unless wakeup before suspend */

        MOV     R0, #0                       /* Set resume parameter to false */
        LDR     R1, =LPM_SystemResume
        BLX     R1                           /* Suspend failed, resume system */

        /* NVIC didn't change in LPM_SystemSuspend, no recovery needed. */

        POP     {R0}                         /* CONTROL not changed, discard the data in stack. */
        POP     {R4-R11, LR}
        MOV     R0, #0                       /* Return false means suspend failure. */
        BX      LR

        .thumb_func
        .align 2
        .globl LPM_Resume
        .type LPM_Resume, %function
LPM_Resume:
        CPSID   I

        /* Enable GPIO clock */
        LDR     R1, =0x40000000
        LDR     R0, =0x41026000
        STR     R1, [R0, #0x3C]  /* PCC0->RGPIO2P0 = PCC_CLKCFG_CGC_MASK; */
        STR     R1, [R0, #0xFC]  /* PCC0->PCTLA = PCC_CLKCFG_CGC_MASK; */
        STR     R1, [R0, #0x100] /* PCC0->PCTLB = PCC_CLKCFG_CGC_MASK; */

        LDR     R1, =0x1FFD1C00
        /* Restore IOMUXC0 MUX register */
        LDR     R2, =0x4103D000
        LDR     R3, =0x4103D0D0
LPM_R0:
        CMP     R2, R3
        ITTT    LT
        LDRLT   R0, [R1], #4
        STRLT   R0, [R2], #4
        BLT     LPM_R0
        /* Restore IOMUXC0 SELECT INPUT register */
        LDR     R2, =0x4103D100
        LDR     R3, =0x4103D22C
LPM_R1:
        CMP     R2, R3
        ITTT    LT
        LDRLT   R0, [R1], #4
        STRLT   R0, [R2], #4
        BLT     LPM_R1
        /* Restore GPIOA register */
        LDR     R2, =0x4100F000
        LDR     R0, [R1], #4
        STR     R0, [R2, #0x0]  /* PDOR */
        LDR     R0, [R1], #4
        STR     R0, [R2, #0x14] /* PDDR */
        /* Restore GPIOB register */
        LDR     R2, =0x4100F040
        LDR     R0, [R1], #4
        STR     R0, [R2, #0x0]  /* PDOR */
        LDR     R0, [R1], #4
        STR     R0, [R2, #0x14] /* PDDR */
        /* Clear pad isolation */
        LDR     R0, =0x410A1028
        LDR     R1, [R0]
        ORR     R1, R1, #0x4000
        STR     R1, [R0]

        /* Judge if the code is running in QSPI flash region [0x04000000, 0x0C000000]. */
        /* If so, need to initialize QSPI first. */
        LDR     R1, =SystemInit
        LDR     R0, =0x04000000
        CMP     R1, R0
        BCC     LPM_MediaReady
        LDR     R0, =0x0C000000
        CMP     R1, R0
        BCS     LPM_MediaReady

        BL      LPM_InitQSPI

LPM_MediaReady:
        LDR     R0, =0xE000ED08
        LDR     R1, =__isr_vector
        STR     R1, [R0]
        LDR     R2, [R1]                     /* Initialize resume stack, which is same as startup stack. */
        MSR     MSP, R2

        LDR     R0, =SystemInit
        BLX     R0                           /* Call SystemInit to initialize SoC */
        MOV     R0, #1                       /* Set resume parameter to true */
        LDR     R1, =LPM_SystemResume
        BLX     R1                           /* Resume from VLLS, return PSP */
        MOV     R4, R0                       /* Save PSP to R4 */

        LDR     R1, =LPM_NvicStateRestore
        BLX     R1                           /* Restore NVIC */

        MOV     R0, R4
        LDMIA   R0!, {R1, R4-R11, LR}        /* Restore core registers, R1 stores CONTROL value */

        /* Restore CONTROL and return */
        MSR     PSP, R0                      /* Recover PSP */
        MSR     CONTROL, R1                  /* Recover CONTROL */
        ISB
        MOV     R0, #1                       /* Return true means suspend successfully. */
        BX      LR                           /* Primask will be cleared outside this function */

LPM_InitQSPI:
        LDR     R0, =0x41027304
        MOV     R1, #0x1
        STR     R1, [R0]                     /* Set FIRC async plat divider to 1 */

        LDR     R0, =0x410B2094
        LDR     R1, =0x43000000
        STR     R1, [R0]                     /* Select QSPI clock to SOSC DIV1 and enable it */

        LDR     R0, =0x410A5000
        LDR     R1, [R0]
        BIC     R1, R1, #0x4000
        STR     R1, [R0]                     /* MCR: QSPIA Enable */

        ISB
        DSB
        BX      LR

        .end
