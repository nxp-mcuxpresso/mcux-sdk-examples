Overview
========
Application demonstrating how to use the a2dp source feature.


Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1060 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-AM510-uSD
  - AzureWave AW-AM457-uSD
  - AzureWave AW-CM358-uSD
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want use the Embedded Artists Type 1ZM module, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want use the Embedded Artists Type 1XK module, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.

Jumper settings for RT1060 (enables external 5V supply):
remove  J1 5-6
connect J1 1-2
connect J2 with external power(controlled by SW1)


Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
  - Both J12 & J13 = 1-2: WLAN-SDIO = 1.8V; and BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkmimxrt1060 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:

----------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
----------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
----------------------------------------------------------------------------------------------


AzureWave Solution Board settings
Jumper settings for AzureWave AW-AM457-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVK and AW-AM457-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVK and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVK and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT1060   | PIN NAME OF RT1060 | GPIO NAME OF RT1060
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW9 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Provide 5V voltage for the target board.

4.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

5.  Download the program to the target board.

6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USB Host stack successfully initialized
Bluetooth initialized

SHELL build: Mar  2 2021
Copyright  2021  NXP

>> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list
2 input "bt discover" to discover connctable bluetooth devices
3 input "bt connect [index]" to create basic bluetooth connection with the discovered device
4 the music start playing after connection success.
