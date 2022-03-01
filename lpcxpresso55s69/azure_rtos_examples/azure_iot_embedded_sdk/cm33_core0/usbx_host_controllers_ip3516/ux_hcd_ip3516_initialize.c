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
/*    _ux_hcd_ip3516_initialize                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the IP3516 controller. It sets the DMA    */
/*    areas, programs all the IP3516 registers, sets up the ED and TD     */
/*    containers, sets the control, bulk and periodic lists.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    HCD                                   Pointer to HCD                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_hcd_ip3516_power_root_hubs        Power root HUBs               */
/*    _ux_hcd_ip3516_register_read          Read IP3516 register          */
/*    _ux_hcd_ip3516_register_write         Write IP3516 register         */
/*    _ux_utility_memory_allocate           Allocate memory block         */
/*    _ux_utility_semaphore_create          Create semaphore              */
/*    _ux_utility_semaphore_delete          Delete semaphore              */
/*    _ux_utility_mutex_create              Create mutex                  */
/*    _ux_utility_mutex_delete              Delete mutex                  */
/*    _ux_utility_set_interrupt_handler     Set interrupt handler         */
/*    _ux_utility_delay_ms                  Delay ms                      */
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
UINT  _ux_hcd_ip3516_initialize(UX_HCD *hcd)
{

UX_HCD_IP3516             *hcd_ip3516;
ULONG                   ip3516_register;
ULONG                   port_index;
UINT                    status = UX_SUCCESS;


    /* The controller initialized here is of IP3516 type.  */
    hcd -> ux_hcd_controller_type =  UX_IP3516_CONTROLLER;

#if UX_MAX_DEVICES > 1
    /* Initialize the max bandwidth for periodic endpoints. On IP3516,
       the spec says no more than 90% to be allocated for periodic.  */
    hcd -> ux_hcd_available_bandwidth =  UX_IP3516_AVAILABLE_BANDWIDTH;
#endif

    /* Allocate memory for this IP3516 HCD instance.  */
    hcd_ip3516 =  _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_HCD_IP3516));
    if (hcd_ip3516 == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Set the pointer to the IP3516 HCD.  */
    hcd -> ux_hcd_controller_hardware =  (VOID *) hcd_ip3516;

    /* Save the register memory address.  */
    hcd_ip3516 -> ux_hcd_ip3516_base =  (ULONG *) hcd -> ux_hcd_io;

    /* Obtain the address of the HCOR registers. This is a byte offset from the
       HCOR Cap registers.  */
    ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCCR_CAP_LENGTH);
    hcd_ip3516 -> ux_hcd_ip3516_hcor =  (ip3516_register & 0xff) >> 2;

    /* Set the generic HCD owner for the IP3516 HCD.  */
    hcd_ip3516 -> ux_hcd_ip3516_hcd_owner =  hcd;

    /* Initialize the function entry for this HCD.  */
    hcd -> ux_hcd_entry_function =  _ux_hcd_ip3516_entry;

    /* Set the state of the controller to HALTED first.  */
    hcd -> ux_hcd_status =  UX_HCD_STATUS_HALTED;

#if UX_MAX_DEVICES > 1

    /* Since this is a USB 2.0 controller, we can safely hardwire the version.  */
    hcd -> ux_hcd_version =  0x200;
