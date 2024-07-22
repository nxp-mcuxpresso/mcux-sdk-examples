Overview
========
The Hello World SWO demo prints the "SWO: Hello World" string to the SWO viewer. The purpose of this demo is to
show how to use the swo, and to provide a simple project for debugging and further development.

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
- MIMXRT1060-EVKC board
- Personal Computer
- Jlink

Board settings
==============
1. Install jumper JP5

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. This is used only to power the board.
2.  Connect Jlink to J2 as debugger and SWO probe.
3.  Download the program to the target board. This step needs to change the project settings according to the IDE used, please refer to AN13234.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the hello world swo demo in the JlinkerSWOView window and the demo will print output periodically or when SW5 is pressed:
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
   c:/Program Files/SEGGER/JLink/JLinkSWOViewerCL.exe -device MIMXRT1062xxx6B -itmport 0 -swoattach 1 -swofreq 4000000

