Overview
========
This is a small ping demo of the high-performance NetX Duo TCP/IP stack.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- A Micro USB cable
- An Ethernet cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  This example uses 1G port by default. If using the 100M ENET port, set the macro
    BOARD_NETWORK_USE_100M_ENET_PORT in board.h to 1.
4.  Compile the demo.
5.  Download the program to the target board.
6.  Insert an Ethernet cable to the default Ethernet RJ45 port labelled "1G ENET" and connect it to
    an Ethernet switch. Note that if set BOARD_NETWORK_USE_100M_ENET_PORT, the RJ45 port should be
    the one labelled "100M ENET".
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo is running, the serial port will output:

Start the ping example...
MAC address: 00:11:22:33:44:56
DHCP In Progress...
IP address: 192.168.2.10
Mask: 255.255.255.0

It shows that the board gets a new IP address, for example, 192.168.2.10. Then, on a PC,
use the ping command to ping the new IP address of the board, for example:

$ ping 192.168.2.10
Pinging 192.168.2.10 with 32 bytes of data:
Reply from 192.168.2.10: bytes=32 time=5ms TTL=127
Reply from 192.168.2.10: bytes=32 time=4ms TTL=127
Reply from 192.168.2.10: bytes=32 time=3ms TTL=127
Reply from 192.168.2.10: bytes=32 time=2ms TTL=127

It shows that the example program running on the board can handle ping packages.

