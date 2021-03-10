Overview
========
The SPIFI Polling Example project is a demonstration program that uses the KSDK software to program external serial
flash using polling and read through AHB bus.


Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
The example change to FLASH_W25Q as the default flash in app.h, user can change the flash type according to the flash type on the board. Support flash list as below:
FLASH_W25Q
FLASH_MX25R
FLASH_MT25Q.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
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
The log below shows example output of the eeprom example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPIFI flash polling example started
All data written is correct!
SPIFI Polling example Finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
