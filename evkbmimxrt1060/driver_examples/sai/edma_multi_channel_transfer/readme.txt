Overview
========
The sai_edma_multi_channel example shows how to use sai driver playback multi channel data with EDMA:

In this example, one sai instance playbacks the audio data stored in flash/SRAM using EDMA channel.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1060-EVKB board
- Personal Computer
- Headphone(OMTP standard)
- CS42448 Audio board

Board settings
==============
Make sure R113 is welded on MIMXRT1060-EVKB board.
Insert AUDIO board into J23.

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
When the demo runs successfully, you can hear the tone playback from J6/J7 and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI EDMA multi channel transfer example started!
SAI EDMA multi channel transfer example finished!
~~~~~~~~~~~~~~~~~~~
