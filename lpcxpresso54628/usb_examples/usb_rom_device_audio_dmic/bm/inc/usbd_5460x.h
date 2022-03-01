/*
 * @brief LPC5460x USB device register block
 *
 * @note
 * Copyright  2012, NXP
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

#ifndef __USBD_5460X_H_
#define __USBD_5460X_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup USBD_5460X CHIP: LPC5460x USB Device driver
 * @ingroup CHIP_5460X_Drivers
 * @{
 */

/**
 * @brief USB device register block structure
 */
typedef volatile struct
{                                   /* USB Structure          */
    volatile uint32_t DEVCMDSTAT;   /* USB Device Command/Status register */
    volatile uint32_t INFO;         /* USB Info register      */
    volatile uint32_t EPLISTSTART;  /* USB EP Command/Status List start address */
    volatile uint32_t DATABUFSTART; /* USB Data buffer start address */
    volatile uint32_t LPM;          /* Link Power Management register */
    volatile uint32_t EPSKIP;       /* USB Endpoint skip      */
    volatile uint32_t EPINUSE;      /* USB Endpoint Buffer in use */
    volatile uint32_t EPBUFCFG;     /* USB Endpoint Buffer Configuration register */
    volatile uint32_t INTSTAT;      /* USB interrupt status register */
    volatile uint32_t INTEN;        /* USB interrupt enable register */
    volatile uint32_t INTSETSTAT;   /* USB set interrupt status register */
    volatile uint32_t INTROUTING;   /* USB interrupt routing register */
    volatile uint32_t RESERVED0[1]; /* HW Module Configuration information */
    volatile uint32_t EPTOGGLE;     /* USB Endpoint toggle register */
} USB_REGS_T;

typedef USB_REGS_T LPC_USB_T;

#define EP_ZERO_BUF_MAX_BYTES (64 * 2) /* EP 0 needs 2 buffers aligned at 64 byte boundary */
                                       /* IN and OUT share single buffer */

#define BUF_ACTIVE  (0x1U << 31)
#define EP_DISABLED (0x1 << 30)
#define EP_STALL    (0x1 << 29)
#define EP_RESET    (0x1 << 28)
#define EP_RF_TV    (0x1 << 27)
#define EP_ISO_TYPE (0x1 << 26)

/* Offset is 16 on IP3511, Offset is 11 on IP3511 HS. Need to handle larger EP buffer on high speed. */
#define FS_EP_NBYTE_OFFSET 16
#define HS_EP_NBYTE_OFFSET 11

/* USB Device Command Status */
#define USB_EN                  (0x1 << 7)  /* Device Enable */
#define USB_SETUP_RCVD          (0x1 << 8)  /* SETUP token received */
#define USB_PLL_ON              (0x1 << 9)  /* PLL is always ON */
#define USB_FORCE_VBUS          (0x1 << 10) /* Force VBUS */
#define USB_LPM                 (0x1 << 11) /* LPM is supported */
#define USB_IntOnNAK_AO         (0x1 << 12) /* Device Interrupt on NAK BULK OUT */
#define USB_IntOnNAK_AI         (0x1 << 13) /* Device Interrupt on NAK BULK IN */
#define USB_IntOnNAK_CO         (0x1 << 14) /* Device Interrupt on NAK CTRL OUT */
#define USB_IntOnNAK_CI         (0x1 << 15) /* Device Interrupt on NAK CTRL IN */
#define USB_DCON                (0x1 << 16) /* Device connect */
#define USB_DSUS                (0x1 << 17) /* Device Suspend */
#define USB_LPM_SUS             (0x1 << 19) /* LPM suspend */
#define USB_REMOTE_WAKE         (0x1 << 20) /* LPM Remote Wakeup */
#define USB_CMD_STAT_SPEED_FULL (0x1 << 22) /* Full speed */
#define USB_CMD_STAT_SPEED_HIGH (0x2 << 22) /* high speed */
#define USB_DCON_C              (0x1 << 24) /* Device connection change */
#define USB_DSUS_C              (0x1 << 25) /* Device SUSPEND change */
#define USB_DRESET_C            (0x1 << 26) /* Device RESET */
#define USB_OTG_C               (0x1 << 27) /* Device OTG status change */
#define USB_VBUS_DBOUNCE        (0x1 << 28) /* Device VBUS detect */

#define USB_TEST_MODE_DIS       (0x0 << 29) /* PHY Test Mode */
#define USB_TEST_MODE_J         (0x1 << 29) /* PHY Test Mode */
#define USB_TEST_MODE_K         (0x2 << 29) /* PHY Test Mode */
#define USB_TEST_MODE_SE0_NAK   (0x3 << 29) /* PHY Test Mode */
#define USB_TEST_MODE_PACKET    (0x4 << 29) /* PHY Test Mode */
#define USB_TEST_MODE_FORCE_ENA (0x5 << 29) /* PHY Test Mode */

/* Device Interrupt Bit Definitions. Excluding control EP 0 IN and OUT, on Aruba IP3511 (USB0 FS),
   there are 8 physical EPs, on IP3511 HS (USB1 FS), there are 10 physical EPs. */
/* Per review comments from Durgesh, EPs are enabled/disabled via EP List buffer, all the
   interrupts will be enabled, but will never be fired if EP buffer is not enabled. */
#define MAX_PHY_EP_INTS (0xFFFF)

#define NZ_EP_OUT_MASK (0x555555554)
#define NZ_EP_IN_MASK  (0xAAAAAAAA8)
#define FRAME_INT      (0x1 << 30)
#define DEV_STAT_INT   (0x80000000)

/* Rx & Tx Packet Length Definitions */
/* IP3511 length mask is 10 bits, IP3511 HS length mask is 15 bits. */
#define FS_PKT_LNGTH_MASK 0x000003FF
#define HS_PKT_LNGTH_MASK 0x00007FFF

/* IP3511 offset mask is 16 bits, IP3511 HS offset mask is 11 bits. */
#define FS_ADDR_OFFSET_MASK 0x0000FFFF
#define HS_ADDR_OFFSET_MASK 0x000007FF

/* Error Status Register Definitions */
#define ERR_NOERROR      0x00
#define ERR_PID_ENCODE   0x01
#define ERR_UNKNOWN_PID  0x02
#define ERR_UNEXPECT_PKT 0x03
#define ERR_TCRC         0x04
#define ERR_DCRC         0x05
#define ERR_TOUT         0x06
#define ERR_BABBIE       0x07
#define ERR_EOF_PKT      0x08
#define ERR_TX_RX_NAK    0x09
#define ERR_SENT_STALL   0x0A
#define ERR_BUF_OVERRUN  0x0B
#define ERR_SENT_EPT_PKT 0x0C
#define ERR_BIT_STUFF    0x0D
#define ERR_SYNC         0x0E
#define ERR_TOGGLE_BIT   0x0F

#ifdef __cplusplus
}
#endif

#endif /* __USBD_5460X_H_ */
