Overview
========
The lpuart_polling_seven_bits Example project is to demonstrate usage of the KSDK lpuart driver with seven data bits feature enabled.
In the example, you can send characters to the console back and they will be printed out onto console
 instantly.
NOTE: Please set com port format to "7 data bits without parity bit" in PC's com port tool

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
- USB to Serial Converter
- Personal Computer

Board settings
==============
The connection should be set as following:
- J1-1(PTA16), TX of USB2COM connected
- J1-2(PTA17), RX of USB2COM connected

Prepare the Demo
================
1. Connect a USB cable between the host PC and the FRDM board J10.
2. Open a serial terminal on PC for USB2COM serial device with these settings:
   - 115200 baud rate
   - 7 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the USB2COM terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Lpuart polling example with seven data bits
Board will send back received characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
