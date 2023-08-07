Overview
========
The DMA interleave example is a simple demonstration program that uses the SDK software.
It executes linked(address interleave) transfer from the source buffer to destination buffer using the SDK DMA drivers.
The destination buffer is 8 words length:
Descriptor A will define a transfer for member 0, 2, 4, 6
Descriptor B will define a transfer  for member 1, 3, 5, 7.
Descriptor A is linked to descriptor B.
The purpose of this example is to show how to use the DMA and to provide a simple example for
debugging and further development.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S28 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
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
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~
DMA interleave transfer example begin.

Destination Buffer:

0   0   0   0   0   0   0   0

DMA interleave transfer example finish.

Destination Buffer:

1   11  2   22  3   33  4   44
~~~~~~~~~~~~~~~~~~~~~
