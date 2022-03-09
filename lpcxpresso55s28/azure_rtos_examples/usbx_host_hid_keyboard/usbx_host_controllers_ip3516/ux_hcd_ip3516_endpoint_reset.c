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
/*    _ux_hcd_ip3516_endpoint_reset                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will reset an endpoint.                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_ip3516                            Pointer to IP3516 controller  */ 
/*    endpoint                              Pointer to endpoint           */ 
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
UINT  _ux_hcd_ip3516_endpoint_reset(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint)
{

usb_host_ip3516hs_atl_struct_t *atl;
usb_host_ip3516hs_ptl_struct_t *ptl;
UX_IP3516_PIPE                 *pipe;


    /* Now get the pipe attached to this endpoint.  */
    pipe =  endpoint -> ux_endpoint_ed;

    /* Isolate the endpoint type.  */
    switch ((endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE)
    {

    case UX_CONTROL_ENDPOINT:
    case UX_BULK_ENDPOINT:
        atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + pipe -> ux_ip3516_pipe_index;
        atl->stateUnion.stateBitField.DT = 0U;
        break;

    case UX_INTERRUPT_ENDPOINT:
    case UX_ISOCHRONOUS_ENDPOINT:
        ptl = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_array + pipe -> ux_ip3516_pipe_index;
        ptl->stateUnion.stateBitField.DT = 0U;
        break;
    }

    /* This operation never fails!  */
    return(UX_SUCCESS);         
}

