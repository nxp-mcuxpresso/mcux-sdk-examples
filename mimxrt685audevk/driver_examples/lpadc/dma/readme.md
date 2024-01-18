Overview
========

The lpadc_dma example shows how to use ADC to trigger a DMA transfer.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the LPADC's
sample input. When running the project, typing any key into debug console would trigger the conversion, the software trigger
API is called to start the conversion. When the ADC conversion is completed, it would trigger the DMA in ping-pong transfer
mode to move the ADC conversion result from ADC conversion data register to user indicated memory. Then the main loop waits
for the transfer to be done and print the latest ADC conversion word and ADC conversion value average to terminal.

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
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============
- Set VREF_L to GND, VREF_H to 1.8V (connect JP9, JP10).
- Input voltage signal(0~1.8V) to J30-1(LPADC0 CH0).

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, following information can be seen on the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPADC DMA Example

ADC Full Range: 4096

Full channel scale (Factor of 1).

Please press any key to trigger the conversion.

Adc conversion word : 0x81006148

ADC conversion value: 3113

Adc conversion word : 0x81006140

ADC conversion value: 3112

Adc conversion word : 0x81006140

ADC conversion value: 3112
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
