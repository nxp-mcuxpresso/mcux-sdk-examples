Overview
========
The GPIO Example project is a demonstration program that uses the KSDK software to manipulate the general-purpose
outputs. The example use LEDs and buttons to demonstrates GPIO API for port and pin manipulation (init, set,
clear, and toggle).



SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 GPIO Driver example

 The LED is taking turns to shine.

 Standard port read: 2000801

 Masked port read: 2000000

 Port state: 2000001

 Port state: 2000000
 ...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When press sw2 the LED BLUE will toggle.
