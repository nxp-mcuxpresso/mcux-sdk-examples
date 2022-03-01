Overview
========
nand flash demo shows the use of nand flash component to erase, program, and read an
external nand flash device.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- EVB-MIMXRT1170 board
- Personal Computer

Board settings
==============
To make the examples works, please add resistors:
R1872,R1873,R1874,R1875,R1876,R1877,R1878,R1879

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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
***NAND Flash Component Demo Start!***

***NAND Flash Initialization Start!***

***NAND Flash Initialization Success!***

***NAND Flash Erase The First Block Start!***

***NAND Flash Erase Check Start!***

***NAND Flash Erase block Success!***

***NAND Flash Page Program Start!***

***NAND Flash Page Read Start!***
.....
.....
.....

***NAND Flash Page Read/Write Success!***

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
