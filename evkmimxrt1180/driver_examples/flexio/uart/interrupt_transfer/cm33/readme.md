Overview
========
The flexio_uart_interrupt example shows how to use flexio uart driver in interrupt way:

In this example, a flexio simulated uart connect to PC through USB-Serial, the board will send back all characters
that PC send to the board. Note: two queued transfer in this example, so please input even number characters.

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
- MIMXRT1180-EVK board
- Personal Computer
- USB to Com Converter

Board settings
==============
Connect pin:
- RX of USB2COM to J41-2
- TX of USB2COM to J41-4
- GND of USB2COM to J44-14

Prepare the Demo
================
1.  Connect the USB2COM Converter to the host PC 
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

~~~~~~~~~~~~~~~~~~~~~
Flexio uart interrupt example
Board receives 8 characters then sends them out
Now please input:
~~~~~~~~~~~~~~~~~~~~~
