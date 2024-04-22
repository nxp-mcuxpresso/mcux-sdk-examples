Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Initialization of crypto HW Success

Generating ECC keypair inside ELE - *Success*

Writing keyblob into buffer- *Success*

Read encypted keyblob into ELE and open keystore and reconstruct Public key- *Success*

Sign message with ECC private key loaded back inside ELE - *Success*

Verify signature with ECC public key reconstructed into CTX - *Success*

  . Reading serial number... ok
  . Seeding the random number generator... ok
  . Setting certificate values ... ok
  . Adding the Basic Constraints extension ... ok
  . Adding the Subject Key Identifier ... ok
  . Adding the Authority Key Identifier ... ok
  . Writing the certificate... ok
  . Loading the certificate ... ok

 End of example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
