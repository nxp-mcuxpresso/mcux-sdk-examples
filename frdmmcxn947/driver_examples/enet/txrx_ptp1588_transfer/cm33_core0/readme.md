Overview
========

The enet_rxtx_ptp1588_transfer example shows the way to use ENET driver to  
 receive and transmit frame in the 1588 feature required cases.

1. This example shows how to initialize the ENET.
2. How to get the time stamp of the PTP 1588 timer.
3. How to use Get the ENET transmit and receive frame time stamp.

The example transmits 20 number PTP event frame, shows the timestamp of the transmitted frame.
The length, source MAC address and destination MAC address of the received frame will be print. 
The time stamp of the received timestamp will be print when the PTP message frame is received(the outside loopback cable can be used to see the right rx time-stamping log since we send the ptp message). 

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer
- Loopback network cable RJ45 standard

Board settings
==============
Set JP13 2-3 on.
Populate R274 to sync reference clock.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
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
Wait for PHY init...
Wait for PHY link up...
 Get the 1-th time 0 second, 18208 nanosecond
 Get the 2-th time 0 second, 199772858 nanosecond
 Get the 3-th time 0 second, 399893806 nanosecond
 Get the 4-th time 0 second, 600016650 nanosecond
 Get the 5-th time 0 second, 800141079 nanosecond
 Get the 6-th time 1 second, 276075 nanosecond
 Get the 7-th time 1 second, 200136436 nanosecond
 Get the 8-th time 1 second, 400272188 nanosecond
 Get the 9-th time 1 second, 600404401 nanosecond
 Get the 10-th time 1 second, 800540642 nanosecond

Transmission start now!
The 1 frame transmitted success!
 the timestamp is 2 second, 3333631 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 3334569 nanosecond
The 2 frame transmitted success!
 the timestamp is 2 second, 19654509 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 19655448 nanosecond
The 3 frame transmitted success!
 the timestamp is 2 second, 36150233 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 36151172 nanosecond
The 4 frame transmitted success!
 the timestamp is 2 second, 52657575 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 52658513 nanosecond
The 5 frame transmitted success!
 the timestamp is 2 second, 69153651 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 69154590 nanosecond
The 6 frame transmitted success!
 the timestamp is 2 second, 85660875 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 85661814 nanosecond
The 7 frame transmitted success!
 the timestamp is 2 second, 102156951 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 102157890 nanosecond
The 8 frame transmitted success!
 the timestamp is 2 second, 118851226 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 118852164 nanosecond
The 9 frame transmitted success!
 the timestamp is 2 second, 135534234 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 135535173 nanosecond
The 10 frame transmitted success!
 the timestamp is 2 second, 152227453 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 152228391 nanosecond
The 11 frame transmitted success!
 the timestamp is 2 second, 169003400 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 169004339 nanosecond
The 12 frame transmitted success!
 the timestamp is 2 second, 185790964 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 185791903 nanosecond
The 13 frame transmitted success!
 the timestamp is 2 second, 202566442 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 202567381 nanosecond
The 14 frame transmitted success!
 the timestamp is 2 second, 219353537 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 219354476 nanosecond
The 15 frame transmitted success!
 the timestamp is 2 second, 236128898 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 236129837 nanosecond
The 16 frame transmitted success!
 the timestamp is 2 second, 252915876 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 252916814 nanosecond
The 17 frame transmitted success!
 the timestamp is 2 second, 269691588 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 269692527 nanosecond
The 18 frame transmitted success!
 the timestamp is 2 second, 286478331 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 286479270 nanosecond
The 19 frame transmitted success!
 the timestamp is 2 second, 303253457 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 303254396 nanosecond
The 20 frame transmitted success!
 the timestamp is 2 second, 320040435 nanosecond
 One frame received. the length 1014
 the timestamp is 2 second, 320041374 nanosecond

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
