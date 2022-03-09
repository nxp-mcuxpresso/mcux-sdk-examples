Overview
========
The LPI2C Example project is a demonstration program that uses the KSDK software to manipulate the Low Power Inter-
Integrated Circuit.
The example uses two instances of LPI2C, one in configured as master and the other one as slave.
The LPI2C master sends data to LPI2C slave. The slave will check the data it receives and shows the log.


Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini USB cable
- FRDM-K32L3A6 board
- Personal Computer

Board settings
==============
To make this example work, connections needed to be as follows:

M0 core:
        LPI2C3        connected to   LPI2C0
SCL     PTB15(J3-15)    -->          PTC10(J2-20)
SDA     PTB16(J3-13)    -->          PTC9(J2-18)

M4 core:
        LPI2C1        connected to   LPI2C0
SCL     PTB14(J1-10)    -->          PTC10(J2-20)
SDA     PTB6(J2-6)      -->          PTC9(J2-18)

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for JLink serial device with these settings:
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
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPI2C example -- MasterInterrupt_SlaveInterrupt.
Master will send data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

I2C slave transfer completed successfully. 


 Transfer successfully!
 
Slave received data :
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
