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
- evkcmimxrt1060 board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists 1XK M.2 Module(EAR00385), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists 1ZM M.2 Module(EAR00364), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.
If you want to use Embedded Artists 1ZM M.2 Module(Rev-A1), please change the macro to WIFI_IW612_BOARD_MURATA_2EL_M2.

Jumper settings for RT1060-EVKC (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the hardware rework guide for evkcmimxrt1060 and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.

The hardware should be reworked according to the hardware rework guide for evkcmimxrt1060 and Murata 1ZM M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
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
