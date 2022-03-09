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
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_mcimx6_endpoint_create                      PORTABLE C      */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will create a physical endpoint.                      */
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
/*    _ux_dcd_mcimx6_endpoint_reset         Reset endpoint                */
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
UINT  _ux_dcd_mcimx6_endpoint_create(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_ENDPOINT *endpoint)
{

UX_DCD_MCIMX6_ED        *ed;
ULONG                   mcimx6_endpoint_register = 0;
UX_DCD_MCIMX6_QED       *qed;
ULONG                    multi_qed_index;
ULONG                    endpoint_qed_address;
ULONG                    endpoint_address;
ULONG                    mcimx6_endpoint_mask;


    /* First extract the endpoint address. We keep this value as the endpoint index for accessing the EPCRn registers.
       The MCIMX6 has 4 endpoints maximum.  Each endpoint has a IN and OUT QH attached to it.  */
    endpoint_address =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* At first the endpoint QED index is the endpoint address * 2 this will change if the endpoint is IN or OUT.  */
    endpoint_qed_address = endpoint_address << 1;

    /* If the endpoint is of control type, we need to create both the IN and OUT QH.  */
    if ((endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE ) == UX_CONTROL_ENDPOINT)

        /* This is a control endpoint, create IN and OUT QH.  */
        multi_qed_index = 2;
    else

        /* This is a non control endpoint, create only what is described by the endpoint direction bit.  */
        multi_qed_index = 1;

    /* For each endpoint QED .... */
    while (multi_qed_index-- != 0)
    {

        /* The direction on non control endpoints will give us the index increment, endpoint OUT are first.  */
        if ((endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE) != UX_CONTROL_ENDPOINT)
        {
            /* Check direction for this non control endpoint.  */
            if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) == UX_ENDPOINT_IN)

                /* The endpoint is a IN.  Adjust the address accordingly.  */
                endpoint_qed_address++;
        }
        else
        {

            /* For control endpoints (normally only endpoint 0), we need to build both IN and OUT based,
               on the multi_qed_index value.  */
            if (multi_qed_index == 0)

                /* This is the IN portion of the Control endpoint.  */
                endpoint_qed_address++;

        }


        /* Fetch the address of the physical endpoint.  */
        ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_qed_address];

        /* Check the endpoint status, if it is free, reserve it. If not reject this endpoint.  */
        if ((ed -> ux_dcd_mcimx6_ed_status & UX_DCD_MCIMX6_ED_STATUS_USED) == 0)
        {

            /* We can use this endpoint.  */
            ed -> ux_dcd_mcimx6_ed_status |=  UX_DCD_MCIMX6_ED_STATUS_USED;

            /* Keep the physical endpoint address in the endpoint container.  */
            endpoint -> ux_slave_endpoint_ed =  (VOID *) ed;

            /* Save the endpoint pointer.  */
            ed -> ux_dcd_mcimx6_ed_endpoint =  endpoint;

            /* Save the endpoint address.  */
            ed -> ux_dcd_mcimx6_ed_address = (ULONG) endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

            /* Build the endpoint mask from the endpoint descriptor.  */
            mcimx6_endpoint_mask =  (endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE) |
                                (endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION);

            /* Build the endpoint EPCRn mask and preset the ed values.  */
            switch (mcimx6_endpoint_mask)
            {

            case UX_CONTROL_ENDPOINT:

                /* This endpoint is enabled by default.  */
                ed -> ux_dcd_mcimx6_ed_type        = UX_CONTROL_ENDPOINT;
                break;

            case UX_BULK_ENDPOINT_IN:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_TXTBULK | UX_DCD_MCIMX6_EPCRN_TXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_BULK_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_IN;

                break;

            case UX_BULK_ENDPOINT_OUT:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_RXTBULK | UX_DCD_MCIMX6_EPCRN_RXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_BULK_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_OUT;

                break;

            case UX_INTERRUPT_ENDPOINT_IN:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_TXTINT | UX_DCD_MCIMX6_EPCRN_TXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_INTERRUPT_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_IN;

                break;

            case UX_INTERRUPT_ENDPOINT_OUT:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_RXTINT | UX_DCD_MCIMX6_EPCRN_RXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_INTERRUPT_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_OUT;

                break;

            case UX_ISOCHRONOUS_ENDPOINT_IN:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_TXTISO | UX_DCD_MCIMX6_EPCRN_TXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_ISOCHRONOUS_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_IN;

                break;

            case UX_ISOCHRONOUS_ENDPOINT_OUT:

                mcimx6_endpoint_register =  UX_DCD_MCIMX6_EPCRN_RXTISO | UX_DCD_MCIMX6_EPCRN_RXE;

                ed -> ux_dcd_mcimx6_ed_type        = UX_ISOCHRONOUS_ENDPOINT;
                ed -> ux_dcd_mcimx6_ed_direction   = UX_REQUEST_OUT;

                break;

            default:

                return(UX_ERROR);
            }

            /* compute the QED address.  */
            qed = dcd_mcimx6 -> ux_dcd_mcimx6_qed_head + endpoint_qed_address;

            /* Initialize the QED primary field with MPS.  */
            qed -> ux_dcd_mcimx6_qed_control = endpoint -> ux_slave_endpoint_descriptor.wMaxPacketSize
                                                            << UX_DCD_MCIMX6_QED_MPL_SHIFT;

            /* On control endpoints, we set the IOS bit to accept interrupts on SETUP packets.  */
            if ((endpoint -> ux_slave_endpoint_descriptor.bmAttributes & UX_MASK_ENDPOINT_TYPE) == UX_CONTROL_ENDPOINT)
                qed -> ux_dcd_mcimx6_qed_control |= UX_DCD_MCIMX6_QED_IOS;

            /* We set the ZLT bit to force the stack to manage Zero Length Packets manually.  */
            qed -> ux_dcd_mcimx6_qed_control |= UX_DCD_MCIMX6_QED_ZLT_DISABLE;

            /* Clear the current QED field and the overlay status.  */
            qed -> ux_dcd_mcimx6_qed_current_qtd = (UX_DCD_MCIMX6_QTD *) UX_NULL;
            qed -> ux_dcd_mcimx6_qed_status = 0;

            /* Set the terminate bit field in the next QTD field.  */
            qed -> ux_dcd_mcimx6_qed_next_qtd = (UX_DCD_MCIMX6_QTD *) UX_DCD_MCIMX6_QED_TERMINATE;

            /* Save the QED in the ed structure.  */
            ed -> ux_dcd_mcimx6_ed_qed = qed;

            /* Reset this endpoint.  */
            _ux_dcd_mcimx6_endpoint_reset(dcd_mcimx6, endpoint);

            /* Enable this endpoint.  We skip endpoint 0 as it is always enabled.  */
            if (endpoint_address != 0)
                _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCR +
                                            (endpoint_address * 4), mcimx6_endpoint_register);

        }

        else
            /* This endpoint is already used, return an error.  */
            return(UX_NO_ED_AVAILABLE);
    }

    /* We are done.  */
    return(UX_SUCCESS);

}
