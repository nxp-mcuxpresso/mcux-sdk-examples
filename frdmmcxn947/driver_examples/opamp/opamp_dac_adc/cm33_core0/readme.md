Overview
========

The OPAMP DAC ADC example demonstrates how to use the OPAMP PGA mode. 
In this example, both the positive and negative inputs should be connected 
to GND, the positive reference voltage is set to the DAC output, and the OPAMP
output is 1X the DAC output. When the DAC output changes, the OPAMP changes accordingly.

In PGA mode, the OPAMP output pin does not support direct measurement. Only the ADC is 
supported to sample the OPAMP output voltage, and the output range of the OPAMP positive
reference voltage is 0 to VDDA-0.8V.

  

  

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
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
Positive input channel0 J4-1(P4_12) and negative input channel J4_3(OPAMP0_INN) connect to GND.

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the CMSIS DAP USB port (J17) on the board
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
OPAMP DAC ADC EXAMPLE!

Please input a value (0 - 4095) for DAC:1500
Input DAC value is 1500
Current DAC output is about 1209.000 mV
Please press any key excluding key (R or r) to get user channel's ADC value.
Vsw1 ADC value: 1499
Actual voltage on Vsw1: 1.208V
Vref ADC value: 1498
Actual voltage on Vref: 1.207V


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Vref stands for positive reference voltage output while Vsw1 stands for OPAMP output voltage. 
OPAMP output voltage ranges around 0 to VDDA-0.8V.
