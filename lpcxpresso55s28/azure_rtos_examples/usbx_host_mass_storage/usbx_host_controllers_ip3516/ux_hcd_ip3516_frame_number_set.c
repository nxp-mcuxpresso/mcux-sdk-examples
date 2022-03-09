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
/*    _ux_hcd_ip3516_frame_number_set                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function will set the current frame number to the one          */ 
/*    specified. This function is mostly used for isochronous purposes.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hcd_ip3516                            Pointer to IP3516 controller  */ 
/*    frame_number                          Frame number                  */ 
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
VOID  _ux_hcd_ip3516_frame_number_set(UX_HCD_IP3516 *hcd_ip3516, ULONG frame_number)
{

ULONG       ip3516_register;


    /* It is illegal in IP3516 to set the frame number while the controller is 
       running.  */
    ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);
    if (ip3516_register & IP3516_HC_IO_RS)
        return;
               
    /* The register is based on micro frames, so we need to multiply the
       value by 8 to get to the millisecond frame number.  */     
    ip3516_register =  frame_number << 3;

    /* Write the frame number, by default the micro frame will be set to 0.  */
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_FRAME_INDEX, ip3516_register);
    
    /* Return to caller.  */
    return;
}

