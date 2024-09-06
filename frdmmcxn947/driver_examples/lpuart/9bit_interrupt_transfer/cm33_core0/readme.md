Overview
========
The lpuart_9bit_interrupt_transfer example shows how to use lpuart driver in 9-bit mode in multi-slave system.
Master can send data to slave with certain address specifically, and slave can only receive data when it is addressed.

In this example, one lpuart instance is used with address configured. Its TX and RX pins are connected together.
First it sends a piece of data out, then addresses itself, after that sends the other piece of data. Only data
sent after the address can be received by itself.

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
LPUART is configured with address, only data sent to itself after matched address can be received
LPUART will send first piece of data out:

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  

LPUART will address itself
LPUART will send the other piece of data out:

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

LPUART received data:

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

All data matches!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
