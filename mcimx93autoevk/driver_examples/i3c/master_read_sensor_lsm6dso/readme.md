Overview
========
The example shows how to use i3c driver as master to read on board sensor lsm6dso.

The example will read WHO_AM_I value from lsm6dso.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93AUTO-EVK board
- Personal Computer

Board settings
==============
Populate R1010 with 10KΩ

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and JLink Plus to the board, switch SW2 to power on the board
2.  Connect a USB Type-C cable between the host PC and the J26 USB port on the target board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I3C master read sensor data example.

Start to do I3C master transfer in SDR mode.

Success to read WHO_AM_I register value from LSDM6DSO on board in I3C SDR mode, the value is 0x6C.
~~~~~~~~~~~~~~~~~~~~~~~~~~~
