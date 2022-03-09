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
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KV31F board
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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2C example -- Read Accelerometer Value
Found an FXOS8700 on board , the device address is 0x1d . 
The accel values:
status_reg = 0xff , x =   111 , y =    34 , z =  2077 
status_reg = 0xff , x =    64 , y =    85 , z =  2061 
status_reg = 0xff , x =    16 , y =    95 , z =  2094 
status_reg = 0xff , x =    63 , y =   385 , z =  2091 
status_reg = 0xff , x =   193 , y =   382 , z =  2080 
status_reg = 0xff , x =   309 , y =   214 , z =  1976 
status_reg = 0xff , x =    67 , y =    23 , z =  1987 
status_reg = 0xff , x =   165 , y =   280 , z =  2050 
status_reg = 0xff , x =   195 , y =   268 , z =  2054 
status_reg = 0xff , x =    42 , y =   189 , z =  2081 

End of I2C example .
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
