Overview
========
The FTM project is a demonstration program of the SDK FTM driver's dual-edge capture feature.
This feature is available only on certain SoC's.
The example sets up a FTM channel-pair for dual-edge capture. Meanwhile, the capture mode is set up one-shot, 
using API FTM_SetupDualEdgeCapture(). Once the input signal is received and the second edge is detected,
the interrupt flag will be cleared and the interrupt overflow will be disabled.
The example gets the capture value of the input signal using API FTM_GetInputCaptureValue().
And this example will print the capture values and period of the input signal on the terminal window.

If high level input is detected on the channel when FTM timer starts, FTM will generate a capture event incorrectly
although there is no rising edge. So this example ignore first pluse and measure second pluse. Need to ensure to
input least two pluse into the channel, a pwm signal is recommended. 

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE16Z board
- Personal Computer

Board settings
==============
Connect input signal to J2-7

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FTM dual-edge capture example
Once the input signal is received the input capture values are printed
The input signal's pulse width is calculated from the capture values & printed

Capture value C(n)V=3

Capture value C(n+1)V=5a8

Input signals pulse width=24 us
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
