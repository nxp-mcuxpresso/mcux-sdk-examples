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
