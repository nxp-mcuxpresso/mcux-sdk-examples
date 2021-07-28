Overview
========

The enet_txrx_multiring_transfer example shows the way to use ENET driver to  
 receive and transmit avb frame in the avb feature supported multi-ring platforms.
 this example is only supported in multi-ring platform.

1. This example shows how to initialize the ENET MAC.
2. How to use ENET MAC to transmit frames in avb supported 
multiple-ring platforms.

The example transmits 30 number frames. For simple demo, we create frames with some special definition.
1. Build transmission frames with broadcast mac address.
2. Build different frames for each ring: 10 normal ethernet frame, 10 similar IEC 61883-6 type frames,
10 similar IEC 61883-8 type frames and the two IEC 1722 avb frames are set with different vlan priority .
 
Notice, To simply show how to use three different rings in rx/tx on one board. This example set PHY local loop and you will see there will be PHY init auto-negotiation fail and this is normal. because phy can not finish
auto-negotiation when phy is in loop back mode. However, the auto-negotiation fail has no impact on this loop back example, so just forget about the failure auto-negotiation.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- Loopback network cable RJ45 standard
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
If Rx frame CRC error occurs, try to populate a suitable C66. The REV C/C1 boards populate 22pF which is high and may result in CRC
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
3.  Insert loopback network cable to Ethernet RJ45 port(J4).
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

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

ENET multi-ring txrx example start.
The 0 frame transmitted from the ring 0 !
The frame received from the ring 0, 1, 2 is 1, 0, 0 now!
The 1 frame transmitted from the ring 1 !
The frame received from the ring 0, 1, 2 is 1, 0, 1 now!
The 1 frame transmitted from the ring 2 !
The frame received from the ring 0, 1, 2 is 1, 1, 1 now!
......
The 9 frame transmitted from the ring 0 !
The frame received from the ring 0, 1, 2 is 0, 0, 0 now!
The 10 frame transmitted from the ring 1 !
The frame received from the ring 0, 1, 2 is 0, 0, 0 now!
The 10 frame transmitted from the ring 2 !
The frame received from the ring 0, 1, 2 is 0, 0, 0 now!

30 frames transmitted succeed!
10 frames successfully received from the ring 0!
10 frames successfully received from the ring 1!
10 frames successfully received from the ring 2!
