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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_ip3511_endpoint_create                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will create a physical endpoint.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_ip3511                            Pointer to device controller  */
/*    endpoint                              Pointer to endpoint container */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    USB_DeviceInitEndpoint                Init endpoint                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    IP3511 Controller Driver                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_ip3511_endpoint_create(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_IP3511_ED     *ed;
ULONG               ip3511_endpoint_index;
usb_device_endpoint_init_struct_t epInitStruct;
usb_device_endpoint_callback_struct_t epCallback;
usb_status_t status;


    /* The endpoint index in the array of the IP3511 must match the endpoint number.  */
    ip3511_endpoint_index =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    ip3511_endpoint_index = ip3511_endpoint_index << 1;

    if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)
        ip3511_endpoint_index++;

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_ip3511 -> ux_dcd_ip3511_ed[ip3511_endpoint_index];

    /* Check the index range and endpoint status, if it is free, reserve it. If not reject this endpoint.  */
    if ((ip3511_endpoint_index < UX_DCD_IP3511_MAX_ED) && ((ed -> ux_dcd_ip3511_ed_status & UX_DCD_IP3511_ED_STATUS_USED) == 0))
    {

        /* We can use this endpoint.  */
        ed -> ux_dcd_ip3511_ed_status |=  UX_DCD_IP3511_ED_STATUS_USED;

        /* Keep the physical endpoint address in the endpoint container.  */
        endpoint -> ux_slave_endpoint_ed =  (VOID *) ed;

        /* Save the endpoint pointer.  */
        ed -> ux_dcd_ip3511_ed_endpoint =  endpoint;

        /* And its index.  */
        ed -> ux_dcd_ip3511_ed_index =  ip3511_endpoint_index;

        /* And its direction.  */
        ed -> ux_dcd_ip3511_ed_direction =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION;

        /* Check if it is non-control endpoint.  */
        if (ip3511_endpoint_index != 0)
        {

            /* Initialize the endpoint structures.  */
            epCallback.callbackFn    = _ux_dcd_ip3511_transfer_callback;
            epCallback.callbackParam = ed;

            epInitStruct.zlt             = 0U;
            epInitStruct.transferType    = endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE;
            epInitStruct.interval        = endpoint -> ux_slave_endpoint_descriptor.bInterval;
            epInitStruct.endpointAddress = endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress;
            epInitStruct.maxPacketSize   = endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize;

            /* Initialize the endpoint.  */
            status = USB_DeviceInitEndpoint(dcd_ip3511 -> handle, &epInitStruct, &epCallback);

            if (kStatus_USB_Success != status)
            {
                return(UX_NO_ED_AVAILABLE);
            }
        }

        /* Return successful completion.  */
        return(UX_SUCCESS);
    }

    /* Return an error.  */
    return(UX_NO_ED_AVAILABLE);
}

