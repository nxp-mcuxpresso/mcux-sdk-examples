/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   MCIMX6 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_mcimx6.h"
#include "ux_device_stack.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                                 RELEASE      */
/*                                                                        */
/*    _ux_dcd_mcimx6_interrupt_handler                      PORTABLE C    */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the interrupt handler for the MCIMX6 controller.   */
/*    The controller will trigger an interrupt when something happens on  */
/*    an endpoint whose mask has been set in the interrupt enable         */
/*    register. We do very little in this ISR but rather relay the work   */
/*    to a thread.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_read             Read register              */
/*    _ux_dcd_mcimx6_register_write            Write register             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Device Stack                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
VOID  _ux_dcd_mcimx6_interrupt_handler(VOID)
{

ULONG                   mcimx6_int_status;
UX_SLAVE_DCD            *dcd;
UX_DCD_MCIMX6           *dcd_mcimx6;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* Get the pointer to the MCIMX6 DCD.  */
    dcd_mcimx6 = (UX_DCD_MCIMX6 *) dcd -> ux_slave_dcd_controller_hardware;

    /* Read the interrupt status register from the controller.  */
    mcimx6_int_status =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBSTS);

    /* Has anything awaken our driver ?  */
    if (mcimx6_int_status != 0)
    {

        /* Yes, something needs to be done at the device controller thread level.
           Save the interrupt status.  We OR the mask because we may have multiple interrupts
           before the thread wakes up. If we have multiple signals of the same source, the thread
           will figure out what needs to be done. */
        dcd_mcimx6 -> ux_dcd_mcimx6_interrupt |=  mcimx6_int_status;

        /* Wake up the controller thread by putting a semaphore. */
        _ux_utility_semaphore_put(&dcd_mcimx6 -> ux_dcd_mcimx6_semaphore);

    }

    /* Clear whatever Interrupt we just had.  */
    _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBSTS, mcimx6_int_status);

    /* Disable USB controller interrupts.  */
    _ux_dcd_mcimx6_register_clear(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBINTR, UX_DCD_MCIMX6_USBINTR_MASK);

}
