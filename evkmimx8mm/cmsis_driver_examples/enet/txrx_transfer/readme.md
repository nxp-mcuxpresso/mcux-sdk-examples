Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_enet_transfer example shows how to use CMSIS ENET driver:

In this example, the ENET transmits 20 number broadcast frames and will print the received frame.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8MM6-EVK board
- J-Link Debug Probe
- 12V power supply
- Loopback network cable RJ45 standard
- Network cable RJ45 standard
- Network switch
- Personal Computer

Board settings
==============
J501 connected with network switch through RJ45 network cable.
Two types of base boards(ENET PHY is RTL8211F or AR8031) are supported and the default is for RTL8211F. For base boards with 
ENET PHY AR8031(older version than RevD) need define macro "EXAMPLE_ENET_PHY_PHYAR8031" to 1 in example source code.

#### Please note this application can't support running with Linux BSP! ####
This example aims to show the basic usage of the IP's function, some of the used Pads/Resources are also used by Cortex-A core.


Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Insert loopback network cable into network switch device(support loopback frames) and connect switch to the board's Ethernet RJ45 port.
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.

Make loopback network cable:
      Standard 1         Standard 2
J1    orange+white       green+white
J2    orange             green
J3    green+white        orange+white
J4    blue               brown+white
J5    blue+white         brown
J6    green              orange
J7    brown+white        blue
J8    brown              blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to the sheet above. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

Running the demo
================
When the demo runs, the log would be seen on the terminal like:

ENET example start.
The 1 frame transmitted success!
The 1 frame has been successfully received!
The 2 frame transmitted success!
The 2 frame has been successfully received!
The 3 frame transmitted success!
The 3 frame has been successfully received!

......

The 18 frame transmitted success!
The 18 frame has been successfully received!
The 19 frame transmitted success!
The 19 frame has been successfully received!
The 20 frame transmitted success!
The 20 frame has been successfully received!
