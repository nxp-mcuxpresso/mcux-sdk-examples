Overview
========
The KPP Example project is a demonstration program that uses the KSDK software to manipulate the Keypad MATRIX.
The example is use the continuous column and rows as 4*4 or 8*8 matrix to show the example.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
Matrix and Jumper settings for KPP:

row6  <----->   J44 #8 (B1_02)
row7  <----->   J44 #6 (B1_00)
col6  <----->   J44 #10 (B1_03)
col7  <----->   J44 #12 (B0_01)

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Connect the 2*2 matrix to the Jumpers mentioned above. 
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

When you press any key on matrix,  the log will show the right key you have pressed.
If the press is long, it will add the long press mention.
The log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 KPP Driver Example Start.
  
 Key SW1 was pressed.

 Key SW3 was pressed.

 This was a long press.

   ......

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
