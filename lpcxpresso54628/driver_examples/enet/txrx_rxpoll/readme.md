Overview
========

The enet_rxtx_rxinterrupt example shows the simplest way to use ENET functional tx/rx API for simple frame receive and transmit.

1. This example shows how to initialize the ENET.
2. How to use ENET to receive frame in polling and to transmit frame.

The example transmits 20 number broadcast frame, print the number and the mac address of 
the recieved frames.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54628 board
- Personal Computer
- Loopback network cable RJ45 standard

Board settings
==============
For LPCXpresso54628 V2.0:JP11 and JP12 1-2 on. 

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert loopback network cable to Ethernet RJ45 port.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Make loopback network cable:
      568B standard 	 Unknowed standard
J1    orange+white       green+white
J2    orange             green
J3    green+white        orange+white
J4    blue               brown+white
J5    blue+white         brown
J6    green              orange
J7	  brown+white        blue
J8    brown              blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to 568B standard or the other standard. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

Running the demo
================
The log below shows example output of the example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

ENET example start.


Transmission start now!
The 1-th frame transmitted success!
The 2-th frame transmitted success!
The 3-th frame transmitted success!
The 4-th frame transmitted success!
The 5-th frame transmitted success!
The 6-th frame transmitted success!
The 7-th frame transmitted success!
The 8-th frame transmitted success!
The 9-th frame transmitted success!
The 10-th frame transmitted success!
The 11-th frame transmitted success!
The 12-th frame transmitted success!
The 13-th frame transmitted success!
The 14-th frame transmitted success!
The 15-th frame transmitted success!
The 16-th frame transmitted success!
The 17-th frame transmitted success!
The 18-th frame transmitted success!
The 19-th frame transmitted success!
The 20-th frame transmitted success!
 One frame received. the length 346
 Dest Address ff:ff:ff:ff:ff:ff Src Address 00:23:24:9d:33:de 
 One frame received. the length 64 
 Dest Address ff:ff:ff:ff:ff:ff Src Address 00:23:24:9d:33:de 
 One frame received. the length 64 
 Dest Address ff:ff:ff:ff:ff:ff Src Address 00:23:24:9d:33:de 
...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
