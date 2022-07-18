Overview
========
The XBAR AOI demo application demonstrates the usage of the XBAR, AOI peripheral to combine interrupt
from CMP and PIT. If the button is pressed(CMP interrupt) and the PIT periodic interrupt occurs a message is printed.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- IMXRT1050-EVKB board
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
The log below shows the output of the xbar aoi demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
XBAR and AOI Demo: Start...

Then change CMP analog input.
    - CMP1_IN0 (J23-5) connected to VCC =3.3V (J24-8): XBAR interrupt occurred
    - CMP1_IN0 (J23-5) connected to GND (J24-7)): XBAT interrupt stopped

XBAR interrupt occurred

XBAR interrupt occurred
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
