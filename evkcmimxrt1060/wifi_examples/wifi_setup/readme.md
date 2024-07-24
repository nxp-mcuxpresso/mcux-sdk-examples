Overview
========
This demo implements a simple Wi-Fi station setup. The application will automatically scan local wireless networks at startup, then the user will 
be able to connect to one of the available networks and setup a ping task that will test the connection. 

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM



SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- evkcmimxrt1060 board
- Personal Computer
- One of the following WiFi modules:
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - Embedded Artists 2EL M.2 Module - direct M2 connection.
  - Embedded Artists 2DS M.2 Module (EAR00386) - direct M2 connection.

Board settings
==============

Jumper settings for RT1060-EVKC (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf
Embedded Artists 2DS module datasheet: https://www.embeddedartists.com/doc/ds/2DS_M2_Datasheet.pdf

RT1060-EVKC Board Rework For M2 Slot Enablement
A) Wi-Fi Rework:
	- Jumper Settings : Connect J109, connect J76 2-3
    - For 2DS M.2 Module: remove R2163
B) Wi-Fi Host Sleep Wakeup GPIO For 1XK/1ZM/2EL:
    - add 0Ohm resistor at position R252
    - 1XK:
        - Connect Fly-Wire between J33.1 and J108.5.
        - J108 is routed on M2.P44 which internally routed on GPIO[2] of Controller 1XK.
    - 1ZM:
        - Connect Fly-Wire between J33.1 and J108.2.
        - J108 is routed on M2.P40 which internally routed on GPIO[13] of Controller 1ZM.
    - 2EL:
        - No fly-wire connection required.
C) Wi-Fi Independent Reset OOB Trigger For 1XK/1ZM/2EL:
	- Connect Fly-Wire between J16.1 and J108.4.
	- J108 is routed on M2.P48 which internally routed on IR GPIO[15] of Controller 1XK/1ZM.
	- For 2EL-M2, No fly-wire connection required.
Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Connect the Wi-Fi module.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
1. When the demo starts, basic initialization proceeds - this might take several seconds.
2. After wifi is initialized, the application will try to scan nearby networks.
3. Once the scan is finished, the user will be prompted to enter SSID and password for one of the scanned networks to join.
4. After successfully joining the network, the user will be requested to specify a valid IPv4 address, which will be used for ping test.
