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
/*    _ux_dcd_mcimx6_endpoint_flush                        PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is invoked when an endpoint needs to be flushed. This */
/*    can happen when a disconnection happens, a second setup packet is   */
/*    received before the data phase/status phase happen.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    endpoint                              Pointer to endpoint           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_set              Set register               */
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
UINT  _ux_dcd_mcimx6_endpoint_flush(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint,
                                    ULONG endpoint_direction)
{

ULONG                   mcimx6_register;
UX_DCD_MCIMX6_ED        *ed;
UX_DCD_MCIMX6_QTD       *qtd;
UX_DCD_MCIMX6_QTD       *next_qtd;
ULONG                   flush_retry;

    /* Get the address of the physical endpoint.  */
    ed =  _ux_dcd_mcimx6_endpoint_address_get(dcd_mcimx6, endpoint, endpoint_direction);

    /* Set the flush retry value to TRUE to ensure Flush has succeeded.  */
    flush_retry =  UX_TRUE;

    /* We may need more than one try to flush the endpoint.  */
    while (flush_retry == UX_TRUE)
    {

        /* Flush the endpoint.  Check for transfer direction.  */
        if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)

            /* Set the FETB field of the EPFLUSH register.  */
            _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPFLUSH,
                                                (1 << ed -> ux_dcd_mcimx6_ed_address) << UX_DCD_MCIMX6_EPFLUSH_FETB_SHIFT);
        else

            /* Set the FERB field of the EPFLUSH register.  */
            _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPFLUSH,
                                                (1 << ed -> ux_dcd_mcimx6_ed_address) << UX_DCD_MCIMX6_EPFLUSH_FERB_SHIFT);

        /* The flushing of the endpoint may take a while. We need to wait for the operation to be completed.  */
        do
        {
            /* Read the EPFLUSH register.  */
            mcimx6_register = _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPFLUSH);

        } while (mcimx6_register != 0);

        /* Read the EPSR register and ensure it is not primed anymore.  */
        mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSR);

        /* Check the bit that represents the endpoint and direction we are trying to flush.  */
        if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)

            /* Mask the Transmit bit.  */
            mcimx6_register &= (1 << ed -> ux_dcd_mcimx6_ed_address) << UX_DCD_MCIMX6_EPSR_ETBR_SHIFT;

        else

            /* Mask the Receive bit.  */
            mcimx6_register &= (1 << ed -> ux_dcd_mcimx6_ed_address) << UX_DCD_MCIMX6_EPSR_ERBR_SHIFT;


        /* Ensure this bit is clear, otherwise the Flush failed.  */
        if(mcimx6_register == 0)

            /* The flush succeeded.  */
            flush_retry =  UX_FALSE;

    }

    /* Get the physical head QTD associated with this QH.  */
    qtd =  ed -> ux_dcd_mcimx6_ed_qtd_head;

    /* All QTDs must be retired and the endpoint must be flushed.  */
    while (qtd != UX_NULL)
    {

        /* Fetch the next QTD in the list.  We remember it before retiring this QTD.  */
        next_qtd = _ux_utility_virtual_address((VOID *) ((ULONG) qtd->ux_dcd_mcimx6_qtd_next_qtd & ~UX_DCD_MCIMX6_QTD_TERMINATE));

        /* Now we can safely retire this QTD.  */
        qtd -> ux_dcd_mcimx6_qtd_control = UX_DCD_MCIMX6_QTD_STATUS_FREE;

        /* Make the next TD the current one.  */
        qtd =  next_qtd;
    }

    /* Clear the head and tail qtds in the ed container.  */
    ed -> ux_dcd_mcimx6_ed_qtd_head = UX_NULL;
    ed -> ux_dcd_mcimx6_ed_qtd_tail = UX_NULL;


    /* We are done.  */
    return(UX_SUCCESS);
}
