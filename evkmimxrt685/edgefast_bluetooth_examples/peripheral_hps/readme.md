Overview
========
Application demonstrating the BLE Peripheral role, except that this application specifically exposes the HTTP Proxy GATT Service.


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
- evkmimxrt685 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510-uSD
  - AzureWave AW-AM457-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 2EL M.2 Module (EAR00409)
  - K32W061

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave AW-AM510-uSD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave AW-AM457-uSD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want use the AzureWave AW-CM358-uSD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD .
If you want to use Embedded Artists Type 1ZM module with uSD-M2 adapter, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want to use Embedded Artists Type 2EL module with uSD-M2 adapter, please change the macro to WIFI_IW612_BOARD_MURATA_2EL_USD.
If you want use the K32W061_TRANSCEIVER, please change the macro to K32W061_TRANSCEIVER.

Jumper settings for RT685:
connect J6 with external power

Jumper settings for evkmimxrt685 Module:
  - JP12 2-3: 3.3V VDDIO

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
  - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkmimxrt685 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:

------------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT685    |  PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J27(pin 1)   |    USART4_RXD      | FC4_RXD_SDA_MOSI_DATA
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J27(pin 2)   |    USART4_TXD      | FC4_TXD_SCL_MISO_WS
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J47(pin 9)   |    USART4_CTS      | FC4_CTS_SDA_SSEL0
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J27(pin 5)   |    USART4_RTS      | FC4_RTS_SCL_SSEL1
------------------------------------------------------------------------------------------------

AzureWave Solution Board settings
Jumper settings for AzureWave AW-AM457-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-AM457-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT685   | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO
Jumper settings for evkmimxrt685 Module:
  - JP12 2-3: 3.3V VDDIO

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT685   | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT685   | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

For K32W061, the readme located in <sdk>/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller/k32w061 explains how to flash the transceiver image and the hardware rework required.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | DK6 (K32W061) |   I.MXRT685   | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  PIO(pin 8)   |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  PIO(pin 9)   |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  PIO(pin 6)   |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  PIO(pin 7)   |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
------------------------------------------------------------------------------------
Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary into qspiflash and boot from qspiflash directly,
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
The demo does not require user interaction. The application will automatically start advertising the HTTP Proxy Service and it will accept the first connection request it receives. The application is then ready to process HTTP requests from the peer. The application simulates processing of the HTTP request. It will always return HTTP Status Code 500 and preset values for HTTP Headers and HTTP Body. Example output:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Processing request...
Request processed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
