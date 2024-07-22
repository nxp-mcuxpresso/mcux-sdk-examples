Overview
========

The example shows the way to use ENET driver to
 receive and transmit frame in the 1588 feature required cases.

1. This example shows how to initialize the ENET.
2. How to get the time stamp of the PTP 1588 timer.
3. How to use Get the ENET transmit and receive frame time stamp.

The example transmits 20 number PTP event frame, shows the timestamp of the transmitted frame.
The length, source MAC address and destination MAC address of the received frame will be print.
The time stamp of the received timestamp will be print when the PTP message frame is received(the outside loopback cable can be used to see the right rx time-stamping log since we send the ptp message).

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- Loopback network cable RJ45 standard
- MCIMX93-QSB board
- Personal Computer

Board settings
==============
No special settings are required.

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert loopback network cable to Ethernet RJ45 port(J501).
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the demo.

Make loopback cable:
    568B standard 	Unknowed standard
J1	orange+white    green+white
J2	orange          green
J3	green+white     orange+white
J4	blue            brown+white
J5	blue+white      brown
J6	green           orange
J7	brown+white     blue
J8	brown           blue+white

Connect J1 => J3, J2 => J6, J4 => J7, J5 => J8. 10/100M transfer only requires J1, J2, J3, J6, and 1G transfer requires all 8 pins.
Check your net cable color order and refer to 568B standard or the other standard. If your cable's color order is not showed in the list,
please connect J1~J8 based on your situation.

Running the demo
================
When the demo runs, the log would be seen on the terminal like:

ENET example start.
 Get the 1-th time 0 second, 1864 nanosecond
 Get the 2-th time 0 second, 7275192 nanosecond
 Get the 3-th time 0 second, 14804820 nanosecond
 Get the 4-th time 0 second, 22420658 nanosecond
 Get the 5-th time 0 second, 30036030 nanosecond
 Get the 6-th time 0 second, 37651402 nanosecond
 Get the 7-th time 0 second, 45267240 nanosecond
 Get the 8-th time 0 second, 52882612 nanosecond
 Get the 9-th time 0 second, 60498450 nanosecond
 Get the 10-th time 0 second, 68113822 nanosecond

Transmission start now!
The 1 frame transmitted success!
 the timestamp is 0 second, 167770756 nanosecond
 One frame received. the length 1014
 the timestamp is 0 second, 167973396 nanosecond
The 2 frame transmitted success!
 the timestamp is 0 second, 200548167 nanosecond
 One frame received. the length 1014
 the timestamp is 0 second, 200750756 nanosecond
The 3 frame transmitted success!
 the timestamp is 0 second, 233324728 nanosecond
 One frame received. the length 1014
 the timestamp is 0 second, 233527436 nanosecond
The 4 frame transmitted success!
 the timestamp is 0 second, 266102139 nanosecond
 One frame received. the length 1014
 the timestamp is 0 second, 266305136 nanosecond
The 5 frame transmitted success!
 the timestamp is 0 second, 298878785 nanosecond

......
The 20 frame transmitted success!
 the timestamp is 0 second, 792377480 nanosecond
 One frame received. the length 1014
 the timestamp is 0 second, 792580698 nanosecond
