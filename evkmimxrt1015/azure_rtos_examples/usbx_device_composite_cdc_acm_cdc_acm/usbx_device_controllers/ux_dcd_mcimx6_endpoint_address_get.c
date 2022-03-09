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
/**   MCIMX6 Controller Driver                                           */
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
/*  FUNCTION                                                RELEASE       */
/*                                                                        */
/*    _ux_dcd_mcimx6_endpoint_address_get                  PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the address of the physical endpoint for a    */
/*    given endpoint address.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    endpoint                              Pointer to endpoint           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ed                                                                  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UX_DCD_MCIMX6_ED *  _ux_dcd_mcimx6_endpoint_address_get(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint,
                                                        ULONG endpoint_direction)
{

UX_DCD_MCIMX6_ED        *ed;
ULONG                   endpoint_ed_address;

    /* First extract the endpoint address. We keep this value as the endpoint index for accessing
       the endpoint registers.  */
    endpoint_ed_address =  (endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION) << 1;

    /* The direction will give us the index increment, endpoint OUT are first.  */
    if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)

        /* The endpoint is a IN.  Adjust the index accordingly.  */
        endpoint_ed_address++;

    /* If this is a control endpoint, the direction is meaningless since it is bi-directional,
       we take the direction from the calling parameter.  */
    if ((endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE) == UX_CONTROL_ENDPOINT)
    {

        /* Check the direction from calling parameter.  */
        if (endpoint_direction == UX_ENDPOINT_IN)

            /* The endpoint is a IN.  Adjust the index accordingly.  */
            endpoint_ed_address++;

    }

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_ed_address];

    /* We are done.  */
    return(ed);
}
