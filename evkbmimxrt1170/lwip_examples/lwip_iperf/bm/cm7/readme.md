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

The example has been tested with iperf 2.1.9, which is not compatible with iperf 2.0.5 used in earlier SDK releases.
Aside from the lack of reverse mode, which has been added in the version 2.1.0, the format of the settings sent
between a client and a server is not exactly the same. Therefore please use iperf 2.1.0 or later with this example.
Iperf 2.1.x can be downloaded from here: https://sourceforge.net/projects/iperf2/files/

To experiment with the receive throughput, try to increase the number of receive buffers.
The porting layer for NXP ethernet peripherals implements zero-copy on receive and the number of buffers
is determined by ENET_RXBUFF_NUM definition (or NETC_RXBUFF_NUM in case when NETC ethernet peripheral is used).
Increasing the PBUF_POOL_SIZE option will not help with receive throughput in this case.
Also increase the TCP receive window by changing TCP_WND definition in the file lwipopts.h or on command line.
Make sure that TCP_WND is not larger than (number of receive buffers / TCP_MSS).
Moving of frequently executed code to RAM may also help.
For RTOS applications, DEFAULT_THREAD_PRIO and TCPIP_THREAD_PRIO values can have effect on maximum throughput as well.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- Network cable RJ45 standard
- MIMXRT1170-EVKB board
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

            0: TCP server mode (RX test)
            1: TCP client mode (TX only test)
            2: TCP client reverse mode (RX only test)
            3: TCP client dual mode (TX and RX in parallel)
            4: TCP client tradeoff mode (TX and RX sequentially)
            5: UDP server mode (RX test)
            6: UDP client mode (TX only test)
            7: UDP client reverse mode (RX only test)
            8: UDP client dual mode (TX and RX in parallel)
            9: UDP client tradeoff mode (TX and RX sequentially)

        Enter mode number:

2. Start the server (on the PC or on the board) first.
    If you want to run server on the board and client on the PC, enter the number of the desired server mode (TCP or UDP) into the terminal.
    If you want to run server on the PC and client on the board, start iperf application on the PC:
        For TCP: iperf.exe -s -p 5001 -i 1 -f k
        For UDP: iperf.exe -s -p 5001 -i 1 -f k -u
3. Start the client.
    If the PC is the client, start iperf application on the PC:
        For TCP: iperf.exe -c 192.168.0.102 -p 5001 -P 1 -i 1 -f k -t 10
        For UDP: iperf.exe -c 192.168.0.102 -p 5001 -P 1 -i 1 -f k -t 10 -u -b 100M
        Parameters like -d, -r, -R could be appended to the command for dual, tradeoff or reverse test modes.
        The iperf implementation in lwIP does not support all 2.1.x features, so it does not make
        sense to use some of the modes which require board cooperation, like --full-duplex.
    If the board is the client, enter the number of the desired client mode into the terminal.
4. When the test is finished, the output log of iperf.exe would be seen like below,
    where occurrences of the symbol "N" would be replaced by actual measured values.
    The log will vary depending on the selected mode:
        iperf.exe -c 192.168.0.102 -p 5001 -P 1 -i 1 -f k -t 10
        ------------------------------------------------------------
        Client connecting to 192.168.0.102, TCP port 5001
        TCP window size: 64.0 KByte (default)
        ------------------------------------------------------------
        [  1] local 192.168.0.100 port 51090 connected with 192.168.0.102 port 5001
        [ ID] Interval       Transfer     Bandwidth
        [  1] 0.00-1.00 sec  N KBytes     N Kbits/sec
        [  1] 1.00-2.00 sec  N KBytes     N Kbits/sec
        [  1] 2.00-3.00 sec  N KBytes     N Kbits/sec
        [  1] 3.00-4.00 sec  N KBytes     N Kbits/sec
        [  1] 4.00-5.00 sec  N KBytes     N Kbits/sec
        [  1] 5.00-6.00 sec  N KBytes     N Kbits/sec
        [  1] 6.00-7.00 sec  N KBytes     N Kbits/sec
        [  1] 7.00-8.00 sec  N KBytes     N Kbits/sec
        [  1] 8.00-9.00 sec  N KBytes     N Kbits/sec
        [  1] 9.00-10.00 sec  N KBytes     N Kbits/sec
        [  1] 0.00-10.11 sec  N KBytes     N Kbits/sec

5. Also, when the test is finished, the log would be seen on the terminal like below,
    where occurrences of the symbol "N" would be replaced by actual measured values.
    The log will vary depending on the selected mode:
        Enter mode number: 0
        Press SPACE to abort the test and return to main menu
        New TCP client (settings flags 0x40010078)

        -------------------------------------------------
         TCP_DONE_SERVER (RX)
         Local address : 192.168.0.102  Port 5001
         Remote address : 192.168.0.100  Port 51090
         Bytes Transferred N
         Duration (ms) N
         Bandwidth (kbitpsec) N

6. It is also possible to press the SPACE key when the test is running or finished.
    If it is pressed when test is in progress, the running test will be aborted
    and the main menu will appear. If the test is already finished, the main menu
    will appear directly. From the main menu, new test can be run.
