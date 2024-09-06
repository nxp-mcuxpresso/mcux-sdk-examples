Overview
========
The lpuart_9bit_transfer example shows how to use lpuart driver in 9-bit mode.

In this example, one lpuart instance is used with 9bit mode. Its TX and RX pins are connected together.
The data with 9bit can be received by itself. The each data in TX and RX data register will be 16-bit data.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-MCXN947 board
- Personal Computer
- USB to TTL converter

Board settings
==============
Connect the USB to TTL converter to FRDM-MCXN947 board.
FRDM-MCXN947 UART pins:
TX pin is connected with RX pin.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPUART0    CONNECTS TO        LPUART0
Pin Name   Board Location     Pin Name  Board Location
TXD        J8-9(P0_12)        RXD       J8-12(P0_13)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1. Connect a micro USB cable between the PC host and the MCU-Link USB port (J17) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the MCU-Link terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPUART 9-bit mode example begins
LPUART is configured with 9bit
LPUART will send data out:


0x100  0x101  0x102  0x103  0x104  0x105  0x106  0x107  
0x108  0x109  0x10a  0x10b  0x10c  0x10d  0x10e  0x10f  

LPUART received data:


0x100  0x101  0x102  0x103  0x104  0x105  0x106  0x107  
0x108  0x109  0x10a  0x10b  0x10c  0x10d  0x10e  0x10f  

All data matches!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
