Overview
========
The sai_sdma_transfer example shows how to use sai driver with SDMA:

In this example, one sai instance playbacks the audio data stored in flash/SRAM/DDR using SDMA channel.

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMX8M Plus board
- J-Link Debug Probe
- 12V power supply
- Personal Computer
- Headphone

Board settings
==============
No special settings are required.

#### NOTE ####
1.  Please note this application can't support running with Linux BSP!

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
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
note:This demo uses codec master mode(internal pll mode).

When the demo runs successfully, you can hear the tone and the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~
SAI SDMA example started!
SAI SDMA example finished!
~~~~~~~~~~~~~~~~~~~

