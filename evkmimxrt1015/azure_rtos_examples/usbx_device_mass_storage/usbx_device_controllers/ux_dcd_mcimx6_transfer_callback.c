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
/*    _ux_dcd_mcimx6_transfer_callback                     PORTABLE C     */
/*                                                           6.0          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is invoked under ISR when an event happens on a       */
/*    specific endpoint.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_mcimx6                            Pointer to device controller  */
/*    ed                                    Pointer to endpoint           */
/*    callback_phase                        either SETUP or other phase   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_address_set               Set address                */
/*    _ux_dcd_mcimx6_register_clear            Clear register             */
/*    _ux_dcd_mcimx6_register_read             Read register              */
/*    _ux_dcd_mcimx6_register_write            Write register             */
/*    _ux_dcd_mcimx6_register_set              Set register               */
/*    _ux_device_stack_control_request_process Process control request    */
/*    _ux_utility_semaphore_get                Get semaphore              */
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
UINT  _ux_dcd_mcimx6_transfer_callback(UX_DCD_MCIMX6 *dcd_mcimx6, UX_DCD_MCIMX6_ED *ed, ULONG callback_phase)
{

UX_SLAVE_TRANSFER       *transfer_request;
UX_SLAVE_ENDPOINT       *endpoint;
UX_DCD_MCIMX6_QED      *qed;
UX_DCD_MCIMX6_QTD      *qtd;
UX_DCD_MCIMX6_QTD      *last_qtd;
UCHAR *                 data_pointer;
ULONG                   qtd_transfer_length_remain;
ULONG                   setup_tripwire;
ULONG                   qtd_value;
ULONG                   qtd_counter;
ULONG                   mcimx6_register;


    /* Get the endpoint container pointer from the endpoint.  */
    endpoint =  ed -> ux_dcd_mcimx6_ed_endpoint;

    /* Get the pointer to the transfer request.  */
    transfer_request =  &endpoint -> ux_slave_endpoint_transfer_request;

    /* Get the physical endpoint associated with this endpoint.  */
    qed =  ed -> ux_dcd_mcimx6_ed_qed;

    /* Get the pointer to the data buffer of the transfer request.  */
    data_pointer =  transfer_request -> ux_slave_transfer_request_data_pointer;

    /* Check if this callback is from a SETUP phase. */
    if (callback_phase == UX_DCD_MCIMX6_EPSETUPSR)
    {

        /* Read the Endpoint Setup Status register.  */
        mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSETUPSR);

        /* Mask the setup register with the endpoint address.  */
        mcimx6_register &= 1 << ed -> ux_dcd_mcimx6_ed_address;

        /* Check the SETUP bit.  */
        if (mcimx6_register != 0)
        {

            /* There may have been a previous SETUP command unfinished. Flush both control FIFOs.  */
            _ux_dcd_mcimx6_transfer_abort(dcd_mcimx6, transfer_request);

            /* Reset the SETUP condition.  */
            _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSETUPSR, mcimx6_register);

            /* Ensure the setup packet cannot be corrupted while reading it.  */
            setup_tripwire = UX_TRUE;

            /* As long as this condition exist, read the setup packet.  */
            while (setup_tripwire == UX_TRUE)
            {

                /* Set the SUTW condition.  */
                _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD, UX_DCD_MCIMX6_USBCMD_SUTW);

                /* We have a setup condition.  The setup command is not in any QTD but rather
                   in the QH.  */
                data_pointer =  transfer_request -> ux_slave_transfer_request_setup;

                /* Transfer the data payload, from the QH into the local buffer.  The setup buffer
                   is divided into 2 32 bit values. */
                _ux_utility_long_put(data_pointer, qed -> ux_dcd_mcimx6_qed_setup_buffer_03);
                data_pointer += 4;
                _ux_utility_long_put(data_pointer, qed -> ux_dcd_mcimx6_qed_setup_buffer_47);

                /* Check the condition of the tripwire.  */
                if (_ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD) & UX_DCD_MCIMX6_USBCMD_SUTW)
                {

                    /* Clear the SUTW condition.  */
                    _ux_dcd_mcimx6_register_clear(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBCMD, UX_DCD_MCIMX6_USBCMD_SUTW);

                    /* Get out of the tripwire condition.  */
                    setup_tripwire = UX_FALSE;

                }
            }

            /* Update the length of the data received.  */
            transfer_request -> ux_slave_transfer_request_actual_length =  8;

            /* Mark the phase as SETUP.  */
            transfer_request -> ux_slave_transfer_request_type =  UX_TRANSFER_PHASE_SETUP;

            /* Mark the transfer as successful.  */
            transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

            /* Check if the transaction is IN.  */
            if (*transfer_request -> ux_slave_transfer_request_setup & UX_REQUEST_IN)
            {


                /* Call the Control Transfer dispatcher.  */
                _ux_device_stack_control_request_process(transfer_request);

                /* Set the phase of the transfer to data IN (OUT from host) for status.  */
                _ux_dcd_mcimx6_status_phase_hook(dcd_mcimx6, endpoint, UX_TRANSFER_PHASE_DATA_IN);
            }
            else
            {
                /* We are in a OUT transaction. Check if there is a data payload. If so, wait for the payload
                   to be delivered.  */
                if (*(transfer_request -> ux_slave_transfer_request_setup + 6) == 0 &&
                    *(transfer_request -> ux_slave_transfer_request_setup + 7) == 0)
                {
                    /* Call the Control Transfer dispatcher.  */
                    _ux_device_stack_control_request_process(transfer_request);

                    /* Set the phase of the transfer to data OUT (IN from host) for status.  */
                    _ux_dcd_mcimx6_status_phase_hook(dcd_mcimx6, endpoint, UX_TRANSFER_PHASE_DATA_OUT);
                }
                else
                {

                    /* Get the length we expect from the SETUP packet.  */
                    transfer_request -> ux_slave_transfer_request_requested_length = _ux_utility_short_get(transfer_request -> ux_slave_transfer_request_setup + 6);

                    /* Reset what we have received so far.  */
                    transfer_request -> ux_slave_transfer_request_actual_length =  0;

                    /* Check if we have enough space for the request.  */
                    if (transfer_request -> ux_slave_transfer_request_requested_length > UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH)
                    {

                        /* No space available, stall the endpoint.  */
                        _ux_dcd_mcimx6_endpoint_stall(dcd_mcimx6, endpoint);

                        /* Next phase is a SETUP.  */
                        dcd_mcimx6 -> ux_dcd_mcimx6_control_state = UX_DCD_MCIMX6_CONTROL_STATE_IDLE;

                        /* We are done treating the SETUP packet of a control endpoint.  */
                        return(UX_SUCCESS);
                    }
                    else
                    {

                        /* And reprogram the current buffer address to the beginning of the buffer.  */
                        transfer_request -> ux_slave_transfer_request_current_data_pointer =  transfer_request -> ux_slave_transfer_request_data_pointer;
                        transfer_request -> ux_slave_transfer_request_phase =  UX_TRANSFER_PHASE_DATA_IN;
                        _ux_dcd_mcimx6_transfer_request(dcd_mcimx6, transfer_request);
                        dcd_mcimx6 -> ux_dcd_mcimx6_control_state = UX_DCD_MCIMX6_CONTROL_STATE_DATA_RX;
                    }
                }

            }


        }

        /* We are done treating the SETUP packet of a control endpoint.  */
        return(UX_SUCCESS);

    }

    /* This callback must be for a EPCOMPLETE phase.  We treat differently regular transfers
       associated with a transfer request and the status phase of a control endpoint.*/
    if (callback_phase == UX_DCD_MCIMX6_EPCOMPLETE)
    {

        /* Get the physical head QTD associated with this QH.  */
        qtd =  ed -> ux_dcd_mcimx6_ed_qtd_head;

        /* Check the nature of this QTD. Is this a data phase QTD ?  */
        if (qtd -> ux_dcd_mcimx6_qtd_control & UX_DCD_MCIMX6_QTD_STATUS_PHASE)
        {

            /* When a OUT Token is received, we need to see if there is change to the Address state. If so, we have to force the
               new address to the controller.  */
            if (dcd_mcimx6 -> ux_dcd_mcimx6_dcd_owner -> ux_slave_dcd_device_address != 0)
            {

                /* The address was changed, force the new one. This can only be done after the Status phase of the control
                   endpoint is received.  */
                _ux_dcd_mcimx6_address_set(dcd_mcimx6, dcd_mcimx6 -> ux_dcd_mcimx6_dcd_owner -> ux_slave_dcd_device_address);

                /* Reset the address now so that we don't change it every time.  */
                dcd_mcimx6 -> ux_dcd_mcimx6_dcd_owner -> ux_slave_dcd_device_address =  0;
            }

            /* There may have been a previous SETUP command unfinished. We need to check if
               the previous data phase was completed or not. This transfer is on the opposite endpoint
               of the data phase.  Since there is only one transfer request for the control endpoint,
               it amounts to aborting the current transfer. */
            _ux_dcd_mcimx6_transfer_abort(dcd_mcimx6, transfer_request);

        }

        else

        {

            /* Reset the QTD counter.  */
            qtd_counter = 0;

            /* There may be multiple QTD completed for this QH.  NOTE : THIS NEED TO BE TESTED !!!!*/
            while (qtd != UX_NULL)
            {

                /* Check the active bit of this QTD. If the active bit is done, we have a QTD completion.
                   If we have a Transaction Error or a Data Buffer Error, we need to take appropriate action.  */
                if ((qtd -> ux_dcd_mcimx6_qtd_status & UX_DCD_MCIMX6_QTD_STATUS_ACTIVE) == 0)
                {

                    /* Check the error code and update the transfer descriptor accordingly.  */
                    if ((qtd -> ux_dcd_mcimx6_qtd_status & UX_DCD_MCIMX6_QTD_STATUS_HALTED) != 0)
                    {
                        /* Set the completion code to error.  */
                        transfer_request -> ux_slave_transfer_request_completion_code =  UX_TRANSFER_STALLED;

                        /* The transfer is completed.  */
                        transfer_request -> ux_slave_transfer_request_status =  UX_TRANSFER_STATUS_COMPLETED;

                        /* We may have other QTDs attached to this endpoint, so flush everything.  */
                        if (ed -> ux_dcd_mcimx6_ed_type != UX_DCD_MCIMX6_ED_TYPE_CONTROL)
                            _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, 0);

                        else
                        {
                            _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, UX_ENDPOINT_OUT);
                            _ux_dcd_mcimx6_endpoint_flush(dcd_mcimx6, endpoint, UX_ENDPOINT_IN);
                        }

                    }

                    else
                    {

                        /* Obtain the number of bytes remaining in the transfer. If the value is 0 all transfer
                           is complete for this QTD.  */
                        qtd_transfer_length_remain = (qtd -> ux_dcd_mcimx6_qtd_status & 0x3FFF0000) >> UX_DCD_MCIMX6_QTD_TOTAL_BYTES_SHIFT;

                        /* Update the length of the data received.  */
                        transfer_request -> ux_slave_transfer_request_actual_length +=
                                                                    (ULONG) qtd -> ux_dcd_mcimx6_qtd_transfer_length - qtd_transfer_length_remain;

                        /* Set the completion code to no error.  */
                        transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;
                    }
                }
                else
                {

                    /* We are done.  */
                    return(UX_SUCCESS);
                }

                /* Memorize this QTD.  */
                last_qtd = qtd;

                /* Locate the next TD in the qed chain.  */
                qtd = qtd -> ux_dcd_mcimx6_qtd_next_qtd;

                /* Cast the pointer into a ULONG to check if link is valid.  */
                qtd_value = (ULONG) qtd;

                /* Check if there is a termination bit.  */
                if (qtd_value & UX_DCD_MCIMX6_QTD_TERMINATE)
                    qtd = UX_NULL;

                /* Now we can free the previous QTD.  */
                last_qtd -> ux_dcd_mcimx6_qtd_control = UX_DCD_MCIMX6_QTD_STATUS_FREE;

                /* And the new QTD is the head.  */
                ed -> ux_dcd_mcimx6_ed_qtd_head = qtd;

                /* We have one more QTD processed.  */
                qtd_counter++;

            }

            /* We may get here because of semaphore being awaken but nothing happened.  */
            if (qtd_counter !=0)
            {

                /* Check what type of endpoint we are using, Control or non Control.  */
                if (ed -> ux_dcd_mcimx6_ed_type != UX_DCD_MCIMX6_ED_TYPE_CONTROL)

                {
                    /* The transfer is completed.  */
                    transfer_request -> ux_slave_transfer_request_status =  UX_TRANSFER_STATUS_COMPLETED;

                    /* Non control endpoint operation, use semaphore.  */
                    _ux_utility_semaphore_put(&transfer_request -> ux_slave_transfer_request_semaphore);
                }
                else
                {

                    if (dcd_mcimx6 -> ux_dcd_mcimx6_control_state == UX_DCD_MCIMX6_CONTROL_STATE_DATA_RX)
                    {

                        /* Set the completion code to no error.  */
                        transfer_request -> ux_slave_transfer_request_completion_code =  UX_SUCCESS;

                        /* We are using a Control endpoint on a OUT transaction and there was a payload.  */
                        _ux_device_stack_control_request_process(transfer_request);

                        /* Set the phase of the transfer to data OUT (IN from host) for status.  */
                        _ux_dcd_mcimx6_status_phase_hook(dcd_mcimx6, endpoint, UX_TRANSFER_PHASE_DATA_OUT);
                        dcd_mcimx6 -> ux_dcd_mcimx6_control_state = UX_DCD_MCIMX6_CONTROL_STATE_IDLE;
                    }
                }

            }
        }
        /* We are done.  */
        return(UX_SUCCESS);

    }
    /* Should never come here but we need to keep compilers happy.  */
    return(UX_SUCCESS);
}
