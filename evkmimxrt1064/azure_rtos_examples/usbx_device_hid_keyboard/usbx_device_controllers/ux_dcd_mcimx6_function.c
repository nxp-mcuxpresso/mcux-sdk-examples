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
#include "ux_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                                 RELEASE      */
/*                                                                        */
/*    _ux_dcd_mcimx6_function                               PORTABLE C    */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function dispatches the DCD function internally to the MCIMX6  */
/*    controller.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd                                   Pointer to device controller  */
/*    function                              Function requested            */
/*    parameter                             Pointer to function parameters*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_endpoint_create        Create endpoint               */
/*    _ux_dcd_mcimx6_endpoint_destroy       Destroy endpoint              */
/*    _ux_dcd_mcimx6_endpoint_reset         Reset endpoint                */
/*    _ux_dcd_mcimx6_endpoint_stall         Stall endpoint                */
/*    _ux_dcd_mcimx6_endpoint_status        Get endpoint status           */
/*    _ux_dcd_mcimx6_frame_number_get       Get frame number              */
/*    _ux_dcd_mcimx6_state_change           Change state                  */
/*    _ux_dcd_mcimx6_transfer_request       Request data transfer         */
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
UINT  _ux_dcd_mcimx6_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter)
{

UINT                status;
UX_DCD_MCIMX6       *dcd_mcimx6;


    /* Check the status of the controller.  */
    if (dcd -> ux_slave_dcd_status == UX_UNUSED)
    {

           /* If trace is enabled, insert this event into the trace buffer.  */
           UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_CONTROLLER_UNKNOWN, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_CONTROLLER_UNKNOWN);
    }

    /* Get the pointer to the MCIMX6 DCD.  */
    dcd_mcimx6 =  (UX_DCD_MCIMX6 *) dcd -> ux_slave_dcd_controller_hardware;

    /* Look at the function and route it.  */
    switch(function)
    {

    case UX_DCD_GET_FRAME_NUMBER:

        status =  _ux_dcd_mcimx6_frame_number_get(dcd_mcimx6, (ULONG *) parameter);
        break;

    case UX_DCD_TRANSFER_REQUEST:

        status =  _ux_dcd_mcimx6_transfer_request(dcd_mcimx6, (UX_SLAVE_TRANSFER *) parameter);
        break;

    case UX_DCD_TRANSFER_ABORT:

        status =  _ux_dcd_mcimx6_transfer_abort(dcd_mcimx6, (UX_SLAVE_TRANSFER *) parameter);
        break;

    case UX_DCD_CREATE_ENDPOINT:

        status =  _ux_dcd_mcimx6_endpoint_create(dcd_mcimx6, parameter);
        break;

    case UX_DCD_DESTROY_ENDPOINT:

        status =  _ux_dcd_mcimx6_endpoint_destroy(dcd_mcimx6, parameter);
        break;

    case UX_DCD_RESET_ENDPOINT:

        status =  _ux_dcd_mcimx6_endpoint_stall_clear(dcd_mcimx6, parameter);
        break;

    case UX_DCD_STALL_ENDPOINT:

        status =  _ux_dcd_mcimx6_endpoint_stall(dcd_mcimx6, parameter);
        break;

    case UX_DCD_SET_DEVICE_ADDRESS:

        status =  UX_SUCCESS;
        break;

    case UX_DCD_CHANGE_STATE:

        status =  _ux_dcd_mcimx6_state_change(dcd_mcimx6, (ULONG) parameter);
        break;

    case UX_DCD_ENDPOINT_STATUS:

        status =  _ux_dcd_mcimx6_endpoint_status(dcd_mcimx6, (ULONG) parameter);
        break;

    default:

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_FUNCTION_NOT_SUPPORTED, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        status =  UX_FUNCTION_NOT_SUPPORTED;
        break;

    }

    /* Return completion status.  */
    return(status);
}
