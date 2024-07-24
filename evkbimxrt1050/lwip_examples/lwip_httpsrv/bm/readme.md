Overview
========

The lwip_httpsrv demo application demonstrates an HTTPServer on the lwIP TCP/IP stack with bare metal SDK.
The user uses an internet browser to send a request for connection. The board acts as an HTTP server and sends a web
page back to the PC.

The example supports both IPv4 and IPv6 protocols. But some combinations of boards or build configurations may have
only IPv4 enabled due to memory constraints. If it fits into memory, it is possible for the example to be compiled
with both IPv4 and IPv6 (LWIP_IPV4=1 and LWIP_IPV6=1 in lwipopts.h), IPv4 only (LWIP_IPV4=1 and LWIP_IPV6=0
in lwipopts.h) or IPv6 only (LWIP_IPV4=0 and LWIP_IPV6=1 in lwipopts.h).
If IPv6 is enabled, the board will assign a link-local IPv6 address to its interface, but this is often not enough
as web browsers tend to not support link-local address with zone index in URL (e.g. http://[FE80::5627:8DFF:FE46:29F8%eth0]).
So if the board is connected directly to the host PC, the PC should be configured to send ICMPv6 Router Advertisement
messages with Prefix option, which the board could use to assign itself a routable address using the Stateless Address
Autoconfiguration mechanism. If the board and the host PC are connected through a router, sending of Router Advertisement
messages could be configured on that router (if it supports IPv6).
Instructions how to set the IPv6 network correctly can be different on each system and it is out of scope of this readme.
It can take a couple of seconds until the addresses are validated and assigned to the board's ethernet interface,
so the board will print "IPv6 address update" messages followed by all valid IPv6 addresses anytime there is a change.
The assigned unique local or global unique addresses could be typed into the web browser's address bar in a form like
"http://[FD00:AABB:CCDD:EEFF:5627:8DFF:FE46:29F8]" to access the web server over IPv6.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- EVKB-IMXRT1050 board
- Personal Computer

Board settings
==============
No special settings are required.

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
5.  Configure the host PC to advertise IPv6 address prefix.
6.  Open a web browser.
7.  Download the program to the target board.
8.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  When the demo runs successfully, the terminal will display the following:
        Initializing PHY...

        ***********************************************************
         HTTP Server example
        ***********************************************************
         IPv4 Address     : 192.168.0.102
         IPv4 Subnet mask : 255.255.255.0
         IPv4 Gateway     : 192.168.0.100
         IPv6 Address0    : -
         IPv6 Address1    : -
         IPv6 Address2    : -
        ***********************************************************
        IPv6 address update, valid addresses:
         IPv6 Address0    : FE80::5627:8DFF:FE46:29F8
         IPv6 Address1    : -
         IPv6 Address2    : -

        IPv6 address update, valid addresses:
         IPv6 Address0    : FE80::5627:8DFF:FE46:29F8
         IPv6 Address1    : FD00:AABB:CCDD:EEFF:5627:8DFF:FE46:29F8
         IPv6 Address2    : -

2.  If the example has been compiled with IPv4 enabled (LWIP_IPV4=1), type 192.168.0.102 (IP address of the board) into the browser address bar.
    The browser should respond as shown below:

		lwIP - A Lightweight TCP/IP stack

		The web page you are watching was served by a simple web server running on top of the lightweight TCP/IP stack lwIP.

		lwIP is an open source implementation of the TCP/IP protocol suite that was originally written by Adam Dunkels of
		the Swedish Institute of Computer Science but now is being actively developed by a team of developers distributed
		world-wide.Since it's release, lwIP has spurred a lot of interest and has been ported to several platforms and
		operating systems. lwIP can be used either with or without an underlying OS.

		The focus of the lwIP TCP/IP implementation is to reduce the RAM usage while still having a full scale TCP. This
		makes lwIP suitable for use in embedded systems with tens of kilobytes of free RAM and room for around 40 kilobytes
		of code ROM.

		More information about lwIP can be found at the lwIP homepage at http://savannah.nongnu.org/projects/lwip/ or at
		the lwIP wiki at http://lwip.wikia.com/.

3.  If the example has been compiled with IPv6 enabled (LWIP_IPV6=1), wait for the unique local or global unique IPv6 address to appear
    in the list of valid addresses in the console output. Then the server could be reached over IPv6.
    For the example console output above, http://[FD00:AABB:CCDD:EEFF:5627:8DFF:FE46:29F8] could be typed into the browser's address bar to make it
    show the web page and communicate with the board over IPv6.
