Overview
========
This demo implements a simple Wi-Fi station setup. The application will automatically scan local wireless networks at startup, then the user will 
be able to connect to one of the available networks and setup a ping task that will test the connection. 

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM



Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT1064 board
- Personal Computer


Board settings
==============

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
