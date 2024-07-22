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
- USB Type-C cable
- MCIMX93AUTO-EVK board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW2 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J26 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the flash.bin to the target board.
    Reference 'How to get flash.bin of TrustZone examples'
    Reference 'Getting Started with MCUXpresso SDK for MCIMX93AUTO-EVK.pdf' to make and download flash.bin.
5.  Press the reset button on your board.

How to get flash.bin of TrustZone examples:
1.  Create a directory named 'tz_bin' in your work direcory.
2.  Build {app}_s and copy sdk20-app.bin into 'tz_bin' then rename it as sdk20-app_s.bin.
3.  Build {app}_ns and copy sdk20-app.bin into 'tz_bin' then rename it as sdk20-app_ns.bin.
4.  Enter the 'tz_bin' directory and execute 'dd' command to combin the two files.
    dd if=sdk20-app_ns.bin of=sdk20-app_s.bin bs=1 count=`ls -l ./sdk20-app_ns.bin|awk '{print $5}'` seek=65536
5.  Rename sdk20-app_s.bin as m33_image.bin, and copy it into imx-mkimage
6.  Build the low power boot flash.bin


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
Address that caused SAU violation is 0x30003000.
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
