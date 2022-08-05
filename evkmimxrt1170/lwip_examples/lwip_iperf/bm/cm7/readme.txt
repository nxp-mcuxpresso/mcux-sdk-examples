Overview
========

This is the IPerf example to check your bandwidth using the network performance measurement IPerf application on a PC as a client or a server.
IPv4 is implemented. The UDP implementation is based on lwIP community experimental patches, therefore some issues could be experienced.
UDP sending rate is calculated from the system time, which has a resolution of 1 ms in lwIP. Therefore the actual sending rate could be
a little lower or higher due to sending a calculated (rounded) number of frames each one or more milliseconds. It can also result in higher jitter.
The desired UDP sending rate is determined by the IPERF_UDP_CLIENT_RATE definition. If you want to change the rate without the need to compile
the application with a new value of IPERF_UDP_CLIENT_RATE, you can run the application as a UDP server instead and control the sending rate
by using the tradeoff mode from the PC application and determining the rate there.
For client modes it assumes the PC application it connects to is running on the gateway.

Instead of the command line IPerf application, for more convenience, it is recommended to use the JPerf2 graphical tool, which can be downloaded here: https://sourceforge.net/projects/iperf/files/jperf/jperf%202.0.0/jperf-2.0.0.zip/download
The example supports IPerf version 2.0.5. JPerf2, downloaded from the link above, contains version 1.7.0 of iperf.exe for Windows however.
Therefore the iperf.exe version has to be updated when using MS Windows. IPerf 2.0.5b for Windows can be downloaded from the following link:
https://iperf.fr/download/windows/iperf-2.0.5b-win32.zip
The contents of the downloaded archive have to be unpacked into jperf-2.0.0/bin folder, overwriting iperf.exe.
The application has been tested also with IPerf 2.0.10, which can be downloaded here:
https://sourceforge.net/projects/iperf2/files/

To experiment with the receive throughput, try to increase the number of receive buffers.
For LPC platforms, where zero-copy on receive is implemented, the number of buffers is determined by ENET_RXBD_NUM definition.
When using ENET QOS, where zero-copy on receive is not implemented, increase the PBUF_POOL_SIZE in the file lwipopts.h or on command line.
For other platforms, where zero-copy on receive is implemented, it is determined by ENET_RXBUFF_NUM definition.
Also increase the TCP receive window by changing TCP_WND definition in the file lwipopts.h or on command line.
Make sure that TCP_WND is not larger than (number of receive buffers / TCP_MSS).
For RTOS applications, DEFAULT_THREAD_PRIO and TCPIP_THREAD_PRIO values can have effect on maximum throughput as well.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
This example uses 1G port(J4) as default. If want to test 100M port(J3), please set the macro BOARD_NETWORK_USE_100M_ENET_PORT to 1.

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
1. When the demo starts, the log would be seen on the terminal like:
		Initializing PHY...
        
        ************************************************
         IPERF example
        ************************************************
         IPv4 Address     : 192.168.0.102
         IPv4 Subnet mask : 255.255.255.0
         IPv4 Gateway     : 192.168.0.100
        ************************************************
        Please select one of the following modes to run IPERF with:

            1: TCP server mode (RX test)
            2: TCP client mode (TX test)
            3: UDP server mode (RX test)
            4: UDP client mode (TX test)

        Enter mode number:

2. Start the JPerf application, using the jperf-2.0.0/jperf.bat batch file.
    It can be downloaded here: https://sourceforge.net/projects/iperf/files/jperf/jperf%202.0.0/jperf-2.0.0.zip/download.
    When using Windows, replace the content of the jperf-2.0.0/bin folder with the files from the following zip: https://iperf.fr/download/windows/iperf-2.0.5b-win32.zip.
    When using Linux, iperf binary version 2.0.5 must be installed separately (possibly using package manager) and present on the system path.
3. To run lwIP IPERF in client mode, select "Server" radio button in JPerf and press the [Run iperf!] button.
4. To run lwIP IPERF in server mode, select "Client radio button and enter the 192.168.0.102 board IPv4 address
    to the "Server address" parameter in JPerf.
5. Enter the desired mode number into the terminal.
6. If server mode has been selected in the terminal (and client mode in JPerf), press the [Run iperf!] button now.
7. When the test is finished, the output log of JPerf would be seen like below,
	where occurrences of the symbol "N" would be replaced by actual measured values.
    The log will vary depending on the selected mode:
        bin/iperf.exe -s -P 0 -i 1 -p 5001 -f k
        ------------------------------------------------------------
        Server listening on TCP port 5001
        TCP window size: 63.0 KByte (default)
        ------------------------------------------------------------
        [  4] local 192.168.0.100 port 5001 connected with 192.168.0.102 port 49156
        [ ID] Interval       Transfer     Bandwidth
        [  4]  0.0- 1.0 sec  N    KBytes  N     Kbits/sec
        [  4]  1.0- 2.0 sec  N    KBytes  N     Kbits/sec
        [  4]  2.0- 3.0 sec  N    KBytes  N     Kbits/sec
        [  4]  3.0- 4.0 sec  N    KBytes  N     Kbits/sec
        [  4]  4.0- 5.0 sec  N    KBytes  N     Kbits/sec
        [  4]  5.0- 6.0 sec  N    KBytes  N     Kbits/sec
        [  4]  6.0- 7.0 sec  N    KBytes  N     Kbits/sec
        [  4]  7.0- 8.0 sec  N    KBytes  N     Kbits/sec
        [  4]  8.0- 9.0 sec  N    KBytes  N     Kbits/sec
        [  4]  0.0-10.0 sec  N    KBytes  N     Kbits/sec

8. Also, when the test is finished, the log would be seen on the terminal like below,
	where occurrences of the symbol "N" would be replaced by actual measured values.
    The log will vary depending on the selected mode:
        Enter mode number: 2
        Press SPACE to abort the test and return to main menu
        -------------------------------------------------
         TCP_DONE_CLIENT (TX)
         Local address : 192.168.0.102  Port 49156
         Remote address : 192.168.0.100  Port 5001
         Bytes Transferred N
         Duration (ms) N
         Bandwidth (kbitpsec) N

9. It is also possible to press the SPACE key when the test is running or finished.
    If it is pressed when test is in progress, the running test will be aborted
    and the main menu will appear. If the test is already finished, the main menu
    will appear directly. From the main menu, new test can be run.
