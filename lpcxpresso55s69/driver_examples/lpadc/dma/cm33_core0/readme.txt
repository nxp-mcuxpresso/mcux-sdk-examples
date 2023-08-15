Overview
========

The lpadc_dma example shows how to use ADC to trigger a DMA transfer.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the LPADC's
sample input. When running the project, typing any key into debug console would trigger the conversion, the software trigger
API is called to start the conversion. When the ADC conversion is completed, it would trigger the DMA in ping-pong transfer
mode to move the ADC conversion result from ADC conversion data register to user indicated memory. Then the main loop waits
for the transfer to be done and print the latest ADC conversion word and ADC conversion value average to terminal.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
- Connect a jumper to P15-2 and P15-3.
- ADC CH0A input signal P28-1(PIO0-23).

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
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
The log below shows the output of the lpadc dma example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPADC DMA Example

ADC Full Range: 4096

Full channel scale (Factor of 1).

Please press any key to trigger the conversion.

Adc conversion word : 0x81006148

ADC conversion value: 3113

Adc conversion word : 0x81006140

ADC conversion value: 3112

Adc conversion word : 0x81006140

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
