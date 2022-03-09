
#ifndef _BOARD_SETUP_H_
#define _BOARD_SETUP_H_

#include "ux_api.h"

void board_setup(void);

void usb_host_hw_setup(void);

UINT usbx_host_hcd_register(VOID);

VOID usbx_mem_init(VOID);

#endif /* _BOARD_SETUP_H_ */
