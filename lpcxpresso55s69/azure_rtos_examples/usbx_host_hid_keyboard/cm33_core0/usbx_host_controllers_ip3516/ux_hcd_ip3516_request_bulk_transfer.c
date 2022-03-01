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
/*    _ux_hcd_ip3516_request_bulk_transfer                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function performs a bulk transfer request. A bulk transfer    */ 
/*     can be larger than the size of the IP3516 buffer so it may be      */
/*     required to chain multiple tds to accommodate this request. A bulk */ 
/*     transfer is non blocking, so we return before the request is       */
/*     completed.                                                         */ 
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
/*    _ux_utility_memory_copy               Copy memory                   */
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
UINT  _ux_hcd_ip3516_request_bulk_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request)
{

UX_ENDPOINT     *endpoint;
ULONG           pid;
usb_host_ip3516hs_atl_struct_t *atl;
UX_IP3516_PIPE              *pipe;


    /* Get the pointer to the Endpoint.  */
    endpoint =  (UX_ENDPOINT *) transfer_request -> ux_transfer_request_endpoint;

    /* Now get the physical ED attached to this endpoint.  */
    pipe =  endpoint -> ux_endpoint_ed;

    atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + pipe->ux_ip3516_pipe_index;

    pipe -> ux_ip3516_ed_transfer_request = transfer_request;
    pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_BULK_DATA;
    
    if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
        pid =  USB_HOST_IP3516HS_PTD_TOKEN_IN;
    else
    {
        _ux_utility_memory_copy(pipe -> ux_ip3516_ed_buffer_address, transfer_request -> ux_transfer_request_data_pointer, transfer_request -> ux_transfer_request_requested_length);

        pid =  USB_HOST_IP3516HS_PTD_TOKEN_OUT;
    }

    atl->control2Union.stateBitField.RL  = 0U;
    atl->stateUnion.stateBitField.NakCnt = 0U;
    atl->stateUnion.stateBitField.Cerr   = 0x3U;
    atl->dataUnion.dataBitField.NrBytesToTransfer   = transfer_request -> ux_transfer_request_requested_length;
    atl->stateUnion.stateBitField.NrBytesToTransfer = 0U;
    atl->dataUnion.dataBitField.DataStartAddress    = ((ULONG)pipe -> ux_ip3516_ed_buffer_address) & 0x0000FFFFU;
    atl->dataUnion.dataBitField.I                   = 1U;
    atl->stateUnion.stateBitField.Token             = pid;
    atl->stateUnion.stateBitField.SC                = 0x00U;
    atl->control1Union.stateBitField.V              = 0x01U;
    atl->stateUnion.stateBitField.A                 = 0x01U;

    /* Return successful completion.  */
    return(UX_SUCCESS);           
}

