Overview
========
The GPIO input interrupt example is a demonstration program that uses the KSDK software to manipulate the general-purpose
GPIO input interrupt.
The example is supported to set with selection of polarity and edge vs level triggering and all GPIO pins can optionally contribute to one of two GPIO interrupts.

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
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
No special settings are required.

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
When the example runs successfully, the following message is displayed in the terminal:
If you turn on the SW2 , and "SW2 is turned on" is shown on the terminal window

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 GPIO Driver example.

 SW2 is turned on.
 ......
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
