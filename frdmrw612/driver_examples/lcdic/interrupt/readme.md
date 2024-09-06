Overview
========
This example shows how to use the LCDIC interrupt driver APIs. In this example, the lcdic
driver is used to send constant color and prefilled color array to panel.
If the project succeed, the panel will show like this

 +----------------------+------------------------+
 |                      |                        |
 |     RED              |      BLUE              |
 |                      |                        |
 |                      |                        |
 |                      |                        |
 +----------------------+------------------------+
 |                      |                        |
 |     GREEN            |      WHITE             |
 |                      |                        |
 |                      |                        |
 +-----------------------------------------------+

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
- LCD-PAR-S035 panel

Board settings
==============
- Set the switch SW1 on LCD-PAR-S035 to 111.
- Attach the LCD to PMOD(J7).

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
If this example runs correctly, the panel shows as the overview description.
