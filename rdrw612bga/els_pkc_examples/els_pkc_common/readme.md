Overview
========
The ELS PKC application provides examples which demonstrate usage of most available algorithms and a crypto library lightweight testing:
- AES-128/192/256 encryption/decryption, modes ECB/CBC/CTR/GCM/CMAC
- SHA2-224/256/384/512 (including sha-direct mode)
- KDF: CKDF (NIST SP 800-108 / SP800-56C), HKDF (RFC5869)
- HMAC
- ECC Sign/Verify/Keygen/Keyexchange (P-256)
- RFC3394 key wrapping/unwrapping
- Key Import/Export/Utilities/Provisioning
- Random number Generation (DRBG/PRNG)

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
When the demo runs successfully, the terminal will display similar information like the following:

ELS PKC common example

ELS get info:pass 
RNG PRNG random:pass 
Flow protection:pass 
Memory data invariant compare:pass 
Memory data invariant copy:pass 
Key component operations:pass 
PKC power down wake-up init:pass 

RESULT: All 7 test PASS!!
ELS example END 

