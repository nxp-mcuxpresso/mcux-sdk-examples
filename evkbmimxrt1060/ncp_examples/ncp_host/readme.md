Overview
========
This is a MCU bridge app demo. Need to use with ncp_bridge application.




SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Prepare the Demo
================
1.  Generate ncp_host_app application. 
    For example: go to bin/generator/batch_v2.
                 run 'ruby all_rdrw610.rb -p ncp_host_app' to generate ncp_host_app.
2.  Compile ncp_host_app through IAR/ARMGCC tool chain.
3.  Download the program to board A.
4.  Download ncp_bridge application to board B.
5.  Connect board A and board B with FLEXCOMM0.
6.  Connect board A/B to Windows/Ubuntu device through MCU-LINK.
7.  Open serial port of board A(MCU-LINK) in Windows/Ubuntu device.


Running the demo
================
1.  Enter 'help' in the terminal to get help information.

    help
    wlan-scan
    get-scan-result
    wlan-connect <ssid>
    get-scan-result
    wlan-disconnect
    ping [c <packet_count>] <ip_address>
    get-ping-result
    iperf [s|su|c <host>|cu <host>|a|] [options]
    get-iperf-result

2.  Enter command

    #wlan-scan

    #get-scan-result

     3 networks found:
      94:10:3E:02:60:F0  [nxp_mrvl]
              channel: 1
              rssi: -25 dBm
              security: OPEN

      94:10:3E:02:60:F1  [nxp_mrvl_5ghz]
              channel: 36
              rssi: -39 dBm
              security: WPA/WPA2 Mixed

      90:72:40:21:B3:1A  [apple_g]
              channel: 11
              rssi: -51 dBm
              security: WPA3 SAE


    #wlan-connect net-5g

    #get-connect-result

     Already connected to an AP:
     SSID = {net-5g}
     IPv4 Address: {192.168.0.97}


    #wlan-disconnect

    Already disconnect to network

    #ping 192.168.0.1 c 20
 
    #get-ping-result

    ---  ping statistics  ---
    20 packets transmitted, 20 packets received, 0% packets loss

    #iperf c 192.168.0.1 t 20
    
    #get-iperf-result

    ---------------------
    TCP_DONE_CLIENT (TX)
    Local address : 192.168.0.97  Port : 49153
    Remote address : 192.168.0.128  Port : 5001
    Bytes Transferred 106642804
    Duration (ms) 20000
    BandWidth (Mbit/sec) 42
