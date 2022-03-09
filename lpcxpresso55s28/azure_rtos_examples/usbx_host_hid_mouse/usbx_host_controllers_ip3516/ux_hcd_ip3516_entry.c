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
/*    _ux_hcd_ip3516_entry                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function dispatch the HCD function internally to the IP3516    */
/*    controller.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    HCD                                   Pointer to HCD                */ 
/*    function                              Requested function            */ 
/*    parameter                             Pointer to parameter(s)       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_hcd_ip3516_asynchronous_endpoint_create   Create endpoint       */
/*    _ux_hcd_ip3516_asynchronous_endpoint_destroy  Destroy endpoint      */
/*    _ux_hcd_ip3516_controller_disable             Disable controller    */
/*    _ux_hcd_ip3516_done_queue_process             Process done queue    */
/*    _ux_hcd_ip3516_endpoint_reset                 Reset endpoint        */
/*    _ux_hcd_ip3516_frame_number_get               Get frame number      */
/*    _ux_hcd_ip3516_frame_number_set               Set frame number      */
/*    _ux_hcd_ip3516_interrupt_endpoint_create      Endpoint create       */
/*    _ux_hcd_ip3516_interrupt_endpoint_destroy     Endpoint destroy      */
/*    _ux_hcd_ip3516_isochronous_endpoint_create    Endpoint create       */
/*    _ux_hcd_ip3516_isochronous_endpoint_destroy   Endpoint destroy      */
/*    _ux_hcd_ip3516_port_disable                   Disable port          */
/*    _ux_hcd_ip3516_port_reset                     Reset port            */
/*    _ux_hcd_ip3516_port_resume                    Resume port           */
/*    _ux_hcd_ip3516_port_status_get                Get port status       */
/*    _ux_hcd_ip3516_port_suspend                   Suspend port          */
/*    _ux_hcd_ip3516_power_down_port                Power down port       */
/*    _ux_hcd_ip3516_power_on_port                  Power on port         */
/*    _ux_hcd_ip3516_request_transfer               Request transfer      */
/*    _ux_hcd_ip3516_transfer_abort                 Abort transfer        */
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
UINT  _ux_hcd_ip3516_entry(UX_HCD *hcd, UINT function, VOID *parameter)
{

UINT            status;
UX_HCD_IP3516     *hcd_ip3516;
    

    /* Check the status of the controller.  */
    if (hcd -> ux_hcd_status == UX_UNUSED)
    {
    
        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_CONTROLLER_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_CONTROLLER_UNKNOWN, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_CONTROLLER_UNKNOWN);
    }        

    /* Get the pointer to the IP3516 HCD.  */
    hcd_ip3516 =  (UX_HCD_IP3516 *) hcd -> ux_hcd_controller_hardware;

    /* Look at the function and route it.  */
    switch(function)
    {

    case UX_HCD_DISABLE_CONTROLLER:
    
        status =  _ux_hcd_ip3516_controller_disable(hcd_ip3516);
        break;
    
    
    case UX_HCD_GET_PORT_STATUS:
        status =  _ux_hcd_ip3516_port_status_get(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_ENABLE_PORT:

        status =  UX_SUCCESS;
        break;
    
    
    case UX_HCD_DISABLE_PORT:

        status =  _ux_hcd_ip3516_port_disable(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_POWER_ON_PORT:

        status =  _ux_hcd_ip3516_power_on_port(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_POWER_DOWN_PORT:

        status =  _ux_hcd_ip3516_power_down_port(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_SUSPEND_PORT:
        status =  _ux_hcd_ip3516_port_suspend(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_RESUME_PORT:

        status =  _ux_hcd_ip3516_port_resume(hcd_ip3516, (UINT) parameter);
        break;
    
    
    case UX_HCD_RESET_PORT:

        status =  _ux_hcd_ip3516_port_reset(hcd_ip3516, (ULONG) parameter);
        break;
    
    
    case UX_HCD_GET_FRAME_NUMBER:

        status =  _ux_hcd_ip3516_frame_number_get(hcd_ip3516, (ULONG *) parameter);
        break;
    
    
    case UX_HCD_SET_FRAME_NUMBER:

        _ux_hcd_ip3516_frame_number_set(hcd_ip3516, (ULONG) parameter);
        status =  UX_SUCCESS;
        break;
    
    
    case UX_HCD_TRANSFER_REQUEST:

        status =  _ux_hcd_ip3516_request_transfer(hcd_ip3516, (UX_TRANSFER *) parameter);
        break;
    
    
    case UX_HCD_TRANSFER_ABORT:

        status =  _ux_hcd_ip3516_transfer_abort(hcd_ip3516, (UX_TRANSFER *) parameter);
        break;
    
    
    case UX_HCD_CREATE_ENDPOINT:

        switch ((((UX_ENDPOINT*) parameter) -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE)
        {

        case UX_CONTROL_ENDPOINT:
        case UX_BULK_ENDPOINT:
            status =  _ux_hcd_ip3516_asynchronous_endpoint_create(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        case UX_INTERRUPT_ENDPOINT:
            status =  _ux_hcd_ip3516_interrupt_endpoint_create(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        case UX_ISOCHRONOUS_ENDPOINT:
            status =  _ux_hcd_ip3516_isochronous_endpoint_create(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        }
        break;


    case UX_HCD_DESTROY_ENDPOINT:

        switch ((((UX_ENDPOINT*) parameter) -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE)
        {

        case UX_CONTROL_ENDPOINT:
        case UX_BULK_ENDPOINT:
            status =  _ux_hcd_ip3516_asynchronous_endpoint_destroy(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        case UX_INTERRUPT_ENDPOINT:
            status =  _ux_hcd_ip3516_interrupt_endpoint_destroy(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        case UX_ISOCHRONOUS_ENDPOINT:
            status =  _ux_hcd_ip3516_isochronous_endpoint_destroy(hcd_ip3516, (UX_ENDPOINT*) parameter);
            break;

        }
        break;


    case UX_HCD_RESET_ENDPOINT:

        status =  _ux_hcd_ip3516_endpoint_reset(hcd_ip3516, (UX_ENDPOINT*) parameter);
        break;


    case UX_HCD_PROCESS_DONE_QUEUE:

        _ux_hcd_ip3516_done_queue_process(hcd_ip3516);
        status =  UX_SUCCESS;
        break;


    default:

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_FUNCTION_NOT_SUPPORTED);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_FUNCTION_NOT_SUPPORTED, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        status =  UX_FUNCTION_NOT_SUPPORTED;
        break;
    }        

    /* Return status to caller.  */
    return(status);
}

