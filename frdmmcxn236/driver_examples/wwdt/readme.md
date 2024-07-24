Overview
========
The WWDT Example project is to demonstrate usage of the KSDK wdog driver.
In this example,quick test to show user how to feed watch dog.
User need to feed the watch dog in time before it warning or timeout interrupt.
The WINDOW register determines the highest TV value allowed when a watchdog feed is
performed. 

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J10) on the target board.
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
If the demo run successfully, you will see the red LED blink every two seconds.
The log below shows example output of the WWDT driver demo in the terminal window:
--- Time out reset test start ---
Watchdog reset occurred

--- Window mode refresh test start---
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!

