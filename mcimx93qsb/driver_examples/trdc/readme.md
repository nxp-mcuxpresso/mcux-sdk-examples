Overview
========
The trdc example shows how to control the memory block checker(MBC) and memory region checker(MRC) access
policy using TRDC.

In this example, a MRC memory region and a MBC memory block are set to unaccessible, then
the hardfault occurs.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-QSB board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
No special settings are required.

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board to begin running the example.

Note: This example has to be run on CORTEX-M boot mode(SW1301:[10x1]).

Running the demo
================
When the demo runs successfully, will get the similar messages on the terminal.

~~~~~~~~~~~~~~~~~~~~~~
TRDC example start
Set the MRC selected memory region not accessiable
Violent access at address:  0x     100
The MRC selected region is accessiable now
Set the MBC selected memory block not accessiable
Violent access at address: 0x47400000
The MBC selected block is accessiable now
TRDC example Success
~~~~~~~~~~~~~~~~~~~~~~
