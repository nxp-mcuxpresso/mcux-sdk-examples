Overview
========
This example is showing data send and receive via FreeRTOS adapted LPUART driver. Program initially send
string into serial terminal via virtual COM. After that, user may send some custom input and
application will return every 4 characters back to console. If delay from last user input exceed
10s (receive timeout) notification about exceeded timeout appear and application will finish.
Example need only single LPUART instance.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini USB cable
- USB-to-Serial connector
- FRDM-K22F board
- Personal Computer

Board settings
==============
Connect pin:
- RX of USB2COM to J1-4
- TX of USB2COM to J1-2
- GND of USB2COM to J2-14

Prepare the Demo
================
1.  Connect a USB-to-Serial connector between the PC host and the LPUART0 pins on the board.
2.  Open a serial terminal on PC with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
You will see the welcome string printed out on the console.
You can send characters to the console back and they will be printed out onto console in a group of 4 characters.

Customization options
=====================

