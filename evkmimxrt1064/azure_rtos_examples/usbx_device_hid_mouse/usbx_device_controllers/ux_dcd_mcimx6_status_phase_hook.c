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
/*    _ux_dcd_mcimx6_status_phase_hook                     PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will hook a status phase after reception of a SETUP   */
/*    packet.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_register_read          Read register                 */
/*    _ux_dcd_mcimx6_register_set           Set register                  */
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
UINT  _ux_dcd_mcimx6_status_phase_hook(UX_DCD_MCIMX6 *dcd_mcimx6,
                                        UX_SLAVE_ENDPOINT *endpoint,
                                        ULONG direction)
{

UX_DCD_MCIMX6_ED        *ed;
UX_DCD_MCIMX6_QED       *qed;
UX_DCD_MCIMX6_QTD       *qtd;
ULONG                   endpoint_address;
ULONG                   next_qtd_value;

    /* First extract the endpoint address. We keep this value as the endpoint index for accessing
       the endpoint registers.  */
    endpoint_address =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* Check for transfer direction.  This is needed to get the right address is the endpoint is Control.  */
    if (direction == UX_TRANSFER_PHASE_DATA_OUT)

        /* Get the address of the physical endpoint.  */
        ed =  _ux_dcd_mcimx6_endpoint_address_get(dcd_mcimx6, endpoint, UX_ENDPOINT_IN);

    else

        /* Get the address of the physical endpoint.  */
        ed =  _ux_dcd_mcimx6_endpoint_address_get(dcd_mcimx6, endpoint, UX_ENDPOINT_OUT);

    /* Get the physical endpoint associated with this endpoint.  */
    qed =  ed -> ux_dcd_mcimx6_ed_qed;

    /* Check the endpoint status, if it is free, We have some init problem or the user tries to send
       data to an endpoint which is not part of the current alternate setting.  */
    if ((ed -> ux_dcd_mcimx6_ed_status & UX_DCD_MCIMX6_ED_STATUS_USED) == 0)

        /* Return an error.  */
        return(UX_NO_ED_AVAILABLE);


    /* Fetch a QTD from the list.  */
    qtd =  _ux_dcd_mcimx6_qtd_get(dcd_mcimx6);

    /* Check to make sure we have QTDs available.  */
    if (qtd == UX_NULL)

        /* No more QTDs available.  */
        return(UX_TRANSFER_ERROR);

    /* Mark this QTD as being part of the status phase.  */
    qtd -> ux_dcd_mcimx6_qtd_control |=  UX_DCD_MCIMX6_QTD_STATUS_PHASE;

    /* Set total bytes transfer in the QTdD to zero .  We set the ACTIVE bit field.  */
    qtd -> ux_dcd_mcimx6_qtd_status = UX_DCD_MCIMX6_QTD_STATUS_ACTIVE;

    /* Put the termination bit in the last QTD.  We need some intermediate steps to avoid weird castings
       that may upset compilers.  */
    next_qtd_value = (ULONG) qtd -> ux_dcd_mcimx6_qtd_next_qtd | UX_DCD_MCIMX6_QTD_TERMINATE;
    qtd -> ux_dcd_mcimx6_qtd_next_qtd =  (UX_DCD_MCIMX6_QTD *) next_qtd_value;

    /* We set the IOC bit field on the last QTD. The idea is to have to the controller wake the driver when either there
       has been an error during a transfer or everything has been transferred successfully.  */
    qtd -> ux_dcd_mcimx6_qtd_status |= UX_DCD_MCIMX6_QTD_IOC;

    /* Update the QH with the head of the QTD.  */
    qed -> ux_dcd_mcimx6_qed_next_qtd = _ux_utility_physical_address(qtd);

    /* We need to reset the active bit in the status of the QTD store in the overlay.  */
    qed -> ux_dcd_mcimx6_qed_status = 0;

    /* Save the QTD in the ED structure. This is necessary when we have to free this TD when
       the status packet is retired.  */
    ed -> ux_dcd_mcimx6_ed_qtd_head =  qtd;

    /* Prime the endpoint.  Check for transfer direction.  */
    if (direction == UX_TRANSFER_PHASE_DATA_OUT)

        /* Prime the PETB field of the EPPRIME register.  */
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPPRIME,
                                            (1 << endpoint_address) << UX_DCD_MCIMX6_EPPRIME_PETB_SHIFT);

    else

        /* Prime the PERB field of the EPPRIME register.  */
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPPRIME,
                                            (1 << endpoint_address) << UX_DCD_MCIMX6_EPPRIME_PERB_SHIFT);

    /* Return to caller with success.  */
    return(UX_SUCCESS);
}
