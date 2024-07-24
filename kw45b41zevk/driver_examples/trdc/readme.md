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
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- KW45B41Z-EVK Board
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
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
If the silicon of the SOC is NXP Fab or NXP Provisioned:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TRDC example start
Set the MRC selected memory region not accessiable
Violent access at address: 0x48800000
The MRC selected region is accessiable now
Set the MBC selected memory block not accessiable
Violent access at address: 0x 2000000
The MBC selected block is accessiable now
TRDC example Success
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Otherwise the memory 0x48800000-0x48A00000 controlled by MRC0 cannot be accessed by CM33 core, the log should be:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TRDC example start
Set the MBC selected memory block not accessiable
Violent access at address: 0x 2000000
The MBC selected block is accessiable now
TRDC example Success
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
