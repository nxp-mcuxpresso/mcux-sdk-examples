Overview
========
The IAP OTP project is a simple demonstration program of the SDK IAP driver. It reads fuse word with input index. A message a printed on the UART terminal as 
fuse read iap operations are performed.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IAP OTP example
Please input the OTP word index to read(0~511):4
Fuse[4] value: 0x1B

Please input the OTP word index to read(0~511):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
