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

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_dcd_nxp_dci.h                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains the NXP_DCI USB device controller definitions.   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef UX_DCD_NXP_DCI_H
#define UX_DCD_NXP_DCI_H

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

/* Define NXP_DCI generic equivalences.  */

#define UX_DCD_NXP_DCI_SLAVE_CONTROLLER                           0x80
#ifndef UX_DCD_NXP_DCI_MAX_ED
#define UX_DCD_NXP_DCI_MAX_ED                                     4
#endif /* UX_DCD_NXP_DCI_MAX_ED */
#define UX_DCD_NXP_DCI_IN_FIFO                                    3


#define UX_DCD_NXP_DCI_FLUSH_RX_FIFO                              0x00000010
#define UX_DCD_NXP_DCI_FLUSH_TX_FIFO                              0x00000020
#define UX_DCD_NXP_DCI_FLUSH_FIFO_ALL                             0x00000010
#define UX_DCD_NXP_DCI_ENDPOINT_SPACE_SIZE                        0x00000020
#define UX_DCD_NXP_DCI_ENDPOINT_CHANNEL_SIZE                      0x00000020


/* Define USB NXP_DCI physical endpoint status definition.  */

#define UX_DCD_NXP_DCI_ED_STATUS_UNUSED                            0
#define UX_DCD_NXP_DCI_ED_STATUS_USED                              1
#define UX_DCD_NXP_DCI_ED_STATUS_TRANSFER                          2
#define UX_DCD_NXP_DCI_ED_STATUS_STALLED                           4

/* Define USB NXP_DCI physical endpoint state machine definition.  */

#define UX_DCD_NXP_DCI_ED_STATE_IDLE                               0
#define UX_DCD_NXP_DCI_ED_STATE_DATA_TX                            1
#define UX_DCD_NXP_DCI_ED_STATE_DATA_RX                            2
#define UX_DCD_NXP_DCI_ED_STATE_STATUS_TX                          3
#define UX_DCD_NXP_DCI_ED_STATE_STATUS_RX                          4

/* Define USB NXP_DCI endpoint transfer status definition.  */

#define UX_DCD_NXP_DCI_ED_TRANSFER_STATUS_IDLE                     0
#define UX_DCD_NXP_DCI_ED_TRANSFER_STATUS_SETUP                    1
#define UX_DCD_NXP_DCI_ED_TRANSFER_STATUS_IN_COMPLETION            2
#define UX_DCD_NXP_DCI_ED_TRANSFER_STATUS_OUT_COMPLETION           3

/* Define USB NXP_DCI physical endpoint structure.  */

typedef struct UX_DCD_NXP_DCI_ED_STRUCT
{

    UCHAR           ux_dcd_nxp_dci_ed_status;
    UCHAR           ux_dcd_nxp_dci_ed_state;
    UCHAR           ux_dcd_nxp_dci_ed_index;
    UCHAR           ux_dcd_nxp_dci_ed_direction;
    struct UX_SLAVE_ENDPOINT_STRUCT
                    *ux_dcd_nxp_dci_ed_endpoint;
} UX_DCD_NXP_DCI_ED;


/* Define USB NXP_DCI DCD structure definition.  */

typedef struct UX_DCD_NXP_DCI_STRUCT
{

    struct UX_SLAVE_DCD_STRUCT
                        *ux_dcd_nxp_dci_dcd_owner;
    struct UX_DCD_NXP_DCI_ED_STRUCT
                        ux_dcd_nxp_dci_ed[UX_DCD_NXP_DCI_MAX_ED];
    struct UX_DCD_NXP_DCI_ED_STRUCT
                        ux_dcd_nxp_dci_ed_in[UX_DCD_NXP_DCI_MAX_ED];
    usb_device_handle   handle;
} UX_DCD_NXP_DCI;


/* Define USB NXP_DCI DCD prototypes.  */

UINT            _ux_dcd_nxp_dci_endpoint_create(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_nxp_dci_endpoint_destroy(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_nxp_dci_endpoint_reset(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_nxp_dci_endpoint_stall(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_nxp_dci_endpoint_status(UX_DCD_NXP_DCI *dcd_nxp_dci, ULONG endpoint_index);
UINT            _ux_dcd_nxp_dci_frame_number_get(UX_DCD_NXP_DCI *dcd_nxp_dci, ULONG *frame_number);
UINT            _ux_dcd_nxp_dci_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter);
UINT            _ux_dcd_nxp_dci_initialize_complete(VOID);
UINT            _ux_dcd_nxp_dci_transfer_request(UX_DCD_NXP_DCI *dcd_nxp_dci, UX_SLAVE_TRANSFER *transfer_request);
UINT            _ux_dcd_nxp_dci_initialize(ULONG controller_id, VOID** handle_ptr);
UINT            _ux_dcd_nxp_dci_uninitialize(ULONG controller_id, VOID** handle_ptr);
usb_status_t    _ux_dcd_nxp_dci_callback(usb_device_handle handle, uint32_t event, void *param);
usb_status_t    _ux_dcd_nxp_dci_control_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam);
usb_status_t    _ux_dcd_nxp_dci_transfer_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam);

#define ux_dcd_nxp_dci_initialize                      _ux_dcd_nxp_dci_initialize

#endif
