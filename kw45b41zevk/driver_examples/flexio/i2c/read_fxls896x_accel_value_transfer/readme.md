Overview
========
The flexio_i2c_read_fxls896x_accel_value example shows how to use FLEXIO I2C Master driver to communicate with an i2c device:

 1. How to use the flexio i2c master driver to read a i2c device who_am_I register.
 2. How to use the flexio i2c master driver to write/read the device registers.

In this example, the values of three-axis accelerometer print to the serial terminal on PC through
the virtual serial port on board.

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
- KW45B41Z-EVK Board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FlexIO I2C example read accelerometer value
Found a FXLS8967 on board, the device address is 0x18.
The accel values:
x =    30 , y =   -12 , z =  1071
x =    30 , y =   -12 , z =  1071
x =    30 , y =   -12 , z =  1071
x =    19 , y =    18 , z =  1078
x =    19 , y =    18 , z =  1078
x =    41 , y =    -8 , z =  1082
x =    41 , y =    -8 , z =  1082
x =    26 , y =    11 , z =  1086
x =    26 , y =    11 , z =  1086
x =    26 , y =    11 , z =  1086

End of I2C example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
