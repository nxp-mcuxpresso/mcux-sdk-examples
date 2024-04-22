Overview
========

The sdramc example shows how to use SEMC controller driver to initialize the external SDRAM chip.



SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
1.  Remove R203, R205-R227 resisters.
2.  Populate R151-R165, R168-R188 with 0ohm resisters.
3.  Remove resistors R2114, R2115, R2118, R2119, R2122, R2123, R2126, R2127, R2130, R2131, R332, R333, to 
    disconnect HyperRAM interface.
4.  Populate these resistors with 0ohm resistor, such as R1329, R2594, R2595, R189-R196, R198, R200-R202, R204.
5.  Remove these resistors, such as R2596, R2597, R228-R235, R238, R240, R242, R244, R246, R248, R250, R252.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEMC SDRAM Example Start!

 SEMC SDRAM Memory 32 bit Write Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM Read 32 bit Data Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM 32 bit Data Write and Read Compare Start!

 SEMC SDRAM 32 bit Data Write and Read Compare Succeed!

 SEMC SDRAM Memory 16 bit Write Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM Read 16 bit Data Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM 16 bit Data Write and Read Compare Start!

 SEMC SDRAM 16 bit Data Write and Read Compare Succeed!

 SEMC SDRAM Memory 8 bit Write Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM Read 8 bit Data Start, Start Address 0x80000000, Data Length 4096 !

 SEMC SDRAM 8 bit Data Write and Read Compare Start!

 SEMC SDRAM 8 bit Data Write and Read Compare Succeed!

 SEMC SDRAM Example End.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
