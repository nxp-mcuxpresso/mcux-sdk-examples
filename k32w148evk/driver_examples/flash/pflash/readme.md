Overview
========
The pflash example shows how to use flash driver to operate program flash:



SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============
This Example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1. Connect a USB cable between the host PC and the EVK board J14.
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

Program Flash block bass address:\t%d KB, Hex: (0x%x)

Program Flash block Size:\t%d KB, Hex: (0x%x)

Program Flash block Sector Size:\t%d KB, Hex: (0x%x)

Program Flash block count Size:\t%d

Total Program Flash Size:\t%d KB, Hex: (0x%x)

Successfully Erased Sector 0x%x -> 0x%x

Program a buffer to a sector of flash:

Successfully Programmed and Verified Location 0x%x -> 0x%x
~~~~~~~~~~~~~~~~~~~~~~~
And you will find the flash hase been programed.
