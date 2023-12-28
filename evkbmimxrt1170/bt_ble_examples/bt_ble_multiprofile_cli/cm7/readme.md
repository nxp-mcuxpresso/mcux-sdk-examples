Overview
========
The example to demonstrate Multiprofiles working Together
//TODO: readme to be updated

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1170 board
- Personal Computer
- One of the following modules:
	- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.
	- Murata uSD-M.2 Adapter (Rev-B2) and Embedded Artists 2EL M.2 Module (Rev-A1)
	- RD Board with SDIO Connector

Hardware re-work for SDIO, UART and PCM lines for EVKB-RT1170:
a. Remove resistors R183 and R1816.
b. Solder 0 ohm resistor to R404, R1901, and R1902.
For PCM(HFP), 
c. Disconnect headers : J79, J80
d. Connect headers : J81, J82
e. Remove Resisters : R1985,R1986,R1987,R1988,R1992,R1993,R1994,R1995
f. Solder 0 ohm resisters : R228, R229, R232, R234, R1903

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use FC (IW61x) RD Board, please change the macro to WIFI_IW61x_BOARD_RD_USD.
If you want to use Murata Type 2EL module with uSD-M2 adapter, please change the macro to WIFI_IW61x_BOARD_MURATA_2EL_USD.
If you want to use Murata Type 2EL M2-A1 module with direct M2 Slot, please change the macro to WIFI_IW61x_BOARD_MURATA_2EL_M2.

Jumper settings for RT1170:
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

NOTE: With direct M2 connection (e.g. 2EL-M2) below connections are not required.

Jumper settings for Murata uSD-M.2 adapter:
  - J12 = 2-3 : M2 VIO set to 3.3v (Blue LED Illuminate)
    J13 = 1-2:  Host VIO set to 3.3v (WLAN-SDIO = 3.3V; and BT-UART & WLAN/BT-CTRL = 3.3V)
  - J1  = 2-3:  3.3V from uSD connector.

The following pins between the evkbmimxrt1170 board and Murata uSD-M.2 Adapter with Embedded Artists 1ZM/1XK/2EL Module are connected using male-to-female jumper cables:

----------------------------------------------------------------------------------------------
PIN NAME         | uSD-M.2 Adapter |   I.MXRT1170    | PIN NAME OF RT1170 | GPIO NAME OF RT1170
----------------------------------------------------------------------------------------------
BT_UART_TXD_HOST |  J9(pin 1)  	   |   J25(pin 13)   |    LPUART7_RXD     | GPIO_AD_01
BT_UART_RXD_HOST |  J9(pin 2)  	   |   J25(pin 15)   |    LPUART7_TXD     | GPIO_AD_00
BT_UART_RTS_HOST |  J8(pin 3)  	   |   J25(pin 11)   |    LPUART7_CTS     | GPIO_AD_02
BT_UART_CTS_HOST |  J8(pin 4)  	   |   J25(pin 9)    |    LPUART7_RTS     | GPIO_AD_03
GND				       |  J7(pin 6)  	   |   J26(pin 1)    |    GND		          | GND
----------------------------------------------------------------------------------------------

Jumper settings for AzureWave AW-AM457-uSD Module:
  - J11 2-3: VIO_SD 3.3V (Voltage level of SDIO pins is 3.3V)
  - J2  1-2: 3.3V VIO_uSD (Power Supply from uSD connector)
  - J4  2-3: 3.3V VIO

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

The hardware should be reworked according to the hardware rework guide for evkbmimxrt1170 and AW-CM358-M.2 in document Hardware Rework Guide for EdgeFast BT PAL.

Note:
After downloaded binary into qspiflash and boot from qspiflash directly, 
please reset the board by pressing SW4 or power off and on the board to run the application.
