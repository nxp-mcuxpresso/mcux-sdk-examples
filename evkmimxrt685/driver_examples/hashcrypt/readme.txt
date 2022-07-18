Overview
========

The Hashcrypt Example project is a demonstration program that uses the KSDK software to encrypt plain text
and decrypt it back using AES and SHA algorithm. SHA-1, SHA-256, AES ECB and AES CBC modes are demonstrated.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

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
The log below shows the output of the hashcrypt example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
AES ECB Test pass
AES CBC Test pass
AES CTR Test pass
SHA-1 Test pass
SHA-256 Test pass
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
