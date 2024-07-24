Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_uart_edma_transfer example shows how to use uart cmsis  driver with EDMA:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-EVK board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
No need special setting.


Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4. Boot Linux BSP to u-boot, and load M core image from SD card to run. (Put the image into SD card before.)
   => load mmc 1:1 0x80000000 /sdk20-app.bin
   => cp.b 0x80000000 0x201e0000 0x20000
   => bootaux 0x1ffe0000 0

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
~~~~~~~~~~~~~~~~~~~~~
USART EDMA example
Send back received data
Echo every 8 characters
~~~~~~~~~~~~~~~~~~~~~
