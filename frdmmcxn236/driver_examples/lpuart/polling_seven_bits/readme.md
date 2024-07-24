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
- FRDM-MCXN236 board
- Personal Computer
- USB to TTL converter

Board settings
==============
Connect the USB to TTL converter to FRDM-MCXN236 board.
MCX-N9XX-EVK UART pins:
- TX of USB2COM to J8-4(P4_0)
- RX of USB2COM to J8-3(P4_1)

Prepare the Demo
================
1. Connect a type-c USB cable between the PC host and the MCU-Link USB port (J10) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 7 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the MCU-Link terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Lpuart polling example with seven data bits
Board will send back received characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
