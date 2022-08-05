Overview
========
This example demonstrates the HFP HF basic functionality, the HFP device support be connected to a HFP AG like a mobile phone or a 
board running a HFP AG application. And the HF example support accept/reject/End the incoming call from HFP AG. 



Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

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

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave AW-AM510-uSD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave AW-AM457-uSD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want use the AzureWave AW-CM358-uSD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want to use Embedded Artists Type 1XK module with uSD-M2 adapter, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD .
If you want to use Embedded Artists Type 1ZM module with uSD-M2 adapter, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.

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
  - JP7  1-2
  - JP8  1-2

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-AM457-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

The pin connect for PCM interface as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM457-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
PCM_IN   |  J9(pin 1)   |   J47(pin 7)   |      I2S2_TXD     | FC2_RXD_SDA_MOSI_DATA
PCM_OUT  |  J9(pin 2)   |   J28(pin 4)   |      I2S5_RXD     | FC5_RXD_SDA_MOSI_DATA
PCM_SYNC |  J9(pin 3)   |   J28(pin 5)   |      I2S5_WS      | FC5_TXD_SCL_MISO_WS
PCM_CLK  |  J9(pin 4)   |   J28(pin 6)   |      I2S5_SCK     | FC5_SCK
GND      |  J9(pin 6)   |   J29(pin 7)   |      GND          | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM510-uSD Module:
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO
Jumper settings for evkmimxrt685 Module:
  - JP12 2-3: 3.3V VDDIO
  - JP7  1-2
  - JP8  1-2

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

The pin connect for PCM interface as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
PCM_IN   |  J11(pin 1)  |   J47(pin 7)   |      I2S2_TXD     | FC2_RXD_SDA_MOSI_DATA
PCM_OUT  |  J11(pin 2)  |   J28(pin 4)   |      I2S5_RXD     | FC5_RXD_SDA_MOSI_DATA   
PCM_SYNC |  J11(pin 3)  |   J28(pin 5)   |      I2S5_WS      | FC5_TXD_SCL_MISO_WS
PCM_CLK  |  J11(pin 4)  |   J28(pin 6)   |      I2S5_SCK     | FC5_SCK
GND      |  J11(pin 6)  |   J29(pin 7)   |      GND          | GND
------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-CM358-uSD Module:
  - J2 1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4 1-2: VIO 1.8V (Voltage level of SDIO pins is 1.8V)

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT685-EVK and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J27(pin 1)   |    USART4_RXD     | FC4_RXD_SDA_MOSI_DATA
UART_RXD |  J10(pin 2)  |   J27(pin 2)   |    USART4_TXD     | FC4_TXD_SCL_MISO_WS
UART_RTS |  J10(pin 6)  |   J47(pin 9)   |    USART4_CTS     | FC4_CTS_SDA_SSEL0
UART_CTS |  J10(pin 8)  |   J27(pin 5)   |    USART4_RTS     | FC4_RTS_SCL_SSEL1
GND      |  J6(pin 7)   |   J29(pin 6)   |    GND            | GND
------------------------------------------------------------------------------------

The pin connect for PCM interface as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT685    | PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------
PCM_IN   |  J11(pin 1)  |   J47(pin 7)   |     I2S2_TXD      | FC2_RXD_SDA_MOSI_DATA
PCM_OUT  |  J11(pin 2)  |   J28(pin 4)   |     I2S5_RXD      | FC5_RXD_SDA_MOSI_DATA
PCM_SYNC |  J11(pin 3)  |   J28(pin 5)   |     I2S5_WS       | FC5_TXD_SCL_MISO_WS
PCM_CLK  |  J11(pin 4)  |   J28(pin 6)   |     I2S5_SCK      | FC5_SCK
GND      |  J11(pin 5)  |   J29(pin 7)   |     GND           | GND
------------------------------------------------------------------------------------
Note:
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
USB Host stack successfully initialized
Bluetooth initialized
BR/EDR set connectable and discoverable done
Now Start SDP Service and the Service is now discoverable by remote device
>>
the bellow commands have been supported:
"bt": BT related function
 USAGE: bt [dial|aincall|eincall]
    dial          dial out call.
    aincall       accept the incoming call.
    eincall       end an incoming call.
    svr           start voice recognition.
    evr           stop voice recognition.
    clip          enable CLIP notification.
    disclip       disable CLIP notification.
    ccwa          enable call waiting notification.
    disccwa       disable call waiting notification.
    micVolume     Update mic Volume.
    speakerVolume Update Speaker Volume.
    lastdial      call the last dial number.
    voicetag      Get Voice-tag Phone Number (BINP).
    multipcall    multiple call option
    
1) "dial" is used to dial out a call with phone number after device is connected, usage :
   bt dial 114
2) "aincall" is used to accept the incoming call when a call is coming, usage :
   bt aincallis 
3) "eincall" is used to reject the incoming call when a call is coming or end an active call
   bt eincall 
4) "svr" is used to start voice recognition, you can check the voice recognition information in peer device side.
   HFP voice recognition :1
5) "evr" is used to stop voice recognition, you can check the voice recognition information in peer device side.
   HFP voice recognition :0
6) "clip" is used to enable CLIP notification, you can see the incoming call phone number is showing in screen when enable the feature 
   Incoming Call...
   Phone call number: 133xxxxxxxx
7) "disclip" is used to disable CLIP notification, you can't see the incoming call phone number is showing in screen when enable the feature
8) "ccwa" is used to enable enable call waiting notification, you can in waiting call phone number is showing in screen when enable the feature and have multiple call
	>> > CALL WAITING Received Number : 133xxxxxxxx
	> Please use <multipcall> to handle multipe call operation
	 bt multipcall 0. Release all Held Calls and set UUDB tone (Reject new incoming waiting call)
	 bt multipcall 1. Release Active Calls and accept held/waiting call
	 bt multipcall 2. Hold Active Call and accept already held/new waiting call
	 bt multipcall 3. Conference all calls
	 bt multipcall 4. Connect other calls and disconnect self from TW
9) "disccwa" is used to disable call waiting notification, you can in waiting call phone number is not showing in screen when enable the feature and have multiple call
10) "micVolume" is used to set mic volume, the value  is from 1 to 15, usage as:
    bt micVolume 8
11) "speakerVolume" is used to set speaker volume, the value is from 1 to 15, usage as:
    bt speakerVolume 8
12) "lastdial" is used to call the last dial number.
    bt lastdial
13) "voicetag" is used to get Voice-tag Phone Number (BINP), need peer side to set Voice-tag Phone Number (BINP)
    bt lastdial
14) "multipcall"  is used call option, need peer side to set Voice-tag Phone Number (BINP), you can refer to ccwa information to do operation, usage:
    bt multipcall 1

Note:
There is a short noise can be heard at headset at the begin audio streaming when in running HFP Unit and HFP ring tone   
and at the end of each ring tone segment. The codec power on pop noise cannot eliminate.


