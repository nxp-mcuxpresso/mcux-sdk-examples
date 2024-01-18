Overview
========
The I2C example shows an application using Azure RTOS threads with the I2C driver:

Two I2C instances of a single board are used. One I2C instance is used as
I2C master and another I2C instance is used as I2C slave.

Two threads are created. The first thread is associated with I2C master operation
and the second thread deals with I2C slave operation.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1020 board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:

    LPI2C1 SCL(GPIO_AD_B1_14, J18-6)    -->    LPI2C2 SCL(GPIO_AD_B1_08, J17-2)
    LPI2C1 SDA(GPIO_AD_B1_15, J18-5)    -->    LPI2C2 SDA(GPIO_AD_B1_09, J17-1)

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
When the example runs successfully, the output is:

Start i2c_example

Master will send data:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Slave received data:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

OK. Data is matched.
