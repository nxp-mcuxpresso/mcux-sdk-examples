Overview
========
The trdc example shows how to control the memory block checker(MBC) and memory region checker(MRC) access
policy using TRDC.

In this example, a MRC memory region and a MBC memory block are set to unaccessible, then
the hardfault occurs.

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
- MCX-N5XX-EVK Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, will get the similar messages on the terminal.

~~~~~~~~~~~~~~~~~~~~~~
TRDC example start
Set the MBC selected memory block not accessiable
Resolve access error
The MBC selected block is accessiable now
TRDC example Success
~~~~~~~~~~~~~~~~~~~~~~
