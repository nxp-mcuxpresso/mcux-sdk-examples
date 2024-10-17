Overview
========

The lpadc_interrupt example shows how to use interrupt with LPADC driver.

In this example, user should indicate a channel to provide a voltage signal (can be controlled by user) as the LPADC's
sample input. When running the project, typing any key into debug console would trigger the conversion. ADC watermark 
interrupt would be asserted once the number of datawords stored in the ADC Result FIFO is greater than watermark value.
In ADC ISR, the watermark flag would be cleared by reading the conversion result value. Also, result information would
be printed when the execution return to the main function.


SDK version
===========
- Version: 2.16.100

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
- ADC CH7A input signal J41-2(GPIO_AD_02).

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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPADC Interrupt Example
ADC Full Range: 65536
Full channel scale (Factor of 1).
Please press any key to get user channel's ADC value.
ADC interrupt count: 1
ADC value: 10538
ADC interrupt count: 2
ADC value: 10538
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The analog voltage input range is theoretically 0-1.8V. However, due to the workaround for ERR051152, the maximum voltage should be limited to 1.78V.
Due to the errata051385, the ADC reference voltage is connect to the VDDA_ADC_1P8.
