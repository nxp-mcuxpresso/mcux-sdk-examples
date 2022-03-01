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
/*    _ux_hcd_ip3516_request_transfer                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*     This function is the handler for all the transactions on the USB.  */
/*     The transfer request passed as parameter contains the endpoint and */ 
/*     the device descriptors in addition to the type of transaction de   */ 
/*     be executed.                                                       */ 
/*                                                                        */
/*     This function routes the transfer_request to according to the type */ 
/*     of transfer to be executed.                                        */ 
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
/*    _ux_hcd_ip3516_request_control_transfer   Start control transfer    */
/*    _ux_hcd_ip3516_request_bulk_transfer      Start bulk transfer       */
/*    _ux_hcd_ip3516_request_interrupt_transfer Start interrupt transfer  */
/*    _ux_hcd_ip3516_request_isochronous_transfer                         */
/*                                              Start iso transfer        */
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
UINT  _ux_hcd_ip3516_request_transfer(UX_HCD_IP3516 *hcd_ip3516, UX_TRANSFER *transfer_request)
{

UX_ENDPOINT     *endpoint;
UINT            status;
    

    /* Get the pointer to the Endpoint.  */
    endpoint =  (UX_ENDPOINT *) transfer_request -> ux_transfer_request_endpoint;

    /* We reset the actual length field of the transfer request as a safety measure.  */
    transfer_request -> ux_transfer_request_actual_length =  0;
    
    /* Isolate the endpoint type and route the transfer request.  */
    switch ((endpoint -> ux_endpoint_descriptor.bmAttributes) & UX_MASK_ENDPOINT_TYPE)
    {

    case UX_CONTROL_ENDPOINT:
    
        status =  _ux_hcd_ip3516_request_control_transfer(hcd_ip3516, transfer_request);
        break;


    case UX_BULK_ENDPOINT:

        status =  _ux_hcd_ip3516_request_bulk_transfer(hcd_ip3516, transfer_request);
        break;


    case UX_INTERRUPT_ENDPOINT:

        status =  _ux_hcd_ip3516_request_interrupt_transfer(hcd_ip3516, transfer_request);
        break;


    case UX_ISOCHRONOUS_ENDPOINT:

        status =  _ux_hcd_ip3516_request_isochronous_transfer(hcd_ip3516, transfer_request);
        break;

    }

    /* Note that it is physically impossible to have a wrong endpoint type here
       so no error checking.  */
    return(status);         
}

