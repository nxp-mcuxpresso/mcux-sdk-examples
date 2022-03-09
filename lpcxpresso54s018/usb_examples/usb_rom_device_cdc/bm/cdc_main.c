/*
 * @brief Vitual communication port example
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
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "romapi_5460x.h"
#include "cdc_vcom.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern void BOARD_InitHardware(void);
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
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static USBD_HANDLE_T g_hUsb;
ALIGN(2048) uint8_t g_memUsbStack[USB_STACK_MEM_SIZE];

typedef struct _buffer_node_struct
{
    struct _buffer_node_struct *next;
    uint32_t length;
    uint8_t buffer[64];
} buffer_node_struct_t;

#define BUFFER_POOL_COUNT (8U)
buffer_node_struct_t buffer[BUFFER_POOL_COUNT];

buffer_node_struct_t *bufferCurrent;
buffer_node_struct_t *bufferPoolList;
buffer_node_struct_t *bufferDataList;

const USBD_API_T *g_pUsbApi;

/*******************************************************************************
 * Code
 ******************************************************************************/

void AddNode(buffer_node_struct_t **list, buffer_node_struct_t *node)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    *list = node;
    if (p)
    {
        node->next = p;
    }
    else
    {
        node->next = NULL;
    }
    __enable_irq();
}

void AddNodeToEnd(buffer_node_struct_t **list, buffer_node_struct_t *node)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    if (p)
    {
        while (p->next)
        {
            p = p->next;
        }
        p->next = node;
    }
    else
    {
        *list = node;
    }
    node->next = NULL;
    __enable_irq();
}

buffer_node_struct_t *GetNode(buffer_node_struct_t **list)
{
    buffer_node_struct_t *p = *list;
    __disable_irq();
    if (p)
    {
        *list = p->next;
    }
    __enable_irq();
    return p;
}

void sendComplete(void)
{
    buffer_node_struct_t *p;

    p = GetNode(&bufferDataList);
    if (p)
    {
        AddNode(&bufferPoolList, p);
    }
    if (bufferDataList)
    {
        vcom_write(&bufferDataList->buffer[0], bufferDataList->length);
    }
}
/**
 * @brief    Handle interrupt from USB0
 * @return    Nothing
 */
void USB0_IRQHandler(void)
{
    USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
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

static void USB_DeviceApplicationInit(void)
{
    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T desc;
    ErrorCode_t ret = LPC_OK;

    /* initialize USBD ROM API pointer. */
    g_pUsbApi = (const USBD_API_T *)LPC_ROM_API->usbdApiBase;

    /* initialize call back structures */
    memset((void *)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));

    usb_param.usb_reg_base = USB0_BASE;
    /*    WORKAROUND for artf44835 ROM driver BUG:
        Code clearing STALL bits in endpoint reset routine corrupts memory area
        next to the endpoint control data. For example When EP0, EP1_IN, EP1_OUT,
        EP2_IN are used we need to specify 3 here. But as a workaround for this
        issue specify 4. So that extra EPs control structure acts as padding buffer
        to avoid data corruption. Corruption of padding memory doesn��t affect the
        stack/program behaviour.
     */
    usb_param.max_num_ep    = 3 + 1;
    usb_param.mem_base      = (uint32_t)&g_memUsbStack;
    usb_param.mem_size      = USB_STACK_MEM_SIZE;
    usb_param.double_buffer = FALSE;

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

    bufferPoolList = &buffer[0];
    bufferDataList = NULL;
    bufferCurrent  = NULL;
    for (int i = 1; i < BUFFER_POOL_COUNT; i++)
    {
        bufferPoolList->next = &buffer[i];
    }
    bufferPoolList->next = NULL;

    /* USB Initialization */
    ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
    if (ret == LPC_OK)
    {
        /* WORKAROUND for artf32219 ROM driver BUG:
          The mem_base parameter part of USB_param structure returned
          by Init() routine is not accurate causing memory allocation issues for
          further components.
        */
        usb_param.mem_base = (uint32_t)&g_memUsbStack + (USB_STACK_MEM_SIZE - usb_param.mem_size);

        /* Init VCOM interface */
        ret = vcom_init(g_hUsb, &desc, &usb_param);
        if (ret == LPC_OK)
        {
            /* Install isr, set priority, and enable IRQ. */
            NVIC_SetPriority(USB0_IRQn, USB_DEVICE_INTERRUPT_PRIORITY);
            NVIC_EnableIRQ(USB0_IRQn);
            /* now connect */
            USBD_API->hw->Connect(g_hUsb, 1);
        }
    }

    if (LPC_OK == ret)
    {
        PRINTF("USB CDC class based virtual Comm port example!\r\n");
    }
    else
    {
        PRINTF("USB CDC example initialization filed!\r\n");
        return;
    }
}

/**
 * @brief    main routine for blinky example
 * @return    Function should not exit.
 */
int main(void)
{
    uint32_t prompt = 0;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset USB0 and USB1 device */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

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

    USB_DeviceApplicationInit();

    while (1U)
    {
        /* Check if host has connected and opened the VCOM port */
        if ((vcom_connected() != 0) && (prompt == 0))
        {
            vcom_write((uint8_t *)"Hello World!!\r\n", 15);
            prompt = 1;
        }
        /* If VCOM port is opened echo whatever we receive back to host. */
        if (prompt)
        {
            if (NULL == bufferCurrent)
            {
                bufferCurrent = GetNode(&bufferPoolList);
            }
            if (bufferCurrent)
            {
                bufferCurrent->length = vcom_bread(&bufferCurrent->buffer[0], 64);
                if (bufferCurrent->length)
                {
                    AddNodeToEnd(&bufferDataList, bufferCurrent);
                    bufferCurrent = NULL;
                }
                __disable_irq();
                if (bufferDataList)
                {
                    vcom_write(&bufferDataList->buffer[0], bufferDataList->length);
                }
                __enable_irq();
            }
        }
    }
}
