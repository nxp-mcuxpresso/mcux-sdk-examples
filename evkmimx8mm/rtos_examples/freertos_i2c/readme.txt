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
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- MIMX8MM6-EVK  board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
Please note only board to board connection is supported, on board connection is not supported.
Transfer data from MASTER_BOARD to SLAVE_BOARD of I2C interface, I2C3 pins of MASTER_BOARD are connected with I2C3 pins of SLAVE_BOARD.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SLAVE_BOARD        CONNECTS TO          MASTER_BOARD
Pin Name   Board Location     Pin Name   Board Location
I2C3_SCL       J1003-5         I2C3_SCL      J1003-5 
I2C3_SDA       J1003-3         I2C3_SDA      J1003-3
GND            J1003-9         GND           J1003-9
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

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


If using 2 boards, you can see the similar information from the terminal associated with master board and slave board as below. (Applicable to all boards)

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
