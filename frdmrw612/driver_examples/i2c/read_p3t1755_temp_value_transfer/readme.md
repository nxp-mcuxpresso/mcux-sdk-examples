Overview
========
The i2c_read_p3t1755_temp_value_transfer example shows how to use I2C driver to communicate with an i2c device p3t1755 to get temperature data.
In this example, the temperature values are printed to the serial terminal on PC.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============
No special settings

Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2C master read sensor data example.

Temperature:28.250000
Temperature:28.312500
Temperature:28.312500
Temperature:28.312500
Temperature:28.312500
Temperature:28.312500
Temperature:28.312500
Temperature:28.375000
Temperature:28.375000
Temperature:28.375000

End of demo.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
