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
/*    _ux_hcd_ip3516_request_control_transfer             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function performs a control transfer from a transfer request. */
/*     The USB control transfer is in 3 phases (setup, data, status).     */
/*     This function will chain all phases of the control sequence before */
/*     setting the IP3516 endpoint as a candidate for transfer.           */
/*                                                                        */
/*     The max aggregated size of a data payload in IP3516 is 16K. We are */
/*     assuming that this size will be sufficient to contain the control  */
/*     packet.                                                            */ 
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
/*    _ux_host_stack_transfer_request_abort Abort transfer request        */ 
/*    _ux_utility_semaphore_get             Get semaphore                 */ 
/*    _ux_utility_short_put                 Write a 16-bit value          */ 
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
UINT  _ux_hcd_ip3516_request_control_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request)
{

UX_DEVICE       *device;
UX_ENDPOINT     *endpoint;
UCHAR *         setup_request;
UINT            status;
UINT            pid;
usb_host_ip3516hs_atl_struct_t *atl;
UX_IP3516_PIPE              *pipe;


    /* Get the pointer to the Endpoint and to the device.  */
    endpoint =  (UX_ENDPOINT *) transfer_request -> ux_transfer_request_endpoint;
    device =    endpoint -> ux_endpoint_device;

    /* Now get the physical ED attached to this endpoint.  */
    pipe =  endpoint -> ux_endpoint_ed;

    atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + pipe->ux_ip3516_pipe_index;

    /* Build the SETUP packet (phase 1 of the control transfer).  */
    setup_request =  pipe->ux_ip3516_ed_buffer_address;

    *setup_request =                            (UCHAR)transfer_request -> ux_transfer_request_function;
    *(setup_request + UX_SETUP_REQUEST_TYPE) =  (UCHAR)transfer_request -> ux_transfer_request_type;
    *(setup_request + UX_SETUP_REQUEST) =       (UCHAR)transfer_request -> ux_transfer_request_function;
    _ux_utility_short_put(setup_request + UX_SETUP_VALUE, (USHORT)transfer_request -> ux_transfer_request_value);
    _ux_utility_short_put(setup_request + UX_SETUP_INDEX, (USHORT)transfer_request -> ux_transfer_request_index);
    _ux_utility_short_put(setup_request + UX_SETUP_LENGTH, (USHORT) transfer_request -> ux_transfer_request_requested_length);

    pipe -> ux_ip3516_ed_transfer_request = transfer_request;
    pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_SETUP;
    /* Set the transfer to pending.  */
    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STATUS_PENDING;

    /* Set the endpoint address (this should have changed after address setting).  */
    atl -> control2Union.stateBitField.DeviceAddress = device -> ux_device_address;
    atl -> control1Union.stateBitField.MaxPacketLength = endpoint -> ux_endpoint_descriptor.wMaxPacketSize;

    atl->control2Union.stateBitField.RL             = 0U;
    atl->stateUnion.stateBitField.P                 = 0U;
    atl->stateUnion.stateBitField.NakCnt            = 0U;
    atl->stateUnion.stateBitField.Cerr              = 0x3U;
    atl->dataUnion.dataBitField.NrBytesToTransfer   = 8U;
    atl->stateUnion.stateBitField.NrBytesToTransfer = 0U;
    atl->dataUnion.dataBitField.DataStartAddress    = ((ULONG)(setup_request)) & 0x0000FFFFU;
    atl->dataUnion.dataBitField.I                   = 1U;
    atl->stateUnion.stateBitField.Token             = USB_HOST_IP3516HS_PTD_TOKEN_SETUP;
    atl->stateUnion.stateBitField.DT                = 0x00U;
    atl->stateUnion.stateBitField.SC                = 0x00U;
    atl->control1Union.stateBitField.V              = 0x01U;
    atl->stateUnion.stateBitField.A                 = 0x01U;

    /* Wait for the completion of the transfer request.  */
    status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, UX_MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));


    /* If the semaphore did not succeed we probably have a time out.  */
    if ((status != UX_SUCCESS) || (transfer_request -> ux_transfer_request_completion_code != UX_TRANSFER_STATUS_PENDING))
    {

        /* All transfers pending need to abort. There may have been a partial transfer.  */
        _ux_host_stack_transfer_request_abort(transfer_request);

        /* There was an error, return to the caller.  */
        transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_TRANSFER_TIMEOUT);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

        /* Return completion status.  */
        return(transfer_request -> ux_transfer_request_completion_code);
    }

    /* Test if data phase required, if so decide the PID to use and build/hook it to the ED.  */
    if (transfer_request -> ux_transfer_request_requested_length != 0)
    {
        pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_SETUP_DATA;

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
        atl->dataUnion.dataBitField.DataStartAddress    = ((ULONG)(pipe -> ux_ip3516_ed_buffer_address)) & 0x0000FFFFU;
        atl->dataUnion.dataBitField.I                   = 1U;
        atl->stateUnion.stateBitField.Token             = pid;
        atl->stateUnion.stateBitField.SC                = 0x00U;
        atl->control1Union.stateBitField.V              = 0x01U;
        atl->stateUnion.stateBitField.A                 = 0x01U;


        /* Wait for the completion of the transfer request.  */
        status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, UX_MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));

        /* If the semaphore did not succeed we probably have a time out.  */
        if ((status != UX_SUCCESS) || (transfer_request -> ux_transfer_request_completion_code != UX_TRANSFER_STATUS_PENDING))
        {

            /* All transfers pending need to abort. There may have been a partial transfer.  */
            _ux_host_stack_transfer_request_abort(transfer_request);

            /* There was an error, return to the caller.  */
            transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

            /* Error trap. */
            _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_TRANSFER_TIMEOUT);

            /* If trace is enabled, insert this event into the trace buffer.  */
            UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

            /* Return completion status.  */
            return(transfer_request -> ux_transfer_request_completion_code);
        }

    }
    pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_SETUP_STATUS;

    /* Program the status phase. the PID is the opposite of the data phase.  */
    if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
        pid =  USB_HOST_IP3516HS_PTD_TOKEN_OUT;
    else            
        pid =  USB_HOST_IP3516HS_PTD_TOKEN_IN;

    atl->control2Union.stateBitField.RL  = 0U;
    atl->stateUnion.stateBitField.NakCnt = 0U;
    atl->stateUnion.stateBitField.Cerr   = 0x3U;
    atl->dataUnion.dataBitField.NrBytesToTransfer   = 0;
    atl->stateUnion.stateBitField.NrBytesToTransfer = 0U;
    atl->dataUnion.dataBitField.DataStartAddress    = 0;
    atl->dataUnion.dataBitField.I                   = 1U;
    atl->stateUnion.stateBitField.Token             = pid;
    atl->stateUnion.stateBitField.DT                = 1U;
    atl->stateUnion.stateBitField.P                 = 0U;
    atl->stateUnion.stateBitField.SC                = 0x00U;
    atl->control1Union.stateBitField.V              = 0x01U;
    atl->stateUnion.stateBitField.A                 = 0x01U;


    /* Wait for the completion of the transfer request.  */
    status =  _ux_utility_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, UX_MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT));

    /* If the semaphore did not succeed we probably have a time out.  */
    if (status != UX_SUCCESS)
    {

        /* All transfers pending need to abort. There may have been a partial transfer.  */
        _ux_host_stack_transfer_request_abort(transfer_request);
        
        /* There was an error, return to the caller.  */
        transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_TRANSFER_TIMEOUT);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)
        
    }            

    /* Return completion status.  */
    return(transfer_request -> ux_transfer_request_completion_code);           
}

