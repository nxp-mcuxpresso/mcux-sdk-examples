/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _USB_CDC_VNIC_H_
#define _USB_CDC_VNIC_H_ 1

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#define DATA_BUFF_SIZE HS_CDC_VCOM_BULK_OUT_PACKET_SIZE

#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#define DATA_BUFF_SIZE FS_CDC_VCOM_BULK_OUT_PACKET_SIZE

#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#define DATA_BUFF_SIZE FS_CDC_VCOM_BULK_OUT_PACKET_SIZE

#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#define DATA_BUFF_SIZE HS_CDC_VCOM_BULK_OUT_PACKET_SIZE
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
/* Currently configured line coding */
#define LINE_CODING_SIZE (0x07)
#define LINE_CODING_DTERATE (115200)
#define LINE_CODING_CHARFORMAT (0x00)
#define LINE_CODING_PARITYTYPE (0x00)
#define LINE_CODING_DATABITS (0x08)

/* Communications feature */
#define COMM_FEATURE_DATA_SIZE (0x02)
#define STATUS_ABSTRACT_STATE (0x0000)
#define COUNTRY_SETTING (0x0000)

/* Notification of serial state */
#define NOTIF_PACKET_SIZE (0x08)
#define UART_BITMAP_SIZE (0x02)

/* Transmit state of virtual nic device */
typedef enum _usb_cdc_vnic_tx_state
{
    TX_IDLE = 0,         /* The usb transmit is in idle state. */
    TX_PART_ONE_PROCESS, /* The usb device is waiting for the completion of the part 1 of the RNDIS packet. */
    TX_PART_ONE_DONE,    /* The usb device finishs sending the part 1 of the RNDIS packet. */
    TX_PART_TWO_PROCESS, /* The usb device is waiting for the completion of the part 2 of the RNDIS packet. */
    TX_PART_TWO_DONE,    /* The usb device finishs sending the part 2 of the RNDIS packet. */
    TX_ZLP_PROCESS,      /* The usb device is waiting for the completion of the zero length packet. */
    TX_ZLP_DONE,         /* The usb device finishs sending the zero length packet. */
} usb_cdc_vnic_tx_state_t;

/* Receive state of virtual nic device */
typedef enum _usb_cdc_vnic_rx_state
{
    RX_IDLE = 0,         /* The usb receive is in idle state. */
    RX_PART_ONE_PROCESS, /* The usb device is waiting for the completion of the part 1 of the RNDIS packet. */
    RX_PART_ONE_DONE,    /* The usb device finishs receiving the part 1 of the RNDIS packet. */
    RX_PART_TWO_PROCESS, /* The usb device is waiting for the completion of the part 2 of the RNDIS packet. */
    RX_PART_TWO_DONE,    /* The usb device finishs receiving the part 2 of the RNDIS packet. */
    RX_USB2ENET_PROCESS, /* The usb device is waiting for the completion of sending the ethernet packet to ethernet
                            module. */
    RX_USB2ENET_DONE,    /* The usb device finishs sending the ethernet packet to ethernet module. */
} usb_cdc_vnic_rx_state_t;

