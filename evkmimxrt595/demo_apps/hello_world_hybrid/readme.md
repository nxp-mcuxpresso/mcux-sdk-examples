Overview
========
The Hello World hybrid demo application provides a sanity check for the new SDK build environments and board bring up. The Hello
World demo prints the "hello world." "Run in flash." strings to the terminal using the SDK UART drivers. The purpose of this demo is
to show how to use the UART, and to provide a simple project for debugging and further development. 
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.
!!!Note!!! 
When debugging this demo using IAR, need force the IAR IDE to use hardware breakpoints as
the software breakpoints will be overwritten during the startup code relocates code from flash to RAM.
The steps to force the IAR IDE to use hardware breakpoints: Change the "Default breakpoint type" setting to 
"Hardware" under the tab of "Options" -> "Debugger"-> "CMSIS DAP" or "J-Link/J-Trace" basing on the debugger being used.

Running the demo
================
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
hello world.
Run in flash.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
