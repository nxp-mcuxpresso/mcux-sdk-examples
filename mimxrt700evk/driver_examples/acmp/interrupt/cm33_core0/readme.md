Overview
========
The ACMP Interrupt project is a simple demonstration program that uses the SDK software. It
compares the selected analog input with ACMP internal DAC output continuously and toggle the LED
when the final comparison result changed. The purpose of this demo is to show how to use the
ACMP driver in SDK software by interrupt way. The ACMP can be configured based on default
configuration returned by the API ACMP_GetDefaultConfig(). The default configuration is: high
speed is not enabled, invert output is not enabled, unfiltered output is not enabled, pin out
is not enabled, offset level is level 0, hysteresis level is level 0.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============
This example project uses the ACMP to compare with the voltage signal from external input(PIO10_8) J3-28 or J7-9. 
Please keep the external input voltage signal stable to avoid floating voltage.
The ACMP positive input voltage is from the internal DAC output, in this example, the internal DAC reference voltage is from
the internal 1.8v PAD and the DAC output voltage is half of the reference. 

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board. 
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
When the demo runs successfully, the following information can be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The example compares analog input to the reference DAC output(CMP positive port).
The terminal will print CMP's output value when press any key.
Please press any key to get CMP's output value.
The analog input is LOWER than DAC output
The analog input is HIGHER than DAC output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
