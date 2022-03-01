/*
 * @brief USB audio descriptors
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "fsl_device_registers.h"
#include "app_usbd_cfg.h"
#include "usbd_rom_api.h"
#include "usbd_adc.h"
#include "audio_usbd.h"
//#include "ui2s_api.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/**
 * USB Standard Device Descriptor
 */
ALIGNED(4)
const uint8_t USB_DeviceDescriptor[] = {
    USB_DEVICE_DESC_SIZE,       /* bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /* bDescriptorType */
    WBVAL(0x0200),              /* bcdUSB */
    0x00,                       /* bDeviceClass */
    0x00,                       /* bDeviceSubClass */
    0x01,                       /* bDeviceProtocol */
    USB_MAX_PACKET0,            /* bMaxPacketSize0 */
    WBVAL(0x1FC9),              /* idVendor */
    WBVAL(0x008E),              /* idProduct */
    WBVAL(0x0100),              /* bcdDevice */
    0x01,                       /* iManufacturer */
    0x02,                       /* iProduct */
    0x03,                       /* iSerialNumber */
    0x01                        /* bNumConfigurations */
};

/**
 * USB FSConfiguration Descriptor
 * All Descriptors (Configuration, Interface, Endpoint, Class, Vendor)
 */
ALIGNED(4)
uint8_t USB_FsConfigDescriptor[] = {
    /* Configuration 1 */
    USB_CONFIGURATION_DESC_SIZE,       /* bLength */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
    WBVAL(                             /* wTotalLength */
          USB_CONFIGURATION_DESC_SIZE + USB_INTERFACE_DESC_SIZE + AUDIO_CONTROL_INTERFACE_DESC_SZ(2) +
          AUDIO_INPUT_TERMINAL_DESC_SIZE + AUDIO_FEATURE_UNIT_DESC_SZ(2, 1) + AUDIO_OUTPUT_TERMINAL_DESC_SIZE +
          AUDIO_INPUT_TERMINAL_DESC_SIZE + AUDIO_OUTPUT_TERMINAL_DESC_SIZE +

          USB_INTERFACE_DESC_SIZE + USB_INTERFACE_DESC_SIZE + AUDIO_STREAMING_INTERFACE_DESC_SIZE +
          AUDIO_FORMAT_TYPE_I_DESC_SZ(1) + AUDIO_STANDARD_ENDPOINT_DESC_SIZE + AUDIO_STREAMING_ENDPOINT_DESC_SIZE +

          USB_INTERFACE_DESC_SIZE + USB_INTERFACE_DESC_SIZE + AUDIO_STREAMING_INTERFACE_DESC_SIZE +
          AUDIO_FORMAT_TYPE_I_DESC_SZ(1) + AUDIO_STANDARD_ENDPOINT_DESC_SIZE + AUDIO_STREAMING_ENDPOINT_DESC_SIZE + 0),
    0x03,                     /* bNumInterfaces */
    0x01,                     /* bConfigurationValue */
    0x00,                     /* iConfiguration */
    USB_CONFIG_SELF_POWERED,  /* bmAttributes  */
    USB_CONFIG_POWER_MA(100), /* bMaxPower */

    /* Interface 0, Alternate Setting 0, Audio Control */
    USB_INTERFACE_DESC_SIZE,       /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_ADC_CTRL_IF,               /* bInterfaceNumber */
    0x00,                          /* bAlternateSetting */
    0x00,                          /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOCONTROL,   /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
    0x00,                          /* iInterface */
    /* Audio Control Interface */
    AUDIO_CONTROL_INTERFACE_DESC_SZ(2), /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,    /* bDescriptorType */
    AUDIO_CONTROL_HEADER,               /* bDescriptorSubtype */
    WBVAL(0x0100), /* 1.00 */           /* bcdADC */
    WBVAL(                              /* wTotalLength */
          AUDIO_CONTROL_INTERFACE_DESC_SZ(2) + AUDIO_INPUT_TERMINAL_DESC_SIZE + AUDIO_FEATURE_UNIT_DESC_SZ(2, 1) +
          AUDIO_OUTPUT_TERMINAL_DESC_SIZE + AUDIO_INPUT_TERMINAL_DESC_SIZE + AUDIO_OUTPUT_TERMINAL_DESC_SIZE),
    0x02,               /* bInCollection */
    USB_ADC_SPEAKER_IF, /* baInterfaceNr(1) */
    USB_ADC_MIC_IF,     /* baInterfaceNr(2) */
    /* Audio Input Terminal */
    AUDIO_INPUT_TERMINAL_DESC_SIZE,           /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    AUDIO_CONTROL_INPUT_TERMINAL,             /* bDescriptorSubtype */
    0x01,                                     /* bTerminalID */
    WBVAL(AUDIO_TERMINAL_USB_STREAMING),      /* wTerminalType */
    0x00,                                     /* bAssocTerminal */
    0x02,                                     /* bNrChannels */
    WBVAL(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R), /* wChannelConfig */
    0x00,                                     /* iChannelNames */
    0x00,                                     /* iTerminal */
    /* Audio Feature Unit */
    AUDIO_FEATURE_UNIT_DESC_SZ(2, 1),          /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,           /* bDescriptorType */
    AUDIO_CONTROL_FEATURE_UNIT,                /* bDescriptorSubtype */
    0x02,                                      /* bUnitID */
    0x01,                                      /* bSourceID */
    0x01,                                      /* bControlSize */
    AUDIO_CONTROL_MUTE | AUDIO_CONTROL_VOLUME, /* bmaControls(0) */
    0x00,                                      /* bmaControls(1) */
    0x00,                                      /* bmaControls(2) */
    0x00,                                      /* iFeature */
    /* Audio Output Terminal */
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE, /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    AUDIO_CONTROL_OUTPUT_TERMINAL,   /* bDescriptorSubtype */
    0x03,                            /* bTerminalID */
    WBVAL(AUDIO_TERMINAL_SPEAKER),   /* wTerminalType */
    0x00,                            /* bAssocTerminal */
    0x02,                            /* bSourceID */
    0x00,                            /* iTerminal */
    /* Audio Input Terminal -microphone */
    AUDIO_INPUT_TERMINAL_DESC_SIZE,           /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,          /* bDescriptorType */
    AUDIO_CONTROL_INPUT_TERMINAL,             /* bDescriptorSubtype */
    0x04,                                     /* bTerminalID */
    WBVAL(AUDIO_TERMINAL_MICROPHONE),         /* wTerminalType */
    0x00,                                     /* bAssocTerminal */
    0x02,                                     /* bNrChannels */
    WBVAL(AUDIO_CHANNEL_L | AUDIO_CHANNEL_R), /* wChannelConfig */
    0x00,                                     /* iChannelNames */
    0x00,                                     /* iTerminal */
    /* Audio Output Terminal -microphone*/
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE,     /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    AUDIO_CONTROL_OUTPUT_TERMINAL,       /* bDescriptorSubtype */
    0x05,                                /* bTerminalID */
    WBVAL(AUDIO_TERMINAL_USB_STREAMING), /* wTerminalType */
    0x00,                                /* bAssocTerminal */
    0x04,                                /* bSourceID */
    0x00,                                /* iTerminal */

    /* Interface 1, Alternate Setting 0, Audio Streaming - Zero Bandwidth */
    USB_INTERFACE_DESC_SIZE,       /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_ADC_SPEAKER_IF,            /* bInterfaceNumber */
    0x00,                          /* bAlternateSetting */
    0x00,                          /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
    0x04,                          /* iInterface */

    /* Interface 1, Alternate Setting 1, Audio Streaming - Operational */
    USB_INTERFACE_DESC_SIZE,       /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_ADC_SPEAKER_IF,            /* bInterfaceNumber */
    0x01,                          /* bAlternateSetting */
    0x01,                          /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
    0x04,                          /* iInterface */
    /* Audio Streaming Interface */
    AUDIO_STREAMING_INTERFACE_DESC_SIZE, /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    AUDIO_STREAMING_GENERAL,             /* bDescriptorSubtype */
    0x01,                                /* bTerminalLink */
    0x00,                                /* bDelay */
    WBVAL(AUDIO_FORMAT_PCM),             /* wFormatTag */
    /* Audio Type I Format */
    AUDIO_FORMAT_TYPE_I_DESC_SZ(1),  /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    AUDIO_STREAMING_FORMAT_TYPE,     /* bDescriptorSubtype */
    AUDIO_FORMAT_TYPE_I,             /* bFormatType */
    DEF_NUM_CHANNELS,                /* bNrChannels */
    AUDIO_BYTES_PER_CHANNEL,         /* bSubFrameSize */
    DEF_RES_BITS,                    /* bBitResolution */
    1,                               /* bSamFreqType */
    B3VAL(DEF_SAMPLE_RATE),          /* tSamFreq */
    /* Endpoint - Standard Descriptor */
    AUDIO_STANDARD_ENDPOINT_DESC_SIZE,                             /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                                  /* bDescriptorType */
    USB_ADC_OUT_EP,                                                /* bEndpointAddress */
    USB_ENDPOINT_SYNC_SYNCHRONOUS | USB_ENDPOINT_TYPE_ISOCHRONOUS, /* bmAttributes */
    WBVAL(AUDIO_MAX_PKT_SZ),                                       /* wMaxPacketSize */
    0x01,                                                          /* bInterval */
    0x00,                                                          /* bRefresh */
    0x00,                                                          /* bSynchAddress */
    /* Endpoint - Audio Streaming */
    AUDIO_STREAMING_ENDPOINT_DESC_SIZE, /* bLength */
    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType */
    AUDIO_ENDPOINT_GENERAL,             /* bDescriptor */
    AUDIO_CONTROL_SAMPLING_FREQ,        /* bmAttributes */
    0x02,                               /* bLockDelayUnits - msec based delay*/
    WBVAL(0x0005),                      /* wLockDelay - 5 msec delay requested*/

    /* Interface 2, Alternate Setting 0, Audio Streaming - Zero Bandwidth */
    USB_INTERFACE_DESC_SIZE,       /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_ADC_MIC_IF,                /* bInterfaceNumber */
    0x00,                          /* bAlternateSetting */
    0x00,                          /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
    0x05,                          /* iInterface */

    /* Interface 2, Alternate Setting 1, Audio Streaming - Operational */
    USB_INTERFACE_DESC_SIZE,       /* bLength */
    USB_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    USB_ADC_MIC_IF,                /* bInterfaceNumber */
    0x01,                          /* bAlternateSetting */
    0x01,                          /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,        /* bInterfaceClass */
    AUDIO_SUBCLASS_AUDIOSTREAMING, /* bInterfaceSubClass */
    AUDIO_PROTOCOL_UNDEFINED,      /* bInterfaceProtocol */
    0x05,                          /* iInterface */
    /* Audio Streaming Interface */
    AUDIO_STREAMING_INTERFACE_DESC_SIZE, /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
    AUDIO_STREAMING_GENERAL,             /* bDescriptorSubtype */
    0x05,                                /* bTerminalLink */
    0x01,                                /* bDelay */
    WBVAL(AUDIO_FORMAT_PCM),             /* wFormatTag */
    /* Audio Type I Format */
    AUDIO_FORMAT_TYPE_I_DESC_SZ(1),  /* bLength */
    AUDIO_INTERFACE_DESCRIPTOR_TYPE, /* bDescriptorType */
    AUDIO_STREAMING_FORMAT_TYPE,     /* bDescriptorSubtype */
    AUDIO_FORMAT_TYPE_I,             /* bFormatType */
    DEF_NUM_CHANNELS,                /* bNrChannels */
    AUDIO_BYTES_PER_CHANNEL,         /* bSubFrameSize */
    DEF_RES_BITS,                    /* bBitResolution */
    1,                               /* bSamFreqType */
    B3VAL(DEF_SAMPLE_RATE),          /* tSamFreq */
    /* Endpoint - Standard Descriptor */
    AUDIO_STANDARD_ENDPOINT_DESC_SIZE,                             /* bLength */
    USB_ENDPOINT_DESCRIPTOR_TYPE,                                  /* bDescriptorType */
    USB_ADC_IN_EP,                                                 /* bEndpointAddress */
    USB_ENDPOINT_SYNC_SYNCHRONOUS | USB_ENDPOINT_TYPE_ISOCHRONOUS, /* bmAttributes */
    WBVAL(AUDIO_MAX_PKT_SZ),                                       /* wMaxPacketSize */
    0x01,                                                          /* bInterval */
    0x00,                                                          /* bRefresh */
    0x00,                                                          /* bSynchAddress */
    /* Endpoint - Audio Streaming */
    AUDIO_STREAMING_ENDPOINT_DESC_SIZE, /* bLength */
    AUDIO_ENDPOINT_DESCRIPTOR_TYPE,     /* bDescriptorType */
    AUDIO_ENDPOINT_GENERAL,             /* bDescriptor */
    AUDIO_CONTROL_SAMPLING_FREQ,        /* bmAttributes */
    0x02,                               /* bLockDelayUnits - msec based delay*/
    WBVAL(0x0005),                      /* wLockDelay - 5 msec delay requested*/

    /* Terminator */
    0 /* bLength */
};

