Overview
========
The LPI2C Example project is a demonstration program that uses the KSDK software to manipulate the Low Power Inter-
Integrated Circuit.
The example uses two instances of LPI2C, one in configured as master and the other one as slave.
The LPI2C master sends data to LPI2C slave. The slave will check the data it receives and shows the log.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE17Z board
- Personal Computer

Board settings
==============
Please note only board to board connection is supported.
Transfer data from MASTER_BOARD to SLAVE_BOARD of I2C interface, I2C0 pins of MASTER_BOARD are connected with I2C0 pins of SLAVE_BOARD.
The example default works as MASTER_BOARD, need change the macro "#define I2C_MASTER_SLAVE isMASTER" to "#define I2C_MASTER_SLAVE isSLAVE" in
freertos_lpi2c.c for SLAVE_BOARD. And the SLAVE_BOARD should start before the MASTER_BOARD.

To make this example work, connections needed to be as follows:
MASTER_BOARD        CONNECTS TO         SLAVE_BOARD
Pin Name     Board Location     Pin Name     Board Location
LPI2C0_SCL   J2-18              LPI2C0_SCL   J2-18
LPI2C0_SDA   J2-20              LPI2C0_SDA   J2-20
GND          J2-14              GND          J2-14

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
When the example runs successfully, you can see the similar information from the terminal as below.
Single Board:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPI2C example -- MasterInterrupt_SlaveInterrupt.
Master will send data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

I2C master transfer completed successfully.
I2C slave transfer completed successfully. 


 Transfer successfully!
 
Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Board to Board:
Master side
~~~~~~~~~~~~~
LPI2C example -- MasterInterrupt_SlaveInterrupt.
Master will send data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

I2C master transfer completed successfully.
~~~~~~~~~~~~~
Slave side:
~~~~~~~~~~~~~

LPI2C example -- MasterInterrupt_SlaveInterrupt.
I2C slave transfer completed successfully. 


 Transfer successfully!
 
Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f 
~~~~~~~~~~~~~

