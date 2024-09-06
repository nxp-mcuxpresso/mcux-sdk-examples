Overview
========
The TDET Example project is a demonstration program that uses the MCUX SDK software to set up Digital Tamper (TDET) peripherial.
Then tests the expected behaviour.


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
- MCX-N5XX-EVK Board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
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
When the demo runs successfully, the terminal displays similar information like the following:
~~~~~~~~~~~~~~~~~~

TDET Peripheral Driver Example

Tampering detected Tamper Detect Flag is set

Passive tamper example
No tampering detected on PIN0

Active tamper example
No tampering detected on active tamper

End of example

~~~~~~~~~~~~~~~~~~
