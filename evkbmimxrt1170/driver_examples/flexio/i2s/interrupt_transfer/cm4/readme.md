Overview
========
The flexio_i2s_interrupt example shows how to use flexio_i2s driver with interrupt:

In this example, flexio acts as I2S module to record data from codec line and playbacks the recorded data at the same time using interrupt.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Headphone(OMTP standard)
- Personal Computer

Board settings
==============
- J101, headphone connected
- connect FLEXIO pins to the pad as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Pin Name   Board Location     Pin Name    Board Location
RX_DATA    J26-2              RX_DATA     J99-2
TX_DATA    J26-4              TX_DATA     J100-2
SYNC       J26-6              SYNC        J97-2
BCLK       J26-8              BCLK        J98-2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================

Note: This demo uses both headphone mic and board main mic(P1) as input source. The headphone mic provides left
channel data, and main mic (P1) provides right channel data. If users found there is noise while do record operation,
most probably it is the headphone standard issue (OMTP and CTIA standard difference). You should use the OMTP
standard headphone. You can disable the left channel by using an headphone without microphone feature.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When the demo runs successfully, the log would be seen on the OpenSDA terminal like as below.

~~~~~~~~~~~~~~~~~~~~~
FLEXIO_I2S interrupt example started!

FLEXIO_I2S interrupt example finished!
~~~~~~~~~~~~~~~~~~~~~
