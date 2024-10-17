Overview
========
The ADC Low Power Async DMA demo application demonstrates the usage of the ADC and DMA peripheral while in a low power mode. The
microcontroller is first set to very low power stop (VLPS) mode. Every 100 ms, low power timer trigger the ADC module convert
value on ADC channel. After 16 times(1,6s) the DMA transfer finish interrupt wake up the CPU to process sampled data, print result to
user and toggle LED.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXC444 board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
2.  Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running
    the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ADC LOW POWER ASYNC DMA DEMO
ADC16_DoAutoCalibration() Done.
ADC Full Range: 65536
Please press any key to trigger ADC conversion.
ADC value: 15670
Please press any key to trigger ADC conversion.
ADC value: 16634
Please press any key to trigger ADC conversion.
ADC value: 16686
Please press any key to trigger ADC conversion.
ADC value: 16681
Please press any key to trigger ADC conversion.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
 - LED will be toggled when the CPU wake up.
