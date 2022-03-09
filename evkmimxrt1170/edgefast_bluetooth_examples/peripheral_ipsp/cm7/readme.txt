Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the Internet Protocol Support GATT Service.


Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1170 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510-M.2
  - AzureWave AW-CM358-M.2
  - AzureWave AW-AM457-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - K32W061

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD .
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module with uSD-M2 adapter, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510MA, please change the macro to WIFI_IW416_BOARD_AW_AM510MA.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358MA, please change the macro to WIFI_88W8987_BOARD_AW_CM358MA.
If you want use the K32W061_TRANSCEIVER, please change the macro to K32W061_TRANSCEIVER.

Jumper settings for RT1170 (enables external 5V supply):
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
  - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkmimxrt1170 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:

----------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT1170    | PIN NAME OF RT1170 | GPIO NAME OF RT1170
----------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
----------------------------------------------------------------------------------------------

NOTE: With direct M2 connection (EAR00364/EAR00385) no additional cabling is needed.

AzureWave Solution Board settings
Jumper settings for AzureWave AW-AM457-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVK and AW-AM457-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT1170  | PIN NAME OF RT1170 | GPIO NAME OF RT1170
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
UART_RXD |  J10(pin 2)  |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
UART_CTS |  J10(pin 8)  |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
UART_RTS |  J10(pin 6)  |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
GND      |  J6(pin 7)   |   J26(pin 1)    |    GND             | GND
------------------------------------------------------------------------------------

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVK and AW-CM358MA in document Hardware Rework Guide for EdgeFast BT PAL.

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVK and AW-CM510MA in document Hardware Rework Guide for EdgeFast BT PAL.

For K32W061, the readme located in <sdk>/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/k32w061 explains how to flash the transceiver image and the hardware rework required.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | DK6 (K32W061) |   I.MXRT1170  | PIN NAME OF RT1170| GPIO NAME OF RT1170
------------------------------------------------------------------------------------
UART_TXD |  PIO(pin 8)   |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
UART_RXD |  PIO(pin 9)   |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
UART_RTS |  PIO(pin 6)   |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
UART_CTS |  PIO(pin 7)   |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
------------------------------------------------------------------------------------

Note:
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW4 or power off and on the board to run the application.
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
The demo does not require user interaction. The application will automatically start advertising the IPSP Service and it will accept the first connection request it receives. The application will perform the required setup for the L2CAP credit-based channel specified by the IPSP Profile. The application will display in console any message it receives from the peer through the L2CAP channel. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Received message: hello
Received message: hello
Received message: hello
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
