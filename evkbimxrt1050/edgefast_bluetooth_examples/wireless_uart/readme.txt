Overview
========
The application implements a custom GATT based Wireless UART Profile that emulates UART over BLE.the application can work as central and peripheral at the same time. central and peripheral role can be switched by user button.
To test the service/profile the "IoT Toolbox" application can be used which is available for both Android and iOS.IoT Toolbox can be found on iTunes or Google playstore.


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
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1ZM M.2 Module (EAR00364)
  - Murata uSD-M.2 Adapter (LBEE0ZZ1WE-uSD-M2) and Embedded Artists 1XK M.2 Module (EAR00385)

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want to use the AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want to use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want to use the Murata Type 1ZM module, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want to use the Murata Type 1XK module, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.

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
