Overview
========
The flash_swap example shows how to use flash swap feature:


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE17Z512 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~
 PFLASH Swap example Start
 PFlash Information: 
 Total Program Flash Size: xx KB, Hex: (xx)
 Total Program Flash Block Count: xx
 Program Flash Sector Size: xx KB, hex: (xx)
 Flash is xx
 Original swap indicator address: xx
 Current swap system status: xx
 Validating the backup example image...
 Backup example image is invalid 
 Start to program backup example image
 Finish programming backup example image
 Checking the system status...
 It is first swap for the system
 Start to erase test data on lower pflash before swapping system
 Finish erasing test data
 Start to program backup test data on upper pflash
 Finish programming backup test data
 Start to swap the system

 Perform a system reset

 FLASH Swap example Start
 PFlash Information: 
 Total Program Flash Size: xx KB, Hex: (xx)
 Total Program Flash Block Count: xx
 Program Flash Sector Size: xx KB, hex: (xx)
 Flash is xx
 Original swap indicator address: xx
 Current swap system status: xx
 Validating the backup example image...
 Backup example image is valid
 Checking the system status...
 The system has been swapped successfully
 Start to erase test data on lower pflash before exiting this example
 Finish erasing test data

 End of PFLASH Swap example
~~~~~~~~~~~~

