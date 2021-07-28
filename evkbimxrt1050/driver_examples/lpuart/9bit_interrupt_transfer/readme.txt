Overview
========
The lpuart_9bit_interrupt_transfer example shows how to use lpuart driver in 9-bit mode in multi-slave system.
Master can send data to slave with certain address specifically, and slave can only receive data when it is addressed.

In this example, one lpuart instance is used with address configured. Its TX and RX pins are connected together.
First it sends a piece of data out, then addresses itself, after that sends the other piece of data. Only data
sent after the address can be received by itself.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer

Board settings
==============
LPUART one board:
Using instance 2 of LPUART interface to transfer data to itself.
TX pin is connected with RX pin.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPUART2    CONNECTS TO       	LPUART2
Pin Name   Board Location     	Pin Name  Board Location
TXD        J22-7            	RXD       J22-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow controlgit
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

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
