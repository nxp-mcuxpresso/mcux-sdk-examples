Overview
========
The example shows how to use SPI driver with Azure RTOS. In this example,
one SPI instance used as master and another SPI instance used as slave.

Before running the example, need to connect the master SPI to the slave SPI
by a cable.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S28 board
- Personal Computer
- 4-pin flat cable

Board settings
==============
Connect SPI7 pins to SPI2 pins:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master(SPI7)		           Slave(SPI2)
Pin Name   Board Location     Pin Name   Board Location
MISO       P17  pin 12         MISO      P18  pin 6
MOSI       P17  pin 10         MOSI      P18  pin 10
SCK        P17  pin 14         SCK       P18  pin 8
SSEL1      P17  pin 1          SSEL0     P17  pin 16
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

When the example runs successfully, the console will output as below.

Start the SPI example

Master transmited:
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  
0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Slave received:
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  
0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f  
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f  

Slave transmited:
0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8  
0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0  
0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8  
0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0  

Master received:
0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8  
0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0  
0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8  
0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0  

Master-to-slave data verified ok.
Slave-to-master data verified ok.

