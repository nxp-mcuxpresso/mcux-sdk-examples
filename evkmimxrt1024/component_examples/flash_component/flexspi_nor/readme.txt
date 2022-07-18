Overview
========
The nor flash demo application demonstrates the use of nor flash component to erase, program, and read an
external flash device.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1024-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
***NOR Flash Component Demo Start!***


***NOR Flash Page 0 Read/Write Success!***


***NOR Flash Page 1 Read/Write Success!***


***NOR Flash Page 2 Read/Write Success!***


.....


***NOR Flash Page 13 Read/Write Success!***


***NOR Flash Page 14 Read/Write Success!***


***NOR Flash Page 15 Read/Write Success!***

***NOR Flash All Pages Read/Write Success!***

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
To download binary into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "hello_world.bin".
2. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J23.
3. Drop the binaray into disk "RT1024-EVK" on PC.
4. Wait for the disk disappear and appear again which will take couple of seconds.
5. Reset the board by pressing SW3 or power off and on the board. 

Note:
To debug in qspiflash, following steps are needed:
1. Select the flash target and compile.
2. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J23.
3. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to qspiflash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
