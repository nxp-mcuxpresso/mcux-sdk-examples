Overview
========
This example shows how to use the LCDIC DMA driver APIs. In this example, the lcdic
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
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer
- LCD-PAR-S035 panel

Board settings
==============
- Short JP3, JP44, JP50
- Wire R624, R625, R626, R627, R629, R630, R631, R616, R662, R663, R664, R665
- Connect LCD-PAR-S035 to header J23, J23-1 match LCD J1-1
- Set the switch SW1 on LCD-PAR-S035 to 011 (IM[2:0), note the left one is IM[0], right one is IM[2].

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
