Overview
========
The xspi_psram_edma_transfer example shows how to use xspi driver with edma:

In this example, xspi will send data and operate the external PSRAM connected with XSPI. Some simple flash command will
be executed, such as Read Data and Write Data.
Example will write/read through the whole chip, using EDMA transfer way.

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
- RT700 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
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
~~~~~~~~~~~~~~~~~~~~~
XSPI edma example started!
EDMA Command Read/Write data successfully at all address range !
~~~~~~~~~~~~~~~~~~~~~

