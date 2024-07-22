Overview
========
Application demonstrating the BLE Peripheral role, This application implements types of beacon applications .
Beacon: A simple application demonstrating the BLE Broadcaster role functionality by advertising Company Identifier, Beacon Identifier, UUID, A, B, C, RSSI.
Eddystone : The Eddystone Configuration Service runs as a GATT service on the beacon while it is connectable and allows configuration of the advertised data, the broadcast power levels, and the advertising intervals.
iBeacon: This simple application demonstrates the BLE Broadcaster role functionality by advertising an Apple iBeacon.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510-uSD
  - AzureWave AW-AM457-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - K32W061
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2EL M.2 Module (EAR00409)

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want to use the AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want to use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want to use the Murata Type 1ZM module, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want to use the Murata Type 1XK module, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.
If you want to use the K32W061_TRANSCEIVER, please change the macro to K32W061_TRANSCEIVER.
If you want to use the Murata Type 2EL module, please change the macro to WIFI_IW612_BOARD_MURATA_2EL_USD.

Jumper settings for Murata uSD-M.2 adapter:
  - J12 = 1-2: WLAN-SDIO = 1.8V
  - J13 = 1-2: BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkbmimxrt1050 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:
------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter | I.MXRT1050 | PIN NAME OF RT1050 | GPIO NAME OF RT1050
------------------------------------------------------------------------------------------
BT_UART_TXD_HOST | J9(pin 1)       | J22(pin 1) | LPUART3_RXD        | GPIO_AD_B1_07
BT_UART_RXD_HOST | J9(pin 2)       | J22(pin 2) | LPUART3_TXD        | GPIO_AD_B1_06
BT_UART_RTS_HOST | J8(pin 3)       | J23(pin 3) | LPUART3_CTS        | GPIO_AD_B1_04
BT_UART_CTS_HOST | J8(pin 4)       | J23(pin 4) | LPUART3_RTS        | GPIO_AD_B1_05
------------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power supply from uSD connector)
  - J4 2-3: 3.3V VIO

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
-------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD | I.MXRT1050 | PIN NAME OF RT1050 | GPIO NAME OF RT1050
-------------------------------------------------------------------------------
UART_TXD | J10(pin 4)   | J22(pin 1) | LPUART3_RXD        | GPIO_AD_B1_07
UART_RXD | J10(pin 2)   | J22(pin 2) | LPUART3_TXD        | GPIO_AD_B1_06
UART_RTS | J10(pin 6)   | J23(pin 3) | LPUART3_CTS        | GPIO_AD_B1_04
UART_CTS | J10(pin 8)   | J23(pin 4) | LPUART3_RTS        | GPIO_AD_B1_05
GND      | J6(pin 7)    | J25(pin 7) | GND                | GND
-------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
-------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD | I.MXRT1050 | PIN NAME OF RT1050 | GPIO NAME OF RT1050
-------------------------------------------------------------------------------
UART_TXD | J10(pin 4)   | J22(pin 1) | LPUART3_RXD        | GPIO_AD_B1_07
UART_RXD | J10(pin 2)   | J22(pin 2) | LPUART3_TXD        | GPIO_AD_B1_06
UART_RTS | J10(pin 6)   | J23(pin 3) | LPUART3_CTS        | GPIO_AD_B1_04
UART_CTS | J10(pin 8)   | J23(pin 4) | LPUART3_RTS        | GPIO_AD_B1_05
GND      | J6(pin 7)    | J25(pin 7) | GND                | GND
-------------------------------------------------------------------------------

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
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
