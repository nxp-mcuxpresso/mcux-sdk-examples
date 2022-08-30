Overview
========
Application demonstrating very basic BLE Central role functionality by scanning for other BLE devices and establishing a connection to the first one with a strong enough signal.
Except that this application specifically looks for Proximity Reporter.


Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510-uSD
  - AzureWave AW-AM457-uSD
  - AzureWave AW-CM358-uSD
  - K32W061
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want use the K32W061_TRANSCEIVER, please change the macro to K32W061_TRANSCEIVER.
If you want use the Murata Type 1ZM module, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want use the Murata Type 1XK module, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT1050   | PIN NAME OF RT1050 | GPIO NAME OF RT1050
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT1050   | PIN NAME OF RT1050 | GPIO NAME OF RT1050
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------


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
The application will automatically start scanning and will connect to the first advertiser who is advertising the Link Loss Service. If the connection is successful, the application performs service discovery to find the characteristics of the Link Loss Service, as well as additional services and characteristics specified by the Proximity Profile, such as Immediate Alert and Tx Power services.

If the Tx Power service and its characteristics have been discovered, the application will read the peer's Tx power and display it. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Read successful - Tx Power Level: 20
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the Immediate Alert service and its characteristics have been discovered, the application will continuously monitor the connection RSSI and will trigger or stop the Immediate Alert on the peer when the value is crossing a preset threshold in either direction.

After the mandatory Link Loss service is discovered, the application will write the Link Loss Alert Level on the peer as HIGH_ALERT. To trigger the Link Loss Alert on the peer, the connection will have to be timed out. The user can trigger this by simply resetting the board (press the RST button).
