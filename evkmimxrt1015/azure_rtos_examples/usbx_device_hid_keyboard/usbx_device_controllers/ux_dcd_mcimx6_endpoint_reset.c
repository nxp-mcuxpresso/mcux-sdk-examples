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
/**   MCIMX6 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_mcimx6.h"
#include "ux_device_stack.h"
#include "ux_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                                 RELEASE      */
/*                                                                        */
/*    _ux_dcd_mcimx6_endpoint_reset                         PORTABLE C    */
/*                                                           6.0          */
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
/*    dcd_mcimx6                            Pointer to device controller  */
/*    endpoint                              Pointer to endpoint container */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_read          Read register                 */
/*    _ux_dcd_mcimx6_register_write         Write register                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    MCIMX6 Controller Driver                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_mcimx6_endpoint_reset(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_MCIMX6_ED       *ed;

    /* Flush the endpoint according to its type and direction.  */
    if ((endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE) != UX_CONTROL_ENDPOINT)

        /* Flush the endpoint first. Non control endpoint, direction is taken from the endpoint descriptor. */
        _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, 0);

    else
    {
        /* Flush the endpoint first. Control endpoint, so flush both directions. */
        _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, UX_ENDPOINT_OUT);
        _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, UX_ENDPOINT_IN);
    }

    /* Get the address of the physical endpoint.  We do not care about the direction here.  */
    ed =  _ux_dcd_mcimx6_endpoint_address_get(dcd_mcimx6, endpoint, UX_ENDPOINT_OUT);

    /* Clear STALL on this endpoint.  Both the receive and transmit STALL bits are cleared here.  */
    _ux_dcd_mcimx6_register_clear(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCR + (ed -> ux_dcd_mcimx6_ed_address * 4),
                                (UX_DCD_MCIMX6_EPCRN_RXS | UX_DCD_MCIMX6_EPCRN_TXS));

    /* Reset Toggle on this endpoint.  We differentiate the direction here. */
    if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCR + (ed -> ux_dcd_mcimx6_ed_address * 4),
                                    UX_DCD_MCIMX6_EPCRN_TXR);
    else
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCR + (ed -> ux_dcd_mcimx6_ed_address * 4),
                                    UX_DCD_MCIMX6_EPCRN_RXR);

    /* Set the state of the endpoint to not stalled.  */
    ed -> ux_dcd_mcimx6_ed_status &=  ~UX_DCD_MCIMX6_ED_STATUS_STALLED;

    /* This function never fails.  */
    return(UX_SUCCESS);
}
