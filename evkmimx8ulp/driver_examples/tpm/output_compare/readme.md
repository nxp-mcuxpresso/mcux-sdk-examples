Overview
========
The TPM project is a demonstration program of the SDK TPM driver's output compare feature.
It sets up one TPM channel to toggle the output when a match occurs with the channel value. The user
should probe the TPM output with a oscilloscope to see the signal toggling.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer
- Oscilloscope

Board settings
==============
- CM33: connect J11-3 and J11-5(GND) to Oscilloscope

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the cpu board.
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
TPM example for output compare
You will see the output signal toggle
Probe the signal using an oscilloscope
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
