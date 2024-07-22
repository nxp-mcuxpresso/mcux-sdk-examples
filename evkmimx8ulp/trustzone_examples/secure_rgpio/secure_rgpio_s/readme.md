Overview
========
The Secure GPIO demo application demonstrates using of TRDC and GPIO PIN control.
TRDC allows user to control GPIO peripheral into secure world or into normal world.
The Pin Control Non-Secure Register (PCNS) configures secure/non-secure access protection for each pin.
Thus for all pins, user can select whether the pin is controlled from secure or non-secure domain.

This Secure GPIO demo uses GPIO peripheral to read SW7 button from secure world and normal world.
If SW7 button is pressed (logical zero is read) the RGPIOA PIN15 is pulled high in secure world
and concurrently the RGPIOA PIN18 is low from normal world.

The second part of the demo is GPIO PIN control Non-secure access protection feature. This feature is controlled by
button SW8. If the SW8 button is released, The SW7 is only allowed by software in secure world. SW7 is read zero
(also is zero When SW7 pressed) in normal world. If the SW8 button is pressed, the SW7 is for normal world. Thus normal
world can read state of SW7 button. The RGPIO PIN18 is pulled high while SW7 is released. Meanwhile the SW7 is pressed
PIN18 is low and PIN15 is pulled high.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the flash.bin to the target board.
    Reference 'How to get flash.bin of TrustZone examples'
    Reference 'Getting Started with MCUXpresso SDK for EVK-MIMX8ULP and EVK9-MIMX8ULP.pdf' to make and download flash.bin.
5.  Press the reset button on your board.

How to get flash.bin of TrustZone examples:
1.  Create a directory named 'tz_bin' in your work direcory.
2.  Build {app}_s and copy sdk2.0-app.bin into 'tz_bin' then rename it as sdk2.0-app_s.bin.
3.  Build {app}_ns and copy sdk2.0-app.bin into 'tz_bin' then rename it as sdk2.0-app_ns.bin.
4.  Enter the 'tz_bin' directory and execute 'dd' command to combin the two files.
    dd if=sdk2.0-app_ns.bin of=sdk2.0-app_s.bin bs=1 count=`ls -l ./sdk2.0-app_ns.bin|awk '{print $5}'` seek=843776
5.  Rename sdk2.0-app_s.bin as m33_image.bin, and copy it into imx-mkimage
6.  Build the corresponding flash.bin

Running the demo
================
Use SW7 button to control output of GPIOA PIN15(J23 A0) and SW8 to control output of GPIOA PIN18(J23 A3) and GPIO mask feature.
Like the below table:
-------------------------------------------------------
SW8       |  GPIO Mask |   SW7     |  PTA15  |  PTA18
-------------------------------------------------------
released  |  enabled   |  released |  low    |  low

released  |  enabled   |  pressed  |  high   |  low

pressed   |  disabled  |  released |  low    |  high

pressed   |  disabled  |  pressed  |  high   |  low
------------------------------------------------------

TrustZone Application Development in SDK
----------------------------------------
Every TrustZone based application consists of two independent parts - secure part/project and non-secure part/project.

The secure project is stored in <application_name>\<application_name>_s directory.
The non-secure project is stored in <application_name>\<application_name>_ns directory. 

The secure projects always contains TrustZone configuration and it is executed after device RESET. The secure project usually
ends by jump to non-secure application/project.
If IDE allows the two projects in single workspace, the user can also find the project with <application_name>.
This project contains both secure and non-secure projects in one workspace (Keil MDK, IAR) and it allows to user easy transition from
one to another project.

Project Structure
-----------------
The all TrustZone files related to TrustZone are located in trustzone virtual directory. The files are:

- tzm_config.c
- tzm_config.h
- veneer_table.c
- veneer_table.h

File tzm_config.c, tzm_config.h
-------------------------------
This file is used by secure project only. It contains one function BOARD_InitTrustZone(), which configures complete TrustZone
environment. It includes SAU, TRDC and some TrustZone related registers from System Control Block.
This function is called from SystemInitHook() function, it means during system initialization.

File veneer_table.c, veneer_table.h
----------------------------------
This file defines all secure functions (secure entry functions) exported to normal world. This file is located in secure
project only. While header file is used by both secure and non-secure projects. The secure entry functions usually contain
validation of all input parameters in order to avoid any data leak from secure world.

The files veneer_table.h and <application_name>_s_import_lib.o or <application_name>_s_CMSE_lib.o create the connection
between secure and non-secure projects. The library file is generated by linker during compilation of secure project and
it is linked to the non-secure project as any other library.

TrustZone application compilation
---------------------------------
Please compile secure project firstly since CMSE library is needed for compilation of non-secure project.
After successful compilation of secure project, compile non-secure project.
