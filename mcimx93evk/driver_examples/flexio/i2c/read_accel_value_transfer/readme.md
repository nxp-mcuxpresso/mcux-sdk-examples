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

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-EVK  board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
The example requires doing connection between FLEXIO1(as master) pins and LPI2C1(as slave) pins.
- LPI2C1_SCL(J1005-3), IOMUXC_PAD_GPIO_IO02__FLEXIO1_FLEXIO02(J1001-3) connected
- LPI2C1_SDA(J1005-5), IOMUXC_PAD_GPIO_IO03__FLEXIO1_FLEXIO03(J1001-5) connected

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
FlexIO I2C example read accelerometer value
Found a LSDM6DSO on board, the device address is 0x6A.
The accel values:
status_reg = 0x5 , x = -28924 , y =  5127 , z = 24640
status_reg = 0x5 , x = 31748 , y =  9991 , z = 27456
status_reg = 0x5 , x = 28164 , y = 14599 , z = 15680
status_reg = 0x5 , x = 24068 , y = 21767 , z = 31808
status_reg = 0x5 , x = 13828 , y = -32505 , z = 22848
status_reg = 0x5 , x = 10500 , y = -26105 , z = 16192
status_reg = 0x5 , x =  6916 , y = -26105 , z = 17472
status_reg = 0x5 , x =  8452 , y = -25081 , z = 21056
status_reg = 0x5 , x =  8964 , y = -26105 , z = 22080
status_reg = 0x5 , x =  7940 , y = -24569 , z = 20800

End of I2C example .
~~~~~~~~~~~~~~~~~~~~~
