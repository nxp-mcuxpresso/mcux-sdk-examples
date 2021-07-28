Overview
========
The DMA linked example is a simple demonstration program that uses the SDK software.
It executes linked transfer(ping pong case) from the source buffer to destination buffer using the SDK DMA drivers.

Two descriptors are used,  A is linked to B, B is linked to A.

A------>B------->
^               |
|               |
---------<-------

The purpose of this example is to show how to use the DMA and to provide a simple example for
debugging and further development.

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018M board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
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
~~~~~~~~~~~~~~~~~~~~~
DMA linked transfer example begin.

Destination Buffer:

0   0   0   0   0   0   0   0

DMA linked transfer example finish.

Destination Buffer:

1   2   3   4   11  22  33  44
~~~~~~~~~~~~~~~~~~~~~

