Overview
========
This is a small Iperf demo of the high-performance NetX Duo TCP/IP stack.


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
1. When the demo is running, the serial port will output:

Start the iperf example...
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
