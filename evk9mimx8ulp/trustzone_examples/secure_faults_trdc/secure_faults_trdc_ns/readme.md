Overview
========
The Secure Faults demo application demonstrates handling of different secure faults. This application is based
on application Hello World. In addition, user can invoke different secure faults by setting testCaseNumber variable (see source code).
The following faults can be invoked:

TEST 0: No any secure fault
TEST 1: Invalid transition from secure to normal world
TEST 2: Invalid entry point from normal to secure world
TEST 3: Invalid data access from normal world, example 1
TEST 4: Invalid input parameters in entry function
TEST 5: Invalid data access from normal world, example 2

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
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

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
The log below shows the output of the secure_faults demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!
This is a text printed from normal world!
Comparing two string as a callback to normal world
String 1: Test1
String 2: Test2
Both strings are not equal!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Change testCaseNumber value in secure_faults_s project to triger different secure error.
The available testCaseNumber values are defined in veneer_table.h as macro,
    #define FAULT_NONE                0
    #define FAULT_INV_S_TO_NS_TRANS   1
    #define FAULT_INV_S_ENTRY         2
    #define FAULT_INV_NS_DATA_ACCESS  3
    #define FAULT_INV_INPUT_PARAMS    4
    #define FAULT_INV_NS_DATA2_ACCESS 5
In case some secure error is raised, the information about error will be printed in the terminal window.

With testCaseNumber = FAULT_INV_S_TO_NS_TRANS, the below log will be shown in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.

Entering HardFault interrupt!
SAU->SFSR:INVTRAN fault: Invalid transition from secure to normal world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With testCaseNumber = FAULT_INV_S_ENTRY, the below log will be shown in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!

Entering HardFault interrupt!
SAU->SFSR:INVEP fault: Invalid entry point to secure world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With testCaseNumber = FAULT_INV_NS_DATA_ACCESS, the below log will be shown in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!

Entering HardFault interrupt!
SAU->SFSR:AUVIOL fault: SAU violation. Access to secure memory from normal world.
Address that caused SAU violation is 0x30000000.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With testCaseNumber = FAULT_INV_INPUT_PARAMS, the below log will be shown in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!
Input data error: String is not located in normal world!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

With testCaseNumber = FAULT_INV_NS_DATA2_ACCESS, the below log will be shown in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Hello from secure world!
Entering normal world.
Welcome in normal world!

Entering HardFault interrupt!
SCB->BFSR:PRECISERR fault: Precise data access error.
Address that caused secure bus violation is 0x20040000.
TRDC MBC slavemem:1 Violent access
    domainId        : 0
    Address         : 0x20040000
    Error state     : 2
    Error attribute : 7
    Error type      : 0
    Error port      : 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

TrustZone Application Development in SDK
----------------------------------------
Every TrustZone based application consists of two independent parts - secure part/project and non-secure part/project.

The secure project is stored in <application_name>\<application_name>_s directory.
The non-secure project is stored in <application_name>\<application_name>_ns directory.

The secure projects always contains TrustZone configuration and it is executed after device RESET. The secure project usually
ends by jump to non-secure application/project.
If IDE allows the two projects in single workspace, the user can also find the project with <application_name>.
This project contains both secure and non-secure projects in one workspace (armgcc, IAR) and it allows to user easy transition from
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
