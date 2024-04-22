Overview
========
The NETC Switch demo application demonstrates how to set up NETC switch feature.
In this demo, NETC acts as network switch.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.3.1

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
NETC Switch example start.
Wait for PHY link up...

MAC learning.
The frame received from port 0. Dest Address 54:27:8d:00:00:f0 Src Address 54:27:8d:00:00:00. Port 0 bounds to MAC 54:27:8d:00:00:00.
The frame received from port 1. Dest Address 54:27:8d:00:00:f0 Src Address 54:27:8d:00:00:01. Port 1 bounds to MAC 54:27:8d:00:00:01.
The frame received from port 2. Dest Address 54:27:8d:00:00:f0 Src Address 54:27:8d:00:00:02. Port 2 bounds to MAC 54:27:8d:00:00:02.
The frame received from port 3. Dest Address 54:27:8d:00:00:f0 Src Address 54:27:8d:00:00:03. Port 3 bounds to MAC 54:27:8d:00:00:03.

Frame forwarding.
The frame received from pseudo port. Dest Address 54:27:8d:00:00:00 Src Address 54:27:8d:00:00:f0. Frame forwarding to port 0.
The frame received from pseudo port. Dest Address 54:27:8d:00:00:01 Src Address 54:27:8d:00:00:f0. Frame forwarding to port 1.
The frame received from pseudo port. Dest Address 54:27:8d:00:00:02 Src Address 54:27:8d:00:00:f0. Frame forwarding to port 2.
The frame received from pseudo port. Dest Address 54:27:8d:00:00:03 Src Address 54:27:8d:00:00:f0. Frame forwarding to port 3.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
