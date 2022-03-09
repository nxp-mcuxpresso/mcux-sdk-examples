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
/**   IP3511 Controller Driver                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    ux_dcd_ip3511.h                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This file contains the IP3511 USB device controller definitions.    */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/

#ifndef UX_DCD_IP3511_H
#define UX_DCD_IP3511_H

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

/* Define IP3511 generic equivalences.  */

#define UX_DCD_IP3511_SLAVE_CONTROLLER                           0x80
#ifndef UX_DCD_IP3511_MAX_ED
#define UX_DCD_IP3511_MAX_ED                                     16
#endif /* UX_DCD_IP3511_MAX_ED */
#define UX_DCD_IP3511_IN_FIFO                                    3


#define UX_DCD_IP3511_FLUSH_RX_FIFO                              0x00000010
#define UX_DCD_IP3511_FLUSH_TX_FIFO                              0x00000020
#define UX_DCD_IP3511_FLUSH_FIFO_ALL                             0x00000010
#define UX_DCD_IP3511_ENDPOINT_SPACE_SIZE                        0x00000020
#define UX_DCD_IP3511_ENDPOINT_CHANNEL_SIZE                      0x00000020


/* Define USB IP3511 physical endpoint status definition.  */

#define UX_DCD_IP3511_ED_STATUS_UNUSED                            0
#define UX_DCD_IP3511_ED_STATUS_USED                              1
#define UX_DCD_IP3511_ED_STATUS_TRANSFER                          2
#define UX_DCD_IP3511_ED_STATUS_STALLED                           4

/* Define USB IP3511 physical endpoint state machine definition.  */

#define UX_DCD_IP3511_ED_STATE_IDLE                               0
#define UX_DCD_IP3511_ED_STATE_DATA_TX                            1
#define UX_DCD_IP3511_ED_STATE_DATA_RX                            2
#define UX_DCD_IP3511_ED_STATE_STATUS_TX                          3
#define UX_DCD_IP3511_ED_STATE_STATUS_RX                          4

/* Define USB IP3511 endpoint transfer status definition.  */

#define UX_DCD_IP3511_ED_TRANSFER_STATUS_IDLE                     0
#define UX_DCD_IP3511_ED_TRANSFER_STATUS_SETUP                    1
#define UX_DCD_IP3511_ED_TRANSFER_STATUS_IN_COMPLETION            2
#define UX_DCD_IP3511_ED_TRANSFER_STATUS_OUT_COMPLETION           3

/* Define USB IP3511 physical endpoint structure.  */

typedef struct UX_DCD_IP3511_ED_STRUCT 
{

    UCHAR           ux_dcd_ip3511_ed_status;
    UCHAR           ux_dcd_ip3511_ed_state;
    UCHAR           ux_dcd_ip3511_ed_index;
    UCHAR           ux_dcd_ip3511_ed_direction;
    struct UX_SLAVE_ENDPOINT_STRUCT             
                    *ux_dcd_ip3511_ed_endpoint;
} UX_DCD_IP3511_ED;


/* Define USB IP3511 DCD structure definition.  */

typedef struct UX_DCD_IP3511_STRUCT
{

    struct UX_SLAVE_DCD_STRUCT
                        *ux_dcd_ip3511_dcd_owner;
    struct UX_DCD_IP3511_ED_STRUCT
                        ux_dcd_ip3511_ed[UX_DCD_IP3511_MAX_ED];
    usb_device_handle   handle;
} UX_DCD_IP3511;


/* Define USB IP3511 DCD prototypes.  */

UINT            _ux_dcd_ip3511_endpoint_create(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_ip3511_endpoint_destroy(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_ip3511_endpoint_reset(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_ip3511_endpoint_stall(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_ENDPOINT *endpoint);
UINT            _ux_dcd_ip3511_endpoint_status(UX_DCD_IP3511 *dcd_ip3511, ULONG endpoint_index);
UINT            _ux_dcd_ip3511_frame_number_get(UX_DCD_IP3511 *dcd_ip3511, ULONG *frame_number);
UINT            _ux_dcd_ip3511_function(UX_SLAVE_DCD *dcd, UINT function, VOID *parameter);
UINT            _ux_dcd_ip3511_initialize_complete(VOID);
UINT            _ux_dcd_ip3511_transfer_request(UX_DCD_IP3511 *dcd_ip3511, UX_SLAVE_TRANSFER *transfer_request);
UINT            _ux_dcd_ip3511_initialize(ULONG controller_id, VOID** handle_ptr);
UINT            _ux_dcd_ip3511_uninitialize(ULONG controller_id, VOID** handle_ptr);
usb_status_t    _ux_dcd_ip3511_callback(usb_device_handle handle, uint32_t event, void *param);
usb_status_t    _ux_dcd_ip3511_control_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam);
usb_status_t    _ux_dcd_ip3511_transfer_callback(usb_device_handle handle,
                                              usb_device_endpoint_callback_message_struct_t *message,
                                              void *callbackParam);

#define ux_dcd_ip3511_initialize                      _ux_dcd_ip3511_initialize

#endif

