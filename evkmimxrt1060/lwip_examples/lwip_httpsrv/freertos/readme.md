Overview
========

The lwip_httpsrv_freertos demo application demonstrates an HTTP server on the lwIP TCP/IP stack with FreeRTOS.
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
- Version: 2.15.100

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
         mDNS hostname    : lwip-http
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
	The browser should show a web page. The board also advertises itself using mDNS so that it can be accessed using URL http://lwip-http.local.
	Please note that your system may not support mDNS out-of-the-box as it is necessary to have an mDNS resolver installed.
    For instance Bonjour Print Services for Windows contain such resolver. In case of Linux nss-mdns serves this purpose.
    Username admin and password admin is required to access "Authorization example" section of the web page.

3.  If the example has been compiled with IPv6 enabled (LWIP_IPV6=1), wait for the unique local or global unique IPv6 address to appear
    in the list of valid addresses in the console output. Then the server could be reached over IPv6.
    For the example output above, http://[FD00:AABB:CCDD:EEFF:5627:8DFF:FE46:29F8] could be typed into the browser's address bar to make it
    show the web page and communicate with the board over IPv6.

4.  If the example has been compiled with IPv6 only and no IPv4 (LWIP_IPV6=1 and LWIP_IPV4=0), the board will still have the mDNS responder
    enabled. But some web browsers seem to send mDNS queries for IPv4 address only, so it might not be possible to access the board using
    http://lwip-http.local URL. Still, some utilities (like avahi-resolve on Linux) could be used to resolve the name to IPv6 address using mDNS.

Modifying content of static web pages
To modify content available through the web server you must complete following steps:
  1. Modify, add or delete files in folder "boards\<board_name>\lwip_examples\lwip_httpsrv_freertos\webpage".
  2. Run the script file "middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl <directory name>" to generate new "httpsrv_fs_data.c".
     Make sure to execute it from a folder where the file "httpsrv_fs_data.c" is. For example:
        C:\sdk\boards\<board_name>\lwip_examples\lwip_httpsrv_freertos> C:\sdk\middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl webpage
		Processing file webpage/auth.html
		Processing file webpage/cgi.html
		Processing file webpage/favicon.ico
		Processing file webpage/help.html
		Processing file webpage/httpsrv.css
		Processing file webpage/index.html
		Processing file webpage/NXP_logo.png
		Processing file webpage/poll.html
		Processing file webpage/request.js
		Processing file webpage/ssi.shtml
		Processing file webpage/welcome.html
		Done.
  3. Make sure the "httpsrv_fs_data.c" file has been overwritten with the newly generated content.
  4. Re-compile the HTTP server application example and download it to your board.
