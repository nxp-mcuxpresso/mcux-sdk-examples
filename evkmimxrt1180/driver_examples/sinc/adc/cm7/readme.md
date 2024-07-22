Overview
========
The sinc adc example shows how to use SINC driver to configure SINC module as single conversion mode to convert
an external ADC sigma-delta modulator bitstream to a data stream. For this example, the over sample ratio is set as 128,
the order is 3, and the SINC result data is signed.
Theoretically,
When ADCin = 0V, ADC output 50% duty of MBIT, sinc result is 0x0(If unsigned mode, sinc result is 128^3/2 = 0x10_0000);
when ADCin = + maximum input voltage, ADC output always output 1(100% duty), SINC output is 128^3 = 0x20_0000.
when ADCin = - maximum input voltage, ADC output always output 0(0% duty), SINC output is -128^3 = 0xE0_0000(If unsigned
mode, sinc result is 0x0).
For ADCin between -maximum input voltage and +maximum input voltage, the SINC result is ADCin/maximum * 0x20_0000.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
1. Please check ADC chip(U45) on EVK board, if it is AMC1106M05 please connect TP41 to U45's pin 7.
2. Connect input voltage to J49(If ADC chip is AMC1106M05, intput voltage ranges from -50mV to +50mV; If ADC chip is
    AD7402, input voltage ranges from -200mV to +200mV).
3. If using the AMC1106M05, note that a differential input of 50 mV will produce a stream of ones and zeros that are
   high 89.06% of the time, not 100%. So result can be verified based on equation: SINC result =(ADCin / 64mV) * (128^3 )

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
The log below shows the output of the SINC ADC demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SINC ADC Example.

Press any key to trigger conversion!

Adc Result: 1193

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
