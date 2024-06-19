Overview
========
The example shows how to use i3c driver as slave to do board to board transfer with a polling master.

In this example, one i3c instance as slave and another i3c instance on the other board as master. Master
first sends a piece of data to slave in I2C mode, and receive a piece of data from slave and check if the
data received from slave is correct. Then master will enter dynamic address cycle to assign address to the
connected slave, after success, will use I3C SDR mode to transfer data, then receive data from the connected
I3C slave and check the data consistency.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 board
- Personal Computer

Board settings
==============
I3C one board:
  + Transfer data from MASTER_BOARD to SLAVE_BOARD of I3C interface, I3C0 pins of MASTER_BOARD are connected with
    I3C0 pins of SLAVE_BOARD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD    CONNECTS TO       SLAVE_BOARD
Pin Name        Board Location    Pin Name   Board Location
I3C0_SCL/P1_9   J8-3              P1_9       J8-3
I3C0_SDA/P1_8   J8-4              P1_8       J8-4
GND             J5-8              GND        J5-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB Type-C cable between the host PC and the MCU-Link USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Press reset button to begin running the demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I3C board2board polling example -- Slave transfer.

Check I3C master I2C transfer.
Slave received data :
0x 1  0x20  0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  
0x 6  0x 7  0x 8  0x 9  0x a  0x b  0x c  0x d  
0x e  0x f  0x10  0x11  0x12  0x13  0x14  0x15  
0x16  0x17  0x18  0x19  0x1a  0x1b  0x1c  0x1d  
0x1e  0x1f  
I3C master I2C transfer finished.

Check I3C master I3C SDR transfer.
Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  
I3C master I3C SDR transfer finished.

Check I3C master I3C HDR transfer.
Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  
I3C master I3C HDR transfer finished.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
