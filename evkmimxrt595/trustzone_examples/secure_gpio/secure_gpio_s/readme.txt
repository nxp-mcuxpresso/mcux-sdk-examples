Overview
========
The Secure GPIO demo application demonstrates using of secure GPIO peripheral and GPIO mask feature in
AHB secure controller. The secure GPIO is standard GPIO peripheral supporting 32 pins on port 0 only.
Having two GPIO peripherals allows user to configure one GPIO peripheral into secure world and another
one into normal world. Thus for all pins on port 0, user can select whether the pin is controlled
from secure or non-secure domain (using IOCON).
Since the every GPIO pin allows to read its state regardless of its configuration, the AHB secure
controller has build in feature GPIO mask for all GPIO pins. It is recommended to mask GPIO pins
for all peripherals assigned to secure domain in order to eliminate information leak from GPIO pins.
For example UART is assigned to secure world. But GPIO peripheral still allows to read state of
TxD and RxD pins. This information can be use to decode UART communication from normal world, which
can lead to unwanted information leak. 

This Secure GPIO demo uses secure GPIO peripheral to read SW4 button from secure world and standard GPIO peripheral to read
the same button SW4. If SW4 button is pressed (logical zero is read) the GREEN LED is switched ON in secure world
and concurrently BLUE LED is switched ON from normal world.
The second part of the demo is AHB secure controller GPIO mask feature. This feature is controlled by
button SW1. If the SW1 button is released, the GPIO mask feature is disabled so state of button SW4 can be read
from both secure and normal world. So user can see both LEDs switched ON when SW4 is pressed down.
If the SW1 button is pressed, the GPIO mask feature is enabled. Thus normal world cannot read state of 
SW1 button and logical zero only is read by application from normal world. Since logical zero corresponds to the pressed button 
the BLUE LED is switched ON while SW1 is pressed and cannot be further controlled by button SW4. The GREEN LED can be still controlled
by SW4 button since state of SW4 button is still read by secure GPIO from secure world.
 


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Use secure project to download the program to target board. Please refer to "TrustZone application debugging" below for details.
4.  Launch the debugger in your IDE to begin running the demo.
Note: Refering to the "Getting started with MCUXpresso SDK for EVK-MIMXRT595" documentation for more information
      on how to build and run the TrustZone examples in various IDEs.

Running the demo
================
Use SW1 button to control blue and green LED(D19, RGB LED) and SW2 to control GPIO mask feature.
If SW1 button is pressed (logical zero is read), the GREEN LED is switched on in secure world
and blue LED is switched on from normal world. So both blue and green LEDs are controlled by SW1 button.
If SW2 button is also pressed, the GPIO mask feature is activated. In this case the blue LED starts to light
regardless of SW1 button state, since normal GPIO cannot read state of SW1 button (only the logic 0 is read). 
The green LED can be still controlled by SW1 button.

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
environment. It includes SAU, MPU's, AHB secure controller and some TrustZone related registers from System Control Block.
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

TrustZone application debugging
-------------------------------
- Download both output file into device memory
- Start execution of secure project since secure project is going to be executed after device RESET.

If IDE (Keil MDK, IAR) allows to manage download both output files as single download, the secure project
is configured to download both secure and non-secure output files so debugging can be fully managed
from secure project.

For IAR, please don't choose Use flash loader option when download ram target output files.

If want to download secure and non-secure binary file into QSPI flash, should the following rules:
Flash target:
    1. secure binary download into 0x8000400 address.
    2. non-secure binary download into 0x8100000 address.
RAM target:
    1. secure binary download into 0x8000400 address for IAR, download into 0x80000000 for other toolchains.
    2. non-secure binary download into 0x8041000 address.

Device header file and secure/non-secure access to the peripherals
------------------------------------------------------------------
Both secure and non-secure project uses identical device header file. The access to secure and non-secure aliases for all peripherals
is managed using compiler macro __ARM_FEATURE_CMSE.

For secure project using <PERIPH_BASE> means access through secure alias (address bit A28=1), 
using <PERIPH_BASE>_NS means access through non-secure alias(address bit A28=0)
For non-secure project using <PERIPH_BASE> means access through non-secure alias (address bit A28=0). 
The non-secure project doesn't have access to secure memory or peripherals regions so the secure access is not defined.
