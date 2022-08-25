Overview
========
The 9_uart_interrupt_transfer example shows how to use uart driver in 9bit interrupt transfer way:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board.

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
No special settings are required.



Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Connect the Pin8 and the Pin10 of J1003 with each other.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.


Running the demo
================
When the demo runs successfully, the log would be seen on the debug terminal like:

  UART will send first piece of data out without addressing itself:
    0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17

  UART will send second piece of data out with addressing itself:
    Address: 0xfe : 0x80  0x81  0x82  0x83  0x84  0x85  0x86  0x87

  RS-485 Slave Address has been detected.
  UART received data:
  Address: 0xfe : 0x80  0x81  0x82  0x83  0x84  0x85  0x86  0x87

  All data matches!
