Overview
========
The IEE APC demo application demonstrates usage of the IEE and IEE APC driver. The Inline Encryption Engine (IEE) together with IEE APC provides a means to perform inline encryption and decryption of information transferred over memory interfaces. This demo application configures region 0 to perform AES XTS encryption and decryption while region 1 to perform AES-CTR decryption from external QSPI FLASH memory. Then data are written into RAM memory (region 0), after that encryption is disabled and encrypted data read. After the successful read, encryption is enabled and memory read again to see that data matches the original written value. Another encrypted data are programmed into FLASH memory (region 1) and after encryption is enabled data are read and compared with expected data. 

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
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
IEE APC Demo started

Erasing Serial NOR over FlexSPI...
Erase data - successfully.
Program data - successfully.
Set up IEE APC regions
Enable IEE APC regions
IEE init
Load keys into IEE
Set up IEE configuration

Load keys into IEE
Set up IEE configuration

Write plain text data to be encrypted at addess 0x20360000:
0x48656c6c6f576f726c64506c61696e21 -> "HelloWorldPlain!"

Turn off ecryption/decryption

Read encrypted data, From address 0x20360000 :
0x26e8146775244a24455d339f45fe098 -> "&ègu$¢DUÓ9ô_à"

Turn on ecryption/decryption

Read decrypted data, From address 0x20360000:
0x48656c6c6f576f726c64506c61696e21 -> "HelloWorldPlain!"

Read decrypted data, From address 0x30014000 (FLASH):
Data are decrypted sucessuly

Turn off ecryption/decryption

Data are encrypted sucessuly

IEE & IEE_APC & Flash demo End.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note:
To download binary of cm7 core into qspiflash and boot from qspiflash directly, following steps are needed:
1. Compile flash target of the project, and get the binaray file "hello_world.bin".
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Drop the binaray into disk "RT1170-EVK" on PC.
5. Wait for the disk disappear and appear again which will take couple of seconds.
7. Reset the board by pressing SW3 or power off and on the board. 
(If default boot core is cm4, binary of cm4 could be downloaded and boot according to steps above.)
