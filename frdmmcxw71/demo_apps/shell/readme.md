Overview
========
The Shell Demo application demonstrates to control Leds by commands.
Note: Please input one character at a time. If you input too many characters each time, the receiver may overflow
because the low level UART uses simple polling way for receiving. If you want to try inputting many characters each time,
just define DEBUG_CONSOLE_TRANSFER_NON_BLOCKING in your project to use the advanced debug console utility.
Besides, the demo does not support semihosting mode. The shell component is based on debug console and 
serial manager. When semihosting is used, debug console and serial manager are bypassed. So the shell demo cannot
work with semihosting.

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
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the FRDM board J10.
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
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright  2024  NXP

SHELL>> help

"help": List all the registered commands

"exit": Exit program

"led arg1 arg2":
 Usage:
    arg1: 1|2|3|4...            Led index
    arg2: on|off                Led status
SHELL>> led 1 on
SHELL>> led 1 off
SHELL>> led 2 on
SHELL>> led 2 off
SHELL>> led 3 on
SHELL>> led 3 off
SHELL>> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
