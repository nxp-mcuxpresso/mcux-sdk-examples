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
/*  FUNCTION                                                RELEASE       */
/*                                                                        */
/*    _ux_dcd_mcimx6_qtd_get                               PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will return a free QTD if available.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    QTD or error.                                                       */
/*                                                                        */
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
UX_DCD_MCIMX6_QTD *  _ux_dcd_mcimx6_qtd_get(UX_DCD_MCIMX6 *dcd_mcimx6)
{

UX_DCD_MCIMX6_QTD    *qtd;
ULONG                qtd_index;

    /* Set the Mutex as this is a critical section.  */
    _ux_utility_mutex_on(&_ux_system -> ux_system_mutex);

    /* Initialize the qtd_index .  */
    qtd_index =  0;

    /* Obtain the very first QTD from the list .  */
    qtd =  dcd_mcimx6 -> ux_dcd_mcimx6_qtd_head;

    /* Parse all the QTD if necessary.  */
    while (qtd_index++ < UX_DCD_MCIMX6_MAX_QTD)
    {

        /* Check the QTD status.  */
        if (qtd -> ux_dcd_mcimx6_qtd_control == UX_DCD_MCIMX6_QTD_STATUS_FREE)
        {

            /* Now the QTD can be used.  */
            qtd -> ux_dcd_mcimx6_qtd_control =  UX_DCD_MCIMX6_QTD_STATUS_USED;

            /* Release the protection.  */
            _ux_utility_mutex_off(&_ux_system -> ux_system_mutex);

            /* We have found a free QTD, return it.  */
            return(qtd);

        }

        /* Next QTD.  */
        qtd++;

    }

    /* Release the protection.  */
    _ux_utility_mutex_off(&_ux_system -> ux_system_mutex);

    /* No more QTDs. Return to caller with error.  */
    return(UX_NULL);
}
