/*
 * @brief Programming API used with HID example
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

#ifndef __HID_GENERIC_H_
#define __HID_GENERIC_H_

#include "app_usbd_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @ingroup EXAMPLES_USBDROM_11U6X_HID_GENERIC
 * @{
 */

/**
 * @brief	Generic HID interface init routine.
 * @param	hUsb		: Handle to USB device stack
 * @param	pIntfDesc	: Pointer to HID interface descriptor
 * @param	mem_base	: Pointer to memory address which can be used by HID driver
 * @param	mem_size	: Size of the memory passed
 * @return	On success returns LPC_OK. Params mem_base and mem_size are updated
 *			to point to new base and available size.
 */
ErrorCode_t usb_hid_init(USBD_HANDLE_T hUsb,
                         USB_INTERFACE_DESCRIPTOR *pIntfDesc,
                         uint32_t *mem_base,
                         uint32_t *mem_size);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __HID_GENERIC_H_ */
