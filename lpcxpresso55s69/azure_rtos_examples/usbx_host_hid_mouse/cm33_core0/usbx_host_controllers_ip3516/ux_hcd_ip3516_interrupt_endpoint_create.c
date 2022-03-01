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
/*    _ux_hcd_ip3516_interrupt_endpoint_create            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will create an interrupt endpoint. The interrupt      */ 
/*    endpoint has an interval of operation from 1 to 255. In IP3516, the */
/*    hardware assisted interrupt is from 1 to 32.                        */
/*                                                                        */
/*    This routine will match the best interval for the IP3516 hardware.  */
/*    It will also determine the best node to hook the endpoint based on  */ 
/*    the load that already exists on the horizontal ED chain.            */
/*                                                                        */
/*    For the ones curious about this coding. The tricky part is to       */
/*    understand how the interrupt matrix is constructed. We have used    */ 
/*    eds with the skip bit on to build a frame of anchor eds. Each ED    */ 
/*    creates a node for an appropriate combination of interval frequency */ 
/*    in the list.                                                        */
/*                                                                        */
/*    After obtaining a pointer to the list with the lowest traffic, we   */
/*    traverse the list from the highest interval until we reach the      */ 
/*    interval required. At that node, we anchor our real ED to the node  */ 
/*    and link the ED that was attached to the node to our ED.            */ 
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
/*    _ux_hcd_ip3516_least_traffic_list_get Get least traffic list        */
/*    _ux_utility_memory_set                Set memory                    */
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
UINT  _ux_hcd_ip3516_interrupt_endpoint_create(UX_HCD_IP3516 *hcd_ip3516, UX_ENDPOINT *endpoint)
{

UX_DEVICE                       *device;
UINT                            interval;
ULONG                           max_packet_size;
ULONG                           num_transaction;
ULONG                           index;
ULONG                           register_value;
usb_host_ip3516hs_ptl_struct_t  *ptl;
usb_host_ip3516hs_sptl_struct_t *sptl;
UX_IP3516_PERIODIC_PIPE         *pipe;
UCHAR                           *memory_buffer;


    /* Get the pointer to the device.  */
    device =  endpoint -> ux_endpoint_device;

    /* And get an available pipe. */
    for (index = 0; index < 32; index++)
    {
        if (hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes[index].ux_ip3516_pipe_state == UX_IP3516_PIPE_STATE_FREE)
        {
            hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes[index].ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_IDLE;
            hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes[index].ux_ip3516_pipe_index = index;
            break;
        }
    }

    if (index == 32)
        return(UX_NO_ED_AVAILABLE);

    ptl = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_array + index;
    sptl = (usb_host_ip3516hs_sptl_struct_t *)ptl;
    pipe = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes + index;

    if (_tx_byte_allocate(&hcd_ip3516 -> ux_hcd_ip3516_usb_memory_pool, (VOID *) &memory_buffer, UX_IP3516_PERIODIC_ENDPOINT_BUFFER_SIZE, TX_NO_WAIT) != TX_SUCCESS)
    {
        pipe -> ux_ip3516_pipe_state = UX_IP3516_PIPE_STATE_FREE;
        return(UX_MEMORY_INSUFFICIENT);
    }
    pipe -> ux_ip3516_ed_buffer_address = memory_buffer;
    pipe -> ux_ip3516_ed_buffer_len = 512;

    _ux_utility_memory_set(ptl, 0, sizeof(usb_host_ip3516hs_ptl_struct_t));

    /* Attach the ED to the endpoint container.  */
    endpoint -> ux_endpoint_ed =   (VOID *) &(hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes[index]);

    /* Set the default MPS Capability info in the ED.  */
    max_packet_size = endpoint -> ux_endpoint_descriptor.wMaxPacketSize & UX_MAX_PACKET_SIZE_MASK;

    ptl->control1Union.stateBitField.MaxPacketLength   = max_packet_size;

    /* Set the device address.  */
    ptl->control2Union.stateBitField.DeviceAddress = device -> ux_device_address;
    
    /* Add the endpoint address.  */
    ptl->control2Union.stateBitField.EP   = endpoint -> ux_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* Set the High Bandwidth Pipe Multiplier to number transactions.  */    
    num_transaction = (endpoint -> ux_endpoint_descriptor.wMaxPacketSize & UX_MAX_NUMBER_OF_TRANSACTIONS_MASK) >> UX_MAX_NUMBER_OF_TRANSACTIONS_SHIFT;
    if (num_transaction < 3)
        num_transaction ++;

    ptl->control1Union.stateBitField.Mult = num_transaction;

    /* Set the device speed for full and low speed devices behind a HUB. The HUB address and the 
       port index must be stored in the endpoint. For low/full speed devices, the C-mask field must be set.  */
    switch (device -> ux_device_speed)
    {

    case UX_HIGH_SPEED_DEVICE:

        /* Set the interval mask for high speed endpoints.  */
        pipe -> ux_ip3516_ed_interval_mask =  (UCHAR)(1 << (endpoint -> ux_endpoint_descriptor.bInterval - 1)) - 1;
        interval = 1 << (endpoint -> ux_endpoint_descriptor.bInterval - 1);
        ptl->control2Union.stateBitField.S = 0;
        break;

    default:

        /* Set the interval mask for other endpoints.  */
        pipe -> ux_ip3516_ed_interval_mask = endpoint -> ux_endpoint_descriptor.bInterval;
        pipe -> ux_ip3516_ed_interval_mask |= pipe -> ux_ip3516_ed_interval_mask >> 1;
        pipe -> ux_ip3516_ed_interval_mask |= pipe -> ux_ip3516_ed_interval_mask >> 2;
        pipe -> ux_ip3516_ed_interval_mask |= pipe -> ux_ip3516_ed_interval_mask >> 4;
        pipe -> ux_ip3516_ed_interval_mask >>= 1;
        interval = endpoint -> ux_endpoint_descriptor.bInterval * 8;

#if UX_MAX_DEVICES > 1
        /* The device must be on a hub for this code to execute. We still do a sanity check.  */
        if (device -> ux_device_parent != UX_NULL)
        {

            /* Store the parent hub device address.  */
            sptl->control2Union.stateBitField.HubAddress =  device -> ux_device_parent -> ux_device_address;

            /* And the port index onto which this device is attached.  */                                    
            sptl->control2Union.stateBitField.PortNumber =  device -> ux_device_port_location;

            if (device -> ux_device_parent -> ux_device_speed == UX_HIGH_SPEED_DEVICE)
            {
                sptl->control2Union.stateBitField.S = 1;
            }
            else
            {
                ptl->control2Union.stateBitField.S = 0;
            }
        }
#endif
        break;
    }

    if (interval < 16)
        ptl->control1Union.stateBitField.uFrame = 0;
    else if (interval < 32)
        ptl->control1Union.stateBitField.uFrame = 1;
    else if (interval < 64)
        ptl->control1Union.stateBitField.uFrame = 2;
    else if (interval < 128)
        ptl->control1Union.stateBitField.uFrame = 3;
    else if (interval < 256)
        ptl->control1Union.stateBitField.uFrame = 4;
    else
        ptl->control1Union.stateBitField.uFrame = 5;

    pipe -> ux_ip3516_ed_interval_position = _ux_hcd_ip3516_least_traffic_list_get(hcd_ip3516, pipe -> ux_ip3516_ed_interval_mask);

    ptl->control1Union.stateBitField.uFrame |= ((pipe -> ux_ip3516_ed_interval_position / 8) << 3);
    pipe -> ux_ip3516_ed_uSA = 0;
    for (index = 0; index < 8; index++)
    {
        if ((index & pipe -> ux_ip3516_ed_interval_mask) == pipe -> ux_ip3516_ed_interval_position)
            pipe -> ux_ip3516_ed_uSA |= (1 << index);
    }

    ptl->control2Union.stateBitField.RL  = 0xFU;
    ptl->stateUnion.stateBitField.NakCnt = 0xFU;
    if (device -> ux_device_speed == UX_LOW_SPEED_DEVICE)
    {
        ptl->control2Union.stateBitField.SE = 2;
    }
    else
    {
        ptl->control2Union.stateBitField.SE = 0;
    }

    ptl->dataUnion.dataBitField.I        = 1U;
    ptl->stateUnion.stateBitField.EpType = endpoint -> ux_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE;
    ptl->stateUnion.stateBitField.DT     = 0U;
    ptl->stateUnion.stateBitField.P      = 0U;

    register_value = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_INTPTDS);

    register_value &= ~(1 << index);

    /* Set the ATL PTD Skip Register.  */
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_INTPTDS, register_value);

    pipe -> ux_ip3516_ed_endpoint = endpoint;

    /* We need to take into account the nature of the HCD to define the max size
       of any transfer in the transfer request.  */
    endpoint -> ux_endpoint_transfer_request.ux_transfer_request_maximum_length =  UX_IP3516_PERIODIC_ENDPOINT_BUFFER_SIZE;

    /* Return successful completion.  */
    return(UX_SUCCESS);         
}
