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
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J10) on the board.
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
1. When the demo starts, basic initialization proceeds
2. After that, device will wait for connection and configuration:

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Starting webconfig DEMO
[i] Trying to load data from mflash.
[i] Nothing stored yet
[i] Initializing Wi-Fi connection...
MAC Address: 9C:50:D1:44:67:5F
[i] Successfully initialized Wi-Fi module
Starting Access Point: SSID: nxp_configuration_access_point, Chnl: 1
[wlcm] Warn: NOTE: uAP will automatically switch to the channel that station is on.
 Now join that network on your device and connect to this IP: 192.168.1.1
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

3. Connect to the access point and in your web browser enter 192.168.1.1
4. Wait for the scan to finish - the demo terminal will print something like:

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Initiating scan...

nxptp
     BSSID         : B0:A7:B9:99:27:52
     RSSI          : -34dBm
     Channel       : 3
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

5. In your web browser you should also see the scan results, click on the network
   you want to add, fill in the credentials and click connect.
6. After you send credentials, device will try connecting to the AP and if successful saves the credentials to the mflash.

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
[i] Chosen ssid: nxptp
[i] Chosen passphrase: "NXP01234"
[i] Chosen security methods: "WPA2 WPA3_SAE"
[i] Joining: nxptp
[i] Successfully joined: nxptp
 Now join that network on your device and connect to this IP: 192.168.0.209
[i] mflash_save_file success
[i] Stopping AP!
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

7. The device is now in station mode and is joined to the selected network.
   Now join this network on your PC and enter the IP from demo terminal.
8. You can try to join different networks or reset the board back to AP mode and start again.
