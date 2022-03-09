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
/*    _ux_hcd_ip3516_least_traffic_list_get               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function return a pointer to the first ED in the periodic tree */
/*    that has the least traffic registered.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    hcd_ip3516                            Pointer to IP3516 controller  */
/*    microframe_load                       Pointer to an array for 8     */
/*                                          micro-frame loads             */
/*    microframe_ssplit_count               Pointer to an array for 8     */
/*                                          micro-frame start split count */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    UX_IP3516_ED *                        Pointer to ED                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT   _ux_hcd_ip3516_least_traffic_list_get(UX_HCD_IP3516 *hcd_ip3516, ULONG interval_mask)
{

UX_IP3516_PERIODIC_PIPE     *pipe;
UINT                list_index;
UINT                index;
ULONG               min_bandwidth_used;
ULONG               bandwidth_used;
UINT                min_bandwidth_slot;


    /* Set the min bandwidth used to a arbitrary maximum value.  */
    min_bandwidth_used =  0xffffffff;

    /* The first ED is the list candidate for now.  */
    min_bandwidth_slot =  0;

    /* All list will be scanned.  */
    for (list_index = 0; (list_index & interval_mask) == list_index; list_index++)
    {

        /* Reset the bandwidth for this list.  */
        bandwidth_used =  0;

        /* Parse the eds in the list.  */
        for (index = 0; index < 32; index++)
        {

            pipe = hcd_ip3516 -> ux_hcd_ip3516_int_ptl_pipes + index;
            if (pipe -> ux_ip3516_ed_endpoint != UX_NULL)
            if ((list_index & pipe -> ux_ip3516_ed_interval_mask) == pipe -> ux_ip3516_ed_interval_position)
            {

                /* Add to the bandwidth used the max packet size pointed by this ED.  */
                bandwidth_used +=  (ULONG) pipe -> ux_ip3516_ed_endpoint -> ux_endpoint_descriptor.wMaxPacketSize;
            }
        }

        /* We have processed a list, check the bandwidth used by this list.
           If this bandwidth is the minimum, we memorize the ED.  */
        if (bandwidth_used < min_bandwidth_used)
        {

            /* We have found a better list with a lower used bandwidth, memorize the bandwidth
               for this list.  */
            min_bandwidth_used =  bandwidth_used;

            /* Memorize the begin ED for this list.  */
            min_bandwidth_slot =  list_index;
        }
    }

    /* Return the ED list with the lowest bandwidth.  */
    return(min_bandwidth_slot);
}
