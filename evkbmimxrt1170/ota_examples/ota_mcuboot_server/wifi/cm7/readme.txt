Overview
========
This application demonstrates OTA update through HTTP server based on lwIP TCP/IP and the MbedTLS stack with
FreeRTOS. The user uses an Internet browser to upload new version of firmware and restarts the board to perform the update

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
- MIMXRT1170-EVKB board
- Personal Computer


Board settings
==============

Platform specific behavior
When an application programmatically performs software-induced reset while in a debugging session, the execution stalls in the ROM code. This is a feature.
Your attention is needed at that moment: please perform reset manually by pressing the on-board hw reset button once you spot "SystemReset" message in the serial console.
Manual reset is not needed while MCU is running freely without a debugger.
Prepare the Demo
================
1. The demo requires MCUBoot bootloader to be present in the FLASH memory to function properly.
   It is recommended to build and program the bootloader first, then go on with the application.
   Please refer to respective readme of the mcuboot_opensource example and follow the steps there before you continue.
2. Prior launching the demo it is recommended to pre-build image with modified version of the application to test the OTA update process.
2. Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
3. Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Install WiFi connectivity module into SD or M.2 slot.

Running the demo
================
To get the application properly executed by the bootloader, it is necessary to put signed application image to the primary application partition.
There are multiple options how to achieve that, however in principle the are two main methods (both presuming the bootlaoder is already in place):

a)  programing signed application image to the primary application partition using an external tool (direct method)
b)  jump-starting the application by debugger, performing OTA update with the signed image, resetting the board and letting the bootloader to perform update (indirect method)

The latter method is described step by step below:

1.  Load the demo project and build it.
    Known issue: MDK linker issues warning about unused boot_hdr sections. This does not affect the functionality of the example.

2.  Prepare signed image of the application from raw binary as described in the mcuboot_opensource readme.
     - In case of MCUXpress raw binary may not be generated automatically. Use binary tools after right clicking Binaries/.axf file in the project tree to generate it manually.

3.  Launch the debugger in your IDE to begin running the demo.
     - In case of MCUXpresso IDE the execution stalls in an endless loop in the bootloader. Pause the debugging and use debugger console and issue command 'jump ResetISR'.

4.  When the demo runs successfully, the terminal will display the following:

        Initializing WiFi connection...
        MAC Address: D8:C0:A6:C0:B0:57
        [net] Initialized TCP/IP networking stack
        WLAN initialized
        WLAN FW Version: w8978o-V0, RF878X, FP91, 16.91.10.p185, WPA2_CVE_FIX 1, PVE_FIX 1
        Successfully initialized WiFi module
        Starting Access Point: SSID: nxp-ota, Chnl: 1
        [wlcm] Warn: NOTE: uAP will automatically switch to the channel that station is on.
        Soft AP started successfully
        This also starts DHCP Server with IP 192.168.1.1
        Join 'nxp-ota' network and visit https://192.168.1.1

5. Connect you PC to WiFi network provided by the board (SSID: nxp-ota, password: NXP0123456789).
   Your PC should acquire IP configuration automatically via DHCP.

6. Open web browser and type https://192.168.1.1 (IP address of the board) on the browser address bar.
   The browser should show the main web page of the example.
   Note: Be sure to include "https" protocol specifier in the address bar, so that your browser attempts to establish secure connection to TCP port 443,
   as browsers generally tend to use non-secure connection to port 80 by default.

7. Go to OTA page, select file with udpated firmware and upload it.

8. After the file is uploaded, click "Reboot" button to start newly uploaded firmware in test mode.

9. Once the updated firmware executes, the "accept" button becomes active. Click it to make the update permanent.
   Note: Make sure your computer is connected to the AP provided by the board at this point as it might automatically connect to another AP during the reboot. 

Known issue:
Browser may not display progress of the upload. This is an issue of the browser rather than OTA demo.
It is possible to watch upload progress in the attached serial console.

Modifying content of static web pages
To modify content available through the web server you must complete following steps:
  1. Modify, add or delete files in folder "boards\<board_name>\lwip_examples\lwip_httpsrv_ota\webpage".
  2. Run the script file "middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl <directory name>" to generate new "httpsrv_fs_data.c".
     Make sure to execute it from a folder where the file "httpsrv_fs_data.c" is. For example:
        C:\sdk\boards\<board_name>\lwip_examples\lwip_httpssrv_ota> C:\sdk\middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl webpage
		Processing file webpage/favicon.ico
        Processing file webpage/httpsrv.css
        Processing file webpage/index.html
        Processing file webpage/NXP_logo.png
        Processing file webpage/ota.shtml
        Processing file webpage/ota_reboot.html
        Processing file webpage/request.js
        Processing file webpage/welcome.html
		Done.
  3. Make sure the "httpsrv_fs_data.c" file has been overwritten with the newly generated content.
  4. Re-compile the HTTP server application example and download it to your board. 
