Overview
========
The mbedTLS SelfTest program performs cryptographic algorithm testing and prints results to the
terminal.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- TWR-KM35Z75M board
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
These similar instructions are displayed/shown on the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
mbedTLS version 2.16.2
fsys=71991296
Using following implementations:
  SHA: MMCAU HW accelerated
  AES: MMCAU HW accelerated
  AES GCM: MMCAU HW accelerated
  DES: MMCAU HW accelerated
  Asymmetric encryption: Software implementation

  MD5 test #1: passed
  MD5 test #2: passed
  
......
  PBKDF2 (SHA1) #3: passed
  PBKDF2 (SHA1) #4: passed
  PBKDF2 (SHA1) #5: passed

  Executed 18 test suites

  [ All tests PASS ]

  
