Overview
========
The application implements a wireless function based on "wifi_cli" demo and "wireless_uart" demo, which enables users to use wifi command-line interface(CLI) over BLE wireless uart.
To test the service/profile the "IoT Toolbox" application can be used which is available for both Android and iOS.IoT Toolbox can be found on iTunes or Google playstore.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board.
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
The demo require user interaction. The application will automatically start advertising the wirless uart Service afte reset.

Then we can use "IoT Toolbox" or another wireless_uart example(use B to refer to) to test the current device. 
Please open "IoT Toolbox", check the "Wireless UART" option, one device named "NXP_WU" will be found, click the "NXP_WU", the software will connect to the NXP_WU, please accept the pair request, or else there maybe pair fail. take android as example, please check the " message notification bar" to find the Pair request. 
After pair, data could be sent/receive in the toolbox.

after reset, the log is shown below, which demostrates the supported commands:

help
wlan-reset
wlan-version
wlan-mac
...
dhcp-stat

Then user can bound the device to the phone or other central devices.

To send commands to the wifi module, type the command in the "IoT Toolbox" command line.

The commands supported by the demo are listed after user types "help" in the command line.
For example, after user types "wlan-mac", the console will show the mac address below:

wlan-mac
MAC address
STA MAC Address:A0:CD:F3:77:E6:FE
uAP MAC Address:A0:CD:F3:77:E6:FE

Notes
When using wireless_uart demo work as central to test wifi_cli_over_ble_wu functions, some macros shall be defined in app_config.h of wireless_uart project to prevent data loss.
1. #define CONFIG_BT_MSG_QUEUE_COUNT 64 or higher.
2. #define CONFIG_BT_ATT_RX_MAX  64 or higher.
3. CONFIG_BT_ATT_RX_MAX shall be less than or equal to CONFIG_BT_MSG_QUEUE_COUNT
Without these steps, wireless_uart console can not print full help menu after user input "help" command.

Known Issues
When using Wireless UART function via IOT Toolbox APP to test this demo，The conosole may not respond after user input.
The root cause is: The IOT Toolbox APP may not send '\n' when user pressed "enter" on keyboard, so that the demo cannot recognize which is the last character of user input.
To avoid this issue, following steps shall be taken: 
1. In "cli.c", change the default macro "#define END_CHAR '\n'" to any other uncommonly used character, such as #define END_CHAR '@' or '$'.
2. Type this special character after the cmd you want to send. Such as "help@" or "wlan-mac@".
3. Press enter on keyboard and the APP will send this cmd to the board.
