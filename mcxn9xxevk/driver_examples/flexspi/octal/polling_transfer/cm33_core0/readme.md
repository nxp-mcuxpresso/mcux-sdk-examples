Overview
========
The flexspi_octal_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external octal flash connected with FLEXSPI. Some simple flash command will
be executed, such as Write Enable, Erase sector, Program page.
Example will first erase the sector and program a page into the flash, at last check if the data in flash is correct.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Micro USB cable
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
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
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FLEXSPI example started!
Vendor ID: 0xef
Erasing Serial NOR over FlexSPI...
Erase data - successfully.
Program data - successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
This example support two flash type: Winbond W25Q64 and MT MT35XU512
1. Define EXAMPLE_FLASH_TYPE=FLASH_W25Q64 for flash type W25Q64
2. Define EXAMPLE_FLASH_TYPE=FLASH_MT35XU512 for flash type MT35XU512
