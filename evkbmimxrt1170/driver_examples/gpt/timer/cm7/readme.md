Overview
========
The gpt_timer project is a simple demonstration program of the SDK GPT driver. It sets up the GPT
hardware block to trigger a periodic interrupt after every 1 second. When the GPT interrupt is triggered
a message a printed on the UART terminal.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

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
When the example runs successfully, following information can be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~

Press any key to start the example
s
Starting GPT timer ...
 GPT interrupt is occured !
 GPT interrupt is occured !
 GPT interrupt is occured !
 GPT interrupt is occured !
 .
 .
 .
 GPT interrupt is occured !
 .
 .
 .
~~~~~~~~~~~~~~~~~~~~~
