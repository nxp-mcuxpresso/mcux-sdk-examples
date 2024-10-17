Overview
========
This is the UART Wi-Fi bridge example to demonstrate the Lab Tool support.

uart_wifi_bridge version: v1.3.r34.p46


SDK version
===========
- Version: 2.16.100

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
3.  Connect the WiFi module to SD card slot.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================
1. Connect the board with Windows PC.
2. Configure Labtool Setup.ini file with correct Baudrate and COM port of device.
3. Run command on Labtool.
   Example Labtool output:
       Name:           Dut labtool
       Version:        2.1.0.14
       Date:           Sep 18 2017 (12:16:01)

       Note:

       Name:           DutApiClass
       Interface:      EtherNet
       Version:        2.1.0.14
       Date:           Sep 18 2017 (12:15:40)

       Note:

       \\.\COM6
        DutIf_InitConnection: 0
       --------------------------------------------------------
                       W87xx (802.11a/g/b/n) TEST MENU
       --------------------------------------------------------
       Enter option: 88
       DLL Version : 2.1.0.14
       LabTool Version: 2.1.0.14
       FW Version:  16.80.207.01       Mfg Version: 2.0.0.63
       SOC:    0000    09
       BBP:    A4      00
       RF:     58      31
       OR Version:     2.3      Customer ID:   0
       Enter option:

Customization options
=====================
For RW610/612 board, before compiling the CPU3 image, there are three macros need to be configured to determine which fw to download.
  - CONFIG_SUPPORT_WIFI
  - CONFIG_SUPPORT_BLE
  - CONFIG_SUPPORT_15D4
Here are some example about fw be used:
By default:only download wifi fw
  - CONFIG_SUPPORT_WIFI=1
  - CONFIG_SUPPORT_BLE=0
  - CONFIG_SUPPORT_15D4=0
wifi+ble
  - CONFIG_SUPPORT_WIFI=1
  - CONFIG_SUPPORT_BLE=1
wifi+ble+15d4
  - CONFIG_SUPPORT_WIFI=1
  - CONFIG_SUPPORT_BLE=1
  - CONFIG_SUPPORT_15D4=1
wifi+15d4
  - CONFIG_SUPPORT_WIFI=1
  - CONFIG_SUPPORT_15D4=1
