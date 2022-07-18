Overview
========

The SHA Example project is a demonstration program that uses the KSDK software to generate SHA checksums.


Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54628 board
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
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
Running the demo
================
The log below shows example output of the SHA driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SHA Peripheral Driver Example

Calculating SHA-256 digest.
Input: Hello SHA-256 world. Hello SHA-256 world. Hello SHA-256 world. Hello SHA-256 world.
Output: E68675E9ACFF8B57E386312E85371E6410BECCE2A249001955C45C98CBE5373E

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
