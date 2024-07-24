Overview
========
This application example demonstrates an OTA update using the ROM API and SB image format.
File transfer is done using a simple HTTP server running on the board.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- RJ45 Network cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============


Board provisioning details
--------------------------
For board provisioning it's best to use MCUXpresso Secure Provisioning tool.
Besides required setup the following settings need to be applied:
    - Boot type set as "Signed"
    - `CFPA_PAGE_VERSION` set to at least 1 (non-zero and non-all-one)
    - Flash swapping enabled; CMPA field `FLASH_CFG.FLASH_REMAP_SIZE` set to 0b11111

The Dual image boot option allows to select for which slot (slot0/slot1) the SB3 file
is being created. To create SB3 file used for an update, the target image should be set
as "Image 1" - this reflects the process of using the second dormant slot as a space
for the new image. It's important to realize that with Flash Swapping enabled the ROM
selects an image with higher Image version. If higher version image is located in
the second slot (offset 0x100000) it swaps the slot0/slot1 addresses making it effectively
appear at the beginning of the flash (offset 0). Therefore also make sure that Image version
of the new image is higher than the currently installed one.

Prepare the Demo
================
1. Connect a USB cable between the PC and the board, make sure the power is on.
2. Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Insert the Ethernet Cable into the target board's RJ45 port and connect it to your PC network adapter.
4. Configure the PC's IP to be in the network 192.168.0.0, so for example 192.168.0.100

Running the demo
================
1.  Once the example is executed, you should see the following output in the console:

	Initializing PHY...

	************************************************
	ROM OTA HTTP Server Example
	Built Jan 26 2024 15:15:52
	************************************************
	 IPv4 Address     : 192.168.0.102
	 IPv4 Subnet mask : 255.255.255.0
	 IPv4 Gateway     : 192.168.0.100
	************************************************

2. Open web browser and type http://192.168.0.102 (IP address of the board) on the browser address bar.
   The browser should show the main web page of the example.
   Note: Make sure to include "http" protocol specifier in the address bar, so that your browser uses port 80

3. Go to OTA page; Here you can upload SB file with update and see current image state

4. Select SB file with image update and upload it

5. If the update was successful you should instantly see change in the image state, otherwise see console log for more details

6. You can now reset the board to let ROM boot the newly installed image

Known issue:
Browser may not display progress of the upload. This is an issue of the browser rather than OTA demo.
It is possible to watch upload progress in the attached serial console.

Modifying content of static web pages
To modify content available through the web server you must complete following steps:
  1. Modify, add or delete files in folder `boards\<board_name>\lwip_examples\lwip_httpsrv_ota\webpage`.
  2. Run the script file `middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl <directory name>` to generate new `httpsrv_fs_data.c`.
     Make sure to execute it from a folder where the file `httpsrv_fs_data.c` is. For example:
        `C:\sdk\boards\<board_name>\lwip_examples\lwip_httpssrv_ota> C:\sdk\middleware\lwip\src\apps\httpsrv\mkfs\mkfs.pl webpage`

  3. Make sure the `httpsrv_fs_data.c` file has been overwritten with the newly generated content.
  4. Re-compile the HTTP server application example and download it to your board. 
