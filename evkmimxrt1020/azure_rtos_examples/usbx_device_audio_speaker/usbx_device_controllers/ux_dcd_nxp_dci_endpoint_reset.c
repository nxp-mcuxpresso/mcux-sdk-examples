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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_nxp_dci_endpoint_reset                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will reset a physical endpoint.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_nxp_dci                           Pointer to device controller  */
/*    endpoint                              Pointer to endpoint container */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    USB_DeviceUnstallEndpoint             Clear STALL                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NXP_DCI Controller Driver                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_nxp_dci_endpoint_reset(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_NXP_DCI_ED     *ed;


    /* Get the physical endpoint address in the endpoint container.  */
    ed =  (UX_DCD_NXP_DCI_ED *) endpoint -> ux_slave_endpoint_ed;

    /* Set the status of the endpoint to not stalled.  */
    ed -> ux_dcd_nxp_dci_ed_status &= ~UX_DCD_NXP_DCI_ED_STATUS_STALLED;

    /* Set the state of the endpoint to IDLE.  */
    ed -> ux_dcd_nxp_dci_ed_state =  UX_DCD_NXP_DCI_ED_STATE_IDLE;

    /* Clear STALL condition.  */
    USB_DeviceUnstallEndpoint(dcd_nxp_dci -> handle, endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress);

    /* This function never fails.  */
    return(UX_SUCCESS);

}
