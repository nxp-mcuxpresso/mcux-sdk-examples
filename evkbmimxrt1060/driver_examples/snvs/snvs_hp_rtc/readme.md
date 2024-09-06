Overview
========
The SNVS HP RTC project is a simple demonstration program of the SDK SNVS HP driver. The test will set up RTC date and time to a predefined value and starts the counter. RTC then triggers an alarm after a user specified time period. The alarm will be set with reference to this predefined date and time.

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
- MIMXRT1060-EVKB board
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
The log below shows the output of snvs example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SNVS HP example:
Set up time to wake up an alarm.
Current datetime: 2014-12-25 19:00:00
Please input the number of second to wait for alarm and press enter
The second must be positive value
Alarm will occur at: 2014-12-25 19:00:16

 Alarm occurs !!!! Current datetime: 2014-12-25 19:00:16
Please input the number of second to wait for alarm and press enter
The second must be positive value

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
