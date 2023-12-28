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
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Loopback network cable RJ45 standard
- Network cable RJ45 standard
- Network switch
- Personal Computer

Board settings
==============
If Rx frame CRC error occurs, try to populate a suitable C66. The REV A1 boards populate 22pF which is high and may result in CRC
errors on some of boards, if this is the case C66 may be removed or replaced by a smaller capacitor.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert loopback network cable into network switch device(support loopback frames) and connect switch to board's Ethernet RJ45 port(J4).
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Note: If not use switch, only insert loopback network cable into Ethernet RJ45 port, auto-negotiation result is 100M. Here we use switch to test 1G.

Make loopback cable:
      Standard 1	     Standard 2
J1    orange+white       green+white
J2    orange             green
J3    green+white        orange+white
J4    blue               brown+white
J5    blue+white         brown
J6    green              orange
J7	  brown+white        blue
J8    brown              blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to the sheet above. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

Running the demo
================
When the demo runs, the log would be seen on the terminal like:

ENET example start.
The 1 frame transmitted success!
A total of 1 frame(s) has been successfully received!
The 2 frame transmitted success!
A total of 2 frame(s) has been successfully received!
The 3 frame transmitted success!
A total of 3 frame(s) has been successfully received!

......

The 20 frame transmitted success!
A total of 20 frame(s) has been successfully received!
