Overview
========
The NETC Tx/Rx transfer demo application demonstrates how to set up NETC frame transmission.
In this demo, NETC PSI sends/receives frames with external loopback cable. Use MISX to handle interrupts in the transmission.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
Populate R469 and set reset pin as open-drain to complete the PHY hardware reset properly.

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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
NETC EP0 frame loopback example start.
Wait for PHY link up...
The 1 frame transmitted success!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00
The 2 frame transmitted success!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00
......
The 19 frame transmitted success!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00
The 20 frame transmitted success!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00

NETC Switch frame loopback example start.
Wait for PHY link up...
The 1 frame transmitted success on port 0!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00 
The 2 frame transmitted success on port 1!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00 
......
The 19 frame transmitted success on port 0!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00 
The 20 frame transmitted success on port 1!
 A frame received. The length is 1000  Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:00:00:00 

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
