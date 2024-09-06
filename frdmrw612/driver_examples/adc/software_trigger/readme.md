Overview
========
The adc_software_trigger example shows how to software trigger ADC conversion.

In this example, ADC resolution is set as 16bit, the reference voltage is selected as the internal 1.8V bandgap, and input
gain is set as 1. So the input voltage range is from 0 to 1.8V. Users can trigger ADC conversion by pressing any key in the
terminal. ADC interrupt will be asserted once the conversion is completed.

The conversion result can be calculated via the following formula:
    Conversion Result = (Vin / Vref) * 32767

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
No special settings

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
These instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
ADC Software Trigger Example!
Resolution: 16 bit.
Input Mode: Single Ended.
Input Range: 0V to 1.2V.

Calibration Success!
Please press any key to trigger conversion.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Connect the analog signal to J7-3(GPIO_46)
Note: Analog input voltage minimum needs to be higher than 0.2v.
