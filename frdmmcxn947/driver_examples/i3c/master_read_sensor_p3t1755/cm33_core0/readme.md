Overview
========
The example shows how to use i3c driver as master to read temperature data from sensor P3T1755.

The example will do dynamic address assignment to the P3T1755 sensor, read the temperature data
and print it in while loop.

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
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J17) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Press reset button to begin running the demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I3C master read sensor data example.

Temperature:27.000000

Temperature:27.000000

......

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
