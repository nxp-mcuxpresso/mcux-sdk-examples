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
/*    _ux_hcd_ip3516_transfer_abort                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This function will abort transactions attached to a transfer       */
/*     request.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hcd_ip3516                            Pointer to IP3516 controller  */
/*    transfer_request                      Pointer to transfer request   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _ux_hcd_ip3516_transfer_abort(UX_HCD_IP3516 *hcd_ip3516,UX_TRANSFER *transfer_request)
{

UX_ENDPOINT                    *endpoint;
UX_IP3516_PIPE                   *pipe;
UX_IP3516_PERIODIC_PIPE          *periodic_pipe;
usb_host_ip3516hs_atl_struct_t *atl;
usb_host_ip3516hs_ptl_struct_t *ptl;
ULONG                          endpoint_type;

    endpoint = transfer_request -> ux_transfer_request_endpoint;
    endpoint_type = endpoint -> ux_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE;
    if ((endpoint_type == UX_CONTROL_ENDPOINT) || (endpoint_type == UX_BULK_ENDPOINT))
    {
        pipe = endpoint -> ux_endpoint_ed;
        atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + pipe -> ux_ip3516_pipe_index;
        atl->control1Union.stateBitField.V = 0;
        atl->stateUnion.stateBitField.A = 0;
    }
    else if (endpoint_type == UX_INTERRUPT_ENDPOINT)
    {
        periodic_pipe = endpoint -> ux_endpoint_ed;
        ptl = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_array + periodic_pipe -> ux_ip3516_pipe_index;
        ptl->control1Union.stateBitField.V = 0;
        ptl->stateUnion.stateBitField.A = 0;
    }

    /* Return successful completion.  */
    return(UX_SUCCESS);
}
