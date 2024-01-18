Overview
========
The Shell Demo application demonstrates ram and flash memory access commands.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
The example is uses QSPI0. The module uses PTB8, PTB15 ~PTB19 pins.
No special jumper needs to be connected.

#### Please note this application can't support running with Linux BSP! ####

#### Please note that this application must be built with ram link file! Because if running the application in QSPI
flash in place, the QSPI flash erase/program feature of the application might affect the instruction fetch. If QSPI
execution in place(XIP) is really needed, please make sure the flash erase/program operation must be handled by RAM
function and disable all interrupts to avoid jumping into interrupt handler in flash area during flash updating. ####

#### It's better to use IAR to debug this application. If you use u-boot to copy the image file to the qspi 
please do not use "flasherase" and "flashwrite" command to do some opperation from the offset address 0x0 to 0x10000.
This range is reserved for the application image. ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

NOTE: Don't build this example with flash linkage file since this example need to modify QSPI controller, so program itself cannot be run in QSPI flash.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below. Use corresponding command to read and modify memory.

~~~~~~~~~~~~~~~~~~~~~
SHELL (build: Nov 25 2016)
Copyright (c) 2015 Freescale Semiconductor
SHELL>> help

"help": Lists all the registered commands

"exit": Exit program

"memget8 arg1 arg2":  read memory in 8bit unit
 Usage:
    arg1: addr            memory address
    arg2: count           memory read count

"memget16 arg1 arg2": read memory in 16bit unit
 Usage:
    arg1: addr            memory address
    arg2: count           memory read count

"memget32 arg1 arg2": read memory in 32bit unit
 Usage:
    arg1: addr            memory address
    arg2: count           memory unit read count

"memset8 arg1 arg2 arg3":  write memory in 8bit unit
 Usage:
    arg1: addr            memory address
    arg2: value           memory write value
    arg3: count           memory unit write count

"memset16 arg1 arg2 arg3": write memory in 16bit unit
 Usage:
    arg1: addr            memory address
    arg2: value           memory write value
    arg3: count           memory unit write count

"memset32 arg1 arg2 arg3": write memory in 32bit unit
 Usage:
    arg1: addr            memory address
    arg2: value           memory write value
    arg3: count           memory unit write count

"memcmp8 arg1 arg2 arg3":  compare memory in 8bit unit
 Usage:
    arg1: addr1           memory address1
    arg2: addr2           memory address2
    arg3: count           memory unit compare count

"memcmp16 arg1 arg2 arg3": compare memory in 16bit unit
 Usage:
    arg1: addr1           memory address1
    arg2: addr2           memory address2
    arg3: count           memory unit compare count

"memcmp32 arg1 arg2 arg3": compare memory in 32bit unit
 Usage:
    arg1: addr1           memory address1
    arg2: addr2           memory address2
    arg3: count           memory unit compare count

"memcpy8 arg1 arg2 arg3":  copy memory in 8bit unit
 Usage:
    arg1: dst             destination memory address
    arg2: src             source memory address
    arg3: count           memory unit copy count

"memcpy16 arg1 arg2 arg3": copy memory in 16bit unit
 Usage:
    arg1: dst             destination memory address
    arg2: src             source memory address
    arg3: count           memory unit copy count

"memcpy32 arg1 arg2 arg3": copy memory in 32bit unit
 Usage:
    arg1: dst             destination memory address
    arg2: src             source memory address
    arg3: count           memory unit copy count

"flasherase arg1 arg2": erase QSPI flash
 Usage:
    arg1: offset          flash memory offset in bytes
    arg2: bytes           flash memory size in bytes

"flashwrite arg1 arg2 arg3": write QSPI flash
 Usage:
    arg1: addr            source memory address
    arg2: offset          flash memory offset in bytes
    arg3: bytes           flash memory size in bytes
SHELL>>
~~~~~~~~~~~~~~~~~~~~~
