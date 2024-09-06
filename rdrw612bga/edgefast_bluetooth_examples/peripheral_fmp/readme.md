Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the Find Me Profile in the Target role.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- rdrw610 board
- Personal Computer

Board settings
==============

Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.

5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The demo does not require user interaction. The application will automatically start advertising the Intermediate Alert Service and it will accept the first connection request it receives.
The application will initially set the default levels for the Link Loss Alert and the Immediate Alert.
The Locator will trigger an Immediate Alert on the application and disconnect. The Target will then resume advertising.
