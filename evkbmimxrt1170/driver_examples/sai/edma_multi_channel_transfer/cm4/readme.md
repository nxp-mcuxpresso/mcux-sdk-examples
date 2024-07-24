Overview
========
The sai_edma_multi_channel example shows how to use sai driver playback multi channel data with EDMA:

In this example, one sai instance playbacks the audio data stored in flash/SRAM using EDMA channel.

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
- MIMXRT1170-EVKB board
- Personal Computer
- Headphone(OMTP standard)
- CS42448 Audio board

Board settings
==============
1.Please mount resistors below,
R2008,R2022,R2011,R2021,R2009,R2010,R2012,R2016,R1998,R2013,R2014,
R2018,R2017,R2000

2.Remove the resistors below:
R2001,R2002,R2003,R2004,R2005,R2006 and R2007.

3.Insert AUDIO board into J76.

Prepare the Demo
================
Note: The demo support CS42448 codec only.

1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4. Insert the headphones into the headphone jack J6 or J7 on the audio board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note: This demo outputs 1KHZ sine wave audio signal.
When the demo runs successfully, you can hear the tone playback from J6/J7 and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI EDMA multi channel transfer example started!
SAI EDMA multi channel transfer example finished!
~~~~~~~~~~~~~~~~~~~
