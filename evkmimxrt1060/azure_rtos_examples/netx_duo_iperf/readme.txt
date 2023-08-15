Overview
========
This is a small Iperf demo of the high-performance NetX Duo TCP/IP stack.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- EVK-MIMXRT1060 board
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
3.  Insert Cable to Ethernet RJ45 port and connect it to a ethernet switch.
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.

Running the demo
================
1. When the demo is running, the serial port will output:

Start the iperf example...
MAC address: 00:11:22:33:44:56
DHCP In Progress...
IP address: 192.168.2.10
Mask: 255.255.255.0

2. The board gets a new IP address, for example, 192.168.2.10. On a Linux PC, open a Web browser to
access the new IP address at http://192.168.2.10, and a iperf test Web page will display.

3. To start a UDP iperf test, run the iperf command in a command console on the Linux PC. For example,

  $ iperf -u -s -i 1

4. In the iperf test Web page, click the "Start UDP Transmit Test" button. The test
will begin.

5. When the test is finished, click the "here" in the right side of the Web page.
It shows the test result like:

UDP Transmit Test Done:

Destination IP Address: 192.168.2.18
Test Time(milliseconds): 10000
Number of Packets Transmitted: 30328
Number of Bytes Transmitted: 44582160
Throughput(Mbps):35


Note that in UDP receive test, Linux host limits the default bandwidth of UDP packet,
use "-b" to specify the bandwidth, for example:

  $ iperf -i 1 -u -b 100m -c 10.193.20.10
