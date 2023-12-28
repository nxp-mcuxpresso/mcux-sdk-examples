Overview
========
The Wireless UART bridge demo implements both the GATT client and server for the custom Wireless UART 
profile and services. It contains a custom service that implements a custom writable ASCII Char 
characteristic that holds the character written by the peer device. The Kinetis BLE Toolbox on the 
mobile phone has similar functionality. The application on the board plays a role as a GAP peripheral. 
It enters GAP General Discoverable Mode and waits for a GAP central node to connect. Once the GAP 
central node (mobile phone) connected, both board and mobile phone can send messages to the peer, and 
the messages will be displayed on board UART terminal or the text area of the mobile application.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- FRDM-KW41Z board
- J-Link Debug Probe
- 5V power supply
- Personal Computer
- Mobile phone that support Bluetooth Low Energy

Board settings
==============
The example requires connecting UART RX and TX between MCIMX7ULP-EVK and FRDM-KW41Z boards.
The connection should be set as following:
- MCIMX7ULP-EVK J10-1(On base board), FRDM-KW41Z J1-1 connected
- MCIMX7ULP-EVK J10-2(On base board), FRDM-KW41Z J1-2 connected
- MCIMX7ULP-EVK J7-5 (On base board), FRDM-KW41Z J3-5 connected
- MCIMX7ULP-EVK J7-7 (On base board), FRDM-KW41Z J3-7 connected
- FRDM-KW41Z J30-1, J30-2 connected
- FRDM-KW41Z J31-1, J31-2 connected

#### Please note this application can only run with RAM link file! And gcc debug target
exceeds the RAM size so only gcc release target is available!
If run it in QSPI flash, there's high latency when instruction cache miss. The demo uses LPART
to communicate with KW41 UART port to control the BLE stack. However frequent cache misses lead
to UART data loss which makes the state machine abnormal. ####

Prepare the Demo
================
KW41 image preparation:
1.  Download Kinetis KW41Z Connectivity Software(REV 1.0.2) from nxp.com
2.  Build boards\frdmkw41z\wireless_examples\bluetooth\ble_fsci_black_box\bare_metal application with "release" configuration
3.  Write ble_fsci_black_box application to KW41Z internal flash.

Mobile application preparation:
1.  Install Kinetis BLE Toolbox on Android OS handheld devices that support Bluetooth Low Energy. The application can be found on Google Play Store.
2.  Run Kinetis BLE Toolbox, select Wireless Console/UART.

i.MX7ULP image preaparation and run:
1.  Generate i.MX7ULP image file with imgutil and write to QSPI flash with U-Boot. For details, please refer to Getting Started with SDK v.2.0 for i.MX 7ULP Derivatives (Doc No: SDK20IMX7ULPGSUG)
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal for A7 core with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Connect 5V power supply and J-Link Debug Probe to the board, connect FRDM-KW41Z to board, switch SW1 to power on the board.
5.  Hit any key to stop autoboot in the terminals, then enter to U-Boot command
6.  Boot Linux
7.  After login, make sure imx_rpmsg_tty kernel module is inserted (lsmod) or insert it (modprobe imx_rpmsg_tty).
8.  Enter "cd /","./unit_tests/Remote_Processor_Messaging/mxc_mcc_tty_test.out /dev/ttyRPMSG1 115200 R 100 1000 & " in sequence.



Running the demo
================
When running the demo, the debug console shows the BLE communication status and user can connect to the board via mobile phone, and once connected, messages can be transfered between board and mobile phone.
The transfered information can be showed on the A7 core terminal, users can also use the command on A7 core terminal,eg, "echo hello world! > /dev/ttyRPMSG1",then mobile can receive "hello world!".

Note the following information shows on M4 core terminal.

~~~~~~~~~~~~~~~~~~~~~
gAdvertisingStateChanged_c
gConnEvtConnected_c
~~~~~~~~~~~~~~~~~~~~~
