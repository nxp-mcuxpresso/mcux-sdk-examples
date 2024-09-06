Overview
========
The RTWDOG Example project is to demonstrate usage of the KSDK rtwdog driver.
In this example, fast testing is first implemented to test the rtwdog.
After this, refreshing the watchdog in None-window mode and window mode is executed.
Note rtwdog is disabled in SystemInit function which means rtwdog is disabled
after chip emerges from reset.

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
- MIMXRT1180-EVK board
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
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~
----- Fast test starts -----
Low Byte test starts
Waiting for timeout reset
Low Byte test succeeded
----- High Byte test starts -----
Waiting for timeout reset
High Byte test succeeded
----- The end of RTWDOG fast test -----

----- Refresh test start -----
----- None-window mode -----
Refresh rtwdog 1 time
Refresh rtwdog 2 time
Refresh rtwdog 3 time
Refresh rtwdog 4 time
Refresh rtwdog 5 time
Refresh rtwdog 6 time
Waiting for time out reset
None-window mode reset succeeded
----- Window mode -----
Refresh rtwdog 1 time
Refresh rtwdog 2 time
Refresh rtwdog 3 time
Waiting for time out reset
Window mode reset succeeded
~~~~~~~~~~~~~~~~~~~~~

Note
Reset event will happen when running this demo, so container header is needed. Please add container header by spsdk.
