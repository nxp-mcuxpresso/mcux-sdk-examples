/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * NOTE that if there is any change in this file, please make sure
 * rebuild the corresponding library and use the new library to
 * replace the one in your project.
 */

#ifndef UX_USER_H
#define UX_USER_H

#define UX_PERIODIC_RATE (TX_TIMER_TICKS_PER_SECOND)

#define UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH      512
#define UX_SLAVE_REQUEST_DATA_MAX_LENGTH         (1024 * 2)
#define UX_HOST_CLASS_STORAGE_MEMORY_BUFFER_SIZE (1024 * 8)

/*
 * Defined, this value represents the maximum number of devices that can be attached to the USB.
 * Normally, the theoretical maximum number on a single USB is 127 devices. This value can be
 * scaled down to conserve memory. Note that this value represents the total number of devices
 * regardless of the number of USB buses in the system.
 */
/* #define UX_MAX_DEVICES  127 */

/*
 * Defined, this value represents the maximum number of Ed,
 * regular TDs and Isochronous TDs. These values depend on
 * the type of host controller and can be reduced in memory
 * constraint environments.
 */
#define UX_MAX_ED     80
#define UX_MAX_TD     64
#define UX_MAX_ISO_TD 1

/*
 * Defined, this value represents the maximum size of
 * the HID decompressed buffer. This cannot be determined
 * in advance so we allocate a big block, usually 4K
 * but for simple HID devices like keyboard and mouse
 *  it can be reduced a lot.
 */
#define UX_HOST_CLASS_HID_DECOMPRESSION_BUFFER 4096

/*
 * Defined, this value represents the maximum number of HID usages
 * for a HID device. Default is 2048 but for simple HID devices
 * like keyboard and mouse it can be reduced a lot.
 */
#define UX_HOST_CLASS_HID_USAGES 2048

/*
 * Defined, this value represents the maximum number of media for
 * the host storage class. Default is 8 but for memory contrained
 * resource systems this can ne reduced to 1.
 */
#define UX_HOST_CLASS_STORAGE_MAX_MEDIA 2

/*
 * Defined, this value represents the number of packets in the
 * CDC_ECM device class.
 * The default is 16.
 */
#define UX_DEVICE_CLASS_CDC_ECM_NX_PKPOOL_ENTRIES 32

/* Defined, this enables CDC ECM class to use the packet pool from NetX instance.  */
#define UX_HOST_CLASS_CDC_ECM_USE_PACKET_POOL_FROM_NETX

/* Defined, this value will only enable the host side of usbx.  */
/* #define UX_HOST_SIDE_ONLY   */

/* Defined, this value will only enable the device side of usbx.  */
/* #define UX_DEVICE_SIDE_ONLY   */

/* defined, this macro enables device audio feedback endpoint support.  */
#define UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT

/* Defined, device HID interrupt OUT transfer is supported.  */
#define UX_DEVICE_CLASS_HID_INTERRUPT_OUT_SUPPORT

/* Defined, this macro enables device bi-directional-endpoint support.  */
#define UX_DEVICE_BIDIRECTIONAL_ENDPOINT_SUPPORT

/*
 * Defined, this value will include the OTG polling thread.
 * OTG can only be active if both host/device are present.
 */
#ifndef UX_HOST_SIDE_ONLY
#ifndef UX_DEVICE_SIDE_ONLY
/* #define UX_OTG_SUPPORT */
#endif
#endif

/*
 * Defined, this value represents the maximum size of single
 * tansfers for the SCSI data phase.
 */
#define UX_HOST_CLASS_STORAGE_MAX_TRANSFER_SIZE (1024 * 1)

/* Defined, this value represents the size of the log pool. */
#define UX_DEBUG_LOG_SIZE (1024 * 16)

extern void usbphy_set_highspeed_mode(void *regs, int on_off);

#define UX_HCD_EHCI_EXT_USBPHY_HIGHSPEED_MODE_SET(hcd_ehci, on_off)             \
    usbphy_set_highspeed_mode(hcd_ehci, on_off)

/* Defined, this value will enable split transaction on EHCI host. */
#define UX_HCD_EHCI_SPLIT_TRANSFER_ENABLE

#endif
