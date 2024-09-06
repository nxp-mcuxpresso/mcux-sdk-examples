Overview
========

The lwip_ping demo application demonstrates a Ping Demo on the lwIP TCP/IP stack which using the ICMP protocol. The
application periodically sends the ICMP echo request to a PC and processes the PC reply. Type the "ping $board_address"
in the PC command window to send an ICMP echo request to the board. The lwIP stack sends the ICMP echo reply back to the
PC.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- Network cable RJ45 standard
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
Load R485, R486, R487, R488, R489, R491, R490, R522, R521, R520, R524, R523, R508, R505
Remove R518, R507, R506


Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert the Ethernet Cable into the target board's RJ45 port and connect it to your PC network adapter.
4.  Configure the host PC IP address to 192.168.0.100.
5.  Open a web browser.
6.  Download the program to the target board.
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs, the log would be seen on the terminal like:
	Initializing PHY...

	************************************************
	 PING example
	************************************************
	 IPv4 Address     : 192.168.0.102
	 IPv4 Subnet mask : 255.255.255.0
	 IPv4 Gateway     : 192.168.0.100
	************************************************
	ping: send
	192.168.0.100


	ping: recv
	192.168.0.100
	 3 ms

	ping: send
	192.168.0.100


	ping: recv
	192.168.0.100
	 3 ms

	ping: send
	192.168.0.100


	ping: recv
	192.168.0.100
	 3 ms
