Overview
========
The lwip_ipv4_ipv6_echo application demonstrates a TCP and UDP echo on the lwIP TCP/IP stack with FreeRTOS.
The demo can use both TCP or UDP protocol over IPv4 or IPv6 and acts as an echo server. The application sends back
the packets received from the PC, which can be used to test whether a TCP or UDP connection is available.

A few notes about IPv6
The demo generates a link-local address after the start. To send something to this (demo) address
from your computer you need to specify the interface over which the demo is reachable by appending % followed by zone index.
- On Windows, the zone index is a number. You can get it from the output of the ipconfig command.
- On Linux, the zone index is an interface name or a number.
To connect to board with address FE80::12:13FF:FE10:1511
- over interface 21 on your Windows machine specify address as FE80::12:13FF:FE10:1511%21
- over interface eth0 on your Linux or Mac machine specify address as FE80::12:13FF:FE10:1511%eth0

The board might support multiple ethernet ports. To connect from board to PC with link-local address FE80::1234:5678
- over interface en0 - specify address as FE80::1234:5678%en0
- over interface en1 - specify address as FE80::1234:5678%en1
You don't have to append zone index in the following situations:
- On boards with single ethernet port - it will be selected automatically.
- On boards with multiple ethernet ports if the application enables only one them (BOARD_NETWORK_USE_DUAL_ENET not defined).
- When connecting to global (routable) IPv6 address - the outgoing port will be selected based on the advertised prefix
or by selecting router from the list of default routers and using the port the selected default router is attached to.

The LwIP stack is trying to get an IPv6 address automatically by neighbor discovery in the background.
This takes some time. You can print all addresses using the command print_ip_cfg any time.

Tools
It is necessary to have installed tools capable of sending and receiving data over TCP or UDP to interact with the demo.
- ncat - Recommended tool. Supports both IPv4 and IPv6. It is part of nmap tools. It can be found at https://nmap.org/download.html.
- nc (netcat) - Basically, the same as ncat, but a lot of antiviruses consider this a virus.
- echotool - Supports only IPv4 and only for Windows. It can be obtained from https://github.com/PavelBansky/EchoTool


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Network cable RJ45 standard
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
Connect JP54. Load R520, R508, R505
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
5.  Download the program to the target board.
6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Some boards support simultaneous use of 2 ethernet ports - refer to the board settings section.
In such case, you will be able to connect another PC:
1. Insert the Ethernet Cable into the 2nd RJ45 port and connect it to your 2nd PC network adapter.
2. Configure the 2nd PC IP address to 192.168.1.200.

Running the demo
================
1. When the demo runs, the log would be seen on the terminal like:
		Initializing PHY...

		Copyright  2022  NXP

		SHELL>>

2. There are several options what can you do. Type help command to see them all:
		"help": List all the registered commands

		"exit": Exit program

		"echo_tcp_client ip_addr port":
		   Connects to specified server and sends back every received data.
		 Usage:
		   ip_addr:     IPv6 or IPv4 server address
		   port:        TCP port number

		"echo_tcp_server port":
		   Listens for up to 2 incoming connections and sends back every received data.
		 Usage:
		   port:        TCP port number

		"echo_udp port":
		   Waits for datagrams and sends them back.
		 Usage:
		   port:        UDP port number

		"end": Ends echo_* command.

		"print_ip_cfg": Prints IP configuration.

3. TCP client echo
	a) Launch ncat -v -l -p 10001 on your computer.
	b) Run command echo_tcp_client 192.168.0.100 10001 in demo shell.
	c) You should see on your PC "Ncat: Connection from 192.168.0.102."
	d) Type some text into ncat (not demo shell) and the demo will send your line back
	   when you hit enter.
	e) You can terminate connection by pressing ctrl+c in ncat or typing end to demo shell.
	
	For IPv6 just replace 192.168.0.100 with IPv6 address of your PC and zone ID.
	
4. TCP server echo
	a) Run command echo_tcp_server 10001 in demo shell.
	b) Launch ncat -v 192.168.0.102 10001 on your computer.
	c) You should see on your PC "Ncat: Connected to 192.168.0.102:10001."
	d) Type some text into ncat (not demo shell) and the demo will send your line back
	   when you hit enter.
	e) You can terminate connection by pressing ctrl+c in ncat or typing end to demo shell.
	
	For IPv6 just replace 192.168.0.102 with IPv6 address of demo FE80::12:13FF:FE10:1511%<zone ID>.

5. UDP echo
	a) Run command echo_udp 10001 in demo shell.
	b) On your computer launch ncat -v -u 192.168.0.102 10001
	c) You should see on your PC "Ncat: Connected to 192.168.0.102:10001."
	d) Type some text into ncat (not demo shell) and the demo will send your line back
	   when you hit enter.
	e) To terminate is necessary by doing both, pressing ctrl+c in ncat and typing end to demo shell.
	
	For IPv6 just replace 192.168.0.102 with IPv6 address of demo FE80::12:13FF:FE10:1511%<zone ID>.
