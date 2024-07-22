Overview
========
The application implements a custom GATT based Wireless UART Profile that emulates UART over BLE.the application can work as central and peripheral at the same time. central and peripheral role can be switched by user button.
To test the service/profile the "IoT Toolbox" application can be used which is available for both Android and iOS.IoT Toolbox can be found on iTunes or Google playstore.


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
- mimxrt685audevk board
- Personal Computer
- One of the following modules:
  - WIFI_IW416_BOARD_MURATA_1XK_M2
  - WIFI_88W8987_BOARD_MURATA_1ZM_M2
  - WIFI_IW612_BOARD_MURATA_2EL_M2

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the WIFI_IW416_BOARD_MURATA_1XK_M2, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want use the WIFI_88W8987_BOARD_MURATA_1ZM_M2, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.
If you want use the WIFI_IW612_BOARD_MURATA_2EL_M2, please change the macro to WIFI_IW612_BOARD_MURATA_2EL_M2.

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

Jumper settings for WIFI_IW416_BOARD_MURATA_1XK_M2:
  - J41 : from position (1-2) to position (2-3)
  - move R300-R305 from position A(2-1) to position B(2-3)

The following pins between the mimxrt685audevk board and Murata uSD-M.2 Adapter with 1XK M.2 Module are connected using male-to-female jumper cables:

------------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT685    |  PIN NAME OF RT685 | GPIO NAME OF RT685
------------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J45(pin 32)  	   |   U18B(G17)   |    USART5_TXD      | FC5_TXD_SCL_MISO_WS
BT_UART_RXD_HOST |  J45(pin 22)  	   |   U18B(J16)   |    USART5_RXD      | FC5_RXD_SDA_MOSI_DATA
BT_UART_RTS_HOST |  J45(pin 36)  	   |   U18B(J15)   |    USART5_RTS      | FC5_RTS_SCL_SSEL1
BT_UART_CTS_HOST |  J45(pin 34)  	   |   U18B(J17)   |    USART5_CTS      | FC5_CTS_SDA_SSEL0
------------------------------------------------------------------------------------------------

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
The demo require user interaction. The application will automatically start advertising the wirless uart Service afte reset, the The application can accepte at most 8 connection when work as peripheral.
The application will start scan and connect to the wirless uart Service automatically,after short prees the user button, the The application can connect at most 8 connection when work as cnentral.

after reset, the Serial port terminal log is:

BLE Wireless Uart demo start...
Bluetooth initialized
Advertising successfully started

then we can use "IoT Toolbox" or another wireless_uart example(use B to refer to) to test the current device. 
peripheral role test:
please open "IoT Toolbox", check the "Wireless UART" option, one device named "NXP_WU" will be found, click the "NXP_WU", the software will connect to the NXP_WU, please accept the pair request, or else there maybe pair fail. take android as example, please check the " message notification bar" to find the Pair request. 
after pair, data could be sent/receive in the toolbox.

central role test:
let B work as default state after reset. 
short press the user button, the example will work as central can automatically connect to any discovered wireless uart example.each time short press, the example will scan and connect to wirelss uart service if new device is found.

BLE Wireless Uart demo start...
Bluetooth initialized
Advertising successfully started
Scanning successfully started
[DEVICE]: 24:FC:E5:9F:EE:EB (public), AD evt type 3, AD data len 28, RSSI -92
[DEVICE]: 64:86:7F:5A:7C:7F (random), AD evt type 0, AD data len 23, RSSI -81
[DEVICE]: 64:86:7F:5A:7C:7F (random), AD evt type 4, AD data len 0, RSSI -80
[DEVICE]: 65:F2:7E:9A:AF:C7 (random), AD evt type 0, AD data len 19, RSSI -89
[DEVICE]: 65:F2:7E:9A:AF:C7 (random), AD evt type 4, AD data len 0, RSSI -89
[DEVICE]: 63:F2:B1:6A:FC:3D (random), AD evt type 0, AD data len 18, RSSI -80
[DEVICE]: 63:F2:B1:6A:FC:3D (random), AD evt type 4, AD data len 0, RSSI -80
[DEVICE]: 78:B3:AA:89:78:3B (random), AD evt type 0, AD data len 18, RSSI -80
[DEVICE]: 78:B3:AA:89:78:3B (random), AD evt type 4, AD data len 0, RSSI -79
[DEVICE]: 80:D2:1D:E8:2B:7E (public), AD evt type 0, AD data len 21, RSSI -43
Connected to 80:D2:1D:E8:2B:7E (public)
GATT MTU exchanged: 65
[ATTRIBUTE] handle 25
[ATTRIBUTE] handle 26
Security changed: 80:D2:1D:E8:2B:7E (public) level 2 (error 0)
Note:
the device address, AD event type data len, and RSSI are variable, it depend on all the bluetooth device in test environment.


send data 12345 in B device's Serial port terminal, then current device will print the following log. 
Data received (length 5): 12345  

send data 123 in current device's Serial port terminal, then B device will print the following log. 
Data received (length 5): 123

long press the user button, the example will work as peripheral again.
