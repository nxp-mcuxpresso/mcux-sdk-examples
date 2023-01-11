Overview
========
Application demonstrating how to use the SPP feature.


Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1060 board
- Personal Computer
- One of the following modules:
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - AzureWave AW-AM510-uSD
  - AzureWave AW-CM358-uSD

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.
If you want to use Embedded Artists Type 1ZM module with uSD-M2 adapter, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.

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
  - J12 = 1-2: WLAN-SDIO & BT-PCM = 1.8V
  - J13 = 1-2: BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkbmimxrt1060 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM M.2 Module or 1XK M.2 Module are connected using male-to-female jumper cables:
---------------------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
---------------------------------------------------------------------------------------------------------
BT_UART_TXD_HOST | J9(pin 1)  	   | J16(pin 1)      | LPUART3_RXD 	           | GPIO_AD_B1_07
BT_UART_RXD_HOST | J9(pin 2)  	   | J16(pin 2)      | LPUART3_TXD  	         | GPIO_AD_B1_06
BT_UART_RTS_HOST | J8(pin 3)  	   | J33(pin 3)      | LPUART3_CTS  	         | GPIO_AD_B1_04
BT_UART_CTS_HOST | J8(pin 4)  	   | J33(pin 4)      | LPUART3_RTS 	           | GPIO_AD_B1_05
GND              | J7(pin 7)  	   | J32(pin 7)	     | GND         	           | GND
---------------------------------------------------------------------------------------------------------

AzureWave Solution Board settings
For AzureWave WIFI_IW416_BOARD_AW_AM510MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-AM510MA in document Hardware Rework Guide for EdgeFast BT PAL.

For AzureWave WIFI_88W8987_BOARD_AW_CM358MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-CM358MA in document Hardware Rework Guide for EdgeFast BT PAL.

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

The hardware should be reworked according to the hardware rework guide for evkbmimxrt1060 and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
----------------------------------------------------------------------------------------------
PIN NAME | AW-AM510-uSD | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
----------------------------------------------------------------------------------------------
UART_TXD | J10(pin 4)   | J16(pin 1)      | LPUART3_RXD             | GPIO_AD_B1_07
UART_RXD | J10(pin 2)   | J16(pin 2)      | LPUART3_TXD             | GPIO_AD_B1_06
UART_RTS | J10(pin 6)   | J33(pin 3)      | LPUART3_CTS             | GPIO_AD_B1_04
UART_CTS | J10(pin 8)   | J33(pin 4)      | LPUART3_RTS             | GPIO_AD_B1_05
GND      | J6(pin 7)    | J32(pin 7)      | GND                     | GND
----------------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the hardware rework guide for evkbmimxrt1060 and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
----------------------------------------------------------------------------------------------
PIN NAME | AW-CM358-uSD | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
----------------------------------------------------------------------------------------------
UART_TXD | J10(pin 4)   | J16(pin 1)      | LPUART3_RXD             | GPIO_AD_B1_07
UART_RXD | J10(pin 2)   | J16(pin 2)      | LPUART3_TXD             | GPIO_AD_B1_06
UART_RTS | J10(pin 6)   | J33(pin 3)      | LPUART3_CTS             | GPIO_AD_B1_04
UART_CTS | J10(pin 8)   | J33(pin 4)      | LPUART3_RTS             | GPIO_AD_B1_05
GND      | J6(pin 7)    | J32(pin 7)      | GND                     | GND
----------------------------------------------------------------------------------------------

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW7 or power off and on the board to run the application.
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
Bluetooth initialized
BR/EDR set connectable and discoverable done

Copyright  2020  NXP

>> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1.  Procedure to run SPP server
1.1 input "help" to show command list
1.2 input "spp register 3" to register spp server channel 3, wait for spp connection
1.3 After spp connection on channel 3 is created, input "spp send [1|2|3|4]" to send data
1.4 After peer device send "spp set_pn client 3", input "spp get_pn server 3" to test pn command
1.5 After peer device send "spp set_port client 5", input "spp register 5" to register spp server channel 5 and wait for spp connection
1.6 After spp connection on channel 5 is created, input "spp get_port server 5" to test remote port negotiation command after spp connection is created
1.7 input "spp handle" to show current active spp handle
1.8 input "spp switch 0" to select the first spp handle
1.9 input "spp disconnect" to disconnect with peer device

2.  Procedure to run SPP client
2.1 input "bt discover" to discover connctable bluetooth device
2.2 input "bt connect [index]" to create basic bluetooth connection with the discovered device
2.3 input "spp discover" to discover registered spp server channel in peer device
2.4 input "spp connect 3" to create spp connection on channel 3 with peer device
2.5 After spp connection on channel 3 is created, input "spp send [1|2|3|4]" to send data
2.6 After spp connection on channel 3 is created, input "spp send_rls" to test remote line status command
2.7 After spp connection on channel 3 is created, input "spp send_msc" to test modem status command
2.8 After spp connection on channel 3 is created, input "spp set_pn client 3" to test parameter command
2.9 input "spp get_port client 5" to test remote port negotiation command before spp connection on channel 5 is created
2.10 input "spp set_port client 5" to test remote port negotiation command before spp connection on channel 5 is created
2.11 input "spp connect 5" to create spp connection on channel 5 with peer device
2.12 input "spp get_port client 5" to test remote port negotiation command after spp connection on channel 5 is created
