Overview
========
The lpadc sample rate count demo application can be used to measure ADC's sample rate roughly. The sample rate
for an ADC is defined as the number of output samples available per unit time, and is specified as samples per
second(SPS).
The sample rate is the reciprocal of one sample total conversion time. Total conversion time = sampling time + compare time
In this demo, users can select sample time, and the compare time is fixed to 17.5 ADC clock cycles.
In this demo the ADC conversion clock is configured as the maximum frequency(Please refer to the datasheet for the
maximim value). So theoretically, sample rate = maximum frequency / total conversion time.


Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

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
ADC High Sample Rate Demo!
Current ADC clock frequency is 88004352 Hz!

Please select sample time!
        A -- 3 ADCK cycles total sample time
        B -- 5 ADCK cycles total sample time
        C -- 7 ADCK cycles total sample time
        D -- 11 ADCK cycles total sample time
        E -- 19 ADCK cycles total sample time
        F -- 35 ADCK cycles total sample time
        G -- 67 ADCK cycles total sample time
        H -- 131 ADCK cycles total sample time
a
Please press any keys to trigger ADC conversion!
Sample Rate: 4187264 SPS

Please select sample time!
        A -- 3 ADCK cycles total sample time
        B -- 5 ADCK cycles total sample time
        C -- 7 ADCK cycles total sample time
        D -- 11 ADCK cycles total sample time
        E -- 19 ADCK cycles total sample time
        F -- 35 ADCK cycles total sample time
        G -- 67 ADCK cycles total sample time
        H -- 131 ADCK cycles total sample time
h
Please press any keys to trigger ADC conversion!
Sample Rate: 590464 SPS

Please select sample time!
        A -- 3 ADCK cycles total sample time
        B -- 5 ADCK cycles total sample time
        C -- 7 ADCK cycles total sample time
        D -- 11 ADCK cycles total sample time
        E -- 19 ADCK cycles total sample time
        F -- 35 ADCK cycles total sample time
        G -- 67 ADCK cycles total sample time
        H -- 131 ADCK cycles total sample time

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
