Overview
========
The ELE Crypto Example project is a demonstration program that uses the MCUX SDK
software to perform crypto operations with EdgeLock Enclave (ELE) and usage of
its services with direct use of Messaging Unit driver.


SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
Example output on terminal:

EdgeLock Enclave Sub-System crypto example:

****************** Load EdgeLock FW ***********************
EndgeLock FW loaded and authenticated successfully.

****************** Compute Hash (SHA256) of massage *******
*SUCCESS* Computed HASH matches the expected value.

*SUCCESS* HASH Init done.

*SUCCESS* HASH Update done.

*SUCCESS* Computed HASH (Finish) matches the expected value.

****************** Compute HMAC (SHA256) of massage *******
*SUCCESS* Computed HMAC (#1) matches the expected value.

*SUCCESS* Computed HMAC (#2) matches the expected value.

End of Example with SUCCESS!!
