Overview
========
The sai_edma_transfer example shows how to use sai driver with EDMA:

In this example, one sai instance playbacks the audio data stored in flash/SRAM using EDMA channel.

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
- Personal Computer
- Headphone(OMTP standard)

Board settings
==============
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
note:This demo uses codec master mode(internal pll mode) and outputs 215HZ sine wave audio signal.

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI EDMA example started!
SAI EDMA example finished!
~~~~~~~~~~~~~~~~~~~
