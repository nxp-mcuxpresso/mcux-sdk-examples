Overview
========
The pdm edma transfer example shows how to use pdm driver with edma:

In this example, pdm will trigger edma to transfer data when one PDM channel watermark value is reached.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-LINK USB port (J54) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs successfully,  the log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~
PDM edma example started!
PDM recieve one channel data:
XXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXX
PDM edma example finished!
~~~~~~~~~~~~~~~~~~~


