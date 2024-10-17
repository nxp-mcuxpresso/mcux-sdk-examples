Overview
========
The ELE Crypto Example project is a demonstration program that uses the MCUX SDK
software to perform crypto operations with EdgeLock Enclave (ELE) and usage of
its services with direct use of Messaging Unit driver.


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
The log below shows the output of the ELE baseline API demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EdgeLock Enclave Sub-System crypto base API example:

****************** Initialize EdgeLock services ***********
EdgeLock services initialized successfully.

****************** Get FW status ELE **********************
Get FW status successfully. Status: 0x0
This means no ELE FW in place

****************** Start RNG ******************************
EdgeLock RNG Start success.
EdgeLock RNG ready to use.

****************** Get FW version ELE *********************
Get FW version successfully. Version: 0xb0300
Bit 27 of fwversion is not set - this means response come from ROM

****************** Load EdgeLock FW ***********************
EdgeLock FW loaded and authenticated successfully.

****************** Get FW version ELE *********************
Get FW version successfully. Version: 0x8010010
Bit 27 is set - this means response come from FW
Bit 24 is not set - BASE FW is running

****************** Get FW status ELE **********************
Get FW status successfully. Status: 0x1
This means ELE FW is authenticated and operational

****************** Read common fuse ***********************
Fuse read successfully. Value of EARLY_FUSES_PGRM is 1.

****************** PING ELE *******************************
Ping ELE successfully.

****************** Generate IEE Key Blob ******************
IEE key blob generated successfully.

****************** Load IEE Key Blob **********************
IEE key blob loaded successfully.

****************** Get Info *******************************
Get Info from ELE successfully.
SOC ID number 0x1180

****************** ELE clock change start *****************
ELE clock change start completed successfully.

****************** ELE clock change finish ****************
ELE clock change finish completed successfully.

ELE clock is now 100Mhz as expected.

****************** Device attestation *********************
Device attestation completed successfully.

****************** BBSM (Battery-Backed Security Module) **
BBSM register write completed successfully.
BBSM register read success, value=cafecafe
Get ELE Events successfully
Set ELE Event policy successfully
Get ELE Events successfully
ELE Event triggered successfully

End of Example with SUCCESS!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The IDs will be different on every run.
