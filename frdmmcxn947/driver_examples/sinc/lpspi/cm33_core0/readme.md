Overview
========
The SINC LPSPI example demonstrates how to use the SINC driver to convert SPI output bitstreams to a data stream.
The LPSPI output the default value continuously, and the SINC module is connected to the LPSPI's output to resume default data.
The SINC module works in trigger mode, and once a user inputs any key via the terminal, one SINC conversion will be triggered.
The bitstream source is set as an external bitstream that connects to the SPI MOSI signal, and the input clock source is set as MCK_OUT0.
Theoretically, the triggered SINC result should be equal to the SPI's output value, but there may be some deviations between different conversions.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
Note: 
Jumper setting:
LPSPI MOSI(J2_8) connect to SINC MBIT0(J8_18)

Other jumpers keep default configuration.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the MCU-Link USB port on the board.
2. Open a serial terminal on PC for MCU-Link serial device with these settings:
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SINC LPSPI Example.
LPSPI default output value: 0x7f

Press any key to trigger sinc conversion!

SINC Result:0x7F


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

