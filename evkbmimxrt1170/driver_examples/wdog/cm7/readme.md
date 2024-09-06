Overview
========
The WDOG Example project is to demonstrate usage of the KSDK wdog driver.
In this example,implemented to test the wdog.
Please notice that because WDOG control registers are write-once only. And
for the field WDT, once software performs a write "1" operation to this bit,
it can not be reset/cleared until the next POR, this bit does not get reset/
cleared due to any system reset. So the WDOG_Init function can be called 
only once after power reset when WDT set, and the WDOG_Disable function can 
be called only once after reset.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~
******** System Start ********
System reset by: Power On Reset!

- 1.Testing System reset by software trigger.
******** System Start ********
System reset by: Software Reset!

- 2.Testing system reset by WDOG timeout.
--- wdog Init done---

******** System Start ********
System reset by: Time Out Reset!

- 3.Test the WDOG refresh function by using interrupt.
--- wdog Init done---

WDOG has be refreshed!
WDOG has be refreshed!
WDOG has be refreshed!

~~~~~~~~~~~~~~~~~~~~~
