Overview
========
The mcuboot_opensource is a second stage bootloader based on MCUBoot project. It is primarily meant to be used together with OTA (over-the-air) update examples
to demonstrate functionality of application self-upgrade.

Flash memory layout
-------------------
Flash memory is divided into multiple regions to allocate space for bootloader, main application
and application update:

 - MCUBoot partition (reserved for bootloader itself, starts at the beginning of the FLASH memory)
 - Primary application partition (active application image)
 - Secondary application partition (candidate application - place to download OTA image to be used for update)

The partitioning is defined by definitions in flash_partitioning.h header file.
The MCUBoot partition starts at the very beginning of the FLASH memory and spans up to BOOT_FLASH_ACT_APP.
The primary partition occupies range starting from BOOT_FLASH_ACT_APP up to BOOT_FLASH_CAND_APP.
The secondary partition starts at BOOT_FLASH_CAND_APP and it is automatically assigned the same size as the primary one.
The rest of the memory may be used by the application for arbitrary purposes.

Important notice: should you need to change the partitioning please make sure to update also the header file used by the OTA application!
If the partitioning information used by the bootloader and the application is not in sync, it may lead to malfunction of boot/OTA process or to upredictable behavior.

Flash remapping functionality
The default upgrade mechanism in MCUBoot is SWAP algorithm. There are several NXP processors which support flash remapping functionality what can be used to speed up the OTA update process and prolong the flash memory wear process by just switching the valid images.
The boards with such processors have example projects configured to use this feature. This is achieved by using MCUBoot DIRECT-XIP (equal slots) mechanism and by activating flash remapping when needed - image is still built to run from primary slot. Keep in mind that DIRECT-XIP mode loads image with the highest version (no rollback support).
Both projects (MCUBoot and evaluated OTA example) using the flash remapping funcionality can be also configured to use and evaluate default SWAP mechanism if needed. 
For more information see "MCUBoot upgrade mode" in sblconfig.h (MCUBoot project).

IMPORTANT NOTE:
Signed application images directly programmed into flash memory by a programmer require additional "--pad --confirm" parameter for imgtool. This parameter adds additional trailer to the signed image and is required by bootloader direct-xip process (see MCUBoot documentation for more information). Signed images used in OTA process do not require "-pad" parameter.

List of boards with projects supporting flash remapping function:
    - MIMXRT1040-EVK
    - MIMXRT1060-EVK
    - MIMXRT1060-EVKB
    - MIMXRT1060-EVKC
    - MIMXRT1064-EVK
    - MIMXRT1160-EVK
    - MIMXRT1170-EVK
    - MIMXRT1170-EVKB
    - RD-RW612-BGA
    - RD-RW612-QFN
    - FRDM-RW612
    - EVK-MIMXRT595
    - EVK-MIMXRT685
    - MIMXRT685-AUD-EVK
    - MCX-N9XX-EVK
    - MCX-N5XX-EVK
    - FRDM-MCXN947

Encrypted XIP support
For more information please see mcuboot_encrypted_xip.md (in mcuboot_opensource/ext/nxp_encrypted_xip)

This extension of MCUboot functionality can be evaluated by enabling define CONFIG_ENCRYPT_XIP_EXT_ENABLE in sblconfig.h

List of boards with projects supporting encrypted XIP:
    - MIMXRT1060-EVK  (BEE)
    - MIMXRT1060-EVKB (BEE)
    - MIMXRT1060-EVKC (BEE)

Signing the application image
-----------------------------
MCUBoot expects signed application image in specific format to be present in the primary partition.
The very same image format it also used for OTA updates.

A dedicated tool (imgtool) is used to obtain application image in the desired format.
It is implemented as a Python script which can be found in the SDK package in `middleware/mcuboot_opensource/scripts folder`.

Alternatively the tool can be installed by the Python package manager:
- "pip install imgtool"

Please note that imgtool version installed by the Python package manager is not guaranteed to be compatible with MCUBoot present in you SDK package.

The `mcuboot_opensource` SDK project comes with its set of private-public keys.
The key pair is stored in the keys subdirectory (e.g. `boards/[board]/mcuboot_opensource/keys`).
The public key is already pre-configured in the source code of MCUBoot in a form of an array initializer.

