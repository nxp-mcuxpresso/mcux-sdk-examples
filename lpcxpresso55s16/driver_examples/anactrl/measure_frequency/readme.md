Overview
========
The anactrl_measure_frequency Example project is to demonstrate usage of the KSDK anactrl driver.
In the example, you can set a reference clock and target clock. The project will calculate the frequency of the
target clock according to the reference clock frequency.

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
- LPCXpresso55S16 board
- Personal Computer

Board settings
==============
The jumper setting:
    Default jumpers configuration does not work,  you will need to add JP20 and JP21 (JP22 optional for ADC use)

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the ANACTRL driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Analog control measure frequency example.

Reference clock source: wdt oscillator.

Target clock source: 32kHz oscillator.

Target clock frequency: 29799 Hz.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
