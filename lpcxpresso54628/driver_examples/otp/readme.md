Overview
========

The OTP example project is a demonstration program that uses the KSDK software to access OTP ROM API.
It just prints the version of API at the moment.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Two micro USB cables
- LPCLPCXpresso54628 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the +5V Power only USB port on the target board (J1).
2.  Connect a USB cable between the host PC and the Debug Link USB port on the target board (J8).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the OTP demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OTP Peripheral Driver Example

OTP ROM API driver version: 0x101

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
