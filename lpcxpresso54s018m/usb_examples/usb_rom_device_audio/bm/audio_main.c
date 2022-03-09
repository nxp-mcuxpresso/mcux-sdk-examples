/*
 * @brief USB subsystem routines.
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

#include <string.h>
#include <stdlib.h>
#include "board.h"
/* USB header files */
#include "audio_usbd.h"
#include "audio_codec.h"
//#include "ui2s_api.h"
#include "romapi_5460x.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static USBD_HANDLE_T g_hUsb;

ALIGN(64) static uint8_t data_buffer[MAX_SUBSTREAMS][AUDIO_MAX_PKT_SZ * NUM_DTDS];
void *const out_buff       = data_buffer[SUBS_SPEAKER];
const uint32_t out_buff_sz = sizeof(data_buffer[SUBS_SPEAKER]);
void *const in_buff        = data_buffer[SUBS_MIC];
const uint32_t in_buff_sz  = sizeof(data_buffer[SUBS_MIC]);

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
/* USB descriptor arrays defined *_desc.c file */
extern const uint8_t USB_DeviceDescriptor[];
extern uint8_t USB_FsConfigDescriptor[];
extern const uint8_t USB_StringDescriptor[];
extern const uint8_t USB_DeviceQualifier[];

/**
 * @brief	Find the address of interface descriptor for given class type.
 * @param	pDesc		: Pointer to configuration descriptor in which the desired class
 *			interface descriptor to be found.
 * @param	intfClass	: Interface class type to be searched.
 * @return	If found returns the address of requested interface else returns NULL.
 */
extern USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);
const USBD_API_T *g_pUsbApi;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Initialize USB sub system */
ErrorCode_t usbd_init(void)
{
    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T desc;
    ADC_INIT_PARAM_T adc_param;
    ErrorCode_t ret = LPC_OK;
    ALIGN(2048) static uint8_t usbd_mem[USB_STACK_MEM_SIZE];

    /* Enable waking-up of MCU from USB */
    SYSCON->STARTERSET[0] = SYSCON_STARTER_USB0_MASK;

    /* initialize USBD ROM API pointer. */
    g_pUsbApi = (const USBD_API_T *)LPC_ROM_API->usbdApiBase;

    /* initialize call back structures */
    memset((void *)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    usb_param.usb_reg_base = USB0_BASE;

    usb_param.max_num_ep          = 2;
    usb_param.USB_Interface_Event = ADC_Interface_Event;
    usb_param.USB_Reset_Event     = ADC_Reset_Event;
    usb_param.USB_SOF_Event       = ADC_SOF_Event;

    usb_param.USB_Suspend_Event = ADC_Suspend_Event;
    usb_param.USB_Resume_Event  = ADC_Resume_Event;

    usb_param.mem_base      = (uint32_t)usbd_mem; // USB_STACK_MEM_BASE;
    usb_param.mem_size      = USB_STACK_MEM_SIZE;
    usb_param.double_buffer = TRUE;

    /* Set the USB descriptors */
    desc.device_desc = (uint8_t *)&USB_DeviceDescriptor[0];
    desc.string_desc = (uint8_t *)&USB_StringDescriptor[0];
    /* Note, to pass USBCV test full-speed only devices should have both
       descriptor arrays point to same location and device_qualifier set to 0.
     */
    usb_param.high_speed_capable = FALSE;
    desc.high_speed_desc         = (uint8_t *)&USB_FsConfigDescriptor[0];
    desc.full_speed_desc         = (uint8_t *)&USB_FsConfigDescriptor[0];
    desc.device_qualifier        = 0;

    /* USB Initialization */
    ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
    if (ret == LPC_OK)
    {
        /* ROM driver blocks memory for ISOC EPs but in this app we don't use */
        usb_param.mem_size = sizeof(data_buffer); /* buffers for ISO-in/out endpoints listed in 24-bit ALT interface */

        /*	WORKAROUND for artf32219 ROM driver BUG:
            The mem_base parameter part of USB_param structure returned
            by Init() routine is not accurate causing memory allocation issues for
            further components.
         */
        usb_param.mem_base = (uint32_t)data_buffer;

        /* construct Audio class initialization parameters */
        memset((void *)&adc_param, 0, sizeof(ADC_INIT_PARAM_T));
        adc_param.hUsb                 = g_hUsb;
        adc_param.param                = &usb_param;
        adc_param.maxp[SUBS_SPEAKER]   = AUDIO_MAX_PKT_SZ;
        adc_param.ep_num[SUBS_SPEAKER] = USB_ADC_OUT_EP;
        adc_param.maxp[SUBS_MIC]       = AUDIO_MAX_PKT_SZ;
        adc_param.ep_num[SUBS_MIC]     = USB_ADC_IN_EP;

        /* Initialize audio class controller */
        ret = ADC_init(&adc_param);
        if (ret == LPC_OK)
        {
            /* Install isr, set priority, and enable IRQ. */
            NVIC_SetPriority(USB0_IRQn, USB_DEVICE_INTERRUPT_PRIORITY);
            /*  enable USB interrupts */
            NVIC_EnableIRQ(USB0_IRQn);
            /* now connect */
            USBD_API->hw->Connect(g_hUsb, 1);
        }
    }
    return ret;
}

/**
 * @brief	Handle interrupt from USB0
 * @return	Nothing
 */
//__attribute__ ((section("RamCodeSections")))
void USB0_IRQHandler(void)
{
    uint32_t *addr = (uint32_t *)USB0->EPLISTSTART;

    /*	WORKAROUND for artf32289 ROM driver BUG:
        As part of USB specification the device should respond
        with STALL condition for any unsupported setup packet. The host will send
        new setup packet/request on seeing STALL condition for EP0 instead of sending
        a clear STALL request. Current driver in ROM doesn't clear the STALL
        condition on new setup packet which should be fixed.
     */
    if (USB0->DEVCMDSTAT & _BIT(8))
    {                             /* if setup packet is received */
        addr[0] &= ~(_BIT(29));   /* clear EP0_OUT stall */
        addr[2] &= ~(_BIT(29));   /* clear EP0_IN stall */
        addr[2] &= ~(0x80000000); /* clear active bit for EP0_IN */
    }
    USBD_API->hw->ISR(g_hUsb);
}
