/*
 * Copyright  2017-2018,  NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

        .syntax unified
        .arch armv7-m

        .section TextLpm, "ax"               /* The LPM_Resume, LPM_ResumeWithBackup and RSTACK should be in always retained memory. */ 

        .align 2
        .thumb
        
        .global  LPM_Suspend
        .global  LPM_Resume
        .global  LPM_ResumeWithBackup
        
        .extern  __vector_table
        .extern  SystemInit
        .extern  LPM_NvicStateSave
        .extern  LPM_NvicStateRestore
        .extern  LPM_SystemSuspend
        .extern  LPM_SystemResume
        .extern  __BACKUP_REGION_START

.type LPM_ConfigDDRAlias, %function
LPM_ConfigDDRAlias:
        LDR     R0, =0xE0080030
        MOVS    R1, #0x7E
        STR     R1, [R0]
        ISB
        BX      LR

.type LPM_Suspend, %function      
LPM_Suspend:
        /* Primask has been set before entering this function */
        PUSH    {R4-R11, LR}                 /* Save return context */
        MRS     R0, CONTROL
        PUSH    {R0}                         /* Save CONTROL register */
        
        LDR     R1, =LPM_NvicStateSave
        BLX     R1
        
        MOV     R0, SP                       /* Set 1st param to SP */
        LDR     R2, =__RStackTop             /*  Change SP to Resume Stack(AON region) */
        MOV     SP, R2

        LDR     R1, =LPM_SystemSuspend
        BLX     R1                           /* Won't return unless wakeup before suspend */

        MOV     R0, #0                       /* Set resume parameter to false */
        LDR     R1, =LPM_SystemResume
        BLX     R1                           /* Suspend failed, resume system */
                                             /* If there's master active(such as A core), partial LLS/VLLS is entered though return fail */
        
        /* NVIC didn't change in LPM_SystemSuspend, no recovery needed. */
        MOV     SP, R0                       /* Recover SP */
        POP     {R0}                         /* CONTROL not changed, discard the data in stack */
        POP     {R4-R11, LR}
        MOV     R0, #0                       /* Return false means suspend failure */
        BX      LR

.type LPM_Resume, %function
LPM_Resume:
        CPSID   I

#ifdef __STARTUP_CONFIG_DDR_ALIAS
        LDR     R1, =LPM_ConfigDDRAlias
        BLX     R1
#endif
        LDR     R0, =0x1FFE0000
        LDR     R1, =__BACKUP_REGION_START
        LDMIA   R1, {R4-R11}
        STMIA   R0, {R4-R11}                 /* Restore the exception vector */
        LDR     R0, =0xE000ED08
        LDR     R1, =__isr_vector
        STR     R1, [R0]
        LDR     R2, =__RStackTop             /*  Initialize resume stack, uses resume stack. */
        MOV     SP, R2

        LDR     R0, =SystemInit
        BLX     R0                           /*  Call SystemInit to initialize SoC */
        MOV     R0, #1                       /*  Set resume parameter to true */
        LDR     R1, =LPM_SystemResume
        BLX     R1                           /*  Resume from VLLS, return PSP */
        MOV     R4, R0                       /*  Save PSP to R4 */

        LDR     R1, =LPM_NvicStateRestore
        BLX     R1                           /*  Restore NVIC */

        MOV     R0, R4
        LDMIA   R0!, {R1, R4-R11, LR}        /*  Restore core registers, R1 stores CONTROL value */

        /*  Restore CONTROL and return */
#ifdef FSL_RTOS_FREE_RTOS
        MSR     PSP, R0                      /*  Recover SP. Recover MSP for bare metal apps, PSP for RTOS environment */
#else
        MSR     MSP, R0
#endif
        MSR     CONTROL, R1                  /*  Recover CONTROL */
        ISB
        MOV     R0, #1                       /*  Return true means suspend successfully. */
        BX      LR                           /*  Primask will be cleared outside this function */

.type LPM_ResumeWithBackup, %function
LPM_ResumeWithBackup:
        CPSID   I
#ifdef __STARTUP_CONFIG_DDR_ALIAS
        LDR     R1, =LPM_ConfigDDRAlias
        BLX     R1
#endif

        /* Disable WDOG */
        LDR    R0, =0x41420000               /* WDOG base */
        LDR    R1, =0xD928C520               /* WDOG_UPDATE_KEY */
        STR    R1, [R0, #4]
        MOV    R1, #0xFFFF
        STR    R1, [R0, #8]
        LDR    R1, [R0, #0]
        BICS   R1, R1, #0x80
        ORRS   R1, R1, #0x20
        STR    R1, [R0]

        /* Enable System cache to accelerate data copy */
        LDR    R0, =0xE0082800               /* LMEM PS cache control register base */
        LDR    R1, [R0]
        ORR    R1, R1, #0x85000000           /* Set INVW1, INVW0, GO bit, start cache command */
        STR    R1, [R0]
.LC0:
        LDR    R1, [R0]
        CMP    R1, #0                        /* Wait Go bit clear */
        BMI    .LC0
        ORR    R1, R1, #0x3                  /* Enable System cache and write buffer */
        STR    R1, [R0]

        LDR    R1, =0x1FFE0000               /* Restore the TCM content, TCM start */
        LDR    R2, =__BACKUP_REGION_START
        LDR    R3, =0x40000                  /* TCM size */
.LC1:
        SUBS    R3, #0x10
        LDMIA   R2!, {R4-R7}
        STMIA   R1!, {R4-R7}
        BGT    .LC1
        
        LDR     R0, =0xE000ED08
        LDR     R1, =__isr_vector
        STR     R1, [R0]
        LDR     R2, =__RStackTop             /*  Initialize resume stack, which is same as startup stack. */
        MOV     SP, R2

        LDR     R0, =SystemInit
        BLX     R0                           /*  Call SystemInit to initialize SoC */
        MOV     R0, #1                       /*  Set resume parameter to true */
        LDR     R1, =LPM_SystemResume

        BLX     R1                           /*  Resume from VLLS, return SP */
        MOV     R4, R0                       /*  Save PSP to R4 */
        LDR     R1, =LPM_NvicStateRestore
        BLX     R1                           /*  Restore NVIC */

        MOV     R0, R4
        LDMIA   R0!, {R1, R4-R11, LR}        /*  Restore core registers, R1 stores CONTROL value */

        /*  Restore CONTROL and return */
#ifdef FSL_RTOS_FREE_RTOS
        MSR     PSP, R0                      /*  Recover SP  */
#else
        MSR     MSP, R0
#endif
        MSR     CONTROL, R1                  /*  Recover CONTROL */
        ISB
        MOV     R0, #1                       /*  Return true means suspend successfully. */
        BX      LR                           /*  Primask will be cleared outside this function */

        .end
