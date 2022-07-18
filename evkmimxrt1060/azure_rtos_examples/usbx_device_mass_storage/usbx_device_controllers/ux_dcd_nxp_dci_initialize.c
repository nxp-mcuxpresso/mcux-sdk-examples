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
/**   NXP_DCI Controller Driver                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE
#define UX_DCD_NXP_DCI_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_dcd_nxp_dci.h"
#include "ux_device_stack.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_dcd_nxp_dci_initialize                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the NXP_DCI USB device controller.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    controller_id                         Controller ID                 */
/*    handle_ptr                            Pointer to driver handle      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    USB_DeviceInit                        Initialize USB driver         */
/*    USB_DeviceRun                         Enable device                 */
/*    _ux_utility_memory_allocate           Allocate memory               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    USBX Device Stack                                                   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _ux_dcd_nxp_dci_initialize(ULONG controller_id, VOID** handle_ptr)
{

UX_SLAVE_DCD            *dcd;
UX_DCD_NXP_DCI            *dcd_nxp_dci;
usb_status_t             status;


    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave -> ux_system_slave_dcd;

    /* The controller initialized here is of NXP_DCI type.  */
    dcd -> ux_slave_dcd_controller_type =  UX_DCD_NXP_DCI_SLAVE_CONTROLLER;

    /* Allocate memory for this NXP_DCI DCD instance.  */
    dcd_nxp_dci =  _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY, sizeof(UX_DCD_NXP_DCI));

    /* Check if memory was properly allocated.  */
    if(dcd_nxp_dci == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    /* Set the pointer to the NXP_DCI DCD.  */
    dcd -> ux_slave_dcd_controller_hardware =  (VOID *) dcd_nxp_dci;

    /* Set the generic DCD owner for the NXP_DCI DCD.  */
    dcd_nxp_dci -> ux_dcd_nxp_dci_dcd_owner =  dcd;

    /* Initialize the function collector for this DCD.  */
    dcd -> ux_slave_dcd_function =  _ux_dcd_nxp_dci_function;

    status = USB_DeviceInit((uint8_t)controller_id, _ux_dcd_nxp_dci_callback, &dcd_nxp_dci -> handle);

    *handle_ptr =  dcd_nxp_dci -> handle;

    USB_DeviceRun(dcd_nxp_dci -> handle);

    /* Set the state of the controller to OPERATIONAL now.  */
    dcd -> ux_slave_dcd_status =  UX_DCD_STATUS_OPERATIONAL;

    /* Return completion status.  */
    return(status);
}
