Overview
========
The Hello World SWO demo prints the "SWO: Hello World" string to the SWO viewer. The purpose of this demo is to
show how to use the swo, and to provide a simple project for debugging and further development.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT595-EVK board
- Personal Computer
- Jlink plus

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and J39 on the target board, the J40 must be left unconnected so that the Link2 Debug probe is left unpowered and does not contend with SWD.
2.  Connect Jlink plus probe to the SWD connector(J2).
3.  Download the program to the target board.
4.  Open JlinkerSWOView(Such as C:\Program Files (x86)\SEGGER\JLink\JlinkerSWOView.exe), select the target device, such as MIMXRT595S_M33 and make sure the core clock and swo clock frequency are equal to the demo setting, 
    the SWO clock frequency is defined in hello_world_swo.c by macro DEMO_DEBUG_CONSOLE_SWO_BAUDRATE(4MHz by default).
5.  After swo/core clock frequency is measured successfully, press ok to continue, make sure the bit 0 is selected in "Data from stimulus port(s)" item.
6.  Press the reset button on your board.
7.  Press SW2.
8.  Note: If use MCUxpresso IDE, you need to set the SDK debug console to UART when importing projects at
    Project Options -> SDK Debug Console -> UART.

Running the demo
================
The log below shows the output of the hello world swo demo in the JlinkerSWOView window and the demo will print output periodically or when SW2 is pressed:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SWO: timer_trigger
SWO: timer_trigger
SWO: timer_trigger
SWO: hello_world
SWO: timer_trigger
SWO: timer_trigger
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
