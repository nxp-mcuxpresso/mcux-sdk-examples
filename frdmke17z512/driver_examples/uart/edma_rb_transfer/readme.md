Overview
========
The uart_edma ring buffer example shows how to use uart driver with EDMA:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board. And data from PC will be firstly 
stored to a ring buffer using EDMA methord, and than application will read the ring buffer 
in a certain time, such as 10ms. While data has already be stored in the ring buffer,
routine will send them back.

And the ring buffer is achieved based on the DMA's source address modulo feature, the base address
should be aligned to a 0-modulo-boundary.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE17Z512 board
- Personal Computer
- USB to Com converter

Board settings
==============
J4-10 connect to TxD of the USB to Com converter.
J4-12 connect to RxD of the USB to Com converter.
J3-14 connect to GND of the USB to Com converter 

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board.
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
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~
UART EDMA example
Send back received data
Echo every 8 characters

~~~~~~~~~~~~~~~~~~~~~~~

