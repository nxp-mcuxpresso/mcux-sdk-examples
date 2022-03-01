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
/*    _ux_dcd_mcimx6_endpoint_destroy                       PORTABLE C    */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will destroy a physical endpoint.                     */
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
UINT  _ux_dcd_mcimx6_endpoint_destroy(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_MCIMX6_ED            *ed;
ULONG                       mcimx6_register;
ULONG                       endpoint_qed_address;
ULONG                       endpoint_address;

    /* First extract the endpoint address. We keep this value as the endpoint index for accessing the EPCRn registers.
       The MCIMX6 has 4 endpoints maximum.  Each endpoint has a IN and OUT QH attached to it.  */
    endpoint_address =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* At first the endpoint QED index is the endpoint address * 2 this will change if the endpoint is IN or OUT.  */
    endpoint_qed_address = endpoint_address << 1;

    /* The direction will give us the index increment, endpoint OUT are first. We
       also use this test to compute the register enable bit to turn off */
    if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)
    {
        /* The endpoint is a IN.  Adjust the index accordingly.  */
        endpoint_qed_address++;

        /* Disable TX endpoint.  */
        mcimx6_register =  UX_DCD_MCIMX6_EPCRN_TXE;
    }
    else
        /* Disable RX endpoint.  */
        mcimx6_register =  UX_DCD_MCIMX6_EPCRN_RXE;

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_qed_address];

    /* We do not disable control endpoint 0. USB spec says it must always be enabled.  */
    if (endpoint_address != 0)

        /* Turn off the endpoint enable flag.  */
        _ux_dcd_mcimx6_register_clear(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCR +
                                        (endpoint_address * 4), mcimx6_register);

    /* We can free this endpoint.  */
    ed -> ux_dcd_mcimx6_ed_status =  UX_DCD_MCIMX6_ED_STATUS_UNUSED;

    /* This function never fails.  */
    return(UX_SUCCESS);
}
