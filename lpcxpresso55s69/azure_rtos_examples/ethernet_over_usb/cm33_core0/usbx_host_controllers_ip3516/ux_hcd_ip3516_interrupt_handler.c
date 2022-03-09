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
/**   IP3516 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_hcd_ip3516.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_hcd_ip3516_interrupt_handler                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function is the interrupt handler for the IP3516 interrupts.   */
/*    Normally an interrupt occurs from the controller when there is      */ 
/*    either a EOF signal and there has been transfers within the frame   */ 
/*    or when there is a change on one of the downstream ports, a         */ 
/*    doorbell signal or an unrecoverable error.                          */
/*                                                                        */
/*    All we need to do in the ISR is scan the controllers to find out    */ 
/*    which one has issued a IRQ. If there is work to do for this         */ 
/*    controller we need to wake up the corresponding thread to take care */ 
/*    of the job.                                                         */ 
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
/*    _ux_hcd_ip3516_register_read          Read IP3516 register          */ 
/*    _ux_hcd_ip3516_register_write         Write IP3516 register         */ 
/*    _ux_utility_semaphore_put             Put semaphore                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    IP3516 Controller Driver                                            */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
VOID  _ux_hcd_ip3516_interrupt_handler(VOID)
{

UINT            hcd_index;
UX_HCD          *hcd;
UX_HCD_IP3516     *hcd_ip3516;
ULONG           ip3516_register;
ULONG           ip3516_register_port_status;
ULONG           root_hub_thread_wakeup = 0;
ULONG           port_index;


    /* We need to parse the controller driver table to find all controllers that registered 
       as IP3516.  */
    for (hcd_index = 0; hcd_index < _ux_system_host -> ux_system_host_registered_hcd; hcd_index++)
    {

        /* Check type of controller.  */
        if (_ux_system_host -> ux_system_host_hcd_array[hcd_index].ux_hcd_controller_type == UX_IP3516_CONTROLLER)
        {

            /* Get the pointers to the generic HCD and IP3516 specific areas.  */
            hcd =  &_ux_system_host -> ux_system_host_hcd_array[hcd_index];
            hcd_ip3516 =  (UX_HCD_IP3516 *) hcd -> ux_hcd_controller_hardware;

            /* Check if the controller is operational, if not, skip it.  */
            if (hcd -> ux_hcd_status == UX_HCD_STATUS_OPERATIONAL)
            {

                /* For debugging purposes, increment the interrupt count.  */
                hcd_ip3516 -> ux_hcd_ip3516_interrupt_count++;

                /* We get the current interrupt status for this controller.  */
                ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_STATUS);

                /* We  acknowledge the interrupts for this controller so that it
                   can continue to work.  */
                _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_STATUS, ip3516_register);

                /* Examine the source of interrupts.  */
                if ((ip3516_register & IP3516_HC_STS_ATL_IRQ) || (ip3516_register & IP3516_HC_STS_INT_IRQ))
                {

                    /* We have some transactions done in the past frame/micro-frame.
                       The controller thread needs to wake up and process them.  */
                    hcd -> ux_hcd_thread_signal++;
                    _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_hcd_semaphore);
                }

                if (ip3516_register & IP3516_HC_STS_PCD)
                {

                    /* The controller has issued a Root hub status change signal. Scan all ports.  */
                    for (port_index = 0; port_index < hcd_ip3516 -> ux_hcd_ip3516_nb_root_hubs; port_index++)
                    {

                        /* Read the port status.  */
                        ip3516_register_port_status =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_PORT_SC + port_index);
                
                        /* Check for Connect Status Change signal.  */
                        if (ip3516_register_port_status &  IP3516_HC_PS_CSC)
                        {                        
                            /* Something happened on this port. Signal it to the root hub thread.  */
                            hcd -> ux_hcd_root_hub_signal[port_index]++;
                            
                            /* Memorize wake up signal.  */
                            root_hub_thread_wakeup ++;

                        }
                        
                    }

                    /* We only wake up the root hub thread if there has been device insertion/extraction.  */
                    if (root_hub_thread_wakeup != 0)

                        /* The controller has issued a Root hub status change signal. 
                           We need to resume the thread in charge of the USB topology.  */
                        _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_enum_semaphore);
                }

            }
        }
    }
}

