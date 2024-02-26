Overview
========

The DAC14 basic example shows how to use the DAC module simply as the general DAC converter. 
No support is needed to be triggered by DAC in the example using the buffer mode. The value 
written into DAC data register will be directly pushed to analog conversion.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 Board
- Personal Computer

Board settings
==============
DAC output pin: J3-2

Prepare the Demo
================
1. Connect the type-c and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.
5. A multimeter may be used to measure the DAC output voltage.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

DAC14 Basic Example.

Please input a value (0 - 16383) to output with DAC: 1000
Input value is 1000
DAC out: 1000

The user can measure the DAC output pin to check the output voltage.

