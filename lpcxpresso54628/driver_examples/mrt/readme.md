Overview
========
The MRT project is a simple demonstration program of the SDK MRT driver. It sets up the MRT
hardware block to trigger a periodic interrupt after every 250 milliseconds. When the PIT interrupt is triggered
a message a printed on the UART terminal and an LED is toggled on the board.

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
- LPCXpresso54628 board
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
The log below shows example output of the MRT driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Starting channel No.0 ...
 Channel No.0 interrupt is occurred !
 Channel No.0 interrupt is occurred !
 Channel No.0 interrupt is occurred !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