/**
 * USB String Descriptor (optional)
 */
ALIGNED(4)
const uint8_t USB_StringDescriptor[] = {
    /* Index 0x00: LANGID Codes */
    0x04,                       /* bLength */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    WBVAL(0x0409),
    /* US English */ /* wLANGID */
    /* Index 0x01: Manufacturer */
    (3 * 2 + 2),                /* bLength (13 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    'N',
    0,
    'X',
    0,
    'P',
    0,
    /* Index 0x02: Product */
    (9 * 2 + 2),                /* bLength */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    'U',
    0,
    'S',
    0,
    'B',
    0,
    ' ',
    0,
    'A',
    0,
    'u',
    0,
    'd',
    0,
    'i',
    0,
    'o',
    0,
    /* Index 0x03: Serial Number */
    (6 * 2 + 2),                /* bLength (8 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    'N',
    0,
    'X',
    0,
    'P',
    0,
    '-',
    0,
    '0',
    0,
    '0',
    0,
    /* Index 0x04: Interface 1, Alternate Setting 0 */
    (7 * 2 + 2),                /* bLength (7 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    'S',
    0,
    'p',
    0,
    'e',
    0,
    'a',
    0,
    'k',
    0,
    'e',
    0,
    'r',
    0,
    /* Index 0x05: Interface 2, Alternate Setting 0 */
    (3 * 2 + 2),                /* bLength (3 Char + Type + length) */
    USB_STRING_DESCRIPTOR_TYPE, /* bDescriptorType */
    'M',
    0,
    'I',
    0,
    'C',
    0,
};
