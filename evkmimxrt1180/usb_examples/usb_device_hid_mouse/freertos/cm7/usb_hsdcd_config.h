/*
 * Copyright 2022 - 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _USB_DEVICE_HSDCD_CONFIG_H_
#define _USB_DEVICE_HSDCD_CONFIG_H_
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*${macro:start}*/
/* USB DCD config*/

/*! @brief The clock speed, Numerical Value of Clock Speed in Binary. */
/*The valid clock range is from 1 to 1023 when clock unit is MHz and 4 to 1023 when clock unit is kHz.this value depend
 * on board desgin, different board may has different value*/
#define USB_HSDCD_CLOCK_SPEED (24000000U)

/*${macro:end}*/

#endif /* _USB_DEVICE_HSDCD_CONFIG_H_ */
