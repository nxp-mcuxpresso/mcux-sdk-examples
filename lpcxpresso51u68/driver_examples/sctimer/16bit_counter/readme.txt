Overview
========
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters.
The example toggles an output per counter when a match occurs.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso51U68 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J6) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the SCTimer 16-bit counter demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to use it in 16-bit mode

The example shows both 16-bit counters running and toggling an output periodically
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
