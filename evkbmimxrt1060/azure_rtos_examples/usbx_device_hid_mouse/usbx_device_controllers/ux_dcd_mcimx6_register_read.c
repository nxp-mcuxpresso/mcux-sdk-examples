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
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_read                        PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads a bit in a register of the MCIMX6.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    width                                 Width of register             */
/*    register                              Register to read              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    value                                                               */
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
ULONG  _ux_dcd_mcimx6_register_read(UX_DCD_MCIMX6 *dcd_mcimx6, ULONG width,
                                    ULONG mcimx6_register)
{
ULONG    value = 0;

    /* Select the width of the value.  */
    switch (width)
    {
        case UX_DCD_MCIMX6_32BIT_REG        :

            /* 32 bit value.  */
            value =  *((volatile ULONG *) (dcd_mcimx6 -> ux_dcd_mcimx6_base + mcimx6_register));
            break;

        case UX_DCD_MCIMX6_16BIT_REG        :

            /* 16 bit value.  */
            value = (ULONG) (*((volatile USHORT *) (dcd_mcimx6 -> ux_dcd_mcimx6_base + mcimx6_register))) ;
            break;

        case UX_DCD_MCIMX6_8BIT_REG        :

            /* 8 bit value.  */
            value =  (ULONG) (*((volatile UCHAR *) (dcd_mcimx6 -> ux_dcd_mcimx6_base + mcimx6_register)));
            break;

    }
    return(value);
}
