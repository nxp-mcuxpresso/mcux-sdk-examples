Overview
========
The mbedTLS Benchmark demo application performs cryptographic algorithm benchmarking and prints results to the
terminal.


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

Sign message with ECC private key inside ELE - *Success*

Verify signature with ECC public key - *Success*

Writing keyblob into buffer- *Success*

Deinit of ELE (Close sessions, services and keystore)- *Success*

Initialization of crypto HW- *Success*

Read encrypted keyblob into ELE and open keystore and reconstruct Public key- *Success*

Sign message with ECC private key loaded back inside ELE - *Success*

Verify signature with ECC public key reconstructed into CTX - *Success*

Compute HMAC with plaintext key - *Success*

Compare expected and generated HMAC - *Success*


 End of example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
