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
#include "ux_system.h"
#include "ux_utility.h"
#include "ux_device_stack.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                                 RELEASE      */
/*                                                                        */
/*    _ux_dcd_mcimx6_initialize                             PORTABLE C    */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the USB slave controller of the RENESAS   */
/*    MCIMX6 chipset.                                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd                                   Address of DCD                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_write         Read register                 */
/*    _ux_utility_memory_allocate           Allocate memory               */
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
UINT  _ux_dcd_mcimx6_initialize(ULONG dcd_io)
{

UX_SLAVE_DCD        *dcd;
UX_DCD_MCIMX6       *dcd_mcimx6;
ULONG               mcimx6_register;
ULONG               reset_loop;
ULONG               status;
UX_DCD_MCIMX6_QED  *qed;
ULONG               qed_index;


    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* The controller initialized here is of MCIMX6 type.  */
    dcd -> ux_slave_dcd_controller_type =  UX_DCD_MCIMX6_SLAVE_CONTROLLER;

    /* Allocate memory for this MCIMX6 DCD instance.  */
    dcd_mcimx6 =  _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_DCD_MCIMX6));

    /* Check if memory was properly allocated.  */
    if(dcd_mcimx6 == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Set the pointer to the MCIMX6 DCD.  */
    dcd -> ux_slave_dcd_controller_hardware =  (VOID *) dcd_mcimx6;

    /* Save the base address of the controller.  */
    dcd -> ux_slave_dcd_io                 =  dcd_io;
    dcd_mcimx6 -> ux_dcd_mcimx6_base     =  dcd_io;

    /* Set the generic DCD owner for the MCIMX6 DCD.  */
    dcd_mcimx6 -> ux_dcd_mcimx6_dcd_owner =  dcd;

    /* Initialize the function collector for this DCD.  */
    dcd -> ux_slave_dcd_function =  _ux_dcd_mcimx6_function;


    /* Set the USBCMD[RST] reset.  */
    _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD, UX_DCD_MCIMX6_USBCMD_RST);

    /* Wait for the controller to reset properly.  Not sure as the RESET_RETRY value here we make it high.
       We needed a mechanism to ensure we would not get stuck in the reset loop in case the controller is dead.  */
    for (reset_loop = 0; reset_loop < UX_DCD_MCIMX6_RESET_RETRY; reset_loop++)
    {

        mcimx6_register = _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD);
        if ((mcimx6_register & UX_DCD_MCIMX6_USBCMD_RST) == 0)
            break;
    }

    /* Check if the controller is reset properly.  */
    if ((mcimx6_register & UX_DCD_MCIMX6_USBCMD_RST) != 0)
    {

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_CONTROLLER_INIT_FAILED, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_CONTROLLER_INIT_FAILED);
    }

    /* Allocate the list of eds. All eds are allocated on 64 byte memory boundary.  BUT the EPPBase is aligned on a
       2K boundary.  */
    dcd_mcimx6 -> ux_dcd_mcimx6_qed_head =  _ux_utility_memory_allocate(UX_ALIGN_2048, UX_CACHE_SAFE_MEMORY, sizeof(UX_DCD_MCIMX6_QED) * UX_DCD_MCIMX6_MAX_ED);
    if (dcd_mcimx6 -> ux_dcd_mcimx6_qed_head == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* We need to make all the QED point to a terminated QTD field.  */
    qed =  dcd_mcimx6 -> ux_dcd_mcimx6_qed_head;

    /* Initialize the qed counter.  */
    qed_index = UX_DCD_MCIMX6_MAX_ED;

    /* Parse all the QEDs and set the QTD termination bit.  */
    while (qed_index-- !=0)
    {

        /* set the termination bit.  */
        qed -> ux_dcd_mcimx6_qed_next_qtd = (UX_DCD_MCIMX6_QTD *) UX_DCD_MCIMX6_QTD_TERMINATE;

        /* Next qed.  */
        qed++;
    }

    /* Allocate the list of tds. All tds are allocated on 32 byte memory boundary.  */
    dcd_mcimx6 -> ux_dcd_mcimx6_qtd_head =  _ux_utility_memory_allocate(UX_ALIGN_32, UX_CACHE_SAFE_MEMORY, sizeof(UX_DCD_MCIMX6_QTD) * UX_DCD_MCIMX6_MAX_QTD);
    if (dcd_mcimx6 -> ux_dcd_mcimx6_qtd_head == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Allocate some memory for the thread stack. */
    dcd_mcimx6 -> ux_dcd_mcimx6_thread_stack =
            _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, UX_THREAD_STACK_SIZE);

    /* Check for successful allocation.  */
    if (dcd_mcimx6 -> ux_dcd_mcimx6_thread_stack == UX_NULL)
    {
        /* Free all resources.  */
        _ux_utility_memory_free(dcd_mcimx6 -> ux_dcd_mcimx6_qed_head);
        _ux_utility_memory_free(dcd_mcimx6 -> ux_dcd_mcimx6_qtd_head);

        /* Return the status to the caller.  */
        return(UX_MEMORY_INSUFFICIENT);
    }

    /* This device controller needs the ISR to be running within a thread.
       We pass a pointer to the mcimx6 instance to the thread. */
    status =  _ux_utility_thread_create(&dcd_mcimx6 -> ux_dcd_mcimx6_thread, "ux_slave_class_thread",
                _ux_dcd_mcimx6_interrupt_thread,
                (ULONG) dcd_mcimx6, dcd_mcimx6 -> ux_dcd_mcimx6_thread_stack,
                UX_THREAD_STACK_SIZE, UX_THREAD_PRIORITY_DCD,
                UX_THREAD_PRIORITY_DCD, UX_NO_TIME_SLICE, TX_AUTO_START);

    /* Check the creation of this thread.  */
    if (status != UX_SUCCESS)
    {
        /* Free some of the resource used. */
        _ux_utility_memory_free(dcd_mcimx6 -> ux_dcd_mcimx6_qed_head);
        _ux_utility_memory_free(dcd_mcimx6 -> ux_dcd_mcimx6_qtd_head);
        _ux_utility_memory_free(dcd_mcimx6 -> ux_dcd_mcimx6_thread_stack);

        return(UX_THREAD_ERROR);
    }

    /* Store the QHead pointer into the Endpoint ListAddr register. This is a physical address.  */
    _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EP_LIST_ADDR,
                                    (ULONG) _ux_utility_physical_address(dcd_mcimx6 -> ux_dcd_mcimx6_qed_head));

    /* Set the controller to device mode.  Little endian is selected.  */
    _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USB_MODE, UX_DCD_MCIMX6_USBMODE_CM_DEVICE);

    /* Clear the ITC field in the Command Register of the controller. ITC is for the Interrupt Control Threshold.
       When reset there is no delay and the interrupts are issued immediately.  */
    _ux_dcd_mcimx6_register_clear(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD, UX_DCD_MCIMX6_USBCMD_ITC_MASK);

    /* Set the Setup Lockout Mode to OFF.  */
    _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USB_MODE, UX_DCD_MCIMX6_USBMODE_SLOM);

    /* Enable USB controller interrupts.  */
    _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBINTR, UX_DCD_MCIMX6_USBINTR_MASK);

    /* Start the USB Device Controller.  */
    _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD, UX_DCD_MCIMX6_USBCMD_RS);

    /* Set the state of the controller to OPERATIONAL now.  A BUS RESET signal will complete the init process. */
    dcd -> ux_slave_dcd_status =  UX_DCD_STATUS_OPERATIONAL;

    /* Return successful completion.  */
    return(UX_SUCCESS);
}
