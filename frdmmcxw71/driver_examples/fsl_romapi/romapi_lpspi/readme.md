Overview
========

The romapi_lpspi example shows how to use ROMAPI to operate program SPI flash:

The project is a simple demonstration program of the SDK FLASIAP driver. It erases and programs
a portion of on-board SPI flash memory. A message a printed on the UART terminal as various
operations on SPI flash memory are performed.


SDK version
===========
- Version: 2.16.000

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
ROM API LPSPI Example Start

Initializing LPSPI flash

LPSPI flash Information:
LPSPI flash base address: xx
LPSPI flash page size: xx
LPSPI flash Total Size: xx

Erase a page of LPSPI flash
Successfully erased page xx -> xx

Program a buffer to a page of LPSPI flash:

Successfully programmed and verified location xx -> xx
~~~~~~~~~~~~~~~~~~~~~~~
And you will find the flash has been programed.
