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
/*    _ux_hcd_ip3516_port_status_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will return the status for each port attached to the  */
/*    root HUB.                                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_ip3516                            Pointer to IP3516 controller  */ 
/*    port_index                            Port index to get status for  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Port Status                                                         */ 
/*                                                                        */ 
/*      Status of the root hub port with the following format:            */
/*                                                                        */ 
/*               bit 0         device connection status                   */
/*                             if 0 : no device connected                 */
/*                             if 1 : device connected to the port        */
/*               bit 1         port enable status                         */
/*                             if 0 : port disabled                       */
/*                             if 1 : port enabled                        */
/*               bit 2         port suspend status                        */
/*                             if 0 : port is not suspended               */
/*                             if 1 : port is suspended                   */
/*               bit 3         port overcurrent status                    */
/*                             if 0 : port has no overcurrent condition   */
/*                             if 1 : port has overcurrent condition      */
/*               bit 4         port reset status                          */
/*                             if 0 : port is not in reset                */
/*                             if 1 : port is in reset                    */
/*               bit 5         port power status                          */
/*                             if 0 : port power is off                   */
/*                             if 1 : port power is on                    */
/*               bit 6-7       device attached speed                      */
/*                             if 00 : low speed device attached          */
/*                             if 01 : full speed device attached         */
/*                             if 10 : high speed device attached         */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_hcd_ip3516_register_read          Read IP3516 register          */ 
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
ULONG  _ux_hcd_ip3516_port_status_get(UX_HCD_IP3516 *hcd_ip3516, ULONG port_index)
{

ULONG       ip3516_register_port_status;
ULONG       port_status;


    /* Check to see if this port is valid on this controller.  */
    if (hcd_ip3516 -> ux_hcd_ip3516_nb_root_hubs < port_index)
    {

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_HCD, UX_PORT_INDEX_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_PORT_INDEX_UNKNOWN, port_index, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_PORT_INDEX_UNKNOWN);
    }
    
    /* The port is valid, build the status mask for this port. This function
       returns a controller agnostic bit field.  */
    port_status =  0;
    ip3516_register_port_status =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_PORT_SC + port_index);
                                    
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_PORT_SC + port_index, ip3516_register_port_status);

    /* Device Connection Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_CCS)
        port_status |=  UX_PS_CCS;
    else
    {

        /* When disconnected PHY does not know speed.  */
        UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ip3516, UX_FALSE);
    }
                                    
    /* Port Enable Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_PE)
        port_status |=  UX_PS_PES;

    /* Port Suspend Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_SUSPEND)
    {
        port_status |=  UX_PS_PSS;

        /* When suspend put PHY in normal to avoid wrong disconnect status.  */
        UX_HCD_IP3516_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ip3516, UX_FALSE);
    }

    /* Port Overcurrent Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_OCC)
        port_status |=  UX_PS_POCI;

    /* Port Reset Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_PR)
        port_status |=  UX_PS_PRS;

    /* Port Power Status.  */
    if (ip3516_register_port_status & IP3516_HC_PS_PP)
        port_status |=  UX_PS_PPS;

    /* Port Device Attached speed. This field is valid only if the CCS bit is active. 
       Only IP3516 high speed devices are meaningful in a regular IP3516 controller. 
       In embedded IP3516 with built-in TTs some bits reflect the true speed of
       the device behind the TT. */
    if (ip3516_register_port_status & IP3516_HC_PS_CCS)
    {
        /* Check for IP3516 with embedded TT.  */
        if (hcd_ip3516 -> ux_hcd_ip3516_embedded_tt == UX_TRUE)
        {
    
            /* Isolate speed from the non IP3516 compliant POTSC bits.  */
            switch (ip3516_register_port_status & IP3516_HC_PS_EMBEDDED_TT_SPEED_MASK)
            {
            
                case IP3516_HC_PS_EMBEDDED_TT_SPEED_FULL        :

                    /* Full speed.  */
                    port_status |=  UX_PS_DS_FS;
                    break;        

                case IP3516_HC_PS_EMBEDDED_TT_SPEED_LOW         :

                    /* Low speed.  */
                    port_status |=  UX_PS_DS_LS;
                    break;        

                case IP3516_HC_PS_EMBEDDED_TT_SPEED_HIGH        :

                    /* High speed.  */
                    port_status |=  UX_PS_DS_HS;
                    break;        

            }
        }
        else

            /* No embedded TT. Fall back to default HS.  */
            port_status |=  UX_PS_DS_HS;
    }
            
    /* Return port status.  */
    return(port_status);            
}

