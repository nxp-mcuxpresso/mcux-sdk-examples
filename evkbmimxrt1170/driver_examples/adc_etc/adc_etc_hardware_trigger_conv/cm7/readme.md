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
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
ADC0_IN0 and ADC0_IN4 are ADC inputs. Please sample voltage by J9-10 and J9-16 pins.

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
The log below shows the output of the ADC_ETC hardware trigger conversion demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ADC_ETC_Hardware_Trigger_Conv Example Start!
ADC Full Range:4096 
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
The analog voltage input range about is 0-1.8v.
