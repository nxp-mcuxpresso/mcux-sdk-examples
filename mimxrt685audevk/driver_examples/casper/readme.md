Overview
========
The CASPER Example project is a demonstration program that uses the KSDK software implement modular exponentiation algorithm using the CASPER software driver.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
