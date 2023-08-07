Overview
========

The demo shows how to measure the temperature within the PMC module.

In this demo, the ADC input channel is mapped to an internal temperature sensor. When running the project, typing
any key into debug console would trigger a series of ADC conversions. When the conversions are done, the temperature is
calculated per the specific formula.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PMC Temperature Sensor Demo
ADC Full Range: 4096
Please press any key to get temperature from the internal temperature sensor.
Current temperature: 28.59
Current temperature: 28.67
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
