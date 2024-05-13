Overview
========

The adc_etc_hardware_trigger_conv example shows how to use the ADC_ETC to generate a ADC trigger by PIT channel0 trigger.

Every 1 second, PIT channel0 would send a trigger signal to ADC_ETC, which can arbitrate and manage multiple external triggers,
and ADC_ETC would generate ADC trigger.

In this example, the ADC is configured with hardware trigger. Once ADC gets the trigger from the ADC_ETC, the conversion goes,
then the ADC_ETC ISR would be executed.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1040-EVK board
- Personal Computer

Board settings
==============
ADC1_IN9 and ADC1_IN10 are ADC inputs. Please sample voltage by J33-3 and J33-4 pins.

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ADC_ETC_Hardware_Trigger_Conv Example Start!
ADC_DoAutoCalibration() Done.
ADC Full Range: XXXX
Please press any key to get user channel's ADC value.
ADC conversion value is 107 and 3882
ADC conversion value is 103 and 3884
ADC conversion value is 104 and 3880
ADC conversion value is 88 and 3890
ADC conversion value is 88 and 3890
ADC conversion value is 88 and 3890
ADC conversion value is 104 and 3882
ADC conversion value is 104 and 3882
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

