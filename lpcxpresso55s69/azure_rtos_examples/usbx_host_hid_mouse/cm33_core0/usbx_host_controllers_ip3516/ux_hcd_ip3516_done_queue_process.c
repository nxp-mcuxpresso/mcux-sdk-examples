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
/*    _ux_hcd_ip3516_done_queue_process                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function process the isochronous, periodic and asynchronous    */
/*    lists in search for transfers that occurred in the past             */
/*    (micro-)frame.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hcd_ip3516                            Pointer to IP3516 controller  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_hcd_ip3516_register_read          Read IP3516 register          */
/*    _ux_hcd_ip3516_register_write         Write IP3516 register         */
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
VOID  _ux_hcd_ip3516_done_queue_process(UX_HCD_IP3516 *hcd_ip3516)
{

ULONG done_map;
ULONG pipe_index;
usb_host_ip3516hs_atl_struct_t *atl;
usb_host_ip3516hs_ptl_struct_t *ptl;
UX_IP3516_PIPE              *pipe;
UX_IP3516_PERIODIC_PIPE     *periodic_pipe;
UX_TRANSFER *             transfer_request;


    done_map = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_ATLPTDD);
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDD, done_map);

    for (pipe_index = 0;
         done_map != 0;
         done_map = done_map >> 1, pipe_index++)
    {
        if (done_map & 1)
        {

            pipe = hcd_ip3516 -> ux_hcd_ip3516_atl_pipes + pipe_index;
            atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + pipe_index;

            if (atl->stateUnion.stateBitField.A)
            {
                continue;
            }
            else
            {
                transfer_request = pipe -> ux_ip3516_ed_transfer_request;
                if (atl->stateUnion.stateBitField.B)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_ERROR;
                else if (atl->stateUnion.stateBitField.X)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_NO_ANSWER;
                else if (atl->stateUnion.stateBitField.H)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STALLED;
                else
                {
                    if((pipe -> ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_SETUP_STATUS) ||
                        (pipe -> ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_BULK_DATA))
                    {
                        transfer_request -> ux_transfer_request_completion_code =  UX_SUCCESS;
                    }
                    if((pipe -> ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_SETUP_DATA)||
                        (pipe -> ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_BULK_DATA))
                    {
                        transfer_request -> ux_transfer_request_actual_length = atl->stateUnion.stateBitField.NrBytesToTransfer;
                        if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
                        {
                            _ux_utility_memory_copy(transfer_request -> ux_transfer_request_data_pointer, pipe -> ux_ip3516_ed_buffer_address, transfer_request -> ux_transfer_request_actual_length);
                        }
                    }
                }

                atl->stateUnion.stateBitField.B = 0;
                atl->stateUnion.stateBitField.X = 0;
                atl->stateUnion.stateBitField.H = 0;

                if ((transfer_request -> ux_transfer_request_completion_code != UX_TRANSFER_STATUS_PENDING) &&
                    (transfer_request -> ux_transfer_request_completion_function != UX_NULL))
                {
                    transfer_request -> ux_transfer_request_completion_function(transfer_request);
                }

                _ux_utility_semaphore_put(&transfer_request -> ux_transfer_request_semaphore);

            }
        }
    }

    done_map = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_INTPTDD);
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_INTPTDD, done_map);

    for (pipe_index = 0;
         done_map != 0;
         done_map = done_map >> 1, pipe_index++)
    {
        if (done_map & 1)
        {

            periodic_pipe = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes + pipe_index;
            ptl = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_array + pipe_index;

            if (ptl->stateUnion.stateBitField.A)
            {
                continue;
            }
            else
            {
                transfer_request = periodic_pipe -> ux_ip3516_ed_transfer_request;
                if (ptl->stateUnion.stateBitField.B)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_ERROR;
                else if (ptl->stateUnion.stateBitField.X)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_NO_ANSWER;
                else if (ptl->stateUnion.stateBitField.H)
                    transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_STALLED;
                else
                {

                    transfer_request -> ux_transfer_request_completion_code =  UX_SUCCESS;
                    transfer_request -> ux_transfer_request_actual_length = ptl->stateUnion.stateBitField.NrBytesToTransfer;
                    if ((transfer_request -> ux_transfer_request_type & UX_REQUEST_DIRECTION) == UX_REQUEST_IN)
                    {
                        _ux_utility_memory_copy(transfer_request -> ux_transfer_request_data_pointer, periodic_pipe -> ux_ip3516_ed_buffer_address, transfer_request -> ux_transfer_request_actual_length);
                    }
                }

                ptl->stateUnion.stateBitField.B = 0;
                ptl->stateUnion.stateBitField.X = 0;
                ptl->stateUnion.stateBitField.H = 0;

                if (transfer_request -> ux_transfer_request_completion_function != UX_NULL)
                {
                    transfer_request -> ux_transfer_request_completion_function(transfer_request);
                }

                _ux_utility_semaphore_put(&transfer_request -> ux_transfer_request_semaphore);

            }
        }
    }
}

