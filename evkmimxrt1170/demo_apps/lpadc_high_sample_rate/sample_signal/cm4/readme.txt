Overview
========
This demo application demonstrates the use of the LPADC to sample the analog signal. In this demo, the ADC clock is
set as the maximum frequency, users should input analog signals to the ADC channel, press any keys to trigger the ADC
conversion. After sampling enough sample points, the ADC will be disabled and the sample data will be print via UART.
Users can use some tools such as EXCEL to recover the input signal based on those printed data.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
ADC1_IN0 is ADC input. Please sample voltage by J9-10 pin.

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
The log below shows the output of the lpadc sample rate count demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ADC High Sample Rate Demo: Sample Input Signal

Please input the analog signal to the ADC channel.
In order to recover the input signal as completely as possible,
please ensure that the frequency of the input signal is less than 1MHz

Please input any key to trigger ADC conversion.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
After getting enough sample point, the ADC will be stopped and the sample point data will be printed via UART.
Use some tools such as EXCEL to redraw the input signal based on those sample point data.
Please note that the amplitude of the input analog signal should be less than 1.8V.
