Overview
========
The pdm sai interrupt example shows how to use pdm driver with interrupt transaction api:

In this example, pdm gather audio data, and sai send it to codec.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMX8M Plus board
- J-Link Debug Probe
- 12V power supply
- Personal Computer
- Microphone
- Headphone

Board settings
==============
Connect microphone module to EVK board:
3V->J21-1
GND->J21-6
CLK->J21-40
DATA->J21-38
Connect headphone to J18

#### NOTE ####
1.  There are two microphones on module, please put the sound source in the middle of microphones for balance.
2.  Please note this application can't support running with Linux BSP!

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW3 to power on the board
2.  Connect a USB cable between the host PC and the J23 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs successfully, you can hear the sound gathered from microphone and the log would be seen on the debugger terminal like:

~~~~~~~~~~~~~~~~~~~
PDM sai interrupt example started!
~~~~~~~~~~~~~~~~~~~




