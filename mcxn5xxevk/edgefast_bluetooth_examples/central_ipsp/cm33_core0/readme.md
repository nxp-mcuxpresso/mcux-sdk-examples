Overview
========
Application demonstrating very basic BLE Central role functionality by scanning for other BLE devices and establishing a connection to the first one with a strong enough signal.
Except that this application specifically looks for IPSP Service and communicates between the devices that support IPSP is done using IPv6 packets over the Bluetooth Low Energy transport once connected.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Micro USB cable
- mcxn5xxevk board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.

Hardware rework guide
The hardware should be reworked according to the Hardware Rework Guide for MCXN547-EVK with Direct Murata M.2 Module in document Hardware Rework Guide for EdgeFast BT PAL.


Murata Solution Board settings
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf


Note
Whole memory must be erased before this demo is flashed.
After downloaded binary into Quad SPI Flash and boot from Quad SPI Flash directly,
please reset the board by pressing SW1 or power off and on the board to run the application.

If you want to get HCI log , please define macro CONGIF_BT_SNOOP to 1 in app_bluetooth_config.h , then connect JUMP26 [9,10] for extra power supply and connect the OTG with U-disk to the J27.
You will get a file named btsnoop on the U-disk.You can change its extension to .cfa , then open it with ComProbe Protocol Analysis System to view the HCI logs.
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
The demo does not require user interaction. The application will automatically start scanning and will connect to the first advertiser who is advertising the IPSP Service. After the L2CAP credit-based channel specified by the IPSP Profile is established, the application will send a predefined test message every 5 seconds through the channel. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Sending message...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
