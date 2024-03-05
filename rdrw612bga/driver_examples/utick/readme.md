Overview
========
The utick project is a simple demonstration program of the SDK utick driver. It sets up the utick
hardware block to trigger a periodic interrupt after every 1 second. When the utick interrupt is triggered
a message a printed on the UART terminal.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the utick example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Utick example start...
UTICK delay 1 second...
UTICK delay 1 second...
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
