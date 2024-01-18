Overview
========
The WWDT Example project is to demonstrate usage of the KSDK wdog driver.
In this example,quick test to show user how to feed watch dog.
User need to feed the watch dog in time before it warning or timeout interrupt.
The WINDOW register determines the highest TV value allowed when a watchdog feed is
performed. 

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============


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
The log below shows example output of the WWDT driver demo in the terminal window:

--- Time out reset test start ---
Watchdog reset occurred

--- Window mode refresh test start ---
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
 WDOG has been refreshed!