#endif

    /* The IP3516 Controller should not be running. */
    ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);
    ip3516_register &=  ~IP3516_HC_IO_RS;
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_COMMAND, ip3516_register);
    _ux_utility_delay_ms(2);

    /* Perform a global reset to the controller.  */
    ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);
    ip3516_register |=  IP3516_HC_IO_HCRESET;
    _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_COMMAND, ip3516_register);

    /* Ensure the reset is complete.  */
    while (ip3516_register & IP3516_HC_IO_HCRESET)
    {

        ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);
    }

    /* Enable host mode for hardware peripheral.  */
    UX_HCD_IP3516_EXT_USB_HOST_MODE_ENABLE(hcd_ip3516);

    /* Get the number of ports on the controller. The number of ports
    needs to be reflected both for the generic HCD container and the
    local ip3516 container.  */
    ip3516_register =                         _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCCR_HCS_PARAMS);
    hcd -> ux_hcd_nb_root_hubs =            (UINT) (ip3516_register & 0xf);
    if (hcd -> ux_hcd_nb_root_hubs > UX_MAX_ROOTHUB_PORT)
        hcd -> ux_hcd_nb_root_hubs = UX_MAX_ROOTHUB_PORT;
    hcd_ip3516 -> ux_hcd_ip3516_nb_root_hubs =  hcd -> ux_hcd_nb_root_hubs;

    /* The controller transceiver can now send the device connection/extraction
    signals to the IP3516 controller.  */

    /* Create mutex for periodic list modification.  */
    status = _ux_utility_mutex_create(&hcd_ip3516 -> ux_hcd_ip3516_periodic_mutex, "ip3516_periodic_mutex");
    if (status != UX_SUCCESS)
        status = (UX_MUTEX_ERROR);

    /* We must enable the HCD protection semaphore.  */
    if (status == UX_SUCCESS)
    {
        status =  _ux_utility_semaphore_create(&hcd_ip3516 -> ux_hcd_ip3516_protect_semaphore, "ux_hcd_protect_semaphore", 1);
        if (status != UX_SUCCESS)
            status = (UX_SEMAPHORE_ERROR);
    }

    if (status == UX_SUCCESS)
    {

        /* The IP3516 Controller can now be Started. */
        ip3516_register =  _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_USB_COMMAND);

        /* Regular IP3516 with embedded TT.  */
        hcd_ip3516 -> ux_hcd_ip3516_embedded_tt = UX_HCD_IP3516_EXT_EMBEDDED_TT_SUPPORT;

        /* All ports must now be powered to pick up device insertion.  */
        _ux_hcd_ip3516_power_root_hubs(hcd_ip3516);

        /* Set the state of the controller to OPERATIONAL.  */
        hcd -> ux_hcd_status =  UX_HCD_STATUS_OPERATIONAL;

        /* Set the IP3516 Interrupt Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_INTERRUPT, IP3516_HC_INTERRUPT_ENABLE_NORMAL);

        /* The controller interrupt must have a handler and be active now.  */
        _ux_utility_set_interrupt_handler(hcd -> ux_hcd_irq, _ux_hcd_ip3516_interrupt_handler);

        /* Set the ATL PTD Skip Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDS, 0xFFFFFFFF);

        /* Set the ISO PTD Skip Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDS, 0xFFFFFFFF);

        /* Set the INT PTD Skip Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTDS, 0xFFFFFFFF);

        _ux_utility_memory_set((void*)0x40100000, 0, 1024 + 1024 + 512);

        /* Set the INT PTD  Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_INTPTD, 0x40100000);

        hcd_ip3516 ->  ux_hcd_ip3516_int_ptl_array = (usb_host_ip3516hs_ptl_struct_t *) 0x40100000;

        /* Set the ISO PTD  Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ISOPTD, 0x40100400);

        /* Set the ATL PTD  Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_ATLPTD, 0x40100800);

        hcd_ip3516 -> ux_hcd_ip3516_atl_array = (usb_host_ip3516hs_atl_struct_t * )0x40100800;

        /* Set the DATAPAYLOAD  Register.  */
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_DATAPAYLOAD, 0x40100000);

        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_LASTPTD, 0x001F1F1F);

        /* Set the Frame list size and the RUN bit.. */
        ip3516_register |= IP3516_HC_IO_RS
                        | IP3516_HC_IO_ATL_EN
                        | IP3516_HC_IO_INT_EN;
        _ux_hcd_ip3516_register_write(hcd_ip3516, IP3516_HCOR_USB_COMMAND, ip3516_register);

        _tx_byte_pool_create(&hcd_ip3516->ux_hcd_ip3516_usb_memory_pool, "USB RAM pool", (VOID*)0x40100A00, 13824);
        /* Force a enum process if CCS detected.
        ** Because CSC may keep zero in this case.
        */
        for (port_index = 0, status = 0; port_index < hcd_ip3516 -> ux_hcd_ip3516_nb_root_hubs; port_index ++)
        {

            /* Read register.  */
            ip3516_register = _ux_hcd_ip3516_register_read(hcd_ip3516, IP3516_HCOR_PORT_SC + port_index);

            /* Check CCS.  */
            if (ip3516_register & IP3516_HC_PS_CCS)
            {
                hcd_ip3516 -> ux_hcd_ip3516_hcd_owner -> ux_hcd_root_hub_signal[port_index]++;
                status ++;
            }
        }

        /* Wakeup enum thread.  */
        if (status != 0)
            _ux_utility_semaphore_put(&_ux_system_host -> ux_system_host_enum_semaphore);

        /* Return successful status.  */
        return(UX_SUCCESS);
    }

    /* Error! Free resources!  */
    if (hcd_ip3516 -> ux_hcd_ip3516_periodic_mutex.tx_mutex_id != 0)
        _ux_utility_mutex_delete(&hcd_ip3516 -> ux_hcd_ip3516_periodic_mutex);
    if (hcd_ip3516 -> ux_hcd_ip3516_protect_semaphore.tx_semaphore_id != 0)
        _ux_utility_semaphore_delete(&hcd_ip3516 -> ux_hcd_ip3516_protect_semaphore);
    _ux_utility_memory_free(hcd_ip3516);

    /* Return error status code.  */
    return(status);
}

