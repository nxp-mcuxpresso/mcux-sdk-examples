Overview
========

The lpdc_dma example shows how to use ADC to trigger a DMA transfer.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the LPADC's
sample input. When running the project, typing any key into debug console would trigger the conversion, the software trigger
API is called to start the conversion. When the ADC conversion is completed, it would trigger the DMA in ping-pong transfer
mode to move the ADC conversion result from ADC conversion data register to user indicated memory. Then the main loop waits
for the transfer to be done and print the result to terminal.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============
- ADC CH0A input signal J13-3(PIO0_23).

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
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

ADC Full Range: 65536

Please press any key to trigger the conversion.

Adc conversion word : 0x81008BA1

ADC conversion value: 35745

Adc conversion word : 0x81008BE7

ADC conversion value: 35815

Adc conversion word : 0x81008B14

ADC conversion value: 35604

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
