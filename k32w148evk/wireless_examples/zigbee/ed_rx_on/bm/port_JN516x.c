/****************************************************************************
 *
 * Copyright 2020 NXP
 *
 * NXP Confidential. 
 * 
 * This software is owned or controlled by NXP and may only be used strictly 
 * in accordance with the applicable license terms.  
 * By expressly accepting such terms or by downloading, installing, activating 
 * and/or otherwise using the software, you are agreeing that you have read, 
 * and that you agree to comply with and are bound by, such license terms.  
 * If you do not agree to be bound by the applicable license terms, 
 * then you may not retain, install, activate or otherwise use the software. 
 * 
 *
 ****************************************************************************/



/* Standard includes. */
#include <stdlib.h>
#include "portmacro_JN516x.h"

/* Platform specific specific includes */
#include <AppHardwareApi.h>
#include <dbg.h>
#include "MicroSpecific.h"

#if !(defined WEAK)
#define WEAK              __attribute__ ((weak))
#endif

#define DBG_PORT     TRUE

#define DBG_EXCEPTION      TRUE


/*-----------------------------------------------------------*/

/* Universal function to print the contents of the stack trace */
void vShowException(void);

/* Run time exception handlers */
void vBusErrorHandler(void);
void vAlignmentErrorHandler(void);
void vIllegalInstructionHandler(void);
void vTrapHandler(void);
void vGenericHandler(void);
void vUnclaimedException();
void vStackOverflowHandler(void);


/* Unexpected call to an ISR for an interrupt which is not enabled */
WEAK void vUnclaimedInterrupt(void)
{
    register uint32 u32Priority;


    GET_IHPR(u32Priority);

    DBG_vPrintf(DBG_EXCEPTION, "Unclaimed Interrupt at priority %d\n", u32Priority);
    vShowException(); 
}



/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/


WEAK void vBusErrorHandler(void)
{
    DBG_vPrintf(DBG_EXCEPTION , "\n!!! Bus error\n");

    vShowException();

}

/*-----------------------------------------------------------*/

WEAK void vAlignmentErrorHandler(void)
{

    DBG_vPrintf(DBG_EXCEPTION , "!!! Alignment error\n");

    vShowException();
}

/*-----------------------------------------------------------*/

WEAK void vIllegalInstructionHandler(void)
{
    DBG_vPrintf(DBG_EXCEPTION , "\n!!! Instruction error\n");

    vShowException();
}

/*-----------------------------------------------------------*/

WEAK void vTrapHandler(void)
{
    DBG_vPrintf(DBG_EXCEPTION , "\n!!! Trap error\n");

    vShowException();
}

/*-----------------------------------------------------------*/

WEAK void vGenericHandler(void)
{
    DBG_vPrintf(DBG_EXCEPTION , "\n!!! Generic error\n");

    vShowException();
}

WEAK void vUnclaimedException()
{
    DBG_vPrintf(DBG_EXCEPTION, "\n!!! Unclaimed error\n");

    vShowException();
}

/*-----------------------------------------------------------*/

WEAK void vStackOverflowHandler(void)
{
    DBG_vPrintf( DBG_EXCEPTION, "\n!!! Overflow error\n");

    vShowException();
}


/****************************************************************************
 *
 * NAME: vShowException
 *
 * DESCRIPTION:  dumps out the contents of the stack trace. 
 *               additionally some interrupt registers are also displayed for
 *               sanity check based on chip types.
 *               
 *
 * RETURNS:
 *
 ****************************************************************************/