To sign an application binary, imgtool must be provided with respective private key and a set of parameters as in the following example for RT1060 EVK board:

IMPORTANT: Note that other boards may require different parameters. Check the section `Board settings` in this readme for details.

 imgtool sign --key sign-rsa2048-priv.pem
	          --align 4
	          --header-size 0x400
	          --pad-header
	          --slot-size 0x200000
	          --version "2.0"
	          app_binary.bin
	          app_binary_SIGNED.bin

The parameters used in the example above are tested with out-of-the-box configuration of MCUBoot and OTA examples in the SDK package.
However, some of them may depend on the application or board setup and thus may need to be modified.
See the MCUBoot documentation for the meaning of the parameters and align them with your project setup if necessary.
https://docs.mcuboot.com/imgtool.html

Using MCUXpresso Secure Provisioning Tool for MCUBoot image signing
-------------------------------------------------------------------
MCUXpresso Secure Provisioning Tool from verion 9 supports automation for MCUBoot image signing. Using this tool
it's possible to setup the device for entire boot chain (ROM->MCUBoot->application) in a few steps.


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
- MCX-N5XX-EVK board
- Personal Computer

Board settings
==============
MCUBoot layout for the internal flash is the following:

  MCUBoot region       0x0100_8000 - 0x0100_FFFF : 32kB
  Primary slot         0x0000_0000 - 0x0010_0000 : 1024kB
  Secondary slot       0x0010_0000 - 0x0200_0000 : 1024kB

- MCUBoot resides in the Bank_1 IFR_0 region
- MCUBoot header size is set to 1024 bytes
- Signing algorithm is ECDSA-P256
- Write alignment is 16 bytes
- MCUBoot is configured to use its DIRECT_XIP image handling strategy
- Flash swapping HW feature is used for zero-overhead image swapping

Image signing example:

  imgtool sign --key sign-ecdsa-p256-priv.pem
               --align 16
               --version 1.1
               --slot-size 0x100000
               --header-size 0x400
               --pad-header
               ota_mcuboot_basic.bin
               ota_mcuboot_basic.SIGNED.bin

Note that for the first image flashed manually together with the bootloader
additional imgtool options "--pad" and "--confirm" must be used. Otherwise
the bootloader would reject the image for missing data in the trailer area.

If the toolchain used for build has problems with writing the bootloader
to the IFR flash region, an alternative can be to use NXP blhost utility
or the MCUXpresso Secure Provisioning Tool.

IMPORTANT:
The CMPA region must be configured so that ROM loader boots from the IFR
region. This is done by setting the BOOT_CFG.BOOT_SRC field to 0b10. This
value directs the ROM loader to start execution from the IFR region where
MCUboot is located. Default CMPA setup with this particular field set is
attached to this example in cmpa.bin binary file.
To flash this configuration together with MCUboot binary the following blhost
commands can be used.  Note that the board must be in the ISP mode to use
the following commands:

  - #Erase IFR0 Bank 1 region
  - blhost -u 0x1FC9,0x014F -- flash-erase-region 0x01008000 32768 0

  - #Flash MCUboot
  - blhost -u 0x1FC9,0x014F -- write-memory 0x01008000 mcuboot_opensource.bin

  - #Flash CMPA
  - blhost -u 0x1FC9,0x014F -- write-memory 0x01004000 cmpa.bin
Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
    - line ending set for LF ('\n')
3.  Build project and program it to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  When the demo runs successfully, the terminal will display the following:
        hello sbl.
        Bootloader Version 1.0.0
2.  Further messages printed to the terminal depend on the content of the FLASH memory.
    In case there was no application programmed so far (i.e. the FLASH was blank), the following would be printed:
        Primary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
        Secondary image: magic=unset, swap_type=0x1, copy_done=0x3, image_ok=0x3
        Boot source: none
        Swap type: none
        erasing trailer; fa_id=2
        Unable to find bootable image
3. At this point the bootloader is in place, resident in the FLASH memory. You may stop debuger, switch to an OTA example and follow the relevant readme.
