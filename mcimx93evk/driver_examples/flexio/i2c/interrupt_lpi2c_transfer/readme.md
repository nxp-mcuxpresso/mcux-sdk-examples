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

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-EVK  board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
The example requires doing connection between FLEXIO1(as master) pins and LPI2C1(as slave) pins.
- LPI2C1_SCL(J1005-3), IOMUXC_PAD_GPIO_IO02__FLEXIO1_FLEXIO02(J1001-3) connected
- LPI2C1_SDA(J1005-5), IOMUXC_PAD_GPIO_IO03__FLEXIO1_FLEXIO03(J1001-5) connected

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
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
~~~~~~~~~~~~~~~~~~~~~
