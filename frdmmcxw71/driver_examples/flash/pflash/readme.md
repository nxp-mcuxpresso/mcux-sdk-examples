Overview
========
The pflash example shows how to use flash driver to operate program flash:



SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============
This Example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1. Connect a USB cable between the host PC and the FRDM board J10.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~
PFlash Example Start

 PFlash Information:
 Program Flash block bass address:      0, Hex: (0x0)
 Program Flash block Size:              1024 KB, Hex: (0x100000)
 Program Flash block Sector Size:       8 KB, Hex: (0x2000)
 Program Flash block count Size:        1
 Total Program Flash Size:              1024 KB, Hex: (0x100000)

 Erase a sector of flash
 Successfully Erased Sector 0xfe000 -> 0x100000

 Program a buffer to a phrase of flash
 Successfully Programmed and Verified Location 0xfe000 -> 0xfe010

 End of PFlash Example
~~~~~~~~~~~~~~~~~~~~~~~
And you will find the flash hase been programed.
