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
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the CMSIS DAP USB port (J1) on the target board.
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
SHELL (build: Jan 25 2016)
Copyright (c) 2015 Freescale Semiconductor

SHELL>> help

"help": Lists all the registered commands

"led arg1 arg2":
 Usage:
    arg1: 1|2|3|4...            Led index
    arg2: on|off                Led status

"exit": Exit program

SHELL>> led 1 on
SHELL>> led 1 off
SHELL>> led 2 on
SHELL>> led 2 off
SHELL>> led 3 on
SHELL>> led 3 off
SHELL>> led 4 on
LED index is wrongs
SHELL>>
~~~~~~~~~~~~~~~~~~~~~
