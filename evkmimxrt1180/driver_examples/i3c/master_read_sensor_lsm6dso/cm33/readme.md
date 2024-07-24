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
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
Weld 1.5kΩ to R778, weld 4.7kΩ to R779.

Prepare the Demo
================
1.  Connect 5V power supply and JLink Plus to the board, switch SW10 to power on the board
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I3C master read sensor data example.

Start to do I3C master transfer in I2C mode.

Success to read WHO_AM_I register value from LSDM6DSO on board, the value is 0x6C.
~~~~~~~~~~~~~~~~~~~~~~~~~~~
