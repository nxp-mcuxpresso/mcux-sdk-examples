#ifndef _RTE_DEVICE_H_
#define _RTE_DEVICE_H_

#include "pin_mux.h"

#include "board.h"
/*----- PORT 1 Configuration */
#define RTE_GPIO_PORT1             1
#define RTE_GPIO_PORT1_SIZE_OF_MAP 2
#define RTE_GPIO_PORT1_MAPS                                                \
    {                                                                      \
        {BOARD_LED_BLUE_GPIO_PIN, kPINT_PinInt0, BOARD_InitLEDBlue, NULL}, \
            {BOARD_SW2_GPIO_PIN, kPINT_PinInt1, BOARD_InitSW2, NULL},      \
    }
#define RTE_GPIO_PORT1_MAX_INTERRUPT_CONTEXTS 1

#endif /* _RTE_DEVICE_H_ */
