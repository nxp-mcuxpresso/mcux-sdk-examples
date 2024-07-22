Overview
========
The vref example shows how to use the vref driver.

In this example, the adc16 module is initiealized and used to measure the VREF output voltage. So, it cannot use interal
VREF as the reference voltage. Instead, it can use VDD_ANA or VREFH, for detailed information, please refer to device
datasheet.

Then, user should configure the VREF output pin as the ADC16's sample input. When running the project, it will measure
the VREF output voltage under different trim value.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============
Remove JP5 3-4,short JP5 5-6.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
VREF example
ADC Full Range: 65536
Default (Factory) trim value is :15

Use trim value: 0
ADC conversion result: 19861
Expected voltage on VREF_OUT: 1.000V
Actual voltage on VREF_OUT: 1.000V

Use trim value: 1
ADC conversion result: 21827
Expected voltage on VREF_OUT: 1.100V
Actual voltage on VREF_OUT: 1.099V

Use trim value: 2
ADC conversion result: 23812
Expected voltage on VREF_OUT: 1.200V
Actual voltage on VREF_OUT: 1.199V

Use trim value: 3
ADC conversion result: 25792
Expected voltage on VREF_OUT: 1.300V
Actual voltage on VREF_OUT: 1.299V

Use trim value: 4
ADC conversion result: 27776
Expected voltage on VREF_OUT: 1.400V
Actual voltage on VREF_OUT: 1.399V

Use trim value: 5
ADC conversion result: 29741
Expected voltage on VREF_OUT: 1.500V
Actual voltage on VREF_OUT: 1.498V

Use trim value: 6
ADC conversion result: 31790
Expected voltage on VREF_OUT: 1.600V
Actual voltage on VREF_OUT: 1.601V

Use trim value: 7
ADC conversion result: 33734
Expected voltage on VREF_OUT: 1.700V
Actual voltage on VREF_OUT: 1.699V

Use trim value: 8
ADC conversion result: 35763
Expected voltage on VREF_OUT: 1.800V
Actual voltage on VREF_OUT: 1.801V

Use trim value: 9
ADC conversion result: 37699
Expected voltage on VREF_OUT: 1.900V
Actual voltage on VREF_OUT: 1.898V

Use trim value: 10
ADC conversion result: 39667
Expected voltage on VREF_OUT: 2.000V
Actual voltage on VREF_OUT: 1.997V

Use trim value: 11
ADC conversion result: 41632
Expected voltage on VREF_OUT: 2.100V
Actual voltage on VREF_OUT: 2.096V

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