/* Traffic information of virtual nic device */
typedef struct _nic_traffic_info
{
    uint32_t enetTxHost2usb;        /* The count of RNDIS packet that is sent from host to usb device. */
    uint32_t enetTxHost2usbDiscard; /* The count of Ethernet packet that is discarded by usb device. */
    uint32_t enetTxUsb2enet;        /* The count of Ethernet packet that is sent from usb device to ethernet. */
    uint32_t enetTxUsb2enetFail; /* The count of Ethernet packet that is failed to sent from usb device to ethernet. */
    uint32_t enetTxUsb2enetSent; /* The count of Ethernet packet that is sent to sent from usb device to ethernet. */
    uint32_t enetTxUsb2hostCleared; /* The count of Ethernet packet that is cleared in ethernet packet Tx queue. */
    uint32_t enetRxEnet2usb;        /* The count of Ethernet packet that is received from ethernet to usb device . */
    uint32_t enetRxUsb2host;        /* The count of RNDIS packet that is received from usb device to host. */
    uint32_t enetRxUsb2hostSent;    /* The count of RNDIS packet that is actually sent by usb device. */
    uint32_t enetRxUsb2hostDiscard; /* The count of Ethernet packet that is discarded by usb device. */
    uint32_t enetRxUsb2hostCleared; /* The count of Ethernet packet that is cleared in ethernet packet Rx queue. */
    uint32_t usbTxPart_1Len;        /* The length of the part 1 of the packet sent by usb device. */
    uint8_t *usbTxNicData;          /* The pointer of the ethernet data packet. */
    uint32_t usbTxRndisPkgLength;   /* The length of the RNDIS packet that is sent by the usb devic. */
    uint32_t usbTxZeroSendFlag;     /* The flag which indicates if a zero length packet will be sent. */
    pbuf_t usbTxEnetPcb; /* The pointer of the ethernet packet control block which contains the ethernet tx packet. */
    volatile uint8_t *usbRxPartOneBuf; /* The buffer pointer of the part 1 packet which is received by usb device. */
    volatile uint32_t usbRxPartOneLen; /* The length of the part 1 of the packet which is received by usb device. */
    volatile uint8_t *usbRxPartTwoBuf; /* The buffer pointer of the part 2 packet which is received by usb device. */
    volatile uint32_t usbRxPartTwoLen; /* The length of the part 2 of the packet which is received by usb device. */
    volatile usb_cdc_vnic_tx_state_t usbTxState; /* The state of the usb transimit direction. */
    volatile usb_cdc_vnic_rx_state_t usbRxState; /* The state of the usb receive direction. */
} nic_traffic_info_t;

/* Define the types for application */
typedef struct _usb_cdc_vnic
{
    usb_device_handle deviceHandle;
    usb_device_cdc_rndis_struct_t *rndisHandle;
    uint8_t attach;
    uint8_t speed;
    nic_traffic_info_t nicTrafficInfo;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_CDC_VNIC_INTERFACE_COUNT];
} usb_cdc_vnic_t;

/* Define packet message format  */
typedef struct _rndis_packet_msg_format
{
    /*Specifies the Remote NDIS message type.
      This is set to RNDIS_PACKET_MSG = 0x1.*/
    uint32_t messageType;
    /*Message length in bytes, including appended packet data, out-of-band
      data, per-packet-info data, and both internal and external padding*/
    uint32_t messageLen;
    /*Specifies the offset in bytes from the start of the DataOffset field of
      this message to the start of the data. This is an integer multiple of 4*/
    uint32_t dataOffset;
    /*Specifies the number of bytes in the data content of this message.*/
    uint32_t dataLen;
    /*Specifies the offset in bytes of the first out of band data record from
      the start of the DataOffset field of this message.  Set to 0 if there is
      no out-of-band data. Otherwise this is an integer multiple of 4*/
    uint32_t oobDataOffset;
    /*Specifies in bytes the total length of the out of band data.*/
    uint32_t oobDataLen;
    /*Specifies the number of out of band records in this message*/
    uint32_t numOobDataElems;
    /*Specifies in bytes the offset from the beginning of the DataOffset field
      in the RNDIS_PACKET_MSG data message to the start of the first per
      packet info data record.  Set to 0 if there is no per-packet data.
      Otherwise this is an integer multiple of 4*/
    uint32_t perPktInfoOffset;
    /*Specifies in bytes the total length of the per packet information
      contained in this message*/
    uint32_t perPktInfoLen;
    /*Reserved for connection-oriented devices.  Set to 0.*/
    uint32_t vcHandle;
    /*Reserved. Set to 0.*/
    uint32_t reserved;
} rndis_packet_msg_format_t;
#endif /* _USB_CDC_VNIC_H_ */
