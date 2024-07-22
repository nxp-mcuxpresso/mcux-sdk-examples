Overview
========
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This demo explains how built in HWVAD( HW voice activity detector) in devices can be used to
wake up the device from sleep mode

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
- MIMXRT685-AUD-EVK boards
- Personal Computer

Board settings
==============
Connect JP44 1-2,JP45 1-2

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Configure DMIC

2. This example wakes up microcontroller on any voice activity.
   Speak on DMIC,you can see blinking GREEN LED(D10). Also serial message "Going into sleep" and "Just woke up" will be transmitted.
