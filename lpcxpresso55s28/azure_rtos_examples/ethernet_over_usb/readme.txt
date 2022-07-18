Overview
========
This example can do network iperf test through a USB Ethernet adapter.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Two Micro USB cables
- One USB A Female to Micro B Male cable
- HP USB to Gigabit RJ45 Adapter
- Target Board
- Personal Computer(PC)

Board settings
==============
This example can work with the USB high speed port (P9) or the USB full speed port (P10).
High speed: Install jumper in position 1-2 pins of J6 and open J7 jumper.
Full speed: Install jumper in position 2-3 pins of J6 and short J7 pins.

Prepare the Demo
================
1.  High speed: Connect the USB A Female to Micro B Male cable between the USB RJ45 adapter and
                the on-board USB high speed port (P9).
    Full speed: Connect the USB A Female to Micro B Male cable between the USB RJ45 adapter and
                the on-board USB full speed port (P10).
2.  Connect a Ethernet cable with the USB RJ45 adapter.
3.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
4.  Connect a USB Micro cable between the PC and the on-board USB power port (P5).
5.  Open a serial terminal on the PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
6.  Compile the demo:
    - High speed: Set USB_HOST_CONFIG_IP3516HS to 1 in board_setup.c
    - Full speed: Set USB_HOST_CONFIG_IP3516HS to 0 in board_setup.c
7.  Download the program to the target board.
8.  Press the on-board RESET button to start the demo.

Running the demo
================
1. When the demo is running, the serial port will output:

Start the USBX Ethernet Over USB example...
USB device: vid=0xbda, pid=0x8153
DHCP In Progress...
IP address: 10.193.20.10
Mask: 255.255.255.0
Gateway: 10.193.20.254

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
Number of Bytes Transmitted: 56158410
Throughput(Mbps):44

