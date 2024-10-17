Overview
========
The flexspi_octal_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external octal flash connected with FLEXSPI. Some simple flash command will
be executed, such as Write Enable, Erase sector, Program page.
Example will first erase the sector and program a page into the flash, at last check if the data in flash is correct.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
In order to interface the on-board octal FLASH it is necessary to populate with 0R or short circuit following positions:
R293, R296, R297, R298 and set jumper J75 to position 1-2. Jumper J59 should remain in the default 2-3 position.

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
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FLEXSPI example started!
Vendor ID: 0x2c
Erasing Serial NOR over FlexSPI...
Erase data - successfully. 
Program data - successfully. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
