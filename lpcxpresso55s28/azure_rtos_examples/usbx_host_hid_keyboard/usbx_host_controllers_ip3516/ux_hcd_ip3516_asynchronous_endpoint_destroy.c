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
/*    _ux_hcd_ip3516_asynchronous_endpoint_destroy          PORTABLE C    */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will destroy an asynchronous endpoint. The control    */
/*    and bulk endpoints fall into this category.                         */ 
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
/*    _ux_hcd_ip3516_register_write         Write register                */
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
UINT  _ux_hcd_ip3516_asynchronous_endpoint_destroy(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint)
{

ULONG                      register_value;
UX_IP3516_PIPE             *pipe;

    /* From the endpoint container fetch the IP3516 ED descriptor.  */
    pipe =  (UX_IP3516_PIPE *) endpoint -> ux_endpoint_ed;

    register_value = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_ATLPTDS);

    register_value |= 1 << pipe -> ux_ip3516_pipe_index;

    /* Set the ATL PTD Skip Register.  */
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDS, register_value);

    if (pipe -> ux_ip3516_ed_buffer_address)
        _tx_byte_release(pipe -> ux_ip3516_ed_buffer_address);

    pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_FREE;

    /* Return successful completion.  */
    return(UX_SUCCESS);        
}

