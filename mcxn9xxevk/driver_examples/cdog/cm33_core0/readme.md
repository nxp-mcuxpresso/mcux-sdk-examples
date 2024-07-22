Overview
========
The CWT Example project is a demonstration program that uses the KSDK software to set up secure counter and instruction timer.
Then tests several times the expected value with value in secure counter. After that miscompare fault is intentionally generated
by comparing secure counter with wrong value. At the end application let the instruction timer reach zero and generate another timeout fault.


SDK version
===========
- Version: 2.16.000

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

CDOG Peripheral Driver Example

CDOG IRQ Reached
* Miscompare fault occured *

intruction timer:   fffc5
intruction timer:   c615e
intruction timer:   8c34e
intruction timer:   5251a
intruction timer:   18703
CDOG IRQ Reached
* Timeout fault occured *

End of example

Note:
To keep the program running correctly, it is recommended to perform a power on reset (POR; SW2) after loading the application.
SW reset does not clear pending fault flags.
