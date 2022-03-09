Overview
========

The src_global_source example shows how to check the global system reset source and clear the global system reset status flags.

Check whether the predicted global reset flag has been asserted when running this example firstly on the board after it being powered up.
If not, enable the predicted global system reset, after which the predicted global reset flag will be asserted. Then clear the flag.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
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
When the demo runs successfully, the log would be seen on the terminal like:

Example: SRC Global System Reset.
Example: SRC Global System Reset.
Global System Reset Occurred.

