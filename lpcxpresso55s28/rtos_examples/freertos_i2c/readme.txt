Overview
========
The freertos_i2c example shows an application using RTOS tasks with I2C driver:

Two I2C instances of the single board are used. One i2c instance is used as I2C master and another I2C instance is used as I2C slave.
Two tasks are created. First task is associated with an I2C master operation and second task deals with I2C slave operation.
Example program flow:
    1. I2C master task sends data to I2C slave task.
    2. I2C slave task receive data from I2C master task and send back inverted value of received data.
    3. I2C master task reads data sent back from I2C slave task.
    4. The transmit data and the receive data of both I2C master task and I2C slave task are printed out on terminal.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S28 board
- Personal Computer

Board settings
==============
To make i2c example work, connections needed to be as follows:
        I2C1              connected to      I2C4
SCL     P0_14(P19 pin9)        -->          P1_20(P17 pin1)
SDA     P0_13(P19 pin11)       -->          P1_21(P17 pin3)

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (P6) on the board.
2. Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
When the example runs successfully:
If using 1 board, you can see the similar information from the terminal as below. (Applicable to all boards except TWR-KM34Z75M)



==FreeRTOS I2C example start.==

This example use one i2c instance as master and another as slave on one board.

Master will send data :

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7

0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f



I2C slave transfer completed successfully.



Slave received data :

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7

0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f



This time , slave will send data: :

0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8

0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0

0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8

0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0



Master received data :

0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8

0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0

0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8

0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0





End of FreeRTOS I2C example.
