Overview
========
The lpadc_trigger_exception example demonstrates the usage of trigger exception feature.

This example involves two conversion sequences. The first works in continuous mode, triggered by software to sample
external input. The second works in one-shot mode, triggered by an onboard button to sample VDD/4. The second sequence
has a higher priority than the first, meaning that if it is triggered, the first sequence will be aborted and the
second will begin executing at a specified breakpoint.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 board
- Personal Computer

Board settings
==============
- ADC CH0A input signal J8_21(PIO2_0).

Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the MCU-Link USB port on the target board.
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of this demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPADC Trigger Exception Example!
Please Press any key to start example
Press SW2 to start high priority conversion.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
