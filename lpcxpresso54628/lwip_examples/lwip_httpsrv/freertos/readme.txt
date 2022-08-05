Overview
========

The lwip_httpsrv demo application demonstrates an HTTPServer on the lwIP TCP/IP stack with bare metal SDK or FreeRTOS.
The user uses an Internet browser to send a request for connection. The board acts as an HTTP server and sends a Web
page back to the PC.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- RJ45 Network cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
For LPCXpresso54628 V2.0:JP11 and JP12 1-2 on. 
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
1.  When the demo runs successfully, the terminal will display the following:
        Initializing PHY...

        ************************************************
         HTTP Server example
        ************************************************
         IPv4 Address     : 192.168.0.102
         IPv4 Subnet mask : 255.255.255.0
         IPv4 Gateway     : 192.168.0.100
        ************************************************
2.  On the browser address bar, type 192.168.0.102(IP address of the board).
	The browser should show a web page. The board also advertises itself using mDNS so that it can be accessed using URL http://lwip-http.local.
	Please note that your system may not support mDNS out-of-the-box as it is necessary to have an mDNS resolver installed.
    For instance Bonjour Print Services for Windows contain such resolver. In case of Linux nss-mdns serves this purpose.
    Username admin and password admin is required to access "Authorization example" section of the web page.


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
