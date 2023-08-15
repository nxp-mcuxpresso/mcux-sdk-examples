Overview
========
The IAP boot project is a simple demonstration program of the SDK IAP driver. It invokes into ROM with user specified parameter.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the iap boot example in the terminal window and system will boot from the selected media:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Boot mode (0: master, 1: ISP) : 0
Boot interface (0: USART 1: I2C 2: SPI 3: USB HID 4:FlexSPI 7:SD 8:MMC) : 4
Boot image index (0-1) : 0
Boot FlexSPI instance (0-3):
FlexSPI PortA1 = 0
FlexSPI PortB1 = 1
FlexSPI PortA2 = 2
FlexSPI PortB2 = 3
Selection : 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
