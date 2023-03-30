Overview
========
The sdcard_fatfs_ota demo application demonstrates an OTA update process using MCUBoot + FATFS + SD card.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- micro USB cable
- LPCXpresso55S69 board
- Personal Computer
- micro SD card

Board settings
==============
No special HW settings.

Prepare the Demo
================
1. The demo requires MCUBoot booloader to be present in the FLASH memory to function properly.
   It is recommended to build and program the bootloader first, then go on with the application.
   Please refer to respective readme of the mcuboot_opensource example and follow the steps there before you continue.
2. Insert a FAT formatted card with a signed OTA image into the SD card slot.
   How to generate a signed image is described in the mcuboot_opensource project readme.
2. Connect a USB cable between the PC host and the OpenSDA (or USB to Serial) USB port on the target board.
3. Open a serial terminal on PC for connected board with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control


Running the demo
================
To get the application properly executed by the bootloader, it is necessary to put signed application image to the primary application partition.
There are multiple options how to achieve that, however in principle the are two main methods (both presuming the bootlaoder is already in place):

a)  programing signed application image to the primary application partition using an external tool (direct method)
b)  jump-starting the application by debugger, performing OTA update with the signed image, resetting the board and letting the bootloader to perform update (indirect method)

The latter method is described step by step below:

1.  Open the demo project and build it.
    Known issue: MDK linker issues warning about unused boot_hdr sections. This does not affect the functionality of the example.
    
2.  Prepare signed image of the application from raw binary as described in the mcuboot_opensource readme.
     - In case of MCUXpress raw binary may not be generated automatically. Use binary tools after right clicking Binaries/.axf file in the project tree to generate it manually.

3.  Launch the debugger in your IDE to jump-start the application.
     - In case of MCUXpresso IDE the execution stalls in an endless loop in the bootloader. Pause the debugging and use debugger console and issue command 'jump ResetISR'.

4.  When the demo starts successfully, the terminal will display shell promt as in the following example:

        OTA example with SD card and FATFS.
        SD card recognized.

        Copyright  2020  NXP

        SHELL>>

5.  Available command can be printed with "help" command
6.  List files on the SD card in the root directory with "ls" command:

        SHELL>> ls
        DIR  DIR_1
        FILE TEST.BIN
        FILE OTA.BIN

7.  Current application version can be checked before update.
    The version is defined in source/version.h header file.

        SHELL>> ota version
        1.0


8.  Select file for installaction with "install <filename>":

        SHELL>> install ota.bin
        Installing OTA image from file ota.bin
        Size of file ota.bin is 47132 bytes
        Erasing 384 sectors of flash from offset 0x50000
        Programming flash...
        write magic number offset = 0x7fe00
        OTA image was installed successfully.

9.  On next reboot MCUBoot will attempt to install this new image.
    Reboot can be done either manually with reset button or with a shell command.

        SHELL>> ota reboot
        System reset!
        hello sbl.
        Bootloader Version 1.9.0
        Swap type: test
        Image upgrade secondary slot -> primary slot
        Erasing the primary slot
        Copying the secondary slot to the primary slot: 0xzx bytes
        erasing secondary header
        erasing secondary trailer
        Bootloader chainload address offset: 0x20000
        Reset_Handler address offset: 0x20400
        Jumping to the image

10.  If version number was modified for the new OTA image it can be checked:

        SHELL>> ota version
        2.0

