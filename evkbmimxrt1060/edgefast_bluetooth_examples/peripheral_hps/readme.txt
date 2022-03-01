Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the HTTP Proxy GATT Service.


Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1060 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510MA
  - AzureWave AW-CM358MA
  - AzureWave AW-AM510-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - K32W061

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510MA-M.2, please change the macro to WIFI_IW416_BOARD_AW_AM510MA.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358MA-M.2, please change the macro to WIFI_88W8987_BOARD_AW_CM358MA.
If you want use the K32W061_TRANSCEIVER, please change the macro to K32W061_TRANSCEIVER.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module with uSD-M2 adapter, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.

Jumper settings for RT1060 (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
  - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkbmimxrt1060 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:

----------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
----------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J16(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J16(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J33(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J33(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
----------------------------------------------------------------------------------------------


AzureWave Solution Board settings
For AzureWave WIFI_IW416_BOARD_AW_AM510MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-AM510MA in document Hardware Rework Guide for EdgeFast BT PAL.

For AzureWave WIFI_88W8987_BOARD_AW_CM358MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-CM358MA in document Hardware Rework Guide for EdgeFast BT PAL.

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the hardware rework guide for evkmimxrt1060 and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J16(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J16(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J33(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J33(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J32(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the hardware rework guide for evkmimxrt1060 and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J16(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J16(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J33(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J33(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J32(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

For K32W061, the readme located in <sdk>/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/k32w061 explains how to flash the transceiver image and the hardware rework required.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | DK6 (K32W061) |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  PIO(pin 8)   |   J16(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  PIO(pin 9)   |   J16(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  PIO(pin 6)   |   J33(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  PIO(pin 7)   |   J33(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
------------------------------------------------------------------------------------

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW7 or power off and on the board to run the application.
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
The demo does not require user interaction. The application will automatically start advertising the HTTP Proxy Service and it will accept the first connection request it receives. The application is then ready to process HTTP requests from the peer. The application simulates processing of the HTTP request. It will always return HTTP Status Code 500 and preset values for HTTP Headers and HTTP Body. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Processing request...
Request processed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
