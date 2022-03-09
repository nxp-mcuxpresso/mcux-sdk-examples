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
/*    _ux_dcd_mcimx6_transfer_request                      PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function will initiate a transfer to a specific endpoint.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    transfer_request                      Pointer to transfer request   */
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
/*    _ux_utility_semaphore_get             Get semaphore                 */
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
UINT  _ux_dcd_mcimx6_transfer_request(UX_DCD_MCIMX6 *dcd_mcimx6, UX_SLAVE_TRANSFER *transfer_request)
{

UX_DCD_MCIMX6_ED        *ed;
UX_DCD_MCIMX6_QED       *qed;
UX_DCD_MCIMX6_QTD       *qtd;
UX_DCD_MCIMX6_QTD       *qtd_head;
UX_DCD_MCIMX6_QTD       *qtd_tail;
ULONG                   transfer_length;
ULONG                   qtd_transfer_length;
ULONG                   endpoint_address;
UCHAR                   *transfer_buffer;
UX_SLAVE_ENDPOINT       *endpoint;
UINT                    status;
UCHAR                   *physical_page;
ULONG                   next_qtd_value;
ULONG                   zlp_flag;

    /* Get the pointer to the logical endpoint from the transfer request.  */
    endpoint =  transfer_request -> ux_slave_transfer_request_endpoint;

    /* First extract the endpoint address. We keep this value as the endpoint index for accessing
       the endpoint registers.  */
    endpoint_address =  endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & ~UX_ENDPOINT_DIRECTION;

    /* Check for transfer direction.  This is needed to get the right address is the endpoint is Control.  */
    if (transfer_request -> ux_slave_transfer_request_phase == UX_TRANSFER_PHASE_DATA_OUT)

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

    /* Get the size of the transfer. */
    transfer_length =  transfer_request -> ux_slave_transfer_request_requested_length;

    /* Point the transfer buffer to the current transfer request buffer address.  */
    transfer_buffer =  transfer_request -> ux_slave_transfer_request_data_pointer;

    /* Reset the QTD tail and head pointers.  */
    qtd_tail = UX_NULL;
    qtd_head = UX_NULL;

    /* Save the ZLP flag.  */
    zlp_flag =  transfer_request -> ux_slave_transfer_request_force_zlp;

    /* Build the QTDs one by one until no more to do.  We may have a ZLP so allow for zero data transfer
       and build a QTD with no data. */
    do
    {

        /* Fetch a QTD from the list.  */
        qtd =  _ux_dcd_mcimx6_qtd_get(dcd_mcimx6);

        /* Ensure we have a qtd.  */
        if (qtd == UX_NULL)
        {
            /* We should do some housekeeping here.  */
            return(UX_TRANSFER_ERROR);

        }

        /* We need to memorize the head pointer if this is the first.  */
        if (qtd_head == UX_NULL)
        {
            /* Memorize the qtd head.  */
            qtd_head = qtd;

            /* And save it in the ED structure. This is necessary when we have to reconstruct
               the list of transferred QTDs once they are retired.  */
            ed -> ux_dcd_mcimx6_ed_qtd_head =  qtd_head;
        }

        /* If the tail pointer is not NULL, we had a previous td and we need to link
           that previous td to the new one.  */
        if (qtd_tail != UX_NULL)
            qtd_tail -> ux_dcd_mcimx6_qtd_next_qtd = _ux_utility_physical_address(qtd);

        /*  Calculate the transfer size for this QTD.  */
        if (transfer_length > UX_DCD_MCIMX6_MAX_QTD_TRANSFER)

            /* We can only transfer the maximum per QTD.  */
            qtd_transfer_length = UX_DCD_MCIMX6_MAX_QTD_TRANSFER;

        else
            /* We transfer whatever is requested in the transfer_request.  */
            qtd_transfer_length = transfer_length;

        /* Adjust the ZLP flag if we treated the Zero Length Packet.  */
        if(qtd_transfer_length == 0)
            zlp_flag =  UX_FALSE;

        /* Adjust the data pointer.  */
        transfer_request -> ux_slave_transfer_request_current_data_pointer += qtd_transfer_length;

        /* Adjust the transfer length remaining.  */
        transfer_request -> ux_slave_transfer_request_in_transfer_length -= qtd_transfer_length;

        /* Set total bytes transfer in the QTdD.  We set the ACTIVE bit field.  */
        qtd -> ux_dcd_mcimx6_qtd_status = (qtd_transfer_length << UX_DCD_MCIMX6_QTD_TOTAL_BYTES_SHIFT)
                                            | UX_DCD_MCIMX6_QTD_STATUS_ACTIVE;

        /* We need to memorize the number of bytes to transfer in this QTD to compute the number
           of bytes actually sent after a short packet is received.  We do this horrible cast here
           to save memory in the QTD. */
        qtd -> ux_dcd_mcimx6_qtd_transfer_length = (USHORT) qtd_transfer_length;

        /* The QTD contains several pages available, each 4 K in length. We have
           to set each buffer page to the correct address within the transmit buffer.  */
        qtd -> ux_dcd_mcimx6_qtd_bp[0] =  (UCHAR *) _ux_utility_physical_address(transfer_buffer);

        /* Fill in the next pages addresses if required.  */
        physical_page =  (UCHAR *) (((ULONG) _ux_utility_physical_address(transfer_buffer)) & UX_DCD_MCIMX6_PAGE_ALIGN);
        qtd -> ux_dcd_mcimx6_qtd_bp[1] =  physical_page + UX_DCD_MCIMX6_PAGE_SIZE;
        qtd -> ux_dcd_mcimx6_qtd_bp[2] =  ((UCHAR *) qtd -> ux_dcd_mcimx6_qtd_bp[1]) + UX_DCD_MCIMX6_PAGE_SIZE;
        qtd -> ux_dcd_mcimx6_qtd_bp[3] =  ((UCHAR *) qtd -> ux_dcd_mcimx6_qtd_bp[2]) + UX_DCD_MCIMX6_PAGE_SIZE;
        qtd -> ux_dcd_mcimx6_qtd_bp[4] =  ((UCHAR *) qtd -> ux_dcd_mcimx6_qtd_bp[3]) + UX_DCD_MCIMX6_PAGE_SIZE;

        /* Adjust the remaining length of the transfer.  */
        transfer_length -= qtd_transfer_length;

        /* Let's make this QTD the tail now.  */
        qtd_tail = qtd;

    }  while((transfer_length != 0) || (zlp_flag == UX_TRUE));

    /* Put the termination bit in the last QTD.  We need some intermediate steps to avoid weird castings
       that may upset compilers.  */
    next_qtd_value = (ULONG) qtd_tail -> ux_dcd_mcimx6_qtd_next_qtd | UX_DCD_MCIMX6_QTD_TERMINATE;
    qtd_tail -> ux_dcd_mcimx6_qtd_next_qtd =  (UX_DCD_MCIMX6_QTD *) next_qtd_value;

    /* We set the IOC bit field on the last QTD. The idea is to have to the controller wake the driver when either there
       has been an error during a transfer or everything has been transferred successfully.  */
        qtd -> ux_dcd_mcimx6_qtd_status |= UX_DCD_MCIMX6_QTD_IOC;

    /* Update the QH with the head of the QTD.  */
    qed -> ux_dcd_mcimx6_qed_next_qtd = _ux_utility_physical_address(qtd_head);

    /* We need to reset the active bit in the status of the QTD store in the overlay.  */
    qed -> ux_dcd_mcimx6_qed_status = 0;

    /* Prime the endpoint.  Check for transfer direction.  */
    if (transfer_request -> ux_slave_transfer_request_phase == UX_TRANSFER_PHASE_DATA_OUT)

        /* Prime the PETB field of the EPPRIME register.  */
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPPRIME,
                                            (1 << endpoint_address) << UX_DCD_MCIMX6_EPPRIME_PETB_SHIFT);

    else

        /* Prime the PERB field of the EPPRIME register.  */
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPPRIME,
                                            (1 << endpoint_address) << UX_DCD_MCIMX6_EPPRIME_PERB_SHIFT);

    /* If the endpoint is a Control endpoint, all this is happening under
       Interrupt and there is no thread to suspend.  */
    if (ed -> ux_dcd_mcimx6_ed_type != UX_DCD_MCIMX6_ED_TYPE_CONTROL)
    {

        /* We should wait for the semaphore to wake us up.  */
        status =  _ux_utility_semaphore_get(&transfer_request -> ux_slave_transfer_request_semaphore, UX_WAIT_FOREVER);

        /* Check the completion code. */
        if (status != UX_SUCCESS)
            return(status);

        /* Check the transfer request completion code. We may have had a BUS reset or
           a device disconnection.  */
        if (transfer_request -> ux_slave_transfer_request_completion_code != UX_SUCCESS)
            return(transfer_request -> ux_slave_transfer_request_completion_code);

        /* Return to caller with success.  */
        return(UX_SUCCESS);
    }

    /* Return to caller with success.  */
    return(UX_SUCCESS);
}
