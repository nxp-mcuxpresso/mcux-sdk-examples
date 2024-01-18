Overview
========
The FTM pwm two channel Example project is a demonstration program that uses the KSDK software to generate a square 
pulse PWM(24kHZ) on 2 channel to control the LED brightness. It sets up the FTM hardware block to output two edge-aligned PWM signals.
The PWM dutycycles are periodically updated.
- FTM generates a PWM with the increasing and decreasing duty cycle.
- LED brightness is increasing and then dimming. This is a continuous process.
The user should probe the FTM output with a oscilloscope to see the PWM.

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
- LPCXpresso860MAX board
- An oscilloscope
- Personal Computer

Board settings
==============
The FTM Pins are J1_5 and J1_7.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
   - 9600 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~
FTM example to output PWM on 2 channels

You will see a change in LED brightness if an LED is connected to the FTM pin

If no LED is connected to the FTM pin, then probe the signal using an oscilloscope

~~~~~~~~~~~~~~~~~~~~~~~
