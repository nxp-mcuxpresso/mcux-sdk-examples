Overview
========
The flexio_i2c_read_accel_value example shows how to use FLEXIO I2C  Master driver to communicate with an i2c device:

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
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
To make this example work, connections needed to be as follows:

    FLEXIO_I2C        connected to  LPI2C2
SCL     J41-2           -->        J44-20
SDA     J41-4           -->        J44-18

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~
FlexIO I2C example read accelerometer value
Found a FXL8974 on board, the device address is 0x18.
The accel values:
status_reg = 0x81 , x =    -8 , y =    11 , z =  1024
status_reg = 0x81 , x =    -8 , y =    18 , z =  1044
status_reg = 0x81 , x =    -4 , y =    -4 , z =  1007
status_reg = 0x81 , x =    -8 , y =    -4 , z =  1006
status_reg = 0x81 , x =     3 , y =     7 , z =  1044
status_reg = 0x81 , x =     3 , y =   -19 , z =  1023
status_reg = 0x81 , x =   -12 , y =     7 , z =  1066
status_reg = 0x81 , x =   -16 , y =    11 , z =  1039
status_reg = 0x81 , x =     7 , y =     3 , z =  1019
status_reg = 0x81 , x =    -8 , y =     0 , z =  1023

End of I2C example .
~~~~~~~~~~~~~~~~~~~~~

Note:
if sensor chip(U115) is not welded on the board, the log would be seen on the OpenSDA terminal like:

FlexIO I2C example read accelerometer value

Not a successful i2c communication

End of I2C example .
