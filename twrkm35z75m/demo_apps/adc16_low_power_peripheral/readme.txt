Overview
========
The advanced Low Power ADC project is designed to work with the Tower System or in a stand alone setting.
The code of this demo has been prepared and updated for use with the MCUXpresso Configuration Tools (Pins/Clocks/Peripherals).
The ADC Low Power demo application demonstrates the usage of the ADC peripheral while in a low power mode. The
microcontroller is first set to very low power stop (VLPS) mode. Every 500 ms, an interrupt wakes up the ADC module and
reads the current temperature of the microcontroller. Increment or decrement the temperature to see the changes, red lights
for higher and blue one for lower than average counted temperatures. You can open the mex file with MCUXpresso Config Tool to 
do further configuration of pin, clock and peripheral.


Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
- Personal Computer

Board settings
==============
No special settings are required.

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
When the demo runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 ADC LOW POWER PERIPHERAL DEMO... ADC16_DoAutoCalibration() Done.

 OPERATING INSTRUCTIONS:
 The advanced Low Power ADC project is designed to work with the Tower System or in a stand alone setting.
 The code of this demo has been prepared and updated for use with the MCUXpresso Configuration Tools (Pins/Clocks/Peripherals).
 1. Set your target board in a place where the temperature is constant.
 2. Wait until the green LED light turns on, after initial temperature measurement finished.
 3. Increment or decrement the temperature to see the changes, red lights for higher and blue one for lower than average counted temperatures.
 Now wait until LED stops blinking...
 Enter any character to begin the demo...
 ---> OK! Main process is running...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

