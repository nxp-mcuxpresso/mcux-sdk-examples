Overview
========
The gpt_compare project is a simple demonstration program of the SDK GPT driver's output compare feature. Once content of an OCRx matches the value in GPT_CNT, output compare timer pin is toggled.

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
* Probe the signal using an oscilloscope
 - At J25-11

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
~~~~~~~~~~~~~~~~~~~~~~~~
Compare output example
~~~~~~~~~~~~~~~~~~~~~~~
