Overview
========
The i2c_read_accel_value example shows how to use I2C driver to communicate with an i2c device:

 1. How to use the i2c driver to read a i2c device who_am_I register.
 2. How to use the i2c driver to write/read the device registers.

In this example, the values of three-axis accelerometer print to the serial terminal on PC through
the virtual serial port on board.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2C example -- Read Accelerometer Value
Found an FXOS8700 on board , the device address is 0x1e .
The accel values:
status_reg = 0xff , x =    20 , y =  -102 , z =  2102
status_reg = 0xff , x =    83 , y =   -51 , z =  2089
status_reg = 0xff , x =    66 , y =   -53 , z =  2101
status_reg = 0xff , x =    51 , y =   -80 , z =  2098
status_reg = 0xff , x =    15 , y =   -99 , z =  2105
status_reg = 0xff , x =    44 , y =   -89 , z =  2097
status_reg = 0xff , x =    53 , y =   -56 , z =  2099
status_reg = 0xff , x =    54 , y =   -78 , z =  2103
status_reg = 0xff , x =    25 , y =   -98 , z =  2098
status_reg = 0xff , x =    17 , y =  -118 , z =  2107

End of I2C example .
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
