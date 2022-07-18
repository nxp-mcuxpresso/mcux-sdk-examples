Overview
========

The fmc example project is a demonstration program that uses the KSDK software to generate flash signature
for a specified flash address range and prints them to the terminal. This example also implements the software
signature algorithm, compares the hardware signature result and software signature result. This is to verify 
flash contents as suggested by user manual.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

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
1.  Connect a micro USB cable between the host PC and the +5V Power only USB port on the target board (J1).
2.  Connect a micro USB cable between the host PC and the Debug Link USB port on the target board (J8).
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
The log below shows example output of the random number generator demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Generate hardware signature: 0x4cd57faa b3191800 8b1e2d7 447d2e1b
 Generate software signature: 0x4cd57faa b3191800 8b1e2d7 447d2e1b
 Success! Signature consistent !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
