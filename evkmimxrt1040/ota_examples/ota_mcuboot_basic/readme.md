Overview
========
This `ota_mcuboot_basic` example demonstrates a basic application that uses MCUBoot as a second stage bootloader.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- micro USB cable
- EVKMIMRXRT1040 board
- Personal Computer

Board settings
==============
No special HW settings.
Prepare the Demo
================
1. The demo requires MCUBoot booloader to be present in the FLASH memory to function properly.
   It is recommended to build and program the bootloader first, then go on with the application.
   Please refer to respective readme of the `mcuboot_opensource` example and follow the steps there before you continue.
2. Connect a USB cable between the PC and the Debug USB port on the target board.
3. Open a serial terminal on PC for connected board with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control


Transfering data to the flash memory
------------------------------------
There are multiple ways how to transfer image updates to the flash memory:

- This examples implements a simple XMODEM-CRC protocol, that can be used to transfer data to the board at slow speed (~10kB/s).
  This provides a convenient method for images that have relatively small size (under lower 100's of kB).
  In the application shell the XMODEM receiving is initiated by `xmodem` command. Terminal emulators like Tera Term or ExtraPutty
  can be used as a transmitting side. Both 128B and 1024B packet sizes are supported.

- An alternative way, more suitable for larger images, is to use the *blhost* utility (part of NXP's SPSDK and MCUXpresso Secure Provisioning Tool).
  Blhost communicates with chip's ROM and supports all basic flash operations. When used via onboard's USB the transfer
  speed is more appropriate for larger files. See board's user manual for details on ISP mode.

- Another option is to use a debug adapter (e.g. JLink, CMSIS DAP...) and flash data using their tools.


Running the demo
================
To get the application properly executed by the bootloader, it is necessary to put signed application image to the primary application partition.
There are multiple options how to achieve that, however in principle the are two main methods (both presuming the bootlaoder is already in place):

a)  programing signed application image to the primary application partition using an external tool (direct method)
b)  jump-starting the application by debugger, performing an image update with the signed image, resetting the board and letting the bootloader to perform the update (indirect method)

The latter method is used in the following step-by-step description:

1.  Open the demo project and build it.
    Known issue: MDK linker issues warning about unused `boot_hdr` sections. This does not affect the functionality of the example.
    
2.  Prepare signed image of the application from raw binary as described in the readme of `mcuboot_opensource` SDK example.
     - In case of MCUXpresso raw binary may not be generated automatically. Use binary tools after right clicking Binaries/.axf file in the project tree to generate it manually.

3.  Launch the debugger in your IDE to jump-start the application.
     - In case of MCUXpresso IDE the execution stalls in an endless loop in the bootloader. Pause the debugging and use debugger console and issue command `jump ResetISR`.

4.  When the demo starts successfully, the terminal will display shell prompt as in the following example:

        *************************************
        * Basic MCUBoot application example *
        *************************************

        Built Feb 22 2024 13:09:14

        $

5.  Available commands can be printed with `help` command

6.  Current image state is printed with `image` command. The output will reflect settings of the used platform and
    may not be exactly the same as the following output:

        $ image
        Image 0; name APP; state None:

          Slot 0 APP_PRIMARY; address 0x20000; size 0x30000 (196608):
            <No Image Found>

          Slot 1 APP_SECONDARY; address 0x50000; size 0x30000 (196608):
            <No Image Found>

7.  From the shown `image` command output it can be observed that there are currently no signed images present neither
    in the primary nor the secondary slot.  Since the example was started by a debugger, it is not signed and hence
    not recognized as a valid signed image in the primary slot.

8.  Before an image update can be downloaded using the `xmodem` command, the secondary image slot needs to be erased:

        $ image erase

9.  XMODEM transfer is initiated using `xmodem` command. When executed, it starts waiting for the transmitting side.
    In this demonstration Tera Term is used to send the update file. The dialog `File->Transfer->XMODEM` is used
    to select a file to be send. In the same dialog a packet size can be extended to 1kB for faster transmittion.

        $ xmodem
        Initiated XMODEM-CRC transfer. Receiving... (Press 'x' to cancel)
        CCC
        Received 31744 bytes
        SHA256 of received data: 547251A815E1DFB074F9...
        SHA256 of flashed data:  547251A815E1DFB074F9...

10. After image downloaded finished the current image status can be checked with `image` command. The output now
    shows that there is an MCUBoot image file detected in the secondary slot. The SHA256 digest is different from
    the digest  computed for the data downloaded via XMODEM. This is because XMODEM protocol pads data at the end
    to make their size multiple of the packet payload size (128/1024 bytes).

        $ image
        Image 0; name APP; state None:

          Slot 0 APP_PRIMARY; address 0x20000; size 0x30000 (196608):
            <No Image Found>

          Slot 1 APP_SECONDARY; address 0x50000; size 0x30000 (196608):
            <IMAGE: size 30352; version 2.2.0+0>
            SHA256 of image payload: 918A708203EE4AD7C616...

11. To let the bootloader install the new image it must be marked as `ReadyForTest`. This is done using `image test`
    command. After running this command the result can be checked by runnig the `image` command again.

        $ image
        Image 0; name APP; state ReadyForTest:
        ...

12. Running `reboot` command or resetting the board manually starts the bootloader and lets it handle the installation
    of new image. If everything went well the image from the secondary slot was moved to the primary slot and the bootloader
    started its execution. The `image` command output should reflect this change:

        $ image
        Image 0; name APP; state None:

          Slot 0 APP_PRIMARY; address 0x20000; size 0x30000 (196608):
            <IMAGE: size 30352; version 2.2.0+0>
            SHA256 of image payload: 918A708203EE4AD7C616...

          Slot 1 APP_SECONDARY; address 0x50000; size 0x30000 (196608):
            <No Image Found>

13. If the bootloader was configured to use the image swapping mode, another image can be now downloaded to test this.
    The process is exactly the same as in the previous steps. The only difference will be after the image is installed
    because this time the image state will be set as `Testing`. In this state the newly running image must confirm itself
    otherwise the booloader will revert back to previous image on the next reboot. The confirmation is done by `image accept`
    command after which the image state will be set as `None`.

