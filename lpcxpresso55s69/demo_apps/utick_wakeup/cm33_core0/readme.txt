Overview
========
The purpose of this demo is to show wakeup from deep sleep mode using MicroTick timer.
The demo sets the MicroTick Timer as a wake up source and puts the device in deep-sleep mode. 
The MicroTick timer wakes up the device. 

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
1. Pin P19_2 is used as a CLKOUT signal. Connect this pin to an Oscilloscope for viewing the signal. 

Running the demo
================
The demo sets the MicroTick Timer as a wake up source and puts the device in sleep mode. 
The MicroTick timer wakes up the device.

 After wake up：

(1)the red LED on the board blinking. 
(2)CLKOUT signal changing can be seen on pin P19_2.And the frequency of  output signal will double.

If the demo run successfully, you will see the log from debug console:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Utick wakeup demo start...
Wakeup from sleep mode...

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
