#ifndef _RTE_DEVICE_H_
#define _RTE_DEVICE_H_

#include "pin_mux.h"

#include "board.h"
/*----- PORT 0 Configuration */
// Enable/Disable GPIO_PORTx API interface
#define RTE_GPIO_PORT0 1
// Size of Map for this GPIO interface.  The map provide GPIO Pin,  Pint Pin, optional Pinmux Init/Deinit function
#define RTE_GPIO_PORT0_SIZE_OF_MAP 1
// Maps of GPIO pins
#define RTE_GPIO_PORT0_MAPS                                        \
    {                                                              \
        {                                                          \
            BOARD_SW2_GPIO_PIN, kPINT_PinInt1, BOARD_InitSW2, NULL \
        }                                                          \
    }
// Max number of interrupt contenxt like callback can be saved in RAM. Set 2 means 2 GPIO pin may work together required
// interrupt callback.
#define RTE_GPIO_PORT0_MAX_INTERRUPT_CONTEXTS 1

/*----- PORT 1 Configuration */
#define RTE_GPIO_PORT3             1
#define RTE_GPIO_PORT3_SIZE_OF_MAP 1
#define RTE_GPIO_PORT3_MAPS                                         \
    {                                                               \
        {BOARD_LED1_GPIO_PIN, kPINT_PinInt0, BOARD_InitLED1, NULL}, \
    }
#define RTE_GPIO_PORT3_MAX_INTERRUPT_CONTEXTS 1

#endif /* _RTE_DEVICE_H_ */
