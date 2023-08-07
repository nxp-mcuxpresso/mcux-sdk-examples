Overview
========
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This demo explains how built in HWVAD( HW voice activity detector) in devices can be used to
wake up the device from sleep mode

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
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
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Configure DMIC

2. This example wakes up microcontroller on any voice activity.
   Speak on DMIC,you can see blinking Red LED(D19). Also serial message "Going into sleep" and "Just woke up" will be transmitted.
