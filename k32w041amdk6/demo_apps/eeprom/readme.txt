Overview
========
The EEPROM demo application demonstrates the use of the driver to erase, write
and read data from onchip EEPROM device.

Toolchain supported
===================
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Mini USB cable
- OM15076-3 Carrier Board
- JN518x Module plugged on the Carrier Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the USB port (J15) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EEPROM Demo

This demo will write data into the EEPROM and then read it back.

256 bytes verified successfully via API EEPROM read.

End of the demo, EEPROM is placed into deep power down mode.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Customization options
=====================

