Overview
========
The LPTMR project is a simple demonstration program of the SDK LPTMR driver. It sets up the LPTMR
hardware block to trigger a periodic interrupt after every 1 second. When the LPTMR interrupt is triggered
a message a printed on the UART terminal and an LED is toggled on the board.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1180-EVK board
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
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~
Low Power Timer Example
LPTMR interrupt No.1
LPTMR interrupt No.2
LPTMR interrupt No.3
....................
....................
~~~~~~~~~~~~~~~~~~~~~~~