void vShowException(void)
{
#if JENNIC_CHIP_FAMILY == JN516x
    volatile uint32 u32EPCR;
    volatile uint32 u32EEAR;
    volatile uint32 u32PICMR;
    volatile uint32 u32IHPR;
    volatile uint32 u32IPMR;
    volatile uint32 u32AINT;
    volatile uint32 u32PINT;
    volatile uint32 u32PICMSR;
    volatile uint32 u32PICSR;

    asm volatile ("l.mfspr %0,r0,0x0020" :"=r"(u32EPCR) : );
    asm volatile ("l.mfspr %0,r0,0x0030" :"=r"(u32EEAR) : );
    asm volatile ("l.mfspr %0,r0,0x4800" :"=r"(u32PICMR) : );
    asm volatile ("l.mfspr %0,r0,0x4802" :"=r"(u32PICSR) : );
    asm volatile ("l.mfspr %0,r0,0x4803" :"=r"(u32PICMSR) : );    
    asm volatile ("l.mfspr %0,r0,0x4810" :"=r"(u32IPMR) : );
    asm volatile ("l.mfspr %0,r0,0x4811" :"=r"(u32IHPR) : );
    asm volatile ("l.mfspr %0,r0,0x4812" :"=r"(u32AINT) : );
    asm volatile ("l.mfspr %0,r0,0x4813" :"=r"(u32PINT) : );
    DBG_vPrintf(DBG_EXCEPTION, "u32PICMR = %x : u32PICSR = %x\n", 
                                u32PICMR, u32PICSR);
    DBG_vPrintf(DBG_EXCEPTION, "u32PICMSR = %x : u32IPMR = %x\n", 
                                u32PICMSR, u32IPMR);
    DBG_vPrintf(DBG_EXCEPTION, "u32IHPR = %x : u32AINT = %x u32PINT %x\n", 
                                u32IHPR, u32AINT, u32PINT);
#endif
    DBG_vDumpStack();
    while (1);
}



/****************************************************************************
 *
 * NAME: ZPS_u8GrabMutexLock
 *
 * DESCRIPTION:  Provides mutex protection for function re-entrancy
 *               
 *               
 *
 * RETURNS: uint8 
 *
 ****************************************************************************/
uint8 ZPS_u8GrabMutexLock( void* hMutex , uint32* psIntStore )
{
    if( hMutex != NULL )
    {
    
        if(*( (bool_t*)(( (void* (*) (void))hMutex )())))
        {
          * ( (uint32*) (0xBADADD04) ) = 1;
        }
    }

    /* disable interrupts */
    MICRO_DISABLE_AND_SAVE_INTERRUPTS(*psIntStore);

    if( hMutex != NULL )
        (*( (bool_t*)(( (void* (*) (void))hMutex )()))) = TRUE;

    MICRO_RESTORE_INTERRUPTS(*psIntStore);

    return 0;

}


/****************************************************************************
 *
 * NAME: ZPS_u8ReleaseMutexLock
 *
 * DESCRIPTION:  Provides mutex protection for function re-entrancy
 *               
 *               
 *
 * RETURNS: uint8 
 *
 ****************************************************************************/
uint8 ZPS_u8ReleaseMutexLock( void* hMutex , uint32* psIntStore )
{

    /* disable interrupts */
    MICRO_DISABLE_AND_SAVE_INTERRUPTS(*psIntStore);

    if( hMutex != NULL )
        (*( (bool_t*)(( (void* (*) (void))hMutex )()))) = FALSE;

    MICRO_RESTORE_INTERRUPTS(*psIntStore);
    return 0;
}


/****************************************************************************
 *
 * NAME: ZPS_eEnterCriticalSection
 *
 * DESCRIPTION:  Provides protection from function re-entrancy and interrupt
 *               pre-emption.
 *               
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC uint8 ZPS_eEnterCriticalSection(void* hMutex, uint32* psIntStore)
{
    if( hMutex != NULL )
    {
    
        if(*( (bool_t*)(( (void* (*) (void))hMutex )())))
        {
          * ( (uint32*) (0xBADADD08) ) = 1;
        }
    }
    /* disable interrupts */
    MICRO_DISABLE_AND_SAVE_INTERRUPTS(*psIntStore);

    if( hMutex != NULL )
        (*( (bool_t*)(( (void* (*) (void))hMutex )()))) = TRUE;

    return 0;
}

/****************************************************************************
 *
 * NAME: ZPS_eExitCriticalSection
 *
 * DESCRIPTION:  Allows interrupt pre-emption and function re-entrancy.
 *               
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC uint8 ZPS_eExitCriticalSection(void* hMutex , uint32* psIntStore)
{
    if( hMutex != NULL )
        (*( (bool_t*)(( (void* (*) (void))hMutex )()))) = FALSE;

    MICRO_RESTORE_INTERRUPTS(*psIntStore);

    return 0;
}
/*-----------------------------------------------------------*/

