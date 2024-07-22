Overview
========
The lpi2c_read_fxls896x_accel_value example shows how to use LPI2C driver to communicate with an lpi2c device:

 1. How to use the lpi2c driver to read a lpi2c device who_am_I register.
 2. How to use the lpi2c driver to write/read the device registers.

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
- Type-C USB cable
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============
No special is needed.

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
5.  Please note, this example could only run on the EVK board.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

LPI2C example -- Read Accelerometer Value
Found an FXLS8964 on board , the device address is 0x19 . 
The accel values:
x =    11 , y =  -103 , z =  1071
x =    11 , y =  -103 , z =  1071
x =    11 , y =  -103 , z =  1071
x =    15 , y =  -130 , z =  1085
x =    15 , y =  -130 , z =  1085
x =     3 , y =  -126 , z =  1066
x =     3 , y =  -126 , z =  1066
x =     0 , y =  -111 , z =  1070
x =     0 , y =  -111 , z =  1070
x =     0 , y =  -111 , z =  1070

End of LPI2C example.
