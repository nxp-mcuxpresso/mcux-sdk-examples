Overview
========
The freertos_i2c example shows an application using RTOS tasks with I2C driver:

The example may support 2 different connections (it depends on the specific board): 
On board connection and board to board connection.

With one board connection, 2 I2C instances of the same board are used. One i2c instance used as I2C master and another I2C instance used as I2C slave .
    Default settings in freertos_i2c.c (in folder boards\<board>\rtos_examples\freertos_i2c) is applied.
    Two tasks are created. One task is associated with an I2C master operation and another task deals with I2C slave operation.
    1. I2C master task sends data to I2C slave task.
    2. I2C master task reads data sent back from I2C slave task.

    The transmit data and the receive data of both I2C master task and I2C slave task are printed out on terminal.

With board to board connection, one I2C instance on one board is used as I2C master and the I2C instance on other board is used as I2C slave. Tasks are created to run on each board to handle I2C communication.
    File freertos_i2c.c should have following definitions:
    #define EXAMPLE_CONNECT_I2C BOARD_TO_BOARD
    For board used as I2C master:
        #define I2C_MASTER_SLAVE isMASTER
    For board used as I2C slave:
        #define I2C_MASTER_SLAVE isSLAVE



Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini USB cable
- TWR-KM34Z50MV3 board
- Personal Computer

Board settings
==============
Using two TWR-KM34Z75M boards:
Connection between two boards as follow:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD        CONNECTS TO          SLAVE_BOARD
Pin Name   Board Location     Pin Name   Board Location
I2C0_SCL       J25-12         I2C0_SCL      J25-12
I2C0_SDA       J25-11         I2C0_SDA      J25-11
GND            J25-10         GND           J25-10
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  In file freertos_i2c.c in folder: boards\twrkm34z75m\rtos_examples\freertos_i2c, do following definition:
           For master, use default definition
                Build project.
                Download the program to one target board (used as master board).
           For slave, use following definition
                #define I2C_MASTER_SLAVE isSLAVE
                Build project.
                Download the program to one target board (used as slave board).
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully,
For 2 TWR-KM34Z75M boards, you can see the similar information from the terminal associated with master board and slave board as below.

For master:

==FreeRTOS I2C example start.==

This example use two boards to connect with one as master and another as slave.

Master will send data :

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7

0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f



Master received data :

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7

0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f





End of FreeRTOS I2C example.


For slave:


==FreeRTOS I2C example start.==

This example use two boards to connect with one as master and another as slave.

I2C slave transfer completed successfully.



Slave received data :

0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7

0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f

0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f





End of FreeRTOS I2C example.
