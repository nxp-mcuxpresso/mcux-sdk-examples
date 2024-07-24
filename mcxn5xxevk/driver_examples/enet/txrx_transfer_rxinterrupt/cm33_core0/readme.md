Overview
========

The enet_rxtx_rxinterrupt example shows the simplest way to use ENET transactional tx/rx API for simple frame receive and transmit.

1. This example shows how to initialize the ENET.
2. How to use ENET to receive frame in interrupt irq handler and to transmit frame.

The example transmits 20 number broadcast frame, print the number of recieved frames. To avoid
the receive number overflow, the transmit/receive loop with automatically break when 20 number
are received.



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
- MCX-N5XX-EVK board
- Personal Computer
- Loopback network cable RJ45 standard (optional)

Board settings
==============
Set JP13 2-3 on.
Populate R274 to sync reference clock.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

 ENET example start.
Wait for PHY init...
Wait for PHY link up...

Transmission start now!
The 1 frame transmitted success!
1 frame has been successfully received
The 2 frame transmitted success!
2 frame has been successfully received
The 3 frame transmitted success!
3 frame has been successfully received
The 4 frame transmitted success!
4 frame has been successfully received
The 5 frame transmitted success!
5 frame has been successfully received
The 6 frame transmitted success!
6 frame has been successfully received
The 7 frame transmitted success!
7 frame has been successfully received
The 8 frame transmitted success!
8 frame has been successfully received
The 9 frame transmitted success!
9 frame has been successfully received
The 10 frame transmitted success!
10 frame has been successfully received
The 11 frame transmitted success!
11 frame has been successfully received
The 12 frame transmitted success!
12 frame has been successfully received
The 13 frame transmitted success!
13 frame has been successfully received
The 14 frame transmitted success!
14 frame has been successfully received
The 15 frame transmitted success!
15 frame has been successfully received
The 16 frame transmitted success!
16 frame has been successfully received
The 17 frame transmitted success!
17 frame has been successfully received
The 18 frame transmitted success!
18 frame has been successfully received
The 19 frame transmitted success!
19 frame has been successfully received
The 20 frame transmitted success!
20 frame has been successfully received

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Optionally, you may use a loop back cable
Make a loopback network cable:
      568B standard 	 Unknowed standard
J1    orange+white       green+white
J2    orange             green
J3    green+white        orange+white
J4    blue               brown+white
J5    blue+white         brown
J6    green              orange
J7    brown+white        blue
J8    brown              blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to the 568B standard or the other standard. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

1.  Add #define EXAMPLE_USES_LOOPBACK_CABLE 1  into app.h and rebuild the example.
2.  Insert loopback network cable to Ethernet RJ45 port.
3.  Run the demo in the same way as described earlier.
