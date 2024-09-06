Overview
========
The flexio_spi_master_interrupt example shows how to use flexio spi master  driver in interrupt way:

In this example, a flexio simulated master connect to a flexio simulated spi slave .

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
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
MOSI       J5 Pin 6                         MOSI      J5 pin 6
MISO       J5 pin 3                         MISO      J5 Pin 3
SCK        J5 pin 4                         SCK       J5 pin 4
PCS        J5 pin 5                         PCS       J5 pin 5
GND        J6 pin 7                         GND       J6 pin 7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board. 
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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FlexIO SPI interrupt example
Master Start...

Master runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
