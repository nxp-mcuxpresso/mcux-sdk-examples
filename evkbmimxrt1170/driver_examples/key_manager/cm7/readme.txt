Overview
========
The Key Manager example shows how to configure Key Manager to select PUF key as Master Key and locks this setting

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Key Manager driver example.
Key Manager Init.
Select PUF for Master key and lock register for writting.
Setting slot 0 (CSR_MASTER_KEY_CTRL register) configuration.
Example end.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
