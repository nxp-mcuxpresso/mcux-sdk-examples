/*
 * FreeRTOS Pre-Release V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* Board specific includes. */
#include "pin_mux.h"
#include "board.h"

/* Trustzone config. */
#include "tzm_config.h"

/* FreeRTOS includes. */
#include "secure_port_macros.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief Start address of non-secure application.
 */
#define mainNONSECURE_APP_START_ADDRESS DEMO_CODE_START_NS
#if (DEMO_CODE_START_NS == 0x08100000U)
#define BOARD_InitTrustZone XIP_BOARD_InitTrustZone
#else
#define BOARD_InitTrustZone RAM_BOARD_InitTrustZone
#endif

/* Start address of non-secure application */
#define mainNONSECURE_APP_START_ADDRESS DEMO_CODE_START_NS

/**
 * @brief typedef for non-secure Reset Handler.
 */
#if defined(__IAR_SYSTEMS_ICC__)
typedef __cmse_nonsecure_call void (*NonSecureResetHandler_t)(void);
#else
typedef void (*NonSecureResetHandler_t)(void) __attribute__((cmse_nonsecure_call));
#endif
/*-----------------------------------------------------------*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Application-specific implementation of the SystemInitHook().
 */
void SystemInitHook(void);

/* Boot into the non-secure code. */
void BootNonSecure(uint32_t ulNonSecureStartAddress);
/*-----------------------------------------------------------*/

/*******************************************************************************
 * Code
 ******************************************************************************/

void SystemInitHook(void)
{
    /* The TrustZone should be configured as early as possible after RESET.
     * Therefore it is called from SystemInit() during startup. The
     * SystemInitHook() weak function overloading is used for this purpose.
     */
    BOARD_InitTrustZone();
}
/*-----------------------------------------------------------*/

void BootNonSecure(uint32_t ulNonSecureStartAddress)
{
    NonSecureResetHandler_t pxNonSecureResetHandler;

    /* Main Stack Pointer value for the non-secure side is
     * the first entry in the non-secure vector table. Read
     * the first entry and assign the same to the non-secure
     * main stack pointer(MSP_NS). */
    secureportSET_MSP_NS(*((uint32_t *)(ulNonSecureStartAddress)));

    /* Reset handler for the non-secure side is the second entry
     * in the non-secure vector table. */
    pxNonSecureResetHandler = (NonSecureResetHandler_t)(*((uint32_t *)((ulNonSecureStartAddress) + 4U)));

    /* Start non-secure state software application by jumping
     * to the non-secure reset handler. */
    pxNonSecureResetHandler();
}
/*-----------------------------------------------------------*/

/* Secure main() */
int main(void)
{
    /* SCB_AIRCR_PRIS_VAL is set to 1 in partition_ARMCM33.h to ensure
     * that non-sure exceptions are of lower priority than secure
     * exceptions. */

    /* Init board hardware. */
    /* Set non-secure vector table */
    SCB_NS->VTOR = mainNONSECURE_APP_START_ADDRESS;

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Boot the non-secure code. */
    BootNonSecure(mainNONSECURE_APP_START_ADDRESS);

    /* Non-secure software does not return, this code is not executed. */
    for (;;)
    {
        /* Should not reach here. */
    }
}
/*-----------------------------------------------------------*/
void vGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
     * away as the variables never actually get used.  If the debugger won't show the
     * values of the variables, make them global my moving their declaration outside
     * of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr;  /* Link register. */
    volatile uint32_t pc;  /* Program counter. */
    volatile uint32_t psr; /* Program status register. */
    volatile uint32_t _CFSR;
    volatile uint32_t _HFSR;
    volatile uint32_t _DFSR;
    volatile uint32_t _AFSR;
    volatile uint32_t _SFSR;
    volatile uint32_t _BFAR;
    volatile uint32_t _MMAR;
    volatile uint32_t _SFAR;

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr  = pulFaultStackAddress[5];
    pc  = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* Configurable Fault Status Register. Consists of MMSR, BFSR and UFSR. */
    _CFSR = (*((volatile unsigned long *)(0xE000ED28)));

    /* Hard Fault Status Register. */
    _HFSR = (*((volatile unsigned long *)(0xE000ED2C)));

    /* Debug Fault Status Register. */
    _DFSR = (*((volatile unsigned long *)(0xE000ED30)));

    /* Auxiliary Fault Status Register. */
    _AFSR = (*((volatile unsigned long *)(0xE000ED3C)));

    /* Secure Fault Status Register. */
    _SFSR = (*((volatile unsigned long *)(0xE000EDE4)));

    /* Read the Fault Address Registers. Note that these may not contain valid
     * values. Check BFARVALID/MMARVALID to see if they are valid values. */
    /* MemManage Fault Address Register. */
    _MMAR = (*((volatile unsigned long *)(0xE000ED34)));

    /* Bus Fault Address Register. */
    _BFAR = (*((volatile unsigned long *)(0xE000ED38)));

    /* Secure Fault Address Register. */
    _SFAR = (*((volatile unsigned long *)(0xE000EDE8)));

    /* Remove compiler warnings about the variables not being used. */
    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;
    (void)r12;
    (void)lr;  /* Link register. */
    (void)pc;  /* Program counter. */
    (void)psr; /* Program status register. */
    (void)_CFSR;
    (void)_HFSR;
    (void)_DFSR;
    (void)_AFSR;
    (void)_SFSR;
    (void)_MMAR;
    (void)_BFAR;
    (void)_SFAR;

    /* When the following line is hit, the variables contain the register values. */
    for (;;)
    {
    }
}
/*-----------------------------------------------------------*/
