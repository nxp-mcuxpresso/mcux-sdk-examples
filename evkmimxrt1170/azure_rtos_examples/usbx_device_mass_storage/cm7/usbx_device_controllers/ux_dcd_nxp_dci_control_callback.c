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
/**   NXP_DCI Controller Driver                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_NXP_DCI_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_nxp_dci.h"
#include "ux_device_stack.h"
#include "ux_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_nxp_dci_control_callback                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles callback from the USB driver.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    handle                                Pointer to device handle      */
/*    message                               Message for callback          */
/*    callbackParam                         Parameter for callback        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_device_stack_control_request_process                            */
/*                                          Process control request       */
/*    USB_DeviceSendRequest                 Transmit data                 */
/*    USB_DeviceRecvRequest                 Receive data                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NXP_DCI Driver                                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
usb_status_t _ux_dcd_nxp_dci_control_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam)
{
    UX_SLAVE_DCD            *dcd;
    UX_DCD_NXP_DCI            *dcd_nxp_dci;
    UX_DCD_NXP_DCI_ED         *ed;
    UX_SLAVE_TRANSFER       *transfer_request;
    UX_SLAVE_ENDPOINT       *endpoint;
    ULONG                   transfer_length;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* Get the pointer to the NXP_DCI DCD.  */
    dcd_nxp_dci = (UX_DCD_NXP_DCI *) dcd -> ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_nxp_dci -> ux_dcd_nxp_dci_ed[0];

    /* Get the pointer to the transfer request.  */
    transfer_request =  &ed -> ux_dcd_nxp_dci_ed_endpoint -> ux_slave_endpoint_transfer_request;

    if (message->isSetup)
    {
        _ux_utility_memory_copy(transfer_request->ux_slave_transfer_request_setup, (void *)(message->buffer),  UX_SETUP_SIZE);

        /* Clear the length of the data received.  */
        transfer_request -> ux_slave_transfer_request_actual_length =  0;

        /* Mark the phase as SETUP.  */
        transfer_request -> ux_slave_transfer_request_type =  UX_TRANSFER_PHASE_SETUP;

        /* Mark the transfer as successful.  */
        transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

        /* Set the status of the endpoint to not stalled.  */
        ed -> ux_dcd_nxp_dci_ed_status &= ~UX_DCD_NXP_DCI_ED_STATUS_STALLED;

        /* Check if the transaction is IN.  */
        if (*transfer_request -> ux_slave_transfer_request_setup & UX_REQUEST_IN)
        {

            /* The endpoint is IN.  This is important to memorize the direction for the control endpoint
               in case of a STALL. */
            ed -> ux_dcd_nxp_dci_ed_direction  = UX_ENDPOINT_IN;

            /* Call the Control Transfer dispatcher.  */
            _ux_device_stack_control_request_process(transfer_request);

            /* Set the state to TX.  */
            ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_DATA_TX;
        }
        else
        {

            /* The endpoint is OUT.  This is important to memorize the direction for the control endpoint
               in case of a STALL. */
            ed -> ux_dcd_nxp_dci_ed_direction  = UX_ENDPOINT_OUT;

            /* We are in a OUT transaction. Check if there is a data payload. If so, wait for the payload
               to be delivered.  */
            if (*(transfer_request -> ux_slave_transfer_request_setup + 6) == 0 &&
                *(transfer_request -> ux_slave_transfer_request_setup + 7) == 0)
            {

                /* The endpoint is IN.  This is important to memorize the direction for the control endpoint
                       in case of a STALL. */
                ed -> ux_dcd_nxp_dci_ed_direction  = UX_ENDPOINT_IN;

                /* Call the Control Transfer dispatcher.  */
                if (_ux_device_stack_control_request_process(transfer_request) == UX_SUCCESS)
                {

                    /* Set the state to STATUS RX.  */
                    ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_STATUS_RX;
                    USB_DeviceSendRequest(handle, 0x00U, UX_NULL, 0U);
                }
            }
            else
            {

                /* Get the pointer to the logical endpoint from the transfer request.  */
                endpoint =  transfer_request -> ux_slave_transfer_request_endpoint;

                /* Get the length we expect from the SETUP packet.  */
                transfer_request -> ux_slave_transfer_request_requested_length = _ux_utility_short_get(transfer_request -> ux_slave_transfer_request_setup + 6);

                /* Check if we have enough space for the request.  */
                if (transfer_request -> ux_slave_transfer_request_requested_length > UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH)
                {

                    /* No space available, stall the endpoint.  */
                    _ux_dcd_nxp_dci_endpoint_stall(dcd_nxp_dci, endpoint);

                    /* Next phase is a SETUP.  */
                    ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_IDLE;

                    /* We are done.  */
                    return kStatus_USB_Success;
                }
                else
                {

                    /* Reset what we have received so far.  */
                    transfer_request -> ux_slave_transfer_request_actual_length =  0;

                    /* And reprogram the current buffer address to the beginning of the buffer.  */
                    transfer_request -> ux_slave_transfer_request_current_data_pointer =  transfer_request -> ux_slave_transfer_request_data_pointer;

                    /* Receive data.  */
                    USB_DeviceRecvRequest(handle,
                                endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress,
                                transfer_request -> ux_slave_transfer_request_current_data_pointer,
                                transfer_request -> ux_slave_transfer_request_requested_length);

                    /* Set the state to RX.  */
                    ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_DATA_RX;
                }
            }
        }
    }
    else
    {
        /* Get the pointer to the logical endpoint from the transfer request.  */
        endpoint =  transfer_request -> ux_slave_transfer_request_endpoint;

        /* Check if we need to send data again on control endpoint. */
        if (ed -> ux_dcd_nxp_dci_ed_state == UX_DCD_NXP_DCI_ED_STATE_DATA_TX)
        {

            /* Arm Status transfer.  */
            USB_DeviceRecvRequest(handle, 0, 0, 0);

            /* Are we done with this transfer ? */
            if (transfer_request -> ux_slave_transfer_request_in_transfer_length <= endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize)
            {

                /* There is no data to send but we may need to send a Zero Length Packet.  */
                if (transfer_request -> ux_slave_transfer_request_force_zlp ==  UX_TRUE)
                {

                    /* Arm a ZLP packet on IN.  */
                    USB_DeviceSendRequest(handle,
                            endpoint->ux_slave_endpoint_descriptor.bEndpointAddress, 0, 0);

                    /* Reset the ZLP condition.  */
                    transfer_request -> ux_slave_transfer_request_force_zlp =  UX_FALSE;

                }
                else
                {

                    /* Set the completion code to no error.  */
                    transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

                    /* The transfer is completed.  */
                    transfer_request -> ux_slave_transfer_request_status =  UX_TRANSFER_STATUS_COMPLETED;

                    /* We are using a Control endpoint, if there is a callback, invoke it. We are still under ISR.  */
                    if (transfer_request -> ux_slave_transfer_request_completion_function)
                        transfer_request -> ux_slave_transfer_request_completion_function (transfer_request) ;

                    /* State is now STATUS RX.  */
                    ed -> ux_dcd_nxp_dci_ed_state = UX_DCD_NXP_DCI_ED_STATE_STATUS_RX;
                }
            }
            else
            {

                /* Get the size of the transfer.  */
                transfer_length = transfer_request -> ux_slave_transfer_request_in_transfer_length - endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;

                /* Check if the endpoint size is bigger that data requested. */
                if (transfer_length > endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize)
                {

                    /* Adjust the transfer size.  */
                    transfer_length =  endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;
                }

                /* Adjust the data pointer.  */
                transfer_request -> ux_slave_transfer_request_current_data_pointer += endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;

                /* Adjust the transfer length remaining.  */
                transfer_request -> ux_slave_transfer_request_in_transfer_length -= transfer_length;

                /* Transmit data.  */
                USB_DeviceSendRequest(handle,
                            endpoint->ux_slave_endpoint_descriptor.bEndpointAddress,
                            transfer_request->ux_slave_transfer_request_current_data_pointer,
                            transfer_length);
            }
        }

        /* Check if we have received something on endpoint 0 during data phase .  */
        if (ed -> ux_dcd_nxp_dci_ed_state == UX_DCD_NXP_DCI_ED_STATE_DATA_RX)
        {

            /* Get the pointer to the logical endpoint from the transfer request.  */
            endpoint =  transfer_request -> ux_slave_transfer_request_endpoint;

            /* Read the received data length for the Control endpoint.  */
            transfer_length = message->length;

            /* Update the length of the data received.  */
            transfer_request -> ux_slave_transfer_request_actual_length += transfer_length;

            /* Can we accept this much?  */
            if (transfer_request -> ux_slave_transfer_request_actual_length <=
                transfer_request -> ux_slave_transfer_request_requested_length)
            {

                /* Are we done with this transfer ? */
                if ((transfer_request -> ux_slave_transfer_request_actual_length ==
                     transfer_request -> ux_slave_transfer_request_requested_length) ||
                    (transfer_length != endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize))
                {

                    /* Set the completion code to no error.  */
                    transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

                    /* The endpoint is IN.  This is important to memorize the direction for the control endpoint
                       in case of a STALL. */
                    ed -> ux_dcd_nxp_dci_ed_direction  = UX_ENDPOINT_IN;

                    /* We are using a Control endpoint on a OUT transaction and there was a payload.  */
                    if (_ux_device_stack_control_request_process(transfer_request) == UX_SUCCESS)
                    {

                        /* Set the state to STATUS phase TX.  */
                        ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_STATUS_TX;

                        /* Arm the status transfer.  */
                        USB_DeviceSendRequest(handle, 0x00U, UX_NULL, 0U);
                    }
                }
                else
                {

                    /* Rearm the OUT control endpoint for one packet. */
                    transfer_request -> ux_slave_transfer_request_current_data_pointer += endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;
                    USB_DeviceRecvRequest(handle,
                                endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress,
                                transfer_request -> ux_slave_transfer_request_current_data_pointer,
                                endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize);
                }
            }
            else
            {

                /*  We have an overflow situation. Set the completion code to overflow.  */
                transfer_request -> ux_slave_transfer_request_completion_code =  UX_TRANSFER_BUFFER_OVERFLOW;

                /* If trace is enabled, insert this event into the trace buffer.  */
                UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_BUFFER_OVERFLOW, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)

                /* We are using a Control endpoint, if there is a callback, invoke it. We are still under ISR.  */
                if (transfer_request -> ux_slave_transfer_request_completion_function)
                    transfer_request -> ux_slave_transfer_request_completion_function (transfer_request) ;
            }
        }
    }

    return kStatus_USB_Success;
}
