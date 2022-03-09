/*
 * @brief This file contains a USB MSC RAM example using USB ROM Drivers.
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

#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "fsl_device_registers.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "msc_disk.h"
#include "romapi_5460x.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern void DataRam_Initialize(void);
extern void BOARD_InitHardware(void);
/* USB descriptor arrays defined *_desc.c file */
extern const uint8_t USB_DeviceDescriptor[];
extern uint8_t USB_HsConfigDescriptor[];
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
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);
/*******************************************************************************
 * Variables
 ******************************************************************************/
ALIGN(2048) uint8_t g_memUsbStack[USB_STACK_MEM_SIZE];

static USBD_HANDLE_T g_hUsb;

const USBD_API_T *g_pUsbApi;

/*******************************************************************************
 * Code
 ******************************************************************************/

/* Interrupt service routine of USB */
void USB0_IRQHandler(void)
{
    uint32_t *addr = (uint32_t *)USB0->EPLISTSTART;

    if (USB0->DEVCMDSTAT & _BIT(8))
    {                             /* if setup packet is received */
        addr[2] &= ~(0x80000000); /* clear active bit for EP0_IN */
    }

    USBD_API->hw->ISR(g_hUsb);
}

/* Function to find the interface descriptor */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
    USB_COMMON_DESCRIPTOR *pD;
    USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
    uint32_t next_desc_adr;

    pD            = (USB_COMMON_DESCRIPTOR *)pDesc;
    next_desc_adr = (uint32_t)pDesc;

    while (pD->bLength)
    {
        /* is it interface descriptor */
        if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
        {
            pIntfDesc = (USB_INTERFACE_DESCRIPTOR *)pD;
            /* did we find the right interface descriptor */
            if (pIntfDesc->bInterfaceClass == intfClass)
            {
                break;
            }
        }
        pIntfDesc     = 0;
        next_desc_adr = (uint32_t)pD + pD->bLength;
        pD            = (USB_COMMON_DESCRIPTOR *)next_desc_adr;
    }

    return pIntfDesc;
}

/* Main application entry point */
int main(void)
{
    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T desc;
    ErrorCode_t ret = LPC_OK;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));

    DataRam_Initialize();

    /* Init USB API structure */
    g_pUsbApi = (const USBD_API_T *)LPC_ROM_API->usbdApiBase;

    /* initialize call back structures */
    memset((void *)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    usb_param.usb_reg_base = USB0_BASE;
    usb_param.mem_base     = (uint32_t)&g_memUsbStack;
    usb_param.mem_size     = USB_STACK_MEM_SIZE;
    usb_param.max_num_ep   = 2;

    /* Set the USB descriptors */
    desc.device_desc = (uint8_t *)USB_DeviceDescriptor;
    desc.string_desc = (uint8_t *)USB_StringDescriptor;

    /* Note, to pass USBCV test full-speed only devices should have both
       descriptor arrays point to same location and device_qualifier set to 0.
     */
    desc.high_speed_desc  = USB_FsConfigDescriptor;
    desc.full_speed_desc  = USB_FsConfigDescriptor;
    desc.device_qualifier = 0;

    /* USB Initialization */
    ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
    if (ret == LPC_OK)
    {
        ret = mscDisk_init(g_hUsb, &desc, &usb_param);
        if (ret == LPC_OK)
        {
            /*  enable USB interrrupts */
            NVIC_EnableIRQ(USB0_IRQn);
            /* now connect */
            USBD_API->hw->Connect(g_hUsb, 1);
        }
    }

    while (1U)
    {
        /* Sleep until next IRQ happens */
        __WFI();
    }
}
