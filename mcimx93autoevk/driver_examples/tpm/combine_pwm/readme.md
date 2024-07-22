Overview
========
The TPM project is a demonstration program of generating a combined PWM signal by the SDK TPM driver. It sets up the TPM
hardware block to output PWM signals(24kHZ) on two TPM channels. The example also show's the complementary mode of operation
and deadtime insertion. The PWM duty cycle is manually updated and is initially 10%.
On boards that have 2 LEDs connected to the TPM pins, the user will see
a change in LED brightness if user enter different values.
And if the board do not support LEDs to show, the outputs can be observed by oscilloscope.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93AUTO-EVK board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer
- Oscilloscope

Board settings
==============
- CM33: connect J17-16(TPM2_CHN0), J17-29(GND) to Oscilloscope
- CM33: connect J17-18(TPM2_CHN1), J17-29(GND) to Oscilloscope

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW2 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J26 USB port on the cpu board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TPM example to output combined complementary PWM signals on two channels

If an LED is connected to the TPM pin, you will see a change in LED brightness if you enter different values
If no LED is connected to the TPM pin, then probe the signal using an oscilloscope
Please enter a value to update the Duty cycle:
Note: The range of value is 1 to 9.
For example: If enter '5', the duty cycle will be set to 50 percent.
Value:5
The duty cycle was successfully updated!

Please enter a value to update the Duty cycle:
Note: The range of value is 1 to 9.
For example: If enter '5', the duty cycle will be set to 50 percent.
Value:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
