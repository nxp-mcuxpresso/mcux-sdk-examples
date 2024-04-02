Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_uart_interrupt_transfer example shows how to use uart cmsis driver in interrupt way:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1. Connect a USB cable between the host PC and the EVK board J14.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Lpuart CMSIS interrupt example
Board receives 8 characters then sends them out
Now please input:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you input 8 characters, system will echo it by LPUART and them would be seen on the OpenSDA terminal.
