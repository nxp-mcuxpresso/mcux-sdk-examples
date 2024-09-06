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
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer
- USB to Com Converter

Board settings
==============
1. Remove Jumper JP22.
2. Connect pin:
- RX of USB2COM to JP13-4
- TX of USB2COM to JP13-5
- GND of USB2COM to JP13-1

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1. Connect a micro USB cable between the PC host and the DEBUG PORT on the board.
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
When the demo runs successfully, the log would be seen on the MCU-LINK terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Lpuart polling example with seven data bits
Board will send back received characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
