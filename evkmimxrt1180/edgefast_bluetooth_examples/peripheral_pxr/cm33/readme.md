Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the Proximity Reporter (including LLS, IAS, and TPS) GATT Service.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1180 board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.
If you want to use Embedded Artists Type 2EL module(Rev-A1 M2 only), please change the macro to WIFI_IW612_BOARD_MURATA_2EL_M2.

Jumper settings for RT1180:
connect J1 1-2
connect J2 with external power(controlled by SW1)
connect J76 2-3
connect J57 2-3

Hardware rework guide:
The hardware should be reworked according to the hardware rework guide for evkmimxrt1180 and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
The hardware should be reworked according to the hardware rework guide for evkmimxrt1180 and Murata 1ZM M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
The rework guide for evkmimxrt1180 and Murata 2EL M.2 Adapter is same as evkmimxrt1180 and Murata 1ZM M.2 Adapter.

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
For Peripheral DMA to be used, the application core must first gain TRDC access from edgelock FW, so this demo
must be runnning with edgelock FW alive.
Whole memory must be erased before this demo is flashed.
After downloaded binary into Quad SPI Flash and boot from Quad SPI Flash directly,
please reset the board by pressing SW3 or power off and on the board to run the application.
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
The demo does not require user interaction. The application will automatically start advertising the Link Loss Service and it will accept the first connection request it receives. The application is then ready to process operations from the peer.

The application will initially set the default levels for the Link Loss Alert and the Immediate Alert. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Locally setting Immediate Alert...
ALERT: OFF
Locally setting Link Loss Alert Level to OFF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Proximity Monitor peer will trigger or stop the Immediate Alert on the application depending on the connection RSSI. Example output:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Monitor is setting Immediate Alert...
ALERT: HIGH
Monitor is setting Immediate Alert...
ALERT: OFF
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the connection with the Proximity Monitor is timed out, the Link Loss Alert will be triggered with the level previously set by the Monitor. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Link Loss Alert Triggered...
ALERT: HIGH
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
