Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the Proximity Reporter (including LLS, IAS, and TPS) GATT Service.


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- rdrw610 board
- Personal Computer

Board settings
==============
U38 DIP 1,2,3,4 all off
HD12 1-2, 3-4 connected

Prepare BLE controller
Download BLE controller FW according to the document component\conn_fwloader\readme.txt.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary to the target board, 
please reset the board by pressing SW5 or power off and on the board to run the application.
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
The demo does not require user interaction. The application will automatically start advertising the Link Loss Service and it will accept the first connection request it receives. The application is then ready to process operations from the peer.

The application will initially set the default levels for the Link Loss Alert and the Immediate Alert. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Locally setting Immediate Alert...
ALERT: OFF
Locally setting Link Loss Alert Level to OFF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Proximity Monitor peer will trigger or stop the Immediate Alert on the application depending on the connection RSSI. Example output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Monitor is setting Immediate Alert...
ALERT: HIGH
Monitor is setting Immediate Alert...
ALERT: OFF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the connection with the Proximity Monitor is timed out, the Link Loss Alert will be triggered with the level previously set by the Monitor. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Link Loss Alert Triggered...
ALERT: HIGH
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
