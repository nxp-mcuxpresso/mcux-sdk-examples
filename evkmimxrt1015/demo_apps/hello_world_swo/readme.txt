Overview
========
The Hello World SWO demo prints the "SWO: Hello World" string to the SWO viewer. The purpose of this demo is to
show how to use the swo, and to provide a simple project for debugging and further development.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT1015 board
- Personal Computer
- Jlink

Board settings
==============
1. Remove jumper of J47, J48, J49, J50

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. This is used only to power the board.
2.  Connect Jlink to J55 as debugger and SWO probe.
3.  Download the program to the target board. This step needs to change the project settings according to the IDE used, please refer to AN13234.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the hello world swo demo in the JlinkerSWOView window and the demo will print output periodically or when SW4 is pressed:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SWO: timer_trigger
SWO: timer_trigger
SWO: timer_trigger
SWO: hello_world
SWO: timer_trigger
SWO: timer_trigger
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
   This demo targets to run SWO trace function in IDE debug mode and none-debug mode.
   In none-debug mode, may use below command(swoattach mode) with Jlink:
   c:/Program Files/SEGGER/JLink/JLinkSWOViewerCL.exe -device MIMXRT1015xxx5A -itmport 0 -swoattach 1 -swofreq 4000000

Note:
To download binary into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "hello_world_swo.bin".
3. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J41.
4. Drop the binaray into disk "RT1015-EVK" on PC.
5. Wait for the disk disappear and appear again which will take couple of seconds.
7. Reset the board by pressing SW3 or power off and on the board. 

Note:
To debug in qspiflash, following steps are needed:
1. Select the flash target and compile.
3. Set the SW8: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J41.
4. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to qspiflash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
