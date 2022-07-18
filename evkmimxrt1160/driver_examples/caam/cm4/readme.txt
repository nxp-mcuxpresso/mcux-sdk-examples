Overview
========

The CAAM Example project is a demonstration program that uses the KSDK software to encrypt plain text
and decrypt it back using AES algorithm. CBC and GCM modes are demonstrated.
The symmetric key is generated at random, using CAAM's random number generator.


Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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
The log below shows the output of the hello world demo in the terminal window:

CAAM AES Peripheral Driver Example

*CAAM Job Ring 0* :

SHA:done successfully.

AES CBC: encrypting using 128 bit key done successfully.
AES CBC: decrypting back done successfully.

AES CBC: encrypting using 192 bit key done successfully.
AES CBC: decrypting back done successfully.

AES CBC: encrypting using 256 bit key done successfully.
AES CBC: decrypting back done successfully.

*CAAM Job Ring 1* :

AES GCM: encrypt done successfully.
AES GCM: decrypt done successfully.

*CAAM Job Ring 2* :

AES CBC: encrypting using 128 bit key done successfully.
AES CBC: decrypting back done successfully.

AES CBC: encrypting using 192 bit key done successfully.
AES CBC: decrypting back done successfully.

AES CBC: encrypting using 256 bit key done successfully.
AES CBC: decrypting back done successfully.

*CAAM Job Ring 3* :

AES GCM: encrypt done successfully.
AES GCM: decrypt done successfully.

Note:
CAAM peripheral works with OCRAM memory. Please avoid using DTCM memory for storage CAAM data.
