Overview
========
Application demonstrating how to use the SPP feature.


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
- mimxrt685audevk board
- Personal Computer
- One of the following modules:
  - WIFI_IW416_BOARD_MURATA_1XK_M2

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the WIFI_IW416_BOARD_MURATA_1XK_M2, please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

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
