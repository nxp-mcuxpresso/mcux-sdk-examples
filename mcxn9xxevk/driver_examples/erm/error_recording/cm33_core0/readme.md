Overview
========
The ERM project is a simple demonstration program of the SDK ERM driver. It shows how to show error events using the ERM driver.

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
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ERM error recording example.

Original ram data correct.

ERM error recording address is 0x20006000.

ERM error recording example finished successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
All system bus masters can access the programming model:
1. Only in supervisor mode
2. Using only 32-bit (word) accesses
Any of the following attempted references to the programming model generates an error termination:
1. In user mode
2. Using non-32-bit access sizes
3. To undefined (reserved) addresses
