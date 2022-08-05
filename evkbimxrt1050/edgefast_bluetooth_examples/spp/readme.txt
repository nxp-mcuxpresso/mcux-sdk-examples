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
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510_USD, please change the macro to WIFI_IW416_BOARD_AW_AM510_USD.
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM457_USD, please change the macro to WIFI_IW416_BOARD_AW_AM457_USD.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358_USD, please change the macro to WIFI_88W8987_BOARD_AW_CM358_USD.
If you want use the Murata Type 1ZM module, please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_USD.
If you want use the Murata Type 1XK module, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_USD.

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-AM510-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-AM510-USD |   I.MXRT1050   | PIN NAME OF RT1050 | GPIO NAME OF RT1050
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

The hardware should be reworked according to the Hardware Rework Guide for EVKB-IMXRT1050 and AW-CM358-uSD in document Hardware Rework Guide for EdgeFast BT PAL.
The pin connect for UART HCI as the following table,
------------------------------------------------------------------------------------
PIN NAME | AW-CM358-USD |   I.MXRT1050   | PIN NAME OF RT1050 | GPIO NAME OF RT1050
------------------------------------------------------------------------------------
UART_TXD |  J10(pin 4)  |   J22(pin 1)   |    LPUART3_RXD     | GPIO_AD_B1_07
UART_RXD |  J10(pin 2)  |   J22(pin 2)   |    LPUART3_TXD     | GPIO_AD_B1_06
UART_RTS |  J10(pin 6)  |   J23(pin 3)   |    LPUART3_CTS     | GPIO_AD_B1_04
UART_CTS |  J10(pin 8)  |   J23(pin 4)   |    LPUART3_RTS     | GPIO_AD_B1_05
GND      |  J6(pin 7)   |   J25(pin 7)   |    GND             | GND
------------------------------------------------------------------------------------

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
