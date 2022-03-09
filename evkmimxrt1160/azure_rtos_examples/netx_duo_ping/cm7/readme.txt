Overview
========
This is a small ping demo of the high-performance NetX Duo TCP/IP stack.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- A Micro USB cable
- An Ethernet cable
- MIMXRT1160-EVK board
- Personal Computer

Board settings
==============
On MIMXRT1160-EVK REVC board, GPIO_AD_32 uses as ENET_MDC in this example which is muxed with the SD1_CD_B,
please check the R1926 and R136 connected to SD1_CD_B. If they are populated with resistor and SD card is
inserted, this time enet can't access PHY.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  If using the 1G ENET port, define the macro EXAMPLE_USE_1G_ENET_PORT when compiling.
4.  Compile the demo.
5.  Download the program to the target board.
6.  Insert an Ethernet cable to the default Ethernet RJ45 port labelled "100M ENET" and connect it to
    an Ethernet switch. Note that if enabling EXAMPLE_USE_1G_ENET_PORT, the RJ45 port should be the one
    labelled "1G ENET".
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo is running, the serial port will output:

Start the ping example...
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

