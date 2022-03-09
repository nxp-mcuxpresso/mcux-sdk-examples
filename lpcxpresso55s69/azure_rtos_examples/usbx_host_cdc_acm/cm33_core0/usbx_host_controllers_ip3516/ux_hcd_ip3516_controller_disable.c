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
/*    _ux_hcd_ip3516_controller_disable                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will disable the IP3516 controller. The controller    */
/*    will release all its resources (memory, IO ...). After this, the    */
/*    controller will not send SOF any longer.                            */
/*                                                                        */
/*    All transactions should have been completed, all classes should     */ 
/*    have been closed.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_ip3516                            Pointer to IP3516 controller  */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
UINT  _ux_hcd_ip3516_controller_disable(UX_HCD_IP3516 *hcd_ip3516)
{

UX_HCD      *hcd;
ULONG       ip3516_register;
    

    /* Point to the generic portion of the host controller structure instance.  */
    hcd =  hcd_ip3516 -> ux_hcd_ip3516_hcd_owner;
    
    /* Stop the controller.  */
    ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);
    ip3516_register =  IP3516_HC_IO_HCRESET;
    ip3516_register &= ~IP3516_HC_IO_RS;
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_COMMAND, ip3516_register);
    
    /* Wait for the Stop signal to be acknowledged by the controller.  */
    ip3516_register =  0;
    while ((ip3516_register&IP3516_HC_STS_HC_HALTED) == 0)
    {

        ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCCR_HCS_PARAMS);
    }
         
    /* Reflect the state of the controller in the main structure.  */
    hcd -> ux_hcd_status =  UX_HCD_STATUS_HALTED;

    /* Return successful completion.  */
    return(UX_SUCCESS);
}

