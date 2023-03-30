Overview
========
The flexio_lpi2c_interrupt example shows how to use flexio i2c master  driver in interrupt way:

In this example, a flexio simulated i2c master connect to a LPI2C slave

Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
To make this example work, connections needed to be as follows:

    FLEXIO_I2C        connected to  LPI2C5
SCL     J26-2           -->        J10-20
SDA     J26-4           -->        J10-18

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
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
You can see the similar message shows following in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FlexIO I2C interrupt - LPI2C interrupt
Master will send data :
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1A  0x1B  0x1C  0x1D  0x1E  0x1F

Slave received data :
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1A  0x1B  0x1C  0x1D  0x1E  0x1F
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
In ARMCC: When reading from the flash, 64M flash requires 4 bytes of address, which leads to slow code 
execution in the flash and flexio FIFO underflow. The project use a method to reduce the baud rate. 
Other methods include putting fsl_flexio_i2c.c fsl_flexio_i2c_master.c fsl_lpi2c.c code in RAM 
or modifying qspiflash_config in evkbmimxrt1170_flexspi_nor_config.c file to use shorter flash address and smaller flash space.
