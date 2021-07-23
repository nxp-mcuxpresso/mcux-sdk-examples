Overview
========
This document explains the freertos_swtimer example. It shows usage of software timer and its
callback.

The example application creates one software timer SwTimer. The timer’s callback SwTimerCallback is
periodically executed and text “Tick.” is printed to terminal.




Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini USB cable
- HVP-KV11Z75M board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.
Running the demo
================
After the board is flashed the Tera Term will show output message.

Example output:
Tick.
Tick.
Tick.
