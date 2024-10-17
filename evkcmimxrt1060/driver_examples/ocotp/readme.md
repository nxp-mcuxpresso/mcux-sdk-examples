Overview
========

The OCOTP example project is a demonstration program that uses the KSDK software to access eFuse map.
Consider of the feature of One-Time programable, this example will just print the version of OCOTP controller.

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
- MIMXRT1060-EVKC board
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
The log below shows the output of the ocotp example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OCOTP Peripheral Driver Example

OCOTP controller version: 0x06000000

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
To download binary into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "ocotp_example.bin".
3. Set the SW7: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J33.
4. Drop the binaray into disk "RT1060-EVK" on PC.
5. Wait for the disk disappear and appear again which will take couple of seconds.
7. Reset the board by pressing SW7 or power off and on the board. 
