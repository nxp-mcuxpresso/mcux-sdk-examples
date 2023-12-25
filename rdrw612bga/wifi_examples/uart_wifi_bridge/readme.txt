Overview
========
This is the UART Wi-Fi bridge example to demonstrate the Lab Tool support.

uart_wifi_bridge version: v1.3.r47.p9


Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

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
