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
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the flash.bin to the target board.
    Reference 'Getting Started with MCUXpresso SDK for EVK-MIMX8ULP and EVK9-MIMX8ULP.pdf' to make and download flash.bin.
5.  Press the reset button on your board.

Running the demo
================
When the demo runs successfully, will get the similar messages on the terminal.

~~~~~~~~~~~~~~~~~~~~~~
TRDC example start
Set the MRC selected memory region not accessiable
Violent access at address: 0x40000000
The MRC selected region is accessiable now
Set the MBC selected memory block not accessiable
Violent access at address: 0x28800000
The MBC selected block is accessiable now
TRDC example Success
~~~~~~~~~~~~~~~~~~~~~~
