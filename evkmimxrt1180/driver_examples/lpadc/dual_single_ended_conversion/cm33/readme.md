Overview
========

The lpadc_dual_single_ended_conversion example shows how to use two channel with LPADC driver.

In this example, user should indicate two channel to provide a voltage signal (can be controlled by user) as the LPADC's
sample input, Channel B does not need to be paired with Channel A, Channel B is user selectable.When running the project, 
typing any key into debug console would trigger the conversion. The execution would check the FIFO valid flag in loop until
the flag is asserted, which means the conversion is completed. Then read the conversion result value and print it to debug console.

Note, the default setting of initialization for the ADC converter is just an available configuration. User can change
the configuration structure's setting in application to fit the special requirement.


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
- ADC CH6B input signal J41-8(GPIO_AD_05).

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
LPADC Triger Dual Channel Example
ADC Full Range: 65536
Please press any key to get user channel's ADC value.

ADC channal A value: 35822
ADC channal B value: 35826

ADC channal A value: 35734
ADC channal B value: 35763

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The analog voltage input range is theoretically 0-1.8V. However, due to the workaround for ERR051152, the maximum voltage should be limited to 1.78V.
Due to the errata051385, the ADC reference voltage is connect to the VDDA_ADC_1P8.
