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
/*    _ux_dcd_mcimx6_endpoint_status                        PORTABLE C    */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will retrieve the status of the endpoint.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    endpoint_index                        Endpoint index                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _ux_dcd_mcimx6_endpoint_status(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG endpoint_index)
{

UX_DCD_MCIMX6_ED      *ed;


    /* Fetch the address of the physical endpoint.  We have the endpoint index here only.
       Since endpoints cannot be bi-directional in USBX, we need to check both directions
       to get the real endpoints.  */
    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_index << 1];

    /* Check the endpoint status, if it is free, we have the wrong direction.  */
    if ((ed -> ux_dcd_mcimx6_ed_status & UX_DCD_MCIMX6_ED_STATUS_USED) == 0)
    {

        /* Check the next entry, i.e. the other endpoint direction.  */
        ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[(endpoint_index << 1) + 1];

        /* Check the endpoint status, if it is free, we have a wrong endpoint.  */
        if ((ed -> ux_dcd_mcimx6_ed_status & UX_DCD_MCIMX6_ED_STATUS_USED) == 0)
            return(UX_ERROR);

    }

    /* Check if the endpoint is stalled.  */
    if ((ed -> ux_dcd_mcimx6_ed_status & UX_DCD_MCIMX6_ED_STATUS_STALLED) == 0)
        return(UX_FALSE);
    else
        return(UX_TRUE);
}
