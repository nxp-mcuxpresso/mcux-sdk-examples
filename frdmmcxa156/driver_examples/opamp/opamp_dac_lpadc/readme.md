Overview
========

The OPAMP DAC LPADC example demonstrates how to use the OPAMP driver. In this example, 
the OUTSW, BUFFEN, ADCSW1, and ADCSW2 are enabled, and the user can use the ADC to measure the
OPAMPx_INT voltage and the OPAMPx_OBS voltage. The user needs to connect the OPAMP positive
input port and negative input port to GND. The positive (OPAMPx_INP0) reference voltage is
set to the DAC output, the user selects the OPAMP negative gain by using the PC keyboard, and the
positive gain is fixed to 1x, So, the OPAMP output voltage is equal to ((1 + Ngain) / 2)Vpref.

Please note that the user needs to ensure that the serial port terminal used is CR instead of LF
or CR+LF when pressing the enter key.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 Board
- Personal Computer

Board settings
==============
Positive input channel J2-12(P2_12, OPAMP0_INP0) connects to GND.
Negative input channel J2_8(P2_13, OPAMP0_INN) connects to GND.
OPAMP output pin: J8-5(P2_15).
Note that on the FRDM-MCXA156 board, the resistor R60 is connected as A-type by default, for running this example, 
the resistor R60 needs to connect as B-type, which enables the OPAMP0_INN to connect to the J2_8. If the user doesn't 
want to rework the board, the user also can connect R60-3 to the GND directly.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port on the board
2.  Open a serial terminal with the following settings (See Appendix A in the Getting Started guide for the description of how to determine the serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
 OPAMP DAC LPADC EXAMPLE.
 Please input a value (0 - 4095) for DAC, press the entry button to indicate the end of input:400
 Input DAC value is 400
 The current DAC output is about 322.400 mV
 Please select OPAMP negative port gain from the following options.
 Input 0 means select 1x.
 Input 1 means select 2x.
 Input 2 means select 4x.
 Input 3 means select 8x.
 Input 4 means select 16x.
 Input 5 means select 33x.
 Input 6 means select 64x.
 3
 Input OPAMP negative port gain is 8x
 Please press any key excluding key (R or r) to get the user channel's ADC value.
 Vsw1 ADC value: 1805
 Actual voltage on Vsw1: 1.454V
 Vpref ADC value: 1807
 Actual voltage on Vpref: 1.456V

 Please input a value (0 - 4095) for DAC, press the entry button to indicate the end of input:500
 Input DAC value is 500
 The current DAC output is about 403.000 mV
 Please select OPAMP negative port gain from the following options.
 Input 0 means select 1x.
 Input 1 means select 2x.
 Input 2 means select 4x.
 Input 3 means select 8x.
 Input 4 means select 16x.
 Input 5 means select 33x.
 Input 6 means select 64x.
 1
 Input OPAMP negative port gain is 2x
 Please press any key excluding key (R or r) to get the user channel's ADC value.
 Vsw1 ADC value: 752
 Actual voltage on Vsw1: 0.606V
 Vpref ADC value: 751
 Actual voltage on Vpref: 0.605V

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
