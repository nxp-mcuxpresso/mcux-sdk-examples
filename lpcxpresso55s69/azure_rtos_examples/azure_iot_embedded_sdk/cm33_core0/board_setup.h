
#ifndef _BOARD_SETUP_H_
#define _BOARD_SETUP_H_

#include "ux_api.h"
#include "ux_hcd_ip3516.h"

#define UX_HCD_NAME         "IP3516HS"

/* define the initialization function of the HCD driver */
#define UX_HCD_INIT_FUNC    _ux_hcd_ip3516_initialize

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void board_setup(void);

void usb_host_setup(void);
void usb_host_interrupt_setup(void);
ULONG usb_host_base(void);

#endif /* _BOARD_SETUP_H_ */
