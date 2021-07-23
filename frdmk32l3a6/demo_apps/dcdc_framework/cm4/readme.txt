Overview
========
This demo is used to show a kind of usage of DCDC function. In this case, the DCDC is used as the voltage regulator instead of on-chip LDOs. For the condition that DCDC input voltage is not so stable, as the battery's voltage decrease after a few working time, a timer is used to trigger the adjustment by sampling the ADC value of bandgap/battery input voltage, calculating them and setting the result into the DCDC module, so that the output of DCDC would be adjusted and kepted constant as much as possible.

Actually, this use case is usually used in the connectivity application, and the period of adjust (configured by the timer's timeout period) is also setup according to requirement.


Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- FRDM-K32L3A6 board
- Personal Computer
- Micro-USB cable

Board settings
==============
Populate the capacitor C41 (10uF).

Prepare the demo
1.  Set the SoC booting up from core 0 (CM4 core), since the program would be downloaded and run in core 0.
2.  Connect the board (OpenSDA port) to PC with the Micro-USB cable.
3.  Open a serial terminal on PC for board's UART-RS232 with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Build the project and download the image into the board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

When the demo runs successfully, the log would be seen on the OpenSDA terminal like as below and LED will blink.
Press any key to display the latest measuring value and the battery voltage value.

~~~~~~~~~~~~~~~~~~~~~

dcdc_framework demo.
Press any key to trigger the measurement.

vBandgapValue: 11248
vBatteryValue: 37600
vBatteryMv   : 3342

~~~~~~~~~~~~~~~~~~~~~
