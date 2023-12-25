Overview
========
Application demonstrating very basic BLE Central role functionality by scanning for other BLE devices and establishing a connection to the first one with a strong enough signal.
This application specifically looks for a peer advertising the Intermediate Alert Service and implementing the Find Me Profile.


Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

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
The locator will automatically start scanning and will connect to the first advertiser who is advertising the Intermediate Alert Service.
If the connection is successful, the application performs service discovery to find the Alert Level characteristic.
The application will trigger periodic alerts on the target device until a threshold is reached, after which it will disconnect.
After the disconnect from the target the locator will restart scanning.
