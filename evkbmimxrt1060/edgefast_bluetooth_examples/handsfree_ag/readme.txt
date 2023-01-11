Overview
========
This example demonstrates the HFP Ag basic functionality, currently support simulate an incoming call, and the call could be answered and ended.
The HFP Ag can connected a HFP HF device like headphone or device running HFP HF device.


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
  - AzureWave AW-AM510-uSD
  - AzureWave AW-CM358-uSD

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.

Jumper settings for RT1060-EVKB (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf

Jumper settings for Murata uSD-M.2 adapter:
  - J12 = 2-3: WLAN-SDIO & BT-PCM = 3.3V
  - J13 = 1-2: BT-UART & WLAN/BT-CTRL = 3.3V
  - J1 = 2-3: 3.3V from uSD connector

The following pins between the evkbmimxrt1060 board and Murata uSD-M.2 Adapter with Embedded Artists 1XK M.2 Module are connected using male-to-female jumper cables:
---------------------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
---------------------------------------------------------------------------------------------------------
BT_UART_TXD_HOST | J9(pin 1)  	   | J16(pin 1)      | LPUART3_RXD 	           | GPIO_AD_B1_07
BT_UART_RXD_HOST | J9(pin 2)  	   | J16(pin 2)      | LPUART3_TXD  	         | GPIO_AD_B1_06
BT_UART_RTS_HOST | J8(pin 3)  	   | J33(pin 3)      | LPUART3_CTS  	         | GPIO_AD_B1_04
BT_UART_CTS_HOST | J8(pin 4)  	   | J33(pin 4)      | LPUART3_RTS 	           | GPIO_AD_B1_05
GND              | J7(pin 7)  	   | J32(pin 7)	     | GND         	           | GND
---------------------------------------------------------------------------------------------------------

The pin connect for PCM interface as the following table,
-------------------------------------------------------------------------------------------------
PIN NAME | uSD-M.2 Adapter | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
-------------------------------------------------------------------------------------------------
PCM_IN   | J5(pin 1)       | J16(pin 5) 	   | SAI2_TXD      	         | GPIO_AD_B0_09
PCM_OUT  | J5(pin 3)       | TP11            | SAI2_RXD                | GPIO_AD_B0_08
PCM_SYNC | J5(pin 5)       | J2(pin 9)  	   | SAI2_RX_SYNC            | GPIO_AD_B0_07
PCM_CLK  | J5(pin 7)       | J10(pin 2)      | SAI2_RX_BCLK            | GPIO_AD_B0_06
GND      | J5(pin 15)      | J2(pin 20)      | GND                     | GND
-------------------------------------------------------------------------------------------------

AzureWave Solution Board settings
For AzureWave WIFI_IW416_BOARD_AW_AM510MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-AM510MA in document Hardware Rework Guide for EdgeFast BT PAL.

For AzureWave WIFI_88W8987_BOARD_AW_CM358MA-M.2, the hardware should be reworked according to the Hardware Rework Guide for MIMXRT1060-EVKB and AW-CM358MA in document Hardware Rework Guide for EdgeFast BT PAL.

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
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

The pin connect for PCM interface as the following table,
----------------------------------------------------------------------------------------------
PIN NAME | AW-AM510-uSD | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
----------------------------------------------------------------------------------------------
PCM_IN   | J11(pin 1)   | J16(pin 5)      | SAI2_TXD                | GPIO_AD_B0_09
PCM_OUT  | J11(pin 2)   | TP11            | SAI2_RXD                | GPIO_AD_B0_08   
PCM_SYNC | J11(pin 3)   | J2(pin 9)       | SAI2_RX_SYNC            | GPIO_AD_B0_07
PCM_CLK  | J11(pin 4)   | J10(pin 2)      | SAI2_RX_BCLK            | GPIO_AD_B0_06
GND      | J11(pin 6)   | J2(pin 20)      | GND                     | GND
----------------------------------------------------------------------------------------------
Note: AzureWave AW-AM510-uSD Module without fireware runing will plug the RT jlink pin, fail to download image, need disconnect J9(pin 2)/TP11 pin.

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 2-3: VIO 3.3V (Voltage level of SDIO pins is 3.3V)

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

The pin connect for PCM interface as the following table,
----------------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD | I.MXRT1060-EVKB | PIN NAME OF RT1060-EVKB | GPIO NAME OF RT1060-EVKB
----------------------------------------------------------------------------------------------
PCM_IN   | J11(pin 1)   | J16(pin 5)      | SAI2_TXD                | GPIO_AD_B0_09
PCM_OUT  | J11(pin 2)   | TP11            | SAI2_RXD                | GPIO_AD_B0_08
PCM_SYNC | J11(pin 3)   | J2(pin 9)       | SAI2_RX_SYNC            | GPIO_AD_B0_07
PCM_CLK  | J11(pin 4)   | J10(pin 2)      | SAI2_RX_BCLK            | GPIO_AD_B0_06
GND      | J11(pin 6)   | J2(pin 20)      | GND                     | GND
----------------------------------------------------------------------------------------------

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW7 or power off and on the board to run the application.
When J2(pin 9) is used as SAI2_RX_SYNC, please remove jumper J9 of RT1060-EVKB.
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
The log below shows the output of the example in the terminal window. 
USB Host stack successfully initialized
Bluetooth initialized

SHELL build: Mar  1 2021
Copyright  2020  NXP

>>
the bellow commands have been supported:
"bt": BT related function
  USAGE: bt [discover|connect|disconnect|delete]
    discover             start to find BT devices
    connect              connect to the device that is found, for example: bt connect n (from 1)
    openaudio            open audio connection without calls
    closeaudio           close audio connection without calls 
    sincall              start an incoming call.
    aincall              accept the call.
    eincall              end an call.
    set_tag              set phone num tag, for example: bt set_tag 123456789.
    select_codec         codec select for codec Negotiation, for example: bt select_codec 2, it will select the codec 2 as codec.
    set_mic_volume       update mic Volume, for example: bt set_mic_volume 14.
    set_speaker_volume   update Speaker Volume, for example: bt set_speaker_volume 14.
    stwcincall           start multiple an incoming call.
    disconnect           disconnect current connection.
    delete               delete all devices. Ensure to disconnect the HCI link connection with the peer device before attempting to delete the bonding information.
1) "discover" start to find BT devices, it will list all device can be found, usage
2) "connect" is used to connect to the device that is found, for example: bt connect n (from 1), usage :
   bt connect 1
3) "openaudio" is used to open audio connection without calls
4) "closeaudio" is used to close audio connection without calls 
5) "sincall" is used to start an incoming call
6) "aincall" is used to accept an incoming call
7) "eincall" is used to end or reject an incoming call
8) "set_tag" is used to set phone num tag, for example: bt set_tag 123456789
9) "select_codec" is used to  codec select for codec Negotiation, for example: bt select_codec 2, it will select the codec 2 as codec, usage:
   bt select_codec 2
10) "set_mic_volume" is used to set mic volume, the value  is from 1 to 15, usage as:
    bt set_mic_volume 8
11) "set_speaker_volume" is used to set speaker volume, the value is from 1 to 15, usage as:
    bt set_speaker_volume 8
12) "stwcincall"  to start multiple an incoming call, need run "sincall" is used to start an incoming call before run the command
13) "disconnect"  to disconnect current connection
14) "delete" is used to delete all devices. Ensure to disconnect the HCI link connection with the peer device before attempting to delete the bonding information.

Note:
There is a short noise can be heard at headset at the begin audio streaming when in running HFP Ag . 
The codec power on pop noise cannot eliminate.

