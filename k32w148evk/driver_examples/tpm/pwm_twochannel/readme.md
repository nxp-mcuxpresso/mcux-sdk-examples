Overview
========
The TPM project is a simple demonstration program of the SDK TPM driver. It sets up the TPM
hardware block to output PWM signals(24KHZ) on two TPM channels. The PWM dutycycle on both channels
is manually updated. The example sets up a TPM channel-pair to output two edge-aligned PWM signals.
On boards that have an LED connected to the TPM pins, the user will see
a change in LED brightness if user enter different values.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPM example for output compare

You will see the output signal toggle
Probe the signal using an oscilloscope
TPM example to output PWM on 2 channels

If an LED is connected to the TPM pin, you will see a change in LED brightness if you enter different values
If no LED is connected to the TPM pin, then probe the signal using an oscilloscope
Please enter a value to update the Duty cycle:
Note: The range of value is 0 to 9.
For example: If enter '5', the duty cycle will be set to 50 percent.
Value:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
