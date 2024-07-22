Overview
========
The pdm hwvad example shows how to use pdm driver with hwvad:


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

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
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


Running the demo
================

When the demo runs successfully, please speak or play a song nearby microphone,
then you will see log on the terminal like:
~~~~~~~~~~~~~~~~~~~
PDM hwvad example started!

Voice detected

Voice detected

...

PDM hwvad example finished!
~~~~~~~~~~~~~~~~~~~

Note: There is a variable in this example which initial as 50, once voice detected, it will decrease by one until 0, then example finish.
