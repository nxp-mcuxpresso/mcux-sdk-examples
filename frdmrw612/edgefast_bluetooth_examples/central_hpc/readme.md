Overview
========
Application demonstrating very basic BLE Central role functionality by scanning for other BLE devices and establishing a connection to the first one with a strong enough signal.
Except that this application specifically looks for HPS Server and programs a set of characteristics that configures a Hyper Text Transfer Protocol (HTTP) request, initiate this request, and then read the response once connected.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Type-C cable
- frdmrw612 board
- Personal Computer

Board settings
==============

Prepare BLE controller
Download BLE controller FW according to the document component\conn_fwloader\readme.txt.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary to the target board, 
please reset the board by pressing SW1 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.

5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The demo does not require user interaction. The application will automatically start scanning and will connect to the first advertiser who is advertising the HTTP Proxy Service. If the connection is successful, the application performs service discovery to find the characteristics of the HTTP Proxy Service. If discovery is successful, the application will perform a GET for the URI http://nxp.com by writing the URI and the Control Point characteristics of the HTTP Proxy Service.

The application will display the received response in the console after it gets notified through the HTTP Status Code characteristic. Example output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Received HTTP Status 500
Reading Headers...
HTTP Headers: HTTPHEADER
Reading Body...
HTTP Body: HTTPBODY
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
