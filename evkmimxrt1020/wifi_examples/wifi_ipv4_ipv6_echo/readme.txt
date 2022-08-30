Overview
========
The wifi_ipv4_ipv6_echo application demonstrates a TCP and UDP echo on the lwIP TCP/IP stack with FreeRTOS.
The demo can use both TCP or UDP protocol over IPv4 or IPv6 and acts as an echo server. The application sends back
the packets received from the PC, which can be used to test whether a TCP or UDP connection is available.

A few notes about IPv6
The demo generates a link-local address (the one from range FE80::/10) after the start. To send something to this (demo) address
from your computer you need to specify the interface over which the demo is reachable by appending % followed by zone index.
- On Windows, the zone index is a number. You can get it from the output of the ipconfig command.
- On Linux, the zone index is an interface name.
To connect to board with address FE80::12:13FF:FE10:1511
- over interface 21 on your Windows machine specify address as FE80::12:13FF:FE10:1511%21
- over interface eth on your Linux or Mac machine specify address as FE80::12:13FF:FE10:1511%eth0
But the demo has only a single interface, so do not append zone ID to any address typed to the demo terminal.

The LwIP stack is trying to get an IPv6 address automatically by neighbor discovery in the background.
This takes some time. You can print all addresses using the command print_ip_cfg any time.

Tools
It is necessary to have installed tools capable of sending and receiving data over TCP or UDP to interact with the demo.
- ncat - Recommended tool. Supports both IPv4 and IPv6. It is part of nmap tools. It can be found at https://nmap.org/download.html.
- nc (netcat) - Basically, the same as ncat, but a lot of antiviruses consider this a virus.
- echotool - Supports only IPv4 and only for Windows. It can be obtained from https://github.com/PavelBansky/EchoTool


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1020 board
- Personal Computer


Board settings
==============

Prepare the Demo
================
Before building the example application, select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection, see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM
    
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Connect the WiFi module to the SD card slot.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

1. When the demo starts, a welcome message would appear on the terminal, press enter for the command prompt:
   Press tab or type help to list out all available CLI commands.

	Initialize WLAN Driver
	MAC Address: 48:E7:DA:9A:CE:39 
	Initialize CLI

	Copyright  2020  NXP

	SHELL>> [net] Initialized TCP/IP networking stack
	app_cb: WLAN: received event 10
	app_cb: WLAN initialized

	SHELL>> help

	"help": List all the registered commands

	"exit": Exit program

	"echo_tcp_client ip_addr port":
	   Connects to specified server and sends back every received data.
	 Usage:
	   ip_addr:     IPv6 or IPv4 server address
	   port:        TCP port number

	"echo_tcp_server port":
	   Listens for incoming connection and sends back every received data.
	 Usage:
	   port:        TCP port number

	"echo_udp port":
	   Waits for datagrams and sends them back.
	 Usage:
	   port:        UDP port number

	"end": Ends echo_* command.

	"print_ip_cfg": Prints IP configuration.

	"wlan_scan": Scans networks.

	"wlan_connect ssid":
	   Connects to the specified network without password.
	 Usage:
	   ssid:        network SSID or BSSID

	"wlan_connect_with_password ssid password":
	   Connects to the specified network with password.
	 Usage:
	   ssid:        network SSID or BSSID
	   password:    password
	SHELL>> 

2. You can list all available networks using wlan_scan command.
	Scanning
	SHELL>> 
	2 networks were found:
	 #1  c4:ad:34:a3:86:11"pine5"
	 #2  00:72:63:fa:1b:96"netis_FA1B96"

3. Connect to the network using one of the following commands:
	wlan_connect <(b)ssid>
	wlan_connect_with_password <(b)ssid> <password>
	
	You can use SSID (the name of your network) or BSSID (it's mac).
	
   On successful connection, the demo prints something like this:	
	app_cb: WLAN: connected to network
	Connected to following BSS:
	SSID = [pine5]
	
   Note: If you see your network in the list of scanned networks but demo 
   repeatedly prints app_cb: WLAN: network not found try to restart your AP.

4. TCP client echo
	a) Launch ncat -v -l -p 10001 on your computer.
	b) Run the command echo_tcp_client <PC IPv4 addr> 10001 in demo shell.
	c) You should see on your PC "Ncat: Connection from <Demo IPv4 addr>."
	d) Type some text into ncat (not demo shell) and the demo will send your line back
	   when you hit enter.
	e) You can terminate the connection by pressing ctrl+c in ncat or typing end to the demo shell.
	
	For IPv6 just replace <PC IPv4 addr> with IPv6 address of your PC.
	
5. TCP server echo
	a) Run the command echo_tcp_server 10001 in demo shell.
	b) Launch ncat -v <Demo IPv4 addr> 10001 on your computer.
	c) You should see on your PC "Ncat: Connected to <Demo IPv4 addr>:10001."
	d) Type some text into ncat (not demo shell) and the demo will send your line back
	   when you hit enter.
	e) You can terminate the connection by pressing ctrl+c in ncat or typing end to the demo shell.
	
	For IPv6, just replace <Demo IPv4 addr> with the IPv6 address of the demo. In case of
	link-local IPv6 address, don't forget to append % followed by zone id of your PC.
	
6. UDP echo
	a) Run the command echo_udp 10001 in demo shell.
	b) On your computer launch ncat -v -u <Demo IPv4 addr> 10001
	c) You should see on your PC "Ncat: Connected to <Demo IPv4 addr>:10001."
	d) Type some text into ncat (not the demo shell) and the demo will send your line back
	   when you hit enter.
	e) To terminate is necessary by doing both, pressing ctrl+c in ncat and typing end to demo shell.
	
	For IPv6, just replace <Demo IPv4 addr> with the IPv6 address of the demo. In case of
	link-local IPv6 address, don't forget to append % followed by zone id of your PC.
