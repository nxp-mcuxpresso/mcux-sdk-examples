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
/**   IP3511 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_IP3511_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_ip3511.h"
#include "ux_device_stack.h"
#include "ux_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_ip3511_transfer_callback                    PORTABLE C      */
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
/*    _ux_utility_semaphore_put             Put semaphore                 */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    IP3511 Driver                                                       */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
usb_status_t _ux_dcd_ip3511_transfer_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam)
{

UX_DCD_IP3511_ED         *ed;
UX_SLAVE_TRANSFER       *transfer_request;

    ed = (UX_DCD_IP3511_ED*) callbackParam;

    /* Get the pointer to the transfer request.  */
    transfer_request =  &(ed -> ux_dcd_ip3511_ed_endpoint -> ux_slave_endpoint_transfer_request);

    if (message->length != USB_UNINITIALIZED_VAL_32)
    {

        if (ed -> ux_dcd_ip3511_ed_direction == UX_ENDPOINT_OUT)
        {

            /* Update the length of the data sent in previous transaction.  */
            transfer_request -> ux_slave_transfer_request_actual_length =  message->length;
        }

        /* Set the completion code to no error.  */
        transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

        /* The transfer is completed.  */
        transfer_request -> ux_slave_transfer_request_status =  UX_TRANSFER_STATUS_COMPLETED;

        /* Non control endpoint operation, use semaphore.  */
        _ux_utility_semaphore_put(&transfer_request -> ux_slave_transfer_request_semaphore);
    }

    return kStatus_USB_Success;
}


