Overview
========
This demo implements a simple web based Wi-Fi configuration utility for connecting the board to the local Wireless network. 

Initially, the board doesn't have the credentials to join the local network, so it starts its own Access Point with SSID: "nxp_configuration_access_point" and password: "NXP0123456789".

The user can connect their device to this SSID and access the HTML UI under 192.168.1.1. The board will scan for the nearby Wi-Fi networks and display a list of them on this page. By clicking on the entries, the user can choose their network, enter the credentials and connect. The board will attempt to join this Wi-Fi network as a client and if it succeeds, it will disconnect its AP and save the credentials to its mflash memory.

On successive restarts, it checks the mflash memory and uses the saved credentials to directly connect to the local Wi-Fi network without starting the AP. 

A simple LED visualization is implemented. The board LED will be on if the device is in AP mode and turns off after the board changes to client mode.

The site allows the user to clear the credentials from the flash memory and reset the board to AP mode. If connection fails, user can also set device to AP mode through serial connection.

The source files for the web interface are located in the webui directory. Use the `<path_to_sdk>/middleware/lwip/src/apps/httpsrv/mkfs/mkfs.pl webui` Perl script in order to convert the webui files into the httpsrv_fs_data.c which is used in order to flash the static files onto the board. Make sure the mkfsl.pl script is executed from the same directory where the file httpsrv_fs_data.c and the directory webui are.

Note that Microsoft Internet Explorer is not supported by this webconfig example.

Before building the example application select Wi-Fi module macro in the app_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
For more information about Wi-Fi module connection see:
    readme_modules.txt
    Getting started guide on supported modules configuration:
    https://www.nxp.com/document/guide/getting-started-with-nxp-wi-fi-modules-using-i-mx-rt-platform:GS-WIFI-MODULES-IMXRT-PLATFORM



Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1040 board
- Personal Computer


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
1. When the demo starts, basic initialization proceeds
2. After that, device will wait for connection and configuration:

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
[i] Trying to load data from mflash.
[i] Initializing WiFi connection...

AsyncInterrupt is not supported
WLAN MAC Address : A0:C9:A0:3D:F9:2F
WLAN Firmware    : wl0: Feb 12 2018 04:08:14 version 7.79.2 (r683798 CY) FWID 01-27b63357
WLAN CLM         : API: 12.2 Data: 9.10.39 Compiler: 1.29.4 ClmImport: 1.36.3 Creation: 2018-02-12 04:00:50
[i] Successfully initialized WiFi module
Scanning available networks...
scan completed

#001 SSID          : nxp
     BSSID         : 00:1F:7B:31:03:9A
     RSSI          : -34dBm (off-channel)
     Max Data Rate : 72 Mbits/s
     Network Type  : Infrastructure
     Security      : WPA2 AES
     Radio Band    : 2.4GHz
     Channel       : 5

[i] Starting Access Point: SSID: nxp_configuration_access_point, Chnl: 1
[i] Connect to access point on this IP to configure device:  192.168.1.1
[i] Waiting....
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

4. Connect to the access point and in your web browser enter 192.168.1.1
5. Wait for the scan to finish and click on the desired network to join.
6. Enter the network password and click on connect.
6. After you send credentials, device will try connecting to the AP and if successful saves the credentials to the mflash.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
[i] Chosen ssid: nxp
[i] Chosen passphrase: NXP0123456789
[i] Stopping AP!
[i] Trying to load data from mflash.
[i] Loaded data: nxp;NXP0123456789
[i] Initializing WiFi connection...

AsyncInterrupt is not supported
WLAN MAC Address : A0:C9:A0:3D:F9:2F
WLAN Firmware    : wl0: Feb 12 2018 04:08:14 version 7.79.2 (r683798 CY) FWID 01-27b63357
WLAN CLM         : API: 12.2 Data: 9.10.39 Compiler: 1.29.4 ClmImport: 1.36.3 Creation: 2018-02-12 04:00:50
[i] Successfully initialized WiFi module
[i] Joining: nxp
[i] Successfully joined: nxp
Getting IP address from DHCP server
IPv4 Address got from DHCP  : 192.168.14.5
[i] Changed web data: connected;nxp;
[i] Waiting....

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

