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
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer
- Adafruit TFT LCD shield w/Cap Touch

Board settings
==============
There are different versions of Adafruit 2.8" TFT LCD shields. The shields marked
v2.0 works directly with this project. For the other shields, please solder
the center pin of IOREF pads to the 3.3V pad, and solder the pads 11, 12, and 13.
See the link for details:
https://community.nxp.com/t5/MCUXpresso-Community-Articles/Modifying-the-latest-Adafruit-2-8-quot-LCD-for-SDK-graphics/ba-p/1131104

- Attach the LCD shield to the board Arduino header.
- Connect jumper JP50.
- Mount 0R on R242/R239/R236/R233
- Mount 0R on R125/R123/R12/R124 and Remove R9/R11/R13/R20/R21
- Short J5 1-2.
- Remove jumper on JP30.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
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
