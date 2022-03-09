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
/*    _ux_hcd_ip3516_request_interrupt_transfer           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function performs an interrupt transfer request. An interrupt */ 
/*     transfer can only be as large as the MaxpacketField in the         */ 
/*     endpoint descriptor. This was verified at the upper layer and does */
/*     not need to be reverified here.                                    */ 
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
UINT  _ux_hcd_ip3516_request_interrupt_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request)
{

UX_ENDPOINT     *endpoint;
ULONG           pid;
usb_host_ip3516hs_ptl_struct_t *ptl;
usb_host_ip3516hs_sptl_struct_t *sptl;
UX_IP3516_PERIODIC_PIPE              *pipe;

    /* Get the pointer to the endpoint.  */
    endpoint =  (UX_ENDPOINT *) transfer_request -> ux_transfer_request_endpoint;

    /* Now get the physical ED attached to this endpoint.  */
    pipe =  endpoint -> ux_endpoint_ed;

    ptl = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_array + pipe->ux_ip3516_pipe_index;

    sptl = (usb_host_ip3516hs_sptl_struct_t *)ptl;

    pipe -> ux_ip3516_ed_transfer_request = transfer_request;
    pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_SETUP_DATA;

    /* Get the correct PID for this transfer.  */
    if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
        pid =  USB_HOST_IP3516HS_PTD_TOKEN_IN;
    else            
    {
        _ux_utility_memory_copy(pipe -> ux_ip3516_ed_buffer_address, transfer_request -> ux_transfer_request_data_pointer, transfer_request -> ux_transfer_request_requested_length);
        pid =  USB_HOST_IP3516HS_PTD_TOKEN_OUT;
    }

    ptl->control2Union.stateBitField.RL             = 0U;
    ptl->stateUnion.stateBitField.NakCnt            = 0x0FU;
    ptl->stateUnion.stateBitField.Cerr              = 0x3U;
    ptl->dataUnion.dataBitField.NrBytesToTransfer   = transfer_request -> ux_transfer_request_requested_length;
    ptl->stateUnion.stateBitField.NrBytesToTransfer = 0U;
    ptl->dataUnion.dataBitField.DataStartAddress    = ((ULONG)(pipe -> ux_ip3516_ed_buffer_address)) & 0x0000FFFFU;
    ptl->dataUnion.dataBitField.I                   = 1U;
    ptl->stateUnion.stateBitField.P                 = 0x00U;
    ptl->stateUnion.stateBitField.SC                = 0x00U;
    ptl->stateUnion.stateBitField.Token = pid;
    ptl->statusUnion.status = 0U;
    ptl->statusUnion.statusBitField.uSA = pipe -> ux_ip3516_ed_uSA;
    if(ptl->control2Union.stateBitField.S)
    {
        sptl->isoInUnion1.bitField.uSCS = 255;
    }
    else
    {
        ptl->isoInUnion1.isoIn  = 0U;
    }

    ptl->isoInUnion2.isoIn  = 0U;
    ptl->isoInUnion3.isoIn  = 0U;

    ptl->control1Union.stateBitField.V = 0x01U;
    ptl->stateUnion.stateBitField.A    = 0x01U;

    /* Return completion status.  */
    return(UX_SUCCESS);
}

