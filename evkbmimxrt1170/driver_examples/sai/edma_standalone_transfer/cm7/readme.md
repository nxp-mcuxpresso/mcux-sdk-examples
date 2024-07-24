Overview
========
The sai_edma_standalone_transfer example shows how to use sai driver with EDMA:

In this example, a sai instance records audio data from the input and processes its data as follows, Then restore the processed data and play it.

original data:
 --------------------------------------------------------------------------------------------------
 |LEFT CHANNEL | RIGHT CHANNEL | LEFT CHANNEL | RIGHT CHANNEL | LEFT CHANNEL | RIGHT CHANNEL | ...|
 --------------------------------------------------------------------------------------------------
received data:
 -------------------------------------------------------------------------------------------------------
 |LEFT CHANNEL | LEFT CHANNEL | LEFT CHANNEL | ...| RIGHT CHANNEL | RIGHT CHANNEL | RIGHT CHANNEL | ...|
 -------------------------------------------------------------------------------------------------------
 sent data:
 --------------------------------------------------------------------------------------------------
 |LEFT CHANNEL | RIGHT CHANNEL | LEFT CHANNEL | RIGHT CHANNEL | LEFT CHANNEL | RIGHT CHANNEL | ...|
 --------------------------------------------------------------------------------------------------

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- Headphone(OMTP standard)

Board settings
==============
To make the examples works, please remove below resistors if on board wifi chip is not DNP:
R228,R229,R232,R234
Ensure the resistors mounted below:
R2001,R2002,R2003,R2004,R2005,R2006 and R2007.

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
4. Insert the headphones into the headphone jack on MIMXRT1170-EVKB board (J101).
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note: This demo uses both headphone mic and board main mic(P1) as input source. The headphone mic provides left
channel data, and main mic (P1) provides right channel data. If users found there is noise while do record operation,
most probably it is the headphone standard issue (OMTP and CTIA standard difference). You should use the OMTP
standard headphone. You can disable the left channel by using an headphone without microphone feature.
This demo uses codec master mode(external mclk mode).

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI EDMA standalone transfer example started!
~~~~~~~~~~~~~~~~~~~
