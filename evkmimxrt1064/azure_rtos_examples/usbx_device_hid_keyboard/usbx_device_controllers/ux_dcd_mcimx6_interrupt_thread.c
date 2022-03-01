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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_mcimx6_interrupt_thread                     PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the interrupt thread for the MCIMX6 controller.    */
/*    The controller cannot perform all interrupt functions in ISR so it  */
/*    relays this job to a high priority thread.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dcd_pointer                         instance of the USB device      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_dcd_mcimx6_initialize_complete       Complete initialization    */
/*    _ux_dcd_mcimx6_register_read             Read register              */
/*    _ux_dcd_mcimx6_register_write            Write register             */
/*    _ux_dcd_mcimx6_transfer_callback         Process callback           */
/*    _ux_device_stack_disconnect              Disconnect device          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Device Stack                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            Used UX_ things instead of  */
/*                                            TX_ things directly,        */
/*                                            resulting in version 6.1    */
/*  xx-xx-xxxx     Chaoqiong Xiao           Modified comment(s),          */
/*                                            improved disconnect check,  */
/*                                            resulting in version 6.x    */
/*                                                                        */
/**************************************************************************/
VOID  _ux_dcd_mcimx6_interrupt_thread(ULONG dcd_pointer)
{

UX_INTERRUPT_SAVE_AREA
UX_DCD_MCIMX6           *dcd_mcimx6;
ULONG                   mcimx6_int_status;
ULONG                   mcimx6_register;
ULONG                   endpoint_index;
UX_DCD_MCIMX6_ED        *ed;
UX_SLAVE_DEVICE         *device;
ULONG                    status;

    /* Properly cast the dcd pointer.  */
    dcd_mcimx6 =  (UX_DCD_MCIMX6 *) dcd_pointer;

    /* Get the pointer to the device.  */
    device =  &_ux_system_slave -> ux_system_slave_device;

    /* we need to create a semaphore to synchronize with the interrupt handler.  */
    status =  _ux_utility_semaphore_create(&dcd_mcimx6 -> ux_dcd_mcimx6_semaphore,
                                              "ux_dcd_mcimx6_semaphore", 0);

    /* Check completion status.  If there is an error, we have a major problem !  */
    if (status != UX_SUCCESS)
        return;

    /* Now wait on this semaphore for an event from the controller. This will trigger
       an interrupt which will awake the semaphore.  */
    while (1)
    {

        /* Get the semaphore that signals something is available for this
           thread to process.  */
        _ux_utility_semaphore_get(&dcd_mcimx6 -> ux_dcd_mcimx6_semaphore, UX_WAIT_FOREVER);

        /* Disable interrupt.  */
        UX_DISABLE

        /* Read the interrupt status register that awaken the controller.  */
        mcimx6_int_status = dcd_mcimx6 -> ux_dcd_mcimx6_interrupt;
        /* Now clear the interrupt status register. NB this register is not the controller
           register but merely a saved register updated by the interrupt handler.  */
        dcd_mcimx6 -> ux_dcd_mcimx6_interrupt =  0;

        /* Restore interrupt.  */
        UX_RESTORE

        /* Check the source of the interrupt. Is it a Bus Reset?  */
        if (mcimx6_int_status & UX_DCD_MCIMX6_USBSTS_URI)
        {

            /* Check if we have multiple BUS_RESET signals.  This may happen and we do not need
               to complete the init in that case.  */
            if (device -> ux_slave_device_state ==  UX_DEVICE_RESET)
            {

                /* Read the EPSETUPSR register and clear it.  */
                mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSETUPSR);
                _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSETUPSR, mcimx6_register);

                /* Read the EPPCOMPLETE register and clear it.  */
                mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCOMPLETE);
                _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCOMPLETE, mcimx6_register);

                do
                {

                    /* Read EPPRIME register and wait until all bits are zeroed.  */
                    mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPPRIME);

                } while (mcimx6_register != 0);

                /* Clear EPFLUSH register.  */
                _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPFLUSH, 0xFFFFFFFF );

                /* Change the device state to BUS reset completed.  */
                device -> ux_slave_device_state =  UX_DEVICE_BUS_RESET_COMPLETED;

            }

            else
            {

                /* A device is disconnected when URI and SLI signals are both presents.  */
                if (mcimx6_int_status & UX_DCD_MCIMX6_USBSTS_SLI)
                {

                    /* Read the port status register.  this will indicate if there is a device connected and
                   at what speed the device is connected at.  */
                    mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_PORTSC);

                    /* Examine the device state and the CCS field. That tells us if the device is connected or not.  */
                    if ((device -> ux_slave_device_state ==  UX_DEVICE_ATTACHED || device -> ux_slave_device_state ==  UX_DEVICE_CONFIGURED)
                         && !(mcimx6_register & UX_DCD_MCIMX6_PORTSC_CCS))
                    {

                        /* We have a device disconnection.  */
                        _ux_device_stack_disconnect();

                    }
                }

            }
        }

        /* Check the source of the interrupt. Is it a  Port Status Change ?  */
        if (mcimx6_int_status & UX_DCD_MCIMX6_USBSTS_PCI)
        {

            /* We need to wait for the reset to be completed before doing anything else.  */
            while (_ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_PORTSC) & UX_DCD_MCIMX6_PORTSC_PR);

            /* Read the port status register.  this will indicate if there is a device connected and
               at what speed the device is connected at.  */
            mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_PORTSC);


            /* Examine the CCS field. That tells us if the device is connected.  */
            if (mcimx6_register & UX_DCD_MCIMX6_PORTSC_CCS)
            {

                /* Check if we are already attached.  */
                if (device -> ux_slave_device_state !=  UX_DEVICE_ATTACHED)
                {

                    /* We have a device connection, read at what speed we are connected.  */
                    if ((mcimx6_register & UX_DCD_MCIMX6_PORTSC_PSPD_MASK) == UX_DCD_MCIMX6_PORTSC_PSPD_HIGH)

                        /* We are connected at high speed.  */
                        _ux_system_slave -> ux_system_slave_speed =  UX_HIGH_SPEED_DEVICE;

                    else

                        /* We are connected at lower speed.  */
                        _ux_system_slave -> ux_system_slave_speed =  UX_FULL_SPEED_DEVICE;

                    /* And complete the device initialization.  */
                    _ux_dcd_mcimx6_initialize_complete();

                       /* Mark the device as attached now.  */
                    device -> ux_slave_device_state =  UX_DEVICE_ATTACHED;

                }
            }
            else
            {
                if (device -> ux_slave_device_state >=  UX_DEVICE_ATTACHED)

                    /* We have a device disconnection.  */
                    _ux_device_stack_disconnect();
            }
        }

        /* Check the source of the interrupt. Is it a completion of USB transfer ?  */
        if (mcimx6_int_status & UX_DCD_MCIMX6_USBSTS_UI)
        {

            /* Read the EPPCOMPLETE register.  */
            mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCOMPLETE);

            /* Now clear it.  */
            _ux_dcd_mcimx6_register_write(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPCOMPLETE, mcimx6_register);

            /* Check all events related to USB endpoints. */
            for (endpoint_index = 0; endpoint_index < UX_DCD_MCIMX6_MAX_ED_VALUE; endpoint_index++)
            {

                /* Check for event on the reception flags.  */
                if (mcimx6_register & ((1 << endpoint_index) << UX_DCD_MCIMX6_EPCOMPLETE_ERCE_SHIFT))
                {

                    /* Get the logical endpoint associated with this endpoint index.  */
                    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_index*2];

                    /* Process the call back.  This callback is for a non SETUP Phase.  */
                    _ux_dcd_mcimx6_transfer_callback(dcd_mcimx6, ed, UX_DCD_MCIMX6_EPCOMPLETE);

                }

                /* Check for event on the transmission flags.  */
                if (mcimx6_register & ((1 << endpoint_index) << UX_DCD_MCIMX6_EPCOMPLETE_ETCE_SHIFT))
                {

                    /* Get the logical endpoint associated with this endpoint index.  */
                    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[(endpoint_index*2) + 1];

                    /* Process the call back.  This callback is for a non SETUP Phase.  */
                    _ux_dcd_mcimx6_transfer_callback(dcd_mcimx6, ed, UX_DCD_MCIMX6_EPCOMPLETE);
                }
            }


            /* Read the EPSETUPSR register.  */
            mcimx6_register =  _ux_dcd_mcimx6_register_read(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_EPSETUPSR);

            /* Check all events related to USB endpoints. */
            for (endpoint_index = 0; endpoint_index < UX_DCD_MCIMX6_MAX_ED_VALUE; endpoint_index++)
            {

                /* Check for event on the reception flags.  */
                if (mcimx6_register & (1 << endpoint_index))
                {

                    /* Get the logical endpoint associated with this endpoint index.  */
                    ed =  &dcd_mcimx6 -> ux_dcd_mcimx6_ed[endpoint_index*2];

                    /* Process the call back.  This callback is for the SETUP phase.  */
                    _ux_dcd_mcimx6_transfer_callback(dcd_mcimx6, ed, UX_DCD_MCIMX6_EPSETUPSR);

                }
            }

        }

        /* Enable USB controller interrupts.  */
        _ux_dcd_mcimx6_register_set(dcd_mcimx6, UX_DCD_MCIMX6_32BIT_REG, UX_DCD_MCIMX6_USBINTR, UX_DCD_MCIMX6_USBINTR_MASK);

    }
}
