Overview
========
The CASPER Example project is a demonstration program that uses the KSDK software implement modular exponentiation algorithm using the CASPER software driver.


Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

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
The log below shows the output of the Casper example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ModExp Test pass.
Casper ECC Demo P256

Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC scalar multiplication tests were succesfull.

Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC double scalar multiplication tests were succesfull.

Casper ECC Demo P384

Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC scalar multiplication tests were succesfull.
Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC double scalar multiplication tests were succesfull.

Casper ECC Demo P521

Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC scalar multiplication tests were succesfull.
Round: 0
Round: 1
Round: 2
Round: 3
Round: 4
Round: 5
Round: 6
Round: 7
All EC double scalar multiplication tests were succesfull.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
