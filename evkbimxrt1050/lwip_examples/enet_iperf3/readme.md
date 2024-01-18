Overview
========
The iperf3 example provides basic commands to measure performance of network stack.
Results will be shown on your iperf server. In the case of 'R' mode, server shows the amount of sent data,
check the terminal output in this case for the total amount of payload in bytes received over UDP,
which marks the device's throughput.

Known Issues
If the iperf3 server shows you that you have 0-bit/s bandwidth, it may be caused due to the version you are using.
This client works with Iperf v3.7. In case of any more issues, see https://github.com/esnet/iperf/labels/bug. 

Rx UDP mode ('R') has limited bandwidth by UDP_RX_BANDWIDTH macro as sending UDP packets too fast may cause
the control TCP socket to lost data and test failure.
Set the macro to a little bit higher than the device can receive or "0" for no limit.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Prepare iperf3 server on your machine:
    - Install iperf3 (Enter command "sudo apt install iperf3")
    - Start the server by running "iperf3 -s"
2.  Connect a micro USB cable between the PC host and the SDA port
3.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.


Running the demo
================
When the demo runs successfully, the log would be seen on the CMSIS DAP terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Enter IP address of a server in format '192.168.1.2'
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Enter IP address of the iperf3 server.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Using IP xxx.xxx.xxx.xxx
Menu:
Press 's' to start client Tx mode
Press 'r' to start client Rx mode
Press 'S' to start client Tx UDP mode
Press 'R' to start client Rx UDP mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Press 's' to start Iperf3 client in sending mode. (TCP)
- Press 'r' to start Iperf3 client in receiving mode. (TCP)
- Press 'S' to start Iperf3 client in sending mode. (UDP)
- Press 'R' to start Iperf3 client in receiving mode. (UDP)

