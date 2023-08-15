Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The cmsis_i2c_read_accel_value example shows how to use CMSIS I2C driver to communicate with an i2c device:

 1. How to use the i2c driver to read a i2c device who_am_I register.
 2. How to use the i2c driver to write/read the device registers.

In this example, the values of three-axis accelerometer print to the serial terminal on PC through
the virtual serial port on board.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini USB cable
- TWR-KM34Z50MV3 board
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
When the demo runs successfully, similar message will be displayed in the terminal:

I2C example -- Read Accelerometer Value
Found MMA8491 on board, the device address is 0x55. 
The accel values:
status_reg = 0xf , x =   -26 , y =    29 , z =  1036 
status_reg = 0xf , x =    -7 , y =    -6 , z =  1017 
status_reg = 0xf , x =    -4 , y =    43 , z =  1031 
status_reg = 0xf , x =   -22 , y =    33 , z =  1035 
status_reg = 0xf , x =   -18 , y =    35 , z =  1020 
status_reg = 0xf , x =     2 , y =    11 , z =  1029 
status_reg = 0xf , x =   -10 , y =    34 , z =  1029 
status_reg = 0xf , x =     1 , y =    15 , z =  1016 
status_reg = 0xf , x =    -8 , y =    41 , z =  1041 
status_reg = 0xf , x =   -21 , y =    20 , z =  1045 

End of I2C example .
