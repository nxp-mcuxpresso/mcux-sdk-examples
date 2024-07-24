Overview
========
The flexio_lpi2c_interrupt example shows how to use flexio i2c master  driver in interrupt way:

In this example, a flexio simulated i2c master connect to a LPI2C slave

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
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
Jumper setting:
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MASTER(FLEXIO_I2C)                connect to        SLAVE(LPI2C1)
    Pin Name    Board Location                      Pin Name    Board Location
    SCL         J8 pin 18                           SCL         J5 pin 5
    SDA         J8 pin 17                           SDA         J5 pin 6
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Other jumpers keep default configuration.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J10.
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
