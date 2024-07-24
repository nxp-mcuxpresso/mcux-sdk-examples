Overview
========
The flexio_uart_edma example shows how to use flexio uart driver in edma way:

In this example, a flexio simulated uart connect to PC through USB-Serial, the board will send back all characters
that PC send to the board.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- KW45B41Z-EVK Board
- USB to Serial Converter
- Personal Computer

Board settings
==============
The flexio_uart_polling example is requires connecting the FLEXIO pins with the USB2COM pins
The connection should be set as following:
- J13-4(PTA17), RX of USB2COM connected
- J13-3(PTA16), TX of USB2COM connected
- J13-8, Ground of USB2COM connected

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the UART Terminal port which connected to the USB2COM like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Flexio uart edma example
Board receives 8 characters then sends them out
Now please input:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
