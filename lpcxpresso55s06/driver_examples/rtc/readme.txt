Overview
========
The RTC project is a simple demonstration program of the SDK RTC driver. It sets up the RTC
hardware block to trigger an alarm after a user specified time period. The test will set the current
date and time to a predefined value. The alarm will be set with reference to this predefined date
and time.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J1) on the target board.
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
The log below shows example output of the RTC driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RTC example: set up time to wake up an alarm
Current datetime: 2014-12-25 19: 0: 0
Please input the number of second to wait for alarm
The second must be positive value
Alarm will occur at: 2014-12-25 19: 0: 5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
