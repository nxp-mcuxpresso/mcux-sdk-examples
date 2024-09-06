Overview
========
The lpuart_edma Ring Buffer Example project is to demonstrate usage of the KSDK lpuart driver.
In the example, a ring buffer will be used with EDMA enabled, data received will be stored in ring 
buffer first, while IDLE line was detected, routine will read out the received characters, and echo back
them to the terminal by using EDMA mode. so, you can see the your pressed characters on the terminal.

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
- MIMXRT1060-EVKC board
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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPUART EDMA ring buffer example.
Board will send back received characters:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
