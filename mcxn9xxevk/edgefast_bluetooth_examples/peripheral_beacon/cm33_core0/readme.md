Overview
========
Application demonstrating the BLE Peripheral role, This application implements types of beacon applications .
Beacon: A simple application demonstrating the BLE Broadcaster role functionality by advertising Company Identifier, Beacon Identifier, UUID, A, B, C, RSSI.
Eddystone : The Eddystone Configuration Service runs as a GATT service on the beacon while it is connectable and allows configuration of the advertised data, the broadcast power levels, and the advertising intervals.
iBeacon: This simple application demonstrates the BLE Broadcaster role functionality by advertising an Apple iBeacon.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MCNX947-EVK board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.

Hardware rework guide:
The hardware should be reworked according to the Hardware Rework Guide for MCXN947-EVK with Direct Murata M.2 Module in document Hardware Rework Guide for EdgeFast BT PAL.

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Note:
For Peripheral DMA to be used, the application core must first gain TRDC access from edgelock FW, so this demo
must be runnning with edgelock FW alive.
Whole memory must be erased before this demo is flashed.
After downloaded binary into Quad SPI Flash and boot from Quad SPI Flash directly,
please reset the board by pressing SW3 or power off and on the board to run the application.

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
Form the app_bluetooth_config.h file we can select witch application is starting. Default Beacon (#define BEACON_APP 1) application is starting.
To start the Eddystone set #define EDDYSTONE 1 and others to 0. To start the iBeacon set #define IBEACON_APP 1 and others to 0.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If BEACON_APP is 1 (and IBEACON_APP and EDDYSTONE are set to 0) the Beacon application is started.
The beacons are non-connectable advertising packets that are sent on the three advertising channels. The latter contains the following fields.
• Company Identifier (2 bytes): 0x0025 (NXP ID as defined by the Bluetooth SIG).
• Beacon Identifier (1 byte): 0xBC (Allows identifying an NXP Beacon alongside with Company Identifier).
• UUID (16 bytes): Beacon sensor unique identifier.
• A (2 bytes): Beacon application data.
• B (2 bytes): Beacon application data.
• C (2 bytes): Beacon application data.
• RSSI at 1m (1 byte): Allows distance-based applications.

Because of the hard-coded values of Beacon UUID, the application is not suitable for production use, but is quite convenient for quick demonstrations of Beacon functionality.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If IBEACON_APP is 1 (and BEACON_APP and EDDYSTONE are set to 0) the iBeacon application is started.
This is a simple application demonstrates the BLE Broadcaster role functionality by advertising an Apple iBeacon. The calibrated RSSI @ 1 meter distance can be set using an IBEACON_RSSI build variable (e.g. IBEACON_RSSI=0xc8 for -56 dBm RSSI), or by manually editing the default value in the ibeacon.c file.

Because of the hard-coded values of iBeacon UUID, major, and minor, the application is not suitable for production use, but is quite convenient for quick demonstrations of iBeacon functionality.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If EDDYSTONE is 1 (and IBEACON_APP and BEACON_APP are set to 0) the Eddystone application is started.

EDDYSTONE application is demonstrating Eddystone Configuration Service

The Eddystone Configuration Service runs as a GATT service on the beacon while it is connectable and allows configuration of the advertised data, the broadcast power levels, and the advertising intervals. It also forms part of the definition of how Eddystone-EID beacons are configured and registered with a trusted resolver.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
