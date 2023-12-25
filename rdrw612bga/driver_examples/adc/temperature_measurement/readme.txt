Overview
========
The adc_temperature_measurement examples shows how to measure internal temperature sensor.

By selecting internal voltage reference 1.2V, 16-bit audio ADC accuracy and by measuring the internal
temperature sensor, the temperature is calculated according to the following formula:
            Tmeans(in C) = Conversion Result/TS_GAIN - TS_OFFSET.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
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
ADC Temperature Measurement Example!

Calibration Success!
Please press any key to get internal diode temperature.




~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

