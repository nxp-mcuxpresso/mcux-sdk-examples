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
/*    _ux_hcd_ip3516_asynchronous_endpoint_create         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will create an asynchronous endpoint. The control     */
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
/*    _tx_byte_allocate                     Allocate buffer               */
/*    _ux_utility_memory_set                Set nemory                    */
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
UINT  _ux_hcd_ip3516_asynchronous_endpoint_create(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint)
{

UX_DEVICE               *device;
ULONG                   index;
ULONG                   register_value;
usb_host_ip3516hs_atl_struct_t *atl;
UCHAR *                 memory_buffer;
ULONG                   buffer_size;
UX_IP3516_PIPE          *pipe;



    /* And get an available pipe. */
    for (index = 0; index < 32; index++)
    {
        if (hcd_ip3516 -> ux_hcd_ip3516_atl_pipes[index].ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_FREE)
        {
            hcd_ip3516 -> ux_hcd_ip3516_atl_pipes[index].ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_IDLE;
            hcd_ip3516 -> ux_hcd_ip3516_atl_pipes[index].ux_ip3516_pipe_index = index;
            break;
        }
    }

    if (index == 32)
        return(UX_NO_ED_AVAILABLE);

    device =  endpoint -> ux_endpoint_device;
    pipe = hcd_ip3516 -> ux_hcd_ip3516_atl_pipes + index;

    if (((endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE) == UX_CONTROL_ENDPOINT)
    {
        buffer_size =  UX_IP3516_CONTROL_ENDPOINT_BUFFER_SIZE;
    }
    else
    {
        if (device -> ux_device_speed == UX_FULL_SPEED_DEVICE)
        {
            buffer_size =  UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE_FS;
        }
        else
        {
            buffer_size =  UX_IP3516_BULK_ENDPOINT_BUFFER_SIZE;
        }
    }

    if (_tx_byte_allocate(&hcd_ip3516 -> ux_hcd_ip3516_usb_memory_pool, (VOID*)&memory_buffer, buffer_size, TX_NO_WAIT) != TX_SUCCESS)
    {
        pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_FREE;
        return(UX_MEMORY_INSUFFICIENT);
    }

    /* We need to take into account the nature of the HCD to define the max size
       of any transfer in the transfer request.  */
    endpoint -> ux_endpoint_transfer_request.ux_transfer_request_maximum_length =  buffer_size;

    pipe -> ux_ip3516_ed_buffer_address = memory_buffer;
    pipe -> ux_ip3516_ed_buffer_len = 2048;

    atl = hcd_ip3516 -> ux_hcd_ip3516_atl_array + index;

    _ux_utility_memory_set(atl, 0, sizeof(usb_host_ip3516hs_atl_struct_t));

    /* Attach the ED to the endpoint container.  */
    endpoint -> ux_endpoint_ed =  (VOID *) &(hcd_ip3516 -> ux_hcd_ip3516_atl_pipes[index]);

    atl -> control1Union.stateBitField.Mult =  1;

    atl -> control2Union.stateBitField.EP = endpoint -> ux_endpoint_descriptor.bEndpointAddress & 0xF;

    /* Set the default MPS Capability info in the ED.  */
    atl -> control1Union.stateBitField.MaxPacketLength = endpoint -> ux_endpoint_descriptor.wMaxPacketSize;

    /* Set the default NAK reload count.  */
    atl->control2Union.stateBitField.RL  = 0xFU;
    atl->stateUnion.stateBitField.NakCnt = 0xFU;
    
    /* If the device is not high speed and the endpoint is control, then the CEF bit must be set to on.  */

    /* Set the device address.  */
    atl -> control2Union.stateBitField.DeviceAddress = device -> ux_device_address;

    /* Add the endpoint address.  */
    /* Set the device speed for full and low speed devices behind a hub the hub address and the 
       port index must be stored in the endpoint.  */
    switch (device -> ux_device_speed)
    {

    case  UX_HIGH_SPEED_DEVICE:

        break;

    case  UX_LOW_SPEED_DEVICE:

        atl->control2Union.stateBitField.SE = 2;

    case  UX_FULL_SPEED_DEVICE:

#if UX_MAX_DEVICES > 1
        /* The device must be on a hub for this code to execute. We still do a sanity check.  */
        if (device -> ux_device_parent != UX_NULL)
        {

            /* Store the parent hub device address.  */
            atl->control2Union.stateBitField.HubAddress =  device -> ux_device_parent -> ux_device_address;

            /* And the port index onto which this device is attached.  */                                    
            atl->control2Union.stateBitField.PortNumber =  device -> ux_device_port_location;

            if (device -> ux_device_parent -> ux_device_speed == UX_HIGH_SPEED_DEVICE)
            {
                atl->control2Union.stateBitField.S = 1;
            }
        }
#endif
        break;
    }

    atl->dataUnion.dataBitField.I        = 1U;
    atl->stateUnion.stateBitField.EpType = endpoint -> ux_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE;
    atl->stateUnion.stateBitField.DT     = 0U;
    atl->stateUnion.stateBitField.P      = 0U;

    register_value = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_ATLPTDS);

    register_value &= ~(1 << index);

    /* Set the ATL PTD Skip Register.  */
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDS, register_value);

    /* Return successful completion.  */
    return(UX_SUCCESS);         
}

