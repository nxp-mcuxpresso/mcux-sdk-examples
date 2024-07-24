Overview
========
The EDMA ping pong transfer example is a simple demonstration program that uses the SDK software.
It excuates ping pong transfer from source buffer to destination buffer using the SDK EDMA drivers.
The purpose of this example is to show how to use ping pong buffer by the EDMA and to provide a simple example for
debugging and further development.
The example demostrate the ping pong transfer by the EDMA scatter gather feature.
Please reference user manual for the detail of the feature.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-QSB board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either re-power up your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EDMA ping pong transfer example begin.

Destination Buffer:

0   0   0   0   0   0   0   0

EDMA ping pong transfer example finish.

Destination Buffer:

1   2   3   4   5   6   7   8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

