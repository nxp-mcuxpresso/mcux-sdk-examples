Overview
========
This example uses smartDMA to get image data from camera interface frame by frame captured by OV7670,
and uses FlexIO LCD interface to display the captured image on st7796s low cost panel

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

Board settings
==============
Attach OV7670 to J9 camera interface(pin5 to pin22).
Attach LCD-PAR-S035 low cost panel to J8.
Attach a usb to ttl converter to the PC, its RXD attached to J2-8,
and TXD attached to J2-12.

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J10) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
