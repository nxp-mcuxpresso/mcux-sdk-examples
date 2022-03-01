Overview
========

The demo shows how to measure the temperature within the PMC module.

In this demo, the ADC input channel is mapped to an internal temperature sensor. When running the project, typing
any key into debug console would trigger a series of ADC conversions. When the conversions are done, the temperature is
calculated per the specific formula.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PMC Temperature Sensor Demo
ADC Full Range: 4096
Please press any key to get temperature from the internal temperature sensor.
Current temperature: 32.59
Current temperature: 32.67
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
