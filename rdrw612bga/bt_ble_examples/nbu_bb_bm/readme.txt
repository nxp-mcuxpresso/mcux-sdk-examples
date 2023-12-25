Overview
========
what is nbu_bb?

nbu_bb is a hci blackbox tool based on bare-metal or freertos.

it can be used to send hci command by a serial port utililty or MarvellHCITool on a Computer.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
Load JP15 and JP44

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
2.  Download the program to the target board.

Running the demo
================
1.  Connect a micro USB cable between the PC host and the USB-UART port (J21) on the back of board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Send the HCI Command with MarvellHCITool or a serial terminal, the hci event can be received.

Customization options
=====================
For RW610/612 board, if want to use 3000000 UART baud rate, the following code needs to be modified:
In file: boards/rdrw612bga/board.h
Origin:
    (&(const clock_frg_clk_config_t){3, kCLOCK_FrgPllDiv, 255, 0}) /*!< Select FRG3 mux as frg_pll */
Change to:
    (&(const clock_frg_clk_config_t){3, kCLOCK_FrgMainClk, 255, 0}) /*!< Select FRG3 mux as frg_pll */

Origin:
#define BOARD_DEBUG_UART_BAUDRATE 115200
Change to:
#define BOARD_DEBUG_UART_BAUDRATE 3000000
