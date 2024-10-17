Overview
========
The TPM project is a demonstration program of the SDK TPM driver's dual-edge capture feature.
This feature is available only on certain SoC's.
The example sets up a TPM channel-pair for dual-edge capture. Meanwhile, the capture mode is set up one-shot, 
using API TPM_SetupDualEdgeCapture(). Once the input signal is received and the second edge is detected,
the interrupt flag will be cleared and the interrupt overflow will be disabled.
The example gets the capture value of the input signal using API TPM_GetChannelValue().
This example will print the capture values and period of the input signal on the terminal window.

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
- KW45B41Z-EVK Board
- Personal Computer

Board settings
==============
connect input signal to J1-8

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPM dual-edge capture example

Once the input signal is received the input capture values are printed
The input signal's pulse width is calculated from the capture values & printed

Capture value C(n)V=11d5a52

Capture value C(n+1)V=11d5a60

Input signals pulse width=2 us
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
