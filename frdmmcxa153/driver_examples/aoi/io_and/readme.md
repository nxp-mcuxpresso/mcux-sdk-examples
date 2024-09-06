Overview
========
The AOI IO_AND demo application demonstrates the use of IO and AOI peripherals.
The example Use the two IO as the input of the aoi function: AOI_OUT = (IO0 & IO1). 
When IO0 and IO1 are both high level, the AOI_OUT is high level.
When any of IO is low level, the AOI_OUT is low level.
The purpose of this demonstration is to show how to use the AOI driver in the SDK software.

Run the demo

After the demonstration runs successfully, you can see the following information in the terminal window:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
aoi_io_and project.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
AOI_OUT = EXT_TRIG_OUT0 P1_2 J6-5
IO0     = EXT_TRIG_IN0 P1_0 J6-6
IO1     = EXT_TRIG_IN1 P1_1 J6-4

AOI_OUT = IO0 & IO1

Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
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

The log below shows the output of the aoi demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AOI_io_and project.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When IO0 and IO1 are both high level, the AOI_OUT is high level.
When any of IO is low level, the AOI_OUT is low level.
