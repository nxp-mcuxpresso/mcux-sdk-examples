Overview
========
The CRC Example project is a demonstration program that uses the KSDK software to generate checksums
for an ASCII string. Several CRC protocols are implemented using the CRC driver API.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- USB-C cable
- FRDM-MCXC242 board
- Personal Computer

Board settings
==============
This example project does not call for any special hardware configurations.
Although not required, the recommendation is to leave the development board's jumper settings
and configurations in default state when running this example.
Running the demo
================
Example output on terminal:

CRC Peripheral Driver Example

Test string: 123456789
CRC-16 CCIT FALSE: 0x29b1
CRC-16 MAXIM: 0x44c2
CRC-16 KERMIT: 0x2189
CRC-32: 0xcbf43926
CRC-32 POSIX: 0x765e7680
